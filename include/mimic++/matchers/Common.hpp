//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHERS_COMMON_HPP
#define MIMICPP_MATCHERS_COMMON_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/utilities/PriorityTag.hpp"

#include <concepts>
#include <type_traits>

namespace mimicpp::custom
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
            } -> std::convertible_to<bool>;
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
        requires requires { { matcher.matches(target, others...) } -> std::convertible_to<bool>; }
    {
        return matcher.matches(target, others...);
    }

    constexpr util::priority_tag<1> maxTag{};

    constexpr auto matches = []<typename Matcher, typename T, typename... Others>(
                                 Matcher const& matcher,
                                 T& target,
                                 Others&... others)
        requires requires {
            {
                matches_impl(maxTag, matcher, target, others...)
            }-> std::convertible_to<bool>; }
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
            } -> std::convertible_to<StringViewT>;
        }
    {
        return custom::matcher_traits<Matcher>{}.describe(matcher);
    }

    template <typename Matcher>
    [[nodiscard]]
    constexpr decltype(auto) describe_impl(
        [[maybe_unused]] util::priority_tag<0> const,
        Matcher const& matcher)
        requires requires { { matcher.describe() } -> std::convertible_to<StringViewT>; }
    {
        return matcher.describe();
    }

    constexpr util::priority_tag<1> maxTag{};

    constexpr auto describe = []<typename Matcher>(Matcher const& matcher) -> decltype(auto)
        requires requires { { describe_impl(maxTag, matcher) } -> std::convertible_to<StringViewT>; }
    {
        return describe_impl(maxTag, matcher);
    };
}

namespace mimicpp
{
    template <typename T, typename First, typename... Others>
    concept matcher_for = std::same_as<T, std::remove_cvref_t<T>>
                       && std::is_move_constructible_v<T>
                       && std::destructible<T>
                       && requires(T const& matcher, First& first, Others&... others) {
                              { detail::matches_hook::matches(matcher, first, others...) } -> std::convertible_to<bool>;
                              { detail::describe_hook::describe(matcher) } -> std::convertible_to<StringViewT>;
                          };
}

#endif
