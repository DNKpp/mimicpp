//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_POLICIES_ARG_REQUIREMENT_POLICIES_HPP
#define MIMICPP_POLICIES_ARG_REQUIREMENT_POLICIES_HPP

#pragma once

#include "mimic++/matchers/Common.hpp"
#include "mimic++/policies/ArgumentList.hpp"
#include "mimic++/utilities/Concepts.hpp"
#include "mimic++/utilities/TypeList.hpp"

#include <concepts>
// ReSharper disable once CppUnusedIncludeDirective
#include <functional> // std::invoke
#include <tuple>
#include <type_traits>
#include <utility>

namespace mimicpp::expectation_policies
{
    template <typename Matcher>
    struct matcher_matches_fn
    {
    public:
        Matcher const& matcher;

        template <typename... Args>
            requires std::invocable<
                decltype(detail::matches_hook::matches),
                const Matcher&,
                Args&...>
        [[nodiscard]]
        // projected arguments may come as value, so Args& won't work in all cases
        // just forward them as lvalue-ref
        constexpr bool operator()(Args&&... args) const
            noexcept(
                std::is_nothrow_invocable_v<
                    decltype(detail::matches_hook::matches),
                    Matcher const&,
                    Args&...>)
        {
            return detail::matches_hook::matches(matcher, args...);
        }
    };

    template <typename Matcher, typename MatchesStrategy, typename DescribeStrategy>
    class ArgsRequirement
    {
    public:
        [[nodiscard]]
        explicit constexpr ArgsRequirement(
            Matcher matcher,
            MatchesStrategy matchesStrategy,
            DescribeStrategy describeStrategy)
            noexcept(
                std::is_nothrow_move_constructible_v<Matcher>
                && std::is_nothrow_move_constructible_v<MatchesStrategy>
                && std::is_nothrow_move_constructible_v<DescribeStrategy>)
            : m_Matcher{std::move(matcher)},
              m_MatchesStrategy{std::move(matchesStrategy)},
              m_DescribeStrategy{std::move(describeStrategy)}
        {
        }

        [[nodiscard]]
        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        template <typename Return, typename... Args>
            requires std::is_invocable_r_v<bool, MatchesStrategy const&, matcher_matches_fn<Matcher>, call::Info<Return, Args...> const&>
        [[nodiscard]]
        constexpr bool matches(const call::Info<Return, Args...>& info) const
            noexcept(std::is_nothrow_invocable_v<MatchesStrategy const&, matcher_matches_fn<Matcher>, call::Info<Return, Args...> const&>)
        {
            return std::invoke(
                m_MatchesStrategy,
                matcher_matches_fn<Matcher>{m_Matcher},
                info);
        }

        template <typename Return, typename... Args>
        static constexpr void consume([[maybe_unused]] call::Info<Return, Args...> const& info) noexcept
        {
        }

        [[nodiscard]]
        std::optional<StringT> describe() const
        {
            [[maybe_unused]] auto const description = detail::describe_hook::describe(m_Matcher);

            if constexpr (util::boolean_testable<decltype(description)>)
            {
                if (description)
                {
                    return std::invoke(m_DescribeStrategy, *description);
                }
            }
            else if constexpr (std::convertible_to<decltype(description), StringViewT>)
            {
                return std::invoke(m_DescribeStrategy, description);
            }

            return std::nullopt;
        }

    private:
        Matcher m_Matcher;
        [[no_unique_address]] MatchesStrategy m_MatchesStrategy;
        [[no_unique_address]] DescribeStrategy m_DescribeStrategy;
    };
}

namespace mimicpp::expect
{
    namespace detail
    {
        template <std::size_t index, std::size_t... others>
        struct arg_requirement_describer
        {
            [[nodiscard]]
            StringT operator()(StringViewT const matcherDescription) const
            {
                StringStreamT out{};
                out << "expect: arg[" << index;
                ((out << ", " << others), ...);
                out << "] " << matcherDescription;
                return std::move(out).str();
            }
        };

        struct all_args_requirement_describer
        {
            [[nodiscard]]
            StringT operator()(StringViewT const matcherDescription) const
            {
                StringStreamT out{};
                out << "expect: arg[all] " << matcherDescription;
                return std::move(out).str();
            }
        };

        template <
            std::size_t... indices,
            typename Matcher,
            typename... Projections>
        [[nodiscard]]
        constexpr auto make_args_policy(
            Matcher&& matcher,
            std::tuple<Projections...>&& projections)
        {
            static_assert(
                sizeof...(indices) == sizeof...(Projections),
                "Indices and projections size mismatch.");

            using arg_selector_t = mimicpp::detail::args_selector_fn<
                std::add_lvalue_reference_t,
                std::index_sequence<indices...>>;
            using apply_strategy_t = mimicpp::detail::arg_list_indirect_apply_fn<std::remove_cvref_t<Projections>...>;
            using describe_strategy_t = arg_requirement_describer<indices...>;

            return expectation_policies::ArgsRequirement{
                std::forward<Matcher>(matcher),
                mimicpp::detail::apply_args_fn(
                    arg_selector_t{},
                    apply_strategy_t{std::move(projections)}),
                describe_strategy_t{}};
        }
    }

    /**
     * \defgroup EXPECTATION_REQUIREMENT requirement
     * \ingroup EXPECTATION
     * \brief Requirements determine, whether an expectation matches an incoming call.
     * \details Requirements are the building blocks, which determine whether a call satisfies the expectation. If any of the specified
     * requirements fail, there is no match.
     * \note An expectation without requirements matches any call.
     *
     *\{
     */

    /**
     * \brief Checks whether the selected argument satisfies the given matcher.
     * \tparam index The index of the selected argument.
     * \tparam Matcher The matcher type.
     * \param matcher The matcher.
     * \param projection Projection to apply to the argument.
     *
     * \details This requirement checks, whether the selected argument matches the given matcher. One argument can be checked multiple times
     * in different requirements and all results will be combined as conjunction.
     *
     * For a list of built-in matchers, see \ref MATCHERS "matcher" section.
     * \snippet Requirements.cpp expect::arg
     */
    template <std::size_t index, typename Matcher, typename Projection = std::identity>
    [[nodiscard]]
    constexpr auto arg(
        Matcher&& matcher,
        Projection&& projection = {})
        noexcept(
            std::is_nothrow_constructible_v<std::remove_cvref_t<Matcher>, Matcher>
            && std::is_nothrow_constructible_v<std::remove_cvref_t<Projection>, Projection>)
    {
        return detail::make_args_policy<index>(
            std::forward<Matcher>(matcher),
            std::forward_as_tuple(std::forward<Projection>(projection)));
    }

    /**
     * \brief Checks whether the selected arguments satisfy the given matcher.
     * \tparam first The index of the first selected argument.
     * \tparam others The indices of other selected arguments.
     * \tparam Matcher The matcher type.
     * \param matcher The matcher.
     * \param projections Projections, the arguments will be applied on.
     *
     * \details This requirement checks, whether the selected arguments match the given matcher.
     * It's useful, when multiple arguments must be checked together, because they have some kind of relationship.
     * When ``n`` indices are provided the matcher must accept ``n`` arguments.
     *
     * The projections will be applied from the beginning: E.g. if ``n`` arguments and ``p`` projections are given
     * (with ``0 <= p <= n``), then the first ``p`` arguments will be applied on these projections (the ``i``-th argument
     * on the ``i``-th projection).
     * \note ``std::identity`` can be used to skip arguments, which shall not be projected.
     * \see https://en.cppreference.com/w/cpp/utility/functional/identity
     *
     * \details For a list of built-in matchers, see \ref MATCHERS "matcher" section.
     * \snippet Requirements.cpp expect::args
     */
    template <
        std::size_t first,
        std::size_t... others,
        typename Matcher,
        typename... Projections>
    [[nodiscard]]
    constexpr auto args(Matcher&& matcher, Projections&&... projections)
        noexcept(
            std::is_nothrow_constructible_v<std::remove_cvref_t<Matcher>, Matcher>
            && (... && std::is_nothrow_move_constructible_v<std::remove_cvref_t<Projections>>))
    {
        static_assert(
            sizeof...(projections) <= 1u + sizeof...(others),
            "The projection count exceeds the amount of indices.");

        return detail::make_args_policy<first, others...>(
            std::forward<Matcher>(matcher),
            util::detail::expand_tuple<std::identity, 1u + sizeof...(others)>(
                std::forward_as_tuple(std::forward<Projections>(projections)...)));
    }

    /**
     * \brief Checks whether the all arguments satisfy the given matcher.
     * \tparam Matcher The matcher type.
     * \param matcher The matcher.
     *
     * \details This requirement checks, whether the all arguments satisfy the given matcher.
     * It's useful, when all arguments must be checked together, because they have some kind of relationship.
     * When ``n`` arguments are provided the matcher must accept ``n`` arguments.
     *
     * \details For a list of built-in matchers, see \ref MATCHERS "matcher" section.
     * \snippet Requirements.cpp expect::all_args
     */
    template <typename Matcher>
    [[nodiscard]]
    constexpr auto all_args(Matcher&& matcher)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Matcher>, Matcher>)
    {
        using arg_selector_t = mimicpp::detail::all_args_selector_fn<std::add_lvalue_reference_t>;
        using apply_strategy_t = mimicpp::detail::arg_list_forward_apply_fn;
        using describe_strategy_t = detail::all_args_requirement_describer;

        return expectation_policies::ArgsRequirement{
            std::forward<Matcher>(matcher),
            mimicpp::detail::apply_args_fn(
                arg_selector_t{},
                apply_strategy_t{}),
            describe_strategy_t{}};
    }

    /**
     * \}
     */
}

#endif
