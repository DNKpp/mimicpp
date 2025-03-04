//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_HPP
#define MIMICPP_EXPECTATION_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/Sequence.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/reporting/CallReport.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/GlobalReporter.hpp"
#include "mimic++/reporting/TargetReport.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

namespace mimicpp::detail
{
    template <typename Return, typename... Params, typename Signature>
    [[nodiscard]]
    std::optional<reporting::RequirementOutcomes> determine_requirement_outcomes(
        reporting::TargetReport const& target,
        call::Info<Return, Params...> const& call,
        Expectation<Signature> const& expectation) noexcept
    {
        try
        {
            return expectation.matches(call);
        }
        catch (...)
        {
            reporting::detail::report_unhandled_exception(
                reporting::make_call_report(target, call),
                expectation.report(),
                std::current_exception());
        }

        return std::nullopt;
    }

    [[nodiscard]]
    std::vector<reporting::ExpectationReport> gather_expectation_reports(auto&& expectationPtrs)
    {
        auto view = expectationPtrs
                  | std::views::transform([](auto const& exp) { return exp->report(); });
        return std::vector<reporting::ExpectationReport>{
            view.begin(),
            view.end()};
    }

    template <typename Signature>
    [[nodiscard]]
    std::vector<reporting::NoMatchReport> make_no_match_reports(
        std::vector<std::tuple<Expectation<Signature>*, reporting::RequirementOutcomes>>&& outcomes)
    {
        std::vector<reporting::NoMatchReport> reports{};
        reports.reserve(outcomes.size());
        for (auto const& [expectationPtr, outcome] : outcomes)
        {
            reports.emplace_back(
                expectationPtr->report(),
                std::move(outcome));
        }

        return reports;
    }

    [[nodiscard]]
    constexpr auto find_best_match(std::span<reporting::ExpectationReport const> const matches)
    {
        constexpr auto ratings = [](auto const& el) noexcept -> const auto& {
            return std::get<reporting::state_applicable>(el.controlReport)
                .sequenceRatings;
        };

        auto best = std::ranges::begin(matches);
        for (auto iter = best + 1;
             iter != std::ranges::end(matches);
             ++iter)
        {
            if (!sequence::detail::has_better_rating(
                    ratings(*best),
                    ratings(*iter)))
            {
                best = iter;
            }
        }

        return std::ranges::distance(std::ranges::begin(matches), best);
    }
}

namespace mimicpp
{
    /**
     * \defgroup EXPECTATION expectation
     * \brief Contains everything related to managing expectations.
     * \details Expectations are one of the two core aspects of ``mimic++``. They define how a ``Mock`` is expected to be called.
     * Users should always store expectations in the ``ScopedExpectation`` RAII-object, which checks whether the expectation is satisfied
     * during destruction. If not, an error is forwarded to the installed reporter.
     * To simplify that process, the macro \ref MIMICPP_SCOPED_EXPECTATION (and the shorthand \ref SCOPED_EXP) are provided.
     *
     * Once an expectation has been fully exhausted, it becomes inactive and can't be matched any further.
     * If at any time multiple active expectations would be valid matches for an incoming call, ``mimic++`` has to make a choice.
     * In such cases, when no ``Sequence`` has been applied, the latest created expectation will be selected. This may seem rather unintuitive at first,
     * but as expectations are bound to their scope, usually expectations within the current scope should be preferred over the expectations from the
     * outer scope.
     *
     * This is the technical explanation for the behavior, but users should not rely too much on that.
     * In fact, ``mimic++`` just makes the guarantee, that expectations from most inside scope will be preferred over expectations from further outside.
     * Users should think about expectations within the same scope as equally applicable and treat such ambiguities as undeterministic outcomes.
     * Nevertheless, ``mimic++`` provides a tool for such cases, which is designed to work in a deterministic manner:
     * \ref EXPECTATION_SEQUENCE "Sequences".
     *
     * Expectations can be created anywhere in the program, you just need an appropriate ``Mock``.
     * \{
     */

    /**
     * \brief The base interface for expectations.
     * \tparam Signature The decayed signature.
     */
    template <typename Signature>
        requires std::same_as<Signature, signature_decay_t<Signature>>
    class Expectation
    {
    public:
        /**
         * \brief The expected call type.
         */
        using CallInfoT = call::info_for_signature_t<Signature>;

        /**
         * \brief The return type.
         */
        using ReturnT = signature_return_type_t<Signature>;

        /**
         * \brief Defaulted virtual destructor.
         */
        virtual ~Expectation() = default;

        /**
         * \brief Defaulted default constructor.
         */
        [[nodiscard]]
        Expectation() = default;

        /**
         * \brief Deleted copy-constructor.
         */
        Expectation(const Expectation&) = delete;

        /**
         * \brief Deleted copy-assignment-operator.
         */
        Expectation& operator=(const Expectation&) = delete;

        /**
         * \brief Deleted move-constructor.
         */
        Expectation(Expectation&&) = delete;

        /**
         * \brief Deleted move-assignment-operator.
         */
        Expectation& operator=(Expectation&&) = delete;

        /**
         * \brief Creates a report of the internal state.
         * \return A newly generated report.
         */
        [[nodiscard]]
        virtual reporting::ExpectationReport report() const = 0;

        /**
         * \brief Queries all policies, whether they are satisfied.
         * \return Returns true, if all policies are satisfied.
         */
        [[nodiscard]]
        virtual bool is_satisfied() const noexcept = 0;

        /**
         * \brief Queries the control policy, whether it's in the applicable state.
         * \return Returns true, if expectation is applicable.
         */
        [[nodiscard]]
        virtual bool is_applicable() const noexcept = 0;

        /**
         * \brief Queries all policies, whether they accept the given call.
         * \param call The call to be matched.
         * \return Returns true, if all policies accept the call.
         */
        [[nodiscard]]
        virtual reporting::RequirementOutcomes matches(const CallInfoT& call) const = 0;

        /**
         * \brief Informs all policies, that the given call has been accepted.
         * \param call The call to be consumed.
         * \details This function is called, when a match has been made.
         */
        virtual void consume(const CallInfoT& call) = 0;

        /**
         * \brief Requests the given call to be finalized.
         * \param call The call to be finalized.
         * \return Returns the call result.
         * \details This function is called, when a match has been made and ``consume`` has been called.
         */
        [[nodiscard]]
        virtual constexpr ReturnT finalize_call(const CallInfoT& call) = 0;

        /**
         * \brief Returns the source-location, where this expectation has been created.
         * \return Immutable reference to the source-location.
         */
        [[nodiscard]]
        virtual constexpr util::SourceLocation const& from() const noexcept = 0;

        /**
         * \brief Returns the name of the related mock.
         * \return Immutable reference to the mock-name.
         */
        [[nodiscard]]
        virtual constexpr StringT const& mock_name() const noexcept = 0;
    };

    /**
     * \brief Collects all expectations for a specific (decayed) signature.
     * \tparam Signature The decayed signature.
     */
    template <typename Signature>
        requires std::same_as<Signature, signature_decay_t<Signature>>
    class ExpectationCollection
    {
    public:
        /**
         * \brief The expected call type.
         */
        using CallInfoT = call::info_for_signature_t<Signature>;

        /**
         * \brief The interface type of the stored expectations.
         */
        using ExpectationT = Expectation<Signature>;

        /**
         * \brief The return type.
         */
        using ReturnT = signature_return_type_t<Signature>;

        /**
         * \brief Defaulted destructor.
         */
        ~ExpectationCollection() = default;

        /**
         * \brief Defaulted default constructor.
         */
        [[nodiscard]]
        ExpectationCollection() = default;

        /**
         * \brief Deleted copy-constructor.
         */
        ExpectationCollection(const ExpectationCollection&) = delete;

        /**
         * \brief Deleted copy-assignment-operator.
         */
        ExpectationCollection& operator=(const ExpectationCollection&) = delete;

        /**
         * \brief Defaulted move-constructor.
         */
        [[nodiscard]]
        ExpectationCollection(ExpectationCollection&&) = default;

        /**
         * \brief Defaulted move-assignment-operator.
         */
        ExpectationCollection& operator=(ExpectationCollection&&) = default;

        /**
         * \brief Inserts the given expectation into the internal storage.
         * \param expectation The expectation to be inserted.
         * \attention Inserting an expectation, which is already element of any ExpectationCollection (including the current one),
         * is undefined behavior.
         */
        void push(std::shared_ptr<ExpectationT> expectation)
        {
            const std::scoped_lock lock{m_ExpectationsMx};

            MIMICPP_ASSERT(std::ranges::find(m_Expectations, expectation) == std::ranges::end(m_Expectations), "Expectation already belongs to this storage.");

            m_Expectations.emplace_back(std::move(expectation));
        }

        /**
         * \brief Removes the given expectation from the internal storage.
         * \param expectation The expectation to be removed.
         * \details This function also checks, whether the removed expectation is satisfied. If not, an
         * "unfulfilled expectation"- report is emitted.
         * \attention Removing an expectation, which is not element of the current ExpectationCollection, is undefined behavior.
         */
        void remove(std::shared_ptr<ExpectationT> expectation)
        {
            const std::scoped_lock lock{m_ExpectationsMx};

            auto iter = std::ranges::find(m_Expectations, expectation);
            MIMICPP_ASSERT(iter != std::ranges::end(m_Expectations), "Expectation does not belong to this storage.");
            m_Expectations.erase(iter);

            if (!expectation->is_satisfied())
            {
                reporting::detail::report_unfulfilled_expectation(
                    expectation->report());
            }
        }

        /**
         * \brief Handles the incoming call.
         * \param target The mock-target, which received the call.
         * \param call The call to be handled.
         * \return Returns an appropriate result from the matched expectation.
         * \details This function queries all stored expectations, whether they accept the call.
         * If multiple matches are possible, the best match is selected and a "matched"-report is emitted.
         * If no matches are found, "no matched"-report is emitted and the call is aborted (e.g. by throwing an exception or terminating).
         * If matches are possible, but all expectations are saturated, an "inapplicable match"-report is emitted.
         */
        [[nodiscard]]
        ReturnT handle_call(reporting::TargetReport target, CallInfoT call)
        {
            std::vector<ExpectationT*> matches{};
            std::vector<ExpectationT*> inapplicableMatches{};
            std::vector<std::tuple<ExpectationT*, reporting::RequirementOutcomes>> noMatches{};

            std::scoped_lock const lock{m_ExpectationsMx};
            evaluate_expectations(target, call, matches, inapplicableMatches, noMatches);

            if (!std::ranges::empty(matches))
            {
                std::vector reports = detail::gather_expectation_reports(matches);
                MIMICPP_ASSERT(matches.size() == reports.size(), "Size mismatch.");
                auto const bestIndex = detail::find_best_match(reports);
                MIMICPP_ASSERT(0 <= bestIndex && bestIndex < std::ssize(reports), "Invalid index.");

                auto& report = reports[bestIndex];
                auto& expectation = *matches[bestIndex];

                // Todo: Avoid the call copy
                // Maybe we can prevent the copy here, but we should keep the instruction order as-is, because
                // in cases of a throwing finalizer, we might introduce bugs. At least there are some tests, which
                // will fail if done wrong.
                reporting::detail::report_full_match(
                    reporting::make_call_report(std::move(target), call),
                    std::move(report));
                expectation.consume(call);
                return expectation.finalize_call(call);
            }

            if (!std::ranges::empty(inapplicableMatches))
            {
                reporting::detail::report_inapplicable_matches(
                    reporting::make_call_report(std::move(target), std::move(call)),
                    detail::gather_expectation_reports(inapplicableMatches));
            }

            reporting::detail::report_no_matches(
                reporting::make_call_report(std::move(target), std::move(call)),
                detail::make_no_match_reports(std::move(noMatches)));
        }

    private:
        std::vector<std::shared_ptr<ExpectationT>> m_Expectations{};
        std::mutex m_ExpectationsMx{};

        void evaluate_expectations(
            reporting::TargetReport const& target,
            CallInfoT const& call,
            std::vector<ExpectationT*>& matches,
            std::vector<ExpectationT*>& inapplicableMatches,
            std::vector<std::tuple<ExpectationT*, reporting::RequirementOutcomes>>& noMatches)
        {
            for (auto const& exp : m_Expectations | std::views::reverse)
            {
                if (std::optional outcomes = detail::determine_requirement_outcomes(target, call, *exp))
                {
                    if (std::ranges::any_of(outcomes->outcomes, [](auto const& el) { return el == false; }))
                    {
                        noMatches.emplace_back(exp.get(), *std::move(outcomes));
                    }
                    else if (!exp->is_applicable())
                    {
                        inapplicableMatches.emplace_back(exp.get());
                    }
                    else
                    {
                        matches.emplace_back(exp.get());
                    }
                }
            }
        }
    };

    /**
     * \brief Determines, whether the given type satisfies the requirements of an expectation-policy for the given signature.
     */
    template <typename T, typename Signature>
    concept expectation_policy_for = std::is_move_constructible_v<T>
                                  && std::is_destructible_v<T>
                                  && std::same_as<T, std::remove_cvref_t<T>>
                                  && requires(T& policy, const call::info_for_signature_t<Signature>& info) {
                                         { std::as_const(policy).is_satisfied() } noexcept -> std::convertible_to<bool>;
                                         { std::as_const(policy).matches(info) } -> std::convertible_to<bool>;
                                         { std::as_const(policy).describe() } -> std::convertible_to<std::optional<StringT>>;
                                         { policy.consume(info) };
                                     };

    /**
     * \brief Determines, whether the given type satisfies the requirements of a finalize-policy for the given signature.
     */
    template <typename T, typename Signature>
    concept finalize_policy_for = std::is_move_constructible_v<T>
                               && std::is_destructible_v<T>
                               && std::same_as<T, std::remove_cvref_t<T>>
                               && requires(T& policy, const call::info_for_signature_t<Signature>& info) {
                                      { policy.finalize_call(info) } -> std::convertible_to<signature_return_type_t<Signature>>;
                                  };

    /**
     * \brief Determines, whether the given type satisfies the requirements of a control-policy.
     */
    template <typename T>
    concept control_policy = std::is_move_constructible_v<T>
                          && std::is_destructible_v<T>
                          && std::same_as<T, std::remove_cvref_t<T>>
                          && requires(T& policy) {
                                 { std::as_const(policy).is_satisfied() } noexcept -> std::convertible_to<bool>;
                                 { std::as_const(policy).state() } -> std::convertible_to<reporting::control_state_t>;
                                 policy.consume();
                             };

    /**
     * \brief The actual expectation template.
     * \tparam Signature The decayed signature.
     * \tparam ControlPolicy The applied control-policy.
     * \tparam FinalizePolicy The applied finalize-policy.
     * \tparam Policies All applied expectation-policies.
     */
    template <
        typename Signature,
        control_policy ControlPolicy,
        finalize_policy_for<Signature> FinalizePolicy,
        expectation_policy_for<Signature>... Policies>
    class BasicExpectation final
        : public Expectation<Signature>
    {
    public:
        using ControlPolicyT = ControlPolicy;
        using FinalizerT = FinalizePolicy;
        using PolicyListT = std::tuple<Policies...>;
        using CallInfoT = call::info_for_signature_t<Signature>;
        using ReturnT = typename Expectation<Signature>::ReturnT;

        /**
         * \brief Constructs the expectation with the given arguments.
         * \tparam ControlPolicyArg The control-policy constructor argument types.
         * \tparam FinalizerArg The finalize-policy constructor argument types.
         * \tparam PolicyArgs The expectation-policies constructor argument types.
         * \param from The source-location, where this expectation was created.
         * \param target Information about the target-mock.
         * \param controlArg The control-policy constructor argument.
         * \param finalizerArg The finalize-policy constructor argument.
         * \param args The expectation-policies constructor arguments.
         */
        template <typename ControlPolicyArg, typename FinalizerArg, typename... PolicyArgs>
            requires std::constructible_from<ControlPolicyT, ControlPolicyArg>
                      && std::constructible_from<FinalizerT, FinalizerArg>
                      && std::constructible_from<PolicyListT, PolicyArgs...>
        constexpr explicit BasicExpectation(
            util::SourceLocation from,
            reporting::TargetReport target,
            ControlPolicyArg&& controlArg,
            FinalizerArg&& finalizerArg,
            PolicyArgs&&... args)
            noexcept(
                std::is_nothrow_constructible_v<ControlPolicyT, ControlPolicyArg>
                && std::is_nothrow_constructible_v<FinalizerT, FinalizerArg>
                && (std::is_nothrow_constructible_v<Policies, PolicyArgs> && ...))
            : m_From{std::move(from)},
              m_Target{std::move(target)},
              m_ControlPolicy{std::forward<ControlPolicyArg>(controlArg)},
              m_Policies{std::forward<PolicyArgs>(args)...},
              m_Finalizer{std::forward<FinalizerArg>(finalizerArg)}
        {
        }

        /**
         * \copydoc Expectation::report
         */
        [[nodiscard]]
        reporting::ExpectationReport report() const override
        {
            return reporting::ExpectationReport{
                .from = m_From,
                .target = m_Target,
                .controlReport = m_ControlPolicy.state(),
                .finalizerDescription = std::nullopt,
                .requirementDescriptions = std::apply(
                    [&](auto const&... policies) {
                        return std::vector<std::optional<StringT>>{
                            policies.describe()...};
                    },
                    m_Policies)};
        }

        /**
         * \copydoc Expectation::is_satisfied
         */
        [[nodiscard]]
        constexpr bool is_satisfied() const noexcept override
        {
            return m_ControlPolicy.is_satisfied()
                && std::apply(
                       [](const auto&... policies) noexcept {
                           return (... && policies.is_satisfied());
                       },
                       m_Policies);
        }

        /**
         * \copydoc Expectation::is_applicable
         */
        [[nodiscard]]
        constexpr bool is_applicable() const noexcept override
        {
            return std::holds_alternative<reporting::state_applicable>(
                m_ControlPolicy.state());
        }

        /**
         * \copydoc Expectation::matches
         */
        [[nodiscard]]
        reporting::RequirementOutcomes matches(const CallInfoT& call) const override
        {
            return reporting::RequirementOutcomes{
                .outcomes = gather_requirement_outcomes(call)};
        }

        /**
         * \copydoc Expectation::consume
         */
        constexpr void consume(const CallInfoT& call) override
        {
            m_ControlPolicy.consume();
            std::apply(
                [&](auto&... policies) noexcept {
                    (..., policies.consume(call));
                },
                m_Policies);
        }

        /**
         * \copydoc Expectation::finalize_call
         */
        [[nodiscard]]
        constexpr ReturnT finalize_call(const CallInfoT& call) override
        {
            return m_Finalizer.finalize_call(call);
        }

        /**
         * \copydoc Expectation::from
         */
        [[nodiscard]]
        constexpr util::SourceLocation const& from() const noexcept override
        {
            return m_From;
        }

        /**
         * \copydoc Expectation::mock_name
         */
        [[nodiscard]]
        constexpr StringT const& mock_name() const noexcept override
        {
            return m_Target.name;
        }

    private:
        util::SourceLocation m_From;
        reporting::TargetReport m_Target;
        ControlPolicyT m_ControlPolicy;
        PolicyListT m_Policies;
        [[no_unique_address]] FinalizerT m_Finalizer{};

        [[nodiscard]]
        std::vector<bool> gather_requirement_outcomes(CallInfoT const& call) const
        {
            return std::apply(
                [&](auto const&... policies) {
                    return std::vector<bool>{policies.matches(call)...};
                },
                m_Policies);
        }
    };

    /**
     * \brief Takes the ownership of an expectation and check whether it's satisfied during destruction.
     * \details The owned Expectation is type-erased. This comes in handy, when users want to store ScopedExpectations
     * in a single container.
     */
    class ScopedExpectation
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
            virtual bool is_satisfied() const = 0;
            [[nodiscard]]
            virtual bool is_applicable() const = 0;

            [[nodiscard]]
            virtual util::SourceLocation const& from() const noexcept = 0;
            [[nodiscard]]
            virtual StringT const& mock_name() const noexcept = 0;

        protected:
            Concept() = default;
        };

        template <typename Signature>
        class Model final
            : public Concept
        {
        public:
            using StorageT = ExpectationCollection<Signature>;
            using ExpectationT = Expectation<Signature>;

            ~Model() noexcept(false) override
            {
                m_Storage->remove(m_Expectation);
            }

            [[nodiscard]]
            explicit Model(
                std::shared_ptr<StorageT>&& storage,
                std::shared_ptr<ExpectationT>&& expectation) noexcept
                : m_Storage{std::move(storage)},
                  m_Expectation{std::move(expectation)}
            {
                MIMICPP_ASSERT(m_Storage, "Storage is nullptr.");
                MIMICPP_ASSERT(m_Expectation, "Expectation is nullptr.");

                m_Storage->push(m_Expectation);
            }

            [[nodiscard]]
            bool is_satisfied() const override
            {
                return m_Expectation->is_satisfied();
            }

            [[nodiscard]]
            bool is_applicable() const override
            {
                return m_Expectation->is_applicable();
            }

            [[nodiscard]]
            util::SourceLocation const& from() const noexcept override
            {
                return m_Expectation->from();
            }

            [[nodiscard]]
            StringT const& mock_name() const noexcept override
            {
                return m_Expectation->mock_name();
            }

        private:
            std::shared_ptr<StorageT> m_Storage;
            std::shared_ptr<ExpectationT> m_Expectation;
        };

    public:
        /**
         * \brief Removes the owned expectation from the ExpectationCollection and checks, whether it's satisfied.
         * \throws In cases of an unsatisfied expectation, the destructor is expected to throw of terminate otherwise.
         */
        ~ScopedExpectation() noexcept(false) // NOLINT(modernize-use-equals-default)
        {
            // we must call the dtor manually here, because std::unique_ptr's dtor mustn't throw.
            delete m_Inner.release(); // NOLINT(*-uniqueptr-delete-release)
        }

        /**
         * \brief Constructor, which generates the type-erase storage.
         * \tparam Signature The signature.
         * \param collection The expectation collection, the expectation will be attached to.
         * \param expectation The expectation.
         */
        template <typename Signature>
        [[nodiscard]]
        explicit ScopedExpectation(
            std::shared_ptr<ExpectationCollection<Signature>> collection,
            std::shared_ptr<typename ExpectationCollection<Signature>::ExpectationT> expectation) noexcept
            : m_Inner{
                  std::make_unique<Model<Signature>>(
                      std::move(collection),
                      std::move(expectation))}
        {
        }

        /**
         * \brief A constructor, which accepts objects, which can be finalized (e.g. ExpectationBuilder).
         * \tparam T The object type.
         * \param object The object to be finalized.
         * \param loc The source-location.
         */
        template <typename T>
            requires requires(const std::source_location& loc) {
                { std::declval<T&&>().finalize(loc) } -> std::convertible_to<ScopedExpectation>;
            }
        [[nodiscard]]
        explicit(false) constexpr ScopedExpectation(T&& object, const std::source_location& loc = std::source_location::current())
            : ScopedExpectation{std::forward<T>(object).finalize(loc)}
        {
        }

        /**
         * \brief Deleted copy-constructor.
         */
        ScopedExpectation(const ScopedExpectation&) = delete;

        /**
         * \brief Deleted copy-assignment-operator.
         */
        ScopedExpectation& operator=(const ScopedExpectation&) = delete;

        /**
         * \brief Defaulted move-constructor.
         */
        [[nodiscard]]
        ScopedExpectation(ScopedExpectation&&) = default;

        /**
         * \brief Defaulted move-assignment-operator.
         */
        ScopedExpectation& operator=(ScopedExpectation&&) = default;

        /**
         * \brief Queries the stored expectation, whether it's satisfied.
         * \return True, if satisfied.
         */
        [[nodiscard]]
        bool is_satisfied() const
        {
            return m_Inner->is_satisfied();
        }

        /**
         * \brief Queries the stored expectation, whether it's applicable.
         * \return True, if applicable.
         */
        [[nodiscard]]
        bool is_applicable() const
        {
            return m_Inner->is_applicable();
        }

        /**
         * \brief Queries the stored expectation for it's stored source-location.
         * \return The stored source-location.
         */
        [[nodiscard]]
        util::SourceLocation const& from() const noexcept
        {
            return m_Inner->from();
        }

        /**
         * \brief Queries the stored expectation for the name of the related mock.
         * \return The stored mock-name.
         */
        [[nodiscard]]
        StringT const& mock_name() const noexcept
        {
            return m_Inner->mock_name();
        }

    private:
        std::unique_ptr<Concept> m_Inner;
    };

    /**
     * \}
     */
}

#endif
