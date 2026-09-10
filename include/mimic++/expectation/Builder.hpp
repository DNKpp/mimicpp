//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_BUILDER_HPP
#define MIMICPP_EXPECTATION_BUILDER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/expectation/Common.hpp"
#include "mimic++/expectation/Owner.hpp"
#include "mimic++/expectation/Registry.hpp"
#include "mimic++/expectation/policies/ControlPolicies.hpp"
#include "mimic++/expectation/policies/GeneralPolicies.hpp"
#include "mimic++/expectation/policies/RequirementPolicies.hpp"
#include "mimic++/macros/ScopedExpectation.hpp"
#include "mimic++/matchers/Common.hpp"
#include "mimic++/matchers/GeneralMatchers.hpp"
#include "mimic++/matchers/StringMatchers.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <functional>
    #include <tuple>
    #include <type_traits>
    #include <utility>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::expectation
{
    namespace detail
    {
        struct BuilderState
        {
            bool timesConfigured = false;
            bool finalizeConfigured = false;

            [[nodiscard]]
            consteval BuilderState with(bool(BuilderState::* option)) const noexcept
            {
                MIMICPP_ASSERT(option, "Option must be non-null");
                BuilderState copy{*this};
                std::invoke(option, copy) = true;

                return copy;
            }
        };

        template <typename SequenceConfig, typename FinalizePolicy, typename... Policies>
        struct BuilderPolicies
        {
            using FinalizePolicyType = FinalizePolicy;

            using TimesConfig = policies::detail::TimesConfig;
            TimesConfig times{};
            SequenceConfig sequences{};
            FinalizePolicy finalize{};
            std::tuple<Policies...> policies{};

            constexpr auto with_times(TimesConfig newTimes) &&
            {
                return BuilderPolicies{
                    std::move(newTimes),
                    std::move(sequences),
                    std::move(finalize),
                    std::move(policies)};
            }

            template <typename Policy>
            constexpr auto with_sequences(Policy&& policy) &&
            {
                auto newSequences = sequences.concat(std::forward<Policy>(policy));

                return BuilderPolicies<decltype(newSequences), FinalizePolicy, Policies...>{
                    std::move(times),
                    std::move(newSequences),
                    std::move(finalize),
                    std::move(policies)};
            }

            template <typename Policy>
            constexpr auto with_finalize(Policy&& policy) &&
            {
                return BuilderPolicies<SequenceConfig, std::remove_cvref_t<Policy>, Policies...>{
                    std::move(times),
                    std::move(sequences),
                    std::forward<Policy>(policy),
                    std::move(policies)};
            }

            template <typename Policy>
            constexpr auto with_policy(Policy&& policy) &&
            {
                return BuilderPolicies<SequenceConfig, FinalizePolicy, Policies..., std::remove_cvref_t<Policy>>{
                    std::move(times),
                    std::move(sequences),
                    std::move(finalize),
                    std::tuple_cat(
                        std::move(policies),
                        std::forward_as_tuple(std::forward<Policy>(policy)))};
            }
        };
    }

    template <typename Signature, detail::BuilderState state, typename Policies>
    class BasicBuilder
    {
    public:
        using TimesConfig = policies::detail::TimesConfig;

        BasicBuilder(BasicBuilder const&) = delete;
        BasicBuilder& operator=(BasicBuilder const&) = delete;

        ~BasicBuilder() = default;

        [[nodiscard]]
        explicit BasicBuilder(Registry::Ptr&& registry, reporting::TargetReport&& target, Policies&& policies)
            : m_Registry{std::move(registry)},
              m_TargetReport{std::move(target)},
              m_Policies{std::move(policies)}
        {
            MIMICPP_ASSERT(m_Registry, "Null registry is not allowed.");
        }

        [[nodiscard]]
        BasicBuilder(BasicBuilder&&) = default;
        BasicBuilder& operator=(BasicBuilder&&) = default;

        template <typename Policy>
            requires finalize_policy_for<std::remove_cvref_t<Policy>, Signature>
        [[nodiscard]]
        friend constexpr auto operator&&(BasicBuilder&& builder, Policy&& policy)
        {
            static_assert(
                !std::same_as<policies::InitFinalize, std::remove_cvref_t<Policy>>,
                "Explicitly specifying the `policies::InitFinalize` is disallowed.");

            static_assert(
                !state.finalizeConfigured,
                "Only one finalize-policy may be specified per expectation. "
                "See: https://dnkpp.github.io/mimicpp/db/d7a/group___e_x_p_e_c_t_a_t_i_o_n___f_i_n_a_l_i_z_e_r.html#details");

            auto newPolicies = std::move(builder.m_Policies).with_finalize(std::forward<Policy>(policy));
            using Builder = BasicBuilder<
                Signature,
                state.with(&detail::BuilderState::finalizeConfigured),
                decltype(newPolicies)>;

            return Builder{
                std::move(builder.m_Registry),
                std::move(builder.m_TargetReport),
                std::move(newPolicies)};
        }

        template <typename Policy>
            requires expectation_policy_for<std::remove_cvref_t<Policy>, Signature>
        [[nodiscard]]
        friend constexpr auto operator&&(BasicBuilder&& builder, Policy&& policy)
        {
            auto newPolicies = std::move(builder.m_Policies).with_policy(std::forward<Policy>(policy));
            using Builder = BasicBuilder<
                Signature,
                state,
                decltype(newPolicies)>;

            return Builder{
                std::move(builder.m_Registry),
                std::move(builder.m_TargetReport),
                std::move(newPolicies)};
        }

        [[nodiscard]]
        friend constexpr auto operator&&(BasicBuilder&& builder, TimesConfig&& config)
        {
            static_assert(
                !state.timesConfigured,
                "Only one times-policy may be specified per expectation. "
                "See: https://dnkpp.github.io/mimicpp/d7/d32/group___e_x_p_e_c_t_a_t_i_o_n___t_i_m_e_s.html#details");

            using Builder = BasicBuilder<
                Signature,
                state.with(&detail::BuilderState::timesConfigured),
                Policies>;

            return Builder{
                std::move(builder.m_Registry),
                std::move(builder.m_TargetReport),
                std::move(builder.m_Policies).with_times(std::forward<TimesConfig>(config))};
        }

        template <typename... Sequences>
        [[nodiscard]]
        friend constexpr auto operator&&(BasicBuilder&& builder, sequence::detail::Config<Sequences...>&& config)
        {
            auto newPolicies = std::move(builder.m_Policies).with_sequences(std::move(config));
            using Builder = BasicBuilder<
                Signature,
                state,
                decltype(newPolicies)>;

            return Builder{
                std::move(builder.m_Registry),
                std::move(builder.m_TargetReport),
                std::move(newPolicies)};
        }

        [[nodiscard]]
        Owner finalize(util::SourceLocation sourceLocation) &&
        {
            static_assert(
                finalize_policy_for<typename Policies::FinalizePolicyType, Signature>,
                "For non-void return types, a finalize-policy must be specified. "
                "See: https://dnkpp.github.io/mimicpp/db/d7a/group___e_x_p_e_c_t_a_t_i_o_n___f_i_n_a_l_i_z_e_r.html#details");

            auto expectation = std::apply(
                [&](auto&... policies) {
                    return m_Registry->create(
                        std::in_place_type<Signature>,
                        std::move(sourceLocation),
                        std::move(m_TargetReport),
                        policies::ControlPolicy{sourceLocation, std::move(m_Policies.times), std::move(m_Policies.sequences)},
                        std::move(m_Policies.finalize),
                        std::move(policies)...);
                },
                m_Policies.policies);

            return Owner{std::move(m_Registry), std::move(expectation)};
        }

    private:
        Registry::Ptr m_Registry;
        reporting::TargetReport m_TargetReport;
        Policies m_Policies;
    };

    namespace detail
    {
        template <typename Param, typename... Canary, matcher_for<Param> Arg>
        [[nodiscard]]
        constexpr auto make_arg_matcher([[maybe_unused]] util::priority_tag<2> const, Arg arg)
        {
            return arg;
        }

        // if the param is a character-pointer, there is no evidence, whether it denotes a null-terminated string or just an
        // actual pointer to a value.
        // But, the Mock user shall know it, thus if `Arg` is not a character-pointer, we enable this matcher.
        template <string Param, typename... Canary, string Arg>
            requires(!std::is_pointer_v<std::remove_reference_t<Param>>)
                 || (!std::is_pointer_v<std::remove_reference_t<Arg>>)
        [[nodiscard]] //
        constexpr auto make_arg_matcher([[maybe_unused]] util::priority_tag<1> const, Arg&& arg)
        {
            return matches::str::eq(std::forward<Arg>(arg));
        }

        template <typename Param, typename... Canary, util::weakly_equality_comparable_with<Param> Arg>
        [[nodiscard]]
        constexpr auto make_arg_matcher([[maybe_unused]] util::priority_tag<0> const, Arg&& arg)
        {
            return matches::eq(std::forward<Arg>(arg));
        }

        inline constexpr util::priority_tag<2> maxMakeArgMatcherTag{};
    }

    /**
     * \brief Determines whether `Arg` can be used as a requirement for `Param`.
     * \ingroup EXPECTATION
     * \details
     * This concept verifies that the given `Arg` specifies actual requirements for `Param`.
     * In fact, it does check whether:
     * - `Arg` already is an actual matcher for `Param` (via `matcher_for` concept),
     * - `Arg` and `Params` are compatible strings (which then utilizes `matches::str::eq`),
     * - or `Arg` and `Params` are already equality-comparable (which then uses `matches::eq`).
     */
    template <typename Arg, typename Param>
    concept requirement_for = requires {
        {
            detail::make_arg_matcher<Param>(detail::maxMakeArgMatcherTag, std::declval<Arg>())
        } -> matcher_for<Param>;
    };

    namespace detail
    {
        template <
            typename Signature,
            std::size_t index,
            typename Arg,
            typename... Canary,
            typename Param = signature_param_type_t<index, Signature>>
            requires requirement_for<Arg, Param>
        constexpr auto make_arg_policy(Arg&& arg)
        {
            return expect::arg<index>(
                detail::make_arg_matcher<Param>(maxMakeArgMatcherTag, std::forward<Arg>(arg)));
        }

        template <typename Signature, typename Builder, std::size_t... indices, typename... Args>
        [[nodiscard]]
        constexpr auto extend_builder_with_arg_policies(
            Builder&& builder,
            std::index_sequence<indices...> const /*seq*/,
            Args&&... args)
        {
            return (
                std::forward<Builder>(builder)
                && ...
                && detail::make_arg_policy<Signature, indices>(std::forward<Args>(args)));
        }

        template <typename Signature, typename... Args>
        auto make_builder(Registry::Ptr registry, reporting::TargetReport target, Args&&... args)
        {
            using Policies = BuilderPolicies<sequence::detail::Config<>, policies::InitFinalize>;
            using Builder = BasicBuilder<Signature, BuilderState{}, Policies>;

            return detail::extend_builder_with_arg_policies<Signature>(
                Builder{std::move(registry), std::move(target), Policies{}},
                std::index_sequence_for<Args...>{},
                std::forward<Args>(args)...);
        }
    }
}

#endif
