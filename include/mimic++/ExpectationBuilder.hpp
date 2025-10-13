//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_BUILDER_HPP
#define MIMICPP_EXPECTATION_BUILDER_HPP

#pragma once

#include "mimic++/Expectation.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/Sequence.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/macros/ScopedExpectation.hpp"
#include "mimic++/matchers/Common.hpp"
#include "mimic++/matchers/GeneralMatchers.hpp"
#include "mimic++/matchers/StringMatchers.hpp"
#include "mimic++/policies/ArgRequirementPolicies.hpp"
#include "mimic++/policies/ControlPolicies.hpp"
#include "mimic++/policies/GeneralPolicies.hpp"
#include "mimic++/reporting/TargetReport.hpp"
#include "mimic++/utilities/Concepts.hpp"
#include "mimic++/utilities/PriorityTag.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <memory>
    #include <tuple>
    #include <utility>
#endif

namespace mimicpp::detail
{
    template <bool outcome>
    consteval bool verify_constraint([[maybe_unused]] std::string_view const diagnostic) noexcept
    {
        return outcome;
    }

    // ReSharper disable once CppFunctionIsNotImplemented
    template <>
    bool verify_constraint<false>([[maybe_unused]] std::string_view diagnostic) noexcept;
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp
{
    template <
        bool timesConfigured,
        typename SequenceConfig,
        typename Signature,
        typename FinalizePolicy,
        expectation_policy_for<Signature>... Policies>
    class BasicExpectationBuilder
    {
    public:
        using StorageT = ExpectationCollection<Signature>;
        using PolicyListT = std::tuple<Policies...>;
        using ReturnT = typename Expectation<Signature>::ReturnT;

        ~BasicExpectationBuilder() = default;

        template <typename FinalizePolicyArg, typename PolicyListArg>
            requires std::constructible_from<FinalizePolicy, FinalizePolicyArg>
                      && std::constructible_from<PolicyListT, PolicyListArg>
        [[nodiscard]]
        explicit constexpr BasicExpectationBuilder(
            std::shared_ptr<StorageT> storage,
            reporting::TargetReport target,
            detail::TimesConfig timesConfig,
            SequenceConfig sequenceConfig,
            FinalizePolicyArg&& finalizePolicyArg,
            PolicyListArg&& policyListArg) noexcept
            : m_Storage{std::move(storage)},
              m_TargetReport{std::move(target)},
              m_TimesConfig{std::move(timesConfig)},
              m_SequenceConfig{std::move(sequenceConfig)},
              m_FinalizePolicy{std::forward<FinalizePolicyArg>(finalizePolicyArg)},
              m_ExpectationPolicies{std::forward<PolicyListArg>(policyListArg)}
        {
            MIMICPP_ASSERT(m_Storage, "Storage is nullptr.");
        }

        BasicExpectationBuilder(const BasicExpectationBuilder&) = delete;
        BasicExpectationBuilder& operator=(const BasicExpectationBuilder&) = delete;

        [[nodiscard]]
        BasicExpectationBuilder(BasicExpectationBuilder&&) = default;
        BasicExpectationBuilder& operator=(BasicExpectationBuilder&&) = default;

        template <typename Policy>
            requires finalize_policy_for<std::remove_cvref_t<Policy>, Signature>
        [[nodiscard]]
        friend constexpr auto operator&&(BasicExpectationBuilder&& builder, Policy&& policy)
        {
            static_assert(
                detail::verify_constraint<!std::same_as<expectation_policies::InitFinalize, std::remove_cvref_t<Policy>>>(
                    "Explicitly specifying the `expectation_policies::InitFinalize` is disallowed."));

            static_assert(
                detail::verify_constraint<std::same_as<expectation_policies::InitFinalize, FinalizePolicy>>(
                    "Only one finalize-policy may be specified per expectation."
                    "See: https://dnkpp.github.io/mimicpp/db/d7a/group___e_x_p_e_c_t_a_t_i_o_n___f_i_n_a_l_i_z_e_r.html#details"));

            using Builder = BasicExpectationBuilder<
                timesConfigured,
                SequenceConfig,
                Signature,
                std::remove_cvref_t<Policy>,
                Policies...>;

            return Builder{
                std::move(builder.m_Storage),
                std::move(builder.m_TargetReport),
                std::move(builder.m_TimesConfig),
                std::move(builder.m_SequenceConfig),
                std::forward<Policy>(policy),
                std::move(builder.m_ExpectationPolicies)};
        }

        template <typename Policy>
            requires expectation_policy_for<std::remove_cvref_t<Policy>, Signature>
        [[nodiscard]]
        friend constexpr auto operator&&(BasicExpectationBuilder&& builder, Policy&& policy)
        {
            using ExtendedExpectationBuilderT = BasicExpectationBuilder<
                timesConfigured,
                SequenceConfig,
                Signature,
                FinalizePolicy,
                Policies...,
                std::remove_cvref_t<Policy>>;

            return ExtendedExpectationBuilderT{
                std::move(builder.m_Storage),
                std::move(builder.m_TargetReport),
                std::move(builder.m_TimesConfig),
                std::move(builder.m_SequenceConfig),
                std::move(builder.m_FinalizePolicy),
                std::tuple_cat(
                    std::move(builder.m_ExpectationPolicies),
                    std::forward_as_tuple(std::forward<Policy>(policy)))};
        }

        [[nodiscard]]
        friend constexpr auto operator&&(BasicExpectationBuilder&& builder, detail::TimesConfig&& config)
        {
            static_assert(
                detail::verify_constraint<!timesConfigured>(
                    "Only one times-policy may be specified per expectation."
                    "See: https://dnkpp.github.io/mimicpp/d7/d32/group___e_x_p_e_c_t_a_t_i_o_n___t_i_m_e_s.html#details"));

            using Builder = BasicExpectationBuilder<
                true,
                SequenceConfig,
                Signature,
                FinalizePolicy,
                Policies...>;

            return Builder{
                std::move(builder.m_Storage),
                std::move(builder.m_TargetReport),
                std::move(config),
                std::move(builder.m_SequenceConfig),
                std::move(builder.m_FinalizePolicy),
                std::move(builder.m_ExpectationPolicies)};
        }

        template <typename... Sequences>
        [[nodiscard]]
        friend constexpr auto operator&&(BasicExpectationBuilder&& builder, sequence::detail::Config<Sequences...>&& config)
        {
            sequence::detail::Config newConfig = builder.m_SequenceConfig.concat(std::move(config));

            using ExtendedExpectationBuilderT = BasicExpectationBuilder<
                timesConfigured,
                decltype(newConfig),
                Signature,
                FinalizePolicy,
                Policies...>;

            return ExtendedExpectationBuilderT{
                std::move(builder.m_Storage),
                std::move(builder.m_TargetReport),
                std::move(builder.m_TimesConfig),
                std::move(newConfig),
                std::move(builder.m_FinalizePolicy),
                std::move(builder.m_ExpectationPolicies)};
        }

        [[nodiscard]]
        ScopedExpectation finalize(util::SourceLocation sourceLocation) &&
            requires requires {
                requires detail::verify_constraint<finalize_policy_for<FinalizePolicy, Signature>>(
                    "For non-void return types, a finalize-policy must be specified."
                    "See: https://dnkpp.github.io/mimicpp/db/d7a/group___e_x_p_e_c_t_a_t_i_o_n___f_i_n_a_l_i_z_e_r.html#details");
            }
        {
            return ScopedExpectation{
                std::move(m_Storage),
                std::apply(
                    [&](auto&... policies) {
                        ControlPolicy controlPolicy{
                            std::move(m_TimesConfig),
                            std::move(m_SequenceConfig)};

                        using ExpectationT = BasicExpectation<
                            Signature,
                            decltype(controlPolicy),
                            FinalizePolicy,
                            Policies...>;

                        return std::make_unique<ExpectationT>(
                            std::move(sourceLocation),
                            std::move(m_TargetReport),
                            std::move(controlPolicy),
                            std::move(m_FinalizePolicy),
                            std::move(policies)...);
                    },
                    m_ExpectationPolicies)};
        }

    private:
        std::shared_ptr<StorageT> m_Storage;
        reporting::TargetReport m_TargetReport;
        detail::TimesConfig m_TimesConfig{};
        SequenceConfig m_SequenceConfig{};
        FinalizePolicy m_FinalizePolicy{};
        PolicyListT m_ExpectationPolicies{};
    };
}

namespace mimicpp::detail
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

    template <typename Arg, typename Target>
    concept requirement_for = requires {
        {
            detail::make_arg_matcher<Target>(maxMakeArgMatcherTag, std::declval<Arg>())
        } -> matcher_for<Target>;
    };

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
        [[maybe_unused]] std::index_sequence<indices...> const,
        Args&&... args)
    {
        return (
            std::forward<Builder>(builder)
            && ...
            && detail::make_arg_policy<Signature, indices>(std::forward<Args>(args)));
    }

    template <typename Signature, typename... Args>
    constexpr auto make_expectation_builder(
        std::shared_ptr<ExpectationCollection<Signature>> expectations,
        reporting::TargetReport target,
        Args&&... args)
    {
        using BaseBuilderT = BasicExpectationBuilder<
            false,
            sequence::detail::Config<>,
            Signature,
            expectation_policies::InitFinalize>;

        return detail::extend_builder_with_arg_policies<Signature>(
            BaseBuilderT{
                std::move(expectations),
                std::move(target),
                TimesConfig{},
                sequence::detail::Config<>{},
                expectation_policies::InitFinalize{},
                std::tuple{}},
            std::index_sequence_for<Args...>{},
            std::forward<Args>(args)...);
    }
}

#endif
