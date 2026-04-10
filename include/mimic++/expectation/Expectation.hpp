//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_EXPECTATION_HPP
#define MIMICPP_EXPECTATION_EXPECTATION_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/expectation/Common.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <any>
    #include <memory>
    #include <optional>
    #include <tuple>
    #include <type_traits>
    #include <utility>
    #include <vector>
#endif

namespace mimicpp::expectation
{
    /**
     * \defgroup EXPECTATION expectation
     * \brief Facilities for creating, owning, and verifying expectations.
     * \details
     * Expectations are one of the two core aspects of *mimic++*.
     * They define how a `Mock` is expected to be called.
     * Users should always store expectations in the `ScopedExpectation` RAII-object,
     * which checks whether the expectation is satisfied during destruction.
     * If not, an error is forwarded to the installed reporter.
     * To simplify that process, the macro \ref MIMICPP_SCOPED_EXPECTATION (and the shorthand \ref SCOPED_EXP) is provided,
     * which just creates a `ScopedExpectation` with a unique name in the current scope.
     *
     * \note
     * Besides individually managed expectations, there is also the `ScopedExpectations` type (note the plural),
     * which is as a convenient scope-bound alternative for collecting and verifying multiple expectations automatically.
     *
     * \details
     * Once an expectation is fully exhausted, it becomes inactive and can't be matched any further.
     * If at any time multiple active expectations are valid matches for an incoming call, *mimic++* has to make a choice.
     * In such cases, when no `Sequence` has been applied, the latest created expectation will be selected.
     * This may seem rather unintuitive at first, but as expectations are bound to their scope,
     * usually expectations within the current scope should be preferred over the expectations from the outer scope.
     *
     * This is the technical explanation for the behavior, but users should not rely too much on that.
     * In fact, *mimic++* just guarantees that expectations from most inside scope will be preferred over expectations from further outside.
     * Users should think about expectations within the same scope as equally applicable and treat such ambiguities as undeterministic outcomes.
     * Nevertheless, *mimic++* provides a tool for such cases, which is designed to work in a deterministic manner:
     * \ref EXPECTATION_SEQUENCE "Sequences".
     *
     * Expectations can be created anywhere in the program, you just need an appropriate `Mock`.
     *
     * \{
     */

    /**
     * \brief The type-erased expectation type with shared ownership.
     * \details This type serves as a value-type for a single expectation and can be freely copied around,
     * without duplicating the actual expectation.
     */
    class Expectation
    {
    private:
        class Concept
        {
        public:
            virtual ~Concept() noexcept(false)
            {
            }

            Concept(const Concept&) = delete;
            Concept& operator=(const Concept&) = delete;
            Concept(Concept&&) = delete;
            Concept& operator=(Concept&&) = delete;

            [[nodiscard]]
            virtual util::SourceLocation const& from() const noexcept = 0;

            [[nodiscard]]
            virtual StringT const& mock_name() const noexcept = 0;

            [[nodiscard]]
            virtual reporting::ExpectationReport report() const = 0;

            [[nodiscard]]
            virtual bool is_satisfied() const noexcept = 0;

            [[nodiscard]]
            virtual bool is_applicable() const noexcept = 0;

            [[nodiscard]]
            virtual std::optional<reporting::RequirementOutcomes> matches(std::any&& opaqueCallInfo) const = 0;

            [[nodiscard]]
            virtual std::any consume(std::any&& opaqueCallInfo) = 0;

        protected:
            Concept() = default;
        };

        template <
            typename Signature,
            control_policy ControlPolicy,
            finalize_policy_for<Signature> FinalizePolicy,
            expectation_policy_for<Signature>... Policies>
        class Model final
            : public Concept
        {
        public:
            using CallInfoRef = std::reference_wrapper<call::info_for_signature_t<Signature> const>;
            using Return = signature_return_type_t<Signature>;
            using PolicyList = std::tuple<Policies...>;

            template <typename ControlPolicyArg, typename FinalizerArg, typename... PolicyArgs>
            [[nodiscard]]
            explicit constexpr Model(
                util::SourceLocation from,
                reporting::TargetReport target,
                ControlPolicyArg&& controlArg,
                FinalizerArg&& finalizerArg,
                PolicyArgs&&... args)
                : m_From{std::move(from)},
                  m_Target{std::move(target)},
                  m_ControlPolicy{std::forward<ControlPolicyArg>(controlArg)},
                  m_Policies{std::forward<PolicyArgs>(args)...},
                  m_Finalizer{std::forward<FinalizerArg>(finalizerArg)}
            {
            }

            [[nodiscard]]
            util::SourceLocation const& from() const noexcept override
            {
                return m_From;
            }

            [[nodiscard]]
            StringT const& mock_name() const noexcept override
            {
                return m_Target.name;
            }

            [[nodiscard]]
            reporting::ExpectationReport report() const override
            {
                return reporting::ExpectationReport{
                    .from = m_From,
                    .target = m_Target,
                    .controlReport = m_ControlPolicy.state(),
                    .finalizerDescription = std::nullopt,
                    .requirementDescriptions = gather_requirement_descriptions()};
            }

            [[nodiscard]]
            bool is_satisfied() const noexcept override
            {
                return m_ControlPolicy.is_satisfied()
                    && std::apply(
                           [](auto const&... policies) { return (... && policies.is_satisfied()); },
                           m_Policies);
            }

            [[nodiscard]]
            bool is_applicable() const noexcept override
            {
                return std::holds_alternative<reporting::state_applicable>(m_ControlPolicy.state());
            }

            [[nodiscard]]
            std::optional<reporting::RequirementOutcomes> matches(std::any&& opaqueCallInfo) const override
            {
                MIMICPP_ASSERT(opaqueCallInfo.has_value(), "Empty call is not allowed.");
                if (auto const* const info = std::any_cast<CallInfoRef>(&opaqueCallInfo))
                {
                    return {gather_requirement_outcomes(info->get())};
                }

                return std::nullopt;
            }

            [[nodiscard]]
            std::any consume(std::any&& opaqueCallInfo) override
            {
                auto const* const info = std::any_cast<CallInfoRef>(&opaqueCallInfo);
                MIMICPP_ASSERT(info, "Invalid call type.");

                m_ControlPolicy.consume();
                std::apply(
                    [&](auto&... policies) { (..., policies.consume(info->get())); },
                    m_Policies);

                return finalize(info->get());
            }

        private:
            util::SourceLocation m_From;
            reporting::TargetReport m_Target;
            ControlPolicy m_ControlPolicy;
            PolicyList m_Policies;
            [[no_unique_address]] FinalizePolicy m_Finalizer{};

            [[nodiscard]]
            reporting::RequirementOutcomes gather_requirement_outcomes(call::info_for_signature_t<Signature> const& call) const
            {
                return std::apply(
                    [&](auto const&... policies) {
                        return reporting::RequirementOutcomes{
                            .outcomes{policies.matches(call)...}};
                    },
                    m_Policies);
            }

            [[nodiscard]]
            std::vector<std::optional<StringT>> gather_requirement_descriptions() const
            {
                return std::apply(
                    [&](auto const&... policies) {
                        return std::vector<std::optional<StringT>>{
                            std::optional<StringT>{policies.describe()}...};
                    },
                    m_Policies);
            }

            [[nodiscard]]
            std::any finalize(call::info_for_signature_t<Signature> const& call)
            {
                if constexpr (std::is_void_v<Return>)
                {
                    m_Finalizer.finalize_call(call);
                    return std::make_any<call::ResultStorage<void>>();
                }
                else
                {
                    return std::make_any<call::ResultStorage<Return>>(m_Finalizer.finalize_call(call));
                }
            }
        };

    public:
        /**
         * \brief Defaulted virtual destructor.
         */
        virtual ~Expectation() = default;

        /**
         * \brief Defaulted copy-constructor.
         */
        Expectation(Expectation const&) = default;

        /**
         * \brief Defaulted copy-assignment-operator.
         */
        Expectation& operator=(Expectation const&) = default;

        /**
         * \brief Defaulted move-constructor.
         */
        Expectation(Expectation&&) = default;

        /**
         * \brief Defaulted move-assignment-operator.
         */
        Expectation& operator=(Expectation&&) = default;

        /**
         * \brief Constructs the expectation with the given arguments.
         * \tparam Signature The target signature.
         * \tparam ControlPolicy The control-policy type.
         * \tparam FinalizePolicy The finalize-policy type.
         * \tparam Policies The expectation-policies types.
         * \param from The source-location, where this expectation was created.
         * \param target Information about the target-mock.
         * \param controlPolicy The control-policy.
         * \param finalizePolicy The finalize-policy.
         * \param policies The expectation-policies.
         */
        template <
            typename Signature,
            control_policy ControlPolicy,
            finalize_policy_for<Signature> FinalizePolicy,
            expectation_policy_for<Signature>... Policies>
        [[nodiscard]] //
        explicit Expectation(
            std::in_place_type_t<Signature> const /*signatureTag*/,
            util::SourceLocation from,
            reporting::TargetReport target,
            ControlPolicy controlPolicy,
            FinalizePolicy finalizePolicy,
            Policies... policies)
            : m_Inner{
                  std::make_shared<Model<Signature, ControlPolicy, FinalizePolicy, Policies...>>(
                      std::move(from),
                      std::move(target),
                      std::move(controlPolicy),
                      std::move(finalizePolicy),
                      std::move(policies)...)}
        {
        }

        /**
         * \brief Returns the source-location, where this expectation was created.
         * \return Immutable reference to the source-location.
         */
        [[nodiscard]]
        util::SourceLocation const& from() const noexcept
        {
            return m_Inner->from();
        }

        /**
         * \brief Returns the name of the related mock.
         * \return Immutable reference to the mock-name.
         */
        [[nodiscard]]
        StringT const& mock_name() const noexcept
        {
            return m_Inner->mock_name();
        }

        /**
         * \brief Creates a report of the internal state.
         * \return A newly generated report.
         */
        [[nodiscard]]
        reporting::ExpectationReport report() const
        {
            return m_Inner->report();
        }

        /**
         * \brief Queries all policies, whether they are satisfied.
         * \return Returns true if all policies are satisfied.
         */
        [[nodiscard]]
        bool is_satisfied() const noexcept
        {
            return m_Inner->is_satisfied();
        }

        /**
         * \brief Queries the control policy, whether it's in the applicable state.
         * \return Returns true if the expectation is applicable.
         */
        [[nodiscard]]
        bool is_applicable() const noexcept
        {
            return m_Inner->is_applicable();
        }

        /**
         * \brief Queries all policies, whether they accept the given call.
         * \param callInfo The call to be matched.
         * \returns the requirements outcomes when the call matches the expectation signature;
         * otherwise `std::nullopt` is returned.
         */
        template <typename Return, typename... Args>
        [[nodiscard]]
        std::optional<reporting::RequirementOutcomes> matches(call::Info<Return, Args...> const& callInfo) const
        {
            return m_Inner->matches(std::any{std::ref(callInfo)});
        }

        /**
         * \brief Informs all policies that the given call was accepted.
         * \param callInfo The  call to be consumed.
         * \details This function is called when a match was made.
         */
        template <typename Return, typename... Args>
        [[nodiscard]]
        Return consume(call::Info<Return, Args...> const& callInfo)
        {
            std::any opaqueResult = m_Inner->consume(std::any{std::ref(callInfo)});
            auto* const resultStorage = std::any_cast<call::ResultStorage<Return>>(&opaqueResult);
            MIMICPP_ASSERT(resultStorage, "Invalid result-type.");

            return resultStorage->extract();
        }

        [[nodiscard]]
        friend bool operator==(Expectation const&, Expectation const&) = default;

    private:
        std::shared_ptr<Concept> m_Inner;
    };

    /**
     * \}
     */
}

#endif
