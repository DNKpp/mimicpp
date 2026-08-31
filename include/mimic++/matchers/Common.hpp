//          Copyright Dominic (DNKpp) Koepke 2024 - 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHERS_COMMON_HPP
#define MIMICPP_MATCHERS_COMMON_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/expectation/Common.hpp"
#include "mimic++/utilities/Concepts.hpp"
#include "mimic++/utilities/PriorityTag.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <functional>
    #include <optional>
    #include <type_traits>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::custom
{
    template <typename Matcher>
    struct matcher_traits;
}

namespace mimicpp::detail::describe_hook
{
    // This section uses trailing return-types because this seems to help clangd in some cases.
    template <typename Matcher>
    [[nodiscard]]
    constexpr auto describe_impl(util::priority_tag<1> const /*tag*/, Matcher const& matcher)
        -> decltype(custom::matcher_traits<Matcher>{}.describe(matcher))
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
    constexpr auto describe_impl(util::priority_tag<0> const /*tag*/, Matcher const& matcher)
        -> decltype(matcher.describe())
        requires requires {
            { matcher.describe() } -> util::explicitly_convertible_to<std::optional<StringT>>;
        }
    {
        return matcher.describe();
    }

    inline constexpr util::priority_tag<1> maxTag{};

    struct describe_fn
    {
        template <typename Matcher>
        [[nodiscard]]
        constexpr auto operator()(Matcher const& matcher) const
            -> decltype(describe_impl(maxTag, matcher))
            requires requires {
                { describe_impl(maxTag, matcher) } -> util::explicitly_convertible_to<std::optional<StringT>>;
            }
        {
            return describe_impl(maxTag, matcher);
        }
    };

    inline constexpr describe_fn describe{};
}

namespace mimicpp::detail::matches_hook
{
    template <typename Matcher, typename... Ts>
    [[nodiscard]]
    constexpr expectation::MatchResult matches_impl(util::priority_tag<3> const /*tag*/, Matcher const& matcher, Ts&... targets)
        requires requires {
            { custom::matcher_traits<Matcher>{}.matches(matcher, targets...) } -> std::convertible_to<expectation::MatchResult>;
        }
    {
        return custom::matcher_traits<Matcher>{}.matches(matcher, targets...);
    }

    template <typename Matcher, typename... Ts>
    [[nodiscard]]
    constexpr expectation::MatchResult matches_impl(util::priority_tag<0> const /*tag*/, Matcher const& matcher, Ts&... targets)
        requires requires {
            { matcher.matches(targets...) } -> std::convertible_to<expectation::MatchResult>;
        }
    {
        return matcher.matches(targets...);
    }

    // deprecated matches overloads
    template <typename Matcher, typename... Ts>
    [[nodiscard]]
    constexpr expectation::MatchResult matches_impl(util::priority_tag<0> const /*tag*/, Matcher const& matcher, Ts&... targets)
        requires requires {
            { custom::matcher_traits<Matcher>{}.matches(matcher, targets...) } -> util::boolean_testable;
        }
    {
        if (custom::matcher_traits<Matcher>{}.matches(matcher, targets...))
        {
            return expectation::MatchSuccess{};
        }

        return expectation::MatchFailure{
            .description = [&matcher] { return std::optional<StringT>{describe_hook::describe(matcher)}; },
        };
    }

    template <typename Matcher, typename... Ts>
    [[nodiscard]]
    constexpr expectation::MatchResult matches_impl(util::priority_tag<0> const /*tag*/, Matcher const& matcher, Ts&... targets)
        requires requires {
            { matcher.matches(targets...) } -> util::boolean_testable;
        }
    {
        if (matcher.matches(targets...))
        {
            return expectation::MatchSuccess{};
        }

        return expectation::MatchFailure{
            .description = [&matcher] { return std::optional<StringT>{describe_hook::describe(matcher)}; },
        };
    }

    inline constexpr util::priority_tag<3> maxTag{};

    struct matches_fn
    {
        template <typename Matcher, typename T, typename... Others>
        [[nodiscard]]
        constexpr expectation::MatchResult operator()(Matcher const& matcher, T& target, Others&... others) const
            requires requires {
                { matches_impl(maxTag, matcher, target, others...) } -> std::convertible_to<expectation::MatchResult>;
            }
        {
            return matches_impl(maxTag, matcher, target, others...);
        }
    };

    inline constexpr matches_fn matches{};
}

namespace mimicpp
{
    namespace detail
    {
        /**
         * \brief Primary template, accepting any `Matcher`/`Args` combination.
         * \tparam Matcher The matcher type.
         * \tparam Args The argument types.
         */
        template <typename Matcher, typename... Args>
        struct is_matcher_accepting
            : public std::true_type
        {
        };

        template <template <typename...> typename is_accepting, typename... Args>
        struct is_matcher_accepting_helper
            : public is_accepting<Args...>
        {
        };

        /**
         * \brief Specialization, reading the `is_accepting` trait from `Matcher`, when such member-type template exists.
         * \tparam Matcher The matcher type.
         * \tparam Args The argument types.
         */
        template <typename Matcher, typename... Args>
            requires requires { typename is_matcher_accepting_helper<Matcher::template is_accepting, Args...>; }
        struct is_matcher_accepting<Matcher, Args...>
            : public is_matcher_accepting_helper<Matcher::template is_accepting, Args...>
        {
        };
    }

    /**
     * \brief Determines, whether the given `Matcher` accepts the specified `Args`.
     * \tparam Matcher The matcher type.
     * \tparam Args The argument types.
     * \details This does check, whether matcher contains a member-type template `is_accepting`,
     * which itself has a `value` bool-member.
     * This is actually used by matchers to reject arguments at compile-time and thus removing specific overloads from
     * the candidate-set.
     * When no such member-type-template is found it defaults to `true`.
     */
    template <typename Matcher, typename... Args>
    inline constexpr bool is_matcher_accepting_v{detail::is_matcher_accepting<Matcher, Args...>::value};
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp
{
    template <typename T, typename First, typename... Others>
    concept matcher_for = std::same_as<T, std::remove_cvref_t<T>>
                       && std::is_move_constructible_v<T>
                       && std::destructible<T>
                       && is_matcher_accepting_v<T, First, Others...>
                       && requires(T const& matcher, First& first, Others&... others) {
                              { detail::matches_hook::matches(matcher, first, others...) } -> std::convertible_to<expectation::MatchResult>;
                          };
}

#endif
