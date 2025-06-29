//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHERS_COMMON_HPP
#define MIMICPP_MATCHERS_COMMON_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/Concepts.hpp"
#include "mimic++/utilities/PriorityTag.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <optional>
    #include <type_traits>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::custom
{
    template <typename Matcher>
    struct matcher_traits;
}

namespace mimicpp::detail::matches_hook
{
    template <typename Matcher, typename T, typename... Others>
    [[nodiscard]]
    constexpr bool matches_impl(
        [[maybe_unused]] util::priority_tag<1> const,
        Matcher const& matcher,
        T& target,
        Others&... others)
        requires requires {
            {
                custom::matcher_traits<Matcher>{}.matches(matcher, target, others...)
            } -> util::boolean_testable;
        }
    {
        return custom::matcher_traits<Matcher>{}.matches(matcher, target, others...);
    }

    template <typename Matcher, typename T, typename... Others>
    [[nodiscard]]
    constexpr bool matches_impl(
        [[maybe_unused]] util::priority_tag<0> const,
        Matcher const& matcher,
        T& target,
        Others&... others)
        requires requires { { matcher.matches(target, others...) } -> util::boolean_testable; }
    {
        return matcher.matches(target, others...);
    }

    inline util::priority_tag<1> constexpr maxTag{};

    inline auto constexpr matches = []<typename Matcher, typename T, typename... Others>(
                                        Matcher const& matcher,
                                        T& target,
                                 Others&... others)
        requires requires {
            {
                matches_impl(maxTag, matcher, target, others...)
            } -> util::boolean_testable; }
    {
        return matches_impl(maxTag, matcher, target, others...);
    };
}

namespace mimicpp::detail::describe_hook
{
    template <typename Matcher>
    [[nodiscard]]
    constexpr decltype(auto) describe_impl(
        [[maybe_unused]] util::priority_tag<1> const,
        Matcher const& matcher)
        requires requires {
            {
                custom::matcher_traits<Matcher>{}.describe(matcher)
            } -> util::explicitly_convertible_to<std::optional<StringT>>;
        }
    {
        return custom::matcher_traits<Matcher>{}.describe(matcher);
    }

    template <typename Matcher>
    [[nodiscard]]
    constexpr decltype(auto) describe_impl(
        [[maybe_unused]] util::priority_tag<0> const,
        Matcher const& matcher)
        requires requires { { matcher.describe() } -> util::explicitly_convertible_to<std::optional<StringT>>; }
    {
        return matcher.describe();
    }

    inline util::priority_tag<1> constexpr maxTag{};

    inline auto constexpr describe = []<typename Matcher>(Matcher const& matcher) -> decltype(auto)
        requires requires { { describe_impl(maxTag, matcher) } -> util::explicitly_convertible_to<std::optional<StringT>>; }
    {
        return describe_impl(maxTag, matcher);
    };
}

namespace mimicpp::matcher::detail
{
    /**
     * \brief Primary template, accepting any `Matcher`/`Args` combination.
     * \tparam Matcher The matcher type.
     * \tparam Args The argument types.
     */
    template <typename Matcher, typename... Args>
    struct is_accepting
        : public std::true_type
    {
    };

    template <typename Matcher, typename... Args>
    inline constexpr bool is_accepting_v{is_accepting<Matcher, Args...>::value};

    /**
     * \brief Specialization, reading the `is_accepting` trait from `Matcher`, when such member-type template exists.
     * \tparam Matcher The matcher type.
     * \tparam Args The argument types.
     */
    template <typename Matcher, typename... Args>
        requires requires(typename Matcher::template is_accepting<Args...> trait) {
            { trait } -> util::boolean_testable;
        }
    struct is_accepting<Matcher, Args...>
        : public Matcher::template is_accepting<Args...>
    {
    };
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp
{
    template <typename T, typename First, typename... Others>
    concept matcher_for = std::same_as<T, std::remove_cvref_t<T>>
                       && std::is_move_constructible_v<T>
                       && std::destructible<T>
                       && matcher::detail::is_accepting_v<T, First, Others...>
                       && requires(T const& matcher, First& first, Others&... others) {
                              { detail::matches_hook::matches(matcher, first, others...) } -> util::boolean_testable;
                              { detail::describe_hook::describe(matcher) } -> util::explicitly_convertible_to<std::optional<StringT>>;
                          };
}

#endif
