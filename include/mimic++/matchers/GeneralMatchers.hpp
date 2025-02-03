//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHERS_GENERAL_MATCHERS_HPP
#define MIMICPP_MATCHERS_GENERAL_MATCHERS_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/matchers/Common.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/StatePrinter.hpp"

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mimicpp::detail
{
    template <typename Arg, typename MatchesProjection = std::identity, typename DescribeProjection = printing::PrintFn>
    struct arg_storage
    {
        using matches_reference = std::invoke_result_t<MatchesProjection, const Arg&>;
        using describe_reference = std::invoke_result_t<DescribeProjection, const Arg&>;

        Arg arg;

        decltype(auto) as_matches_arg() const noexcept(std::is_nothrow_invocable_v<MatchesProjection, const Arg&>)
        {
            return std::invoke(MatchesProjection{}, arg);
        }

        decltype(auto) as_describe_arg() const noexcept(std::is_nothrow_invocable_v<DescribeProjection, const Arg&>)
        {
            return std::invoke(DescribeProjection{}, arg);
        }
    };

    template <typename T>
    struct to_arg_storage
    {
        using type = arg_storage<T>;
    };

    template <typename Arg, typename MatchesProjection, typename DescribeProjection>
    struct to_arg_storage<arg_storage<Arg, MatchesProjection, DescribeProjection>>
    {
        using type = arg_storage<Arg, MatchesProjection, DescribeProjection>;
    };

    template <typename T>
    using to_arg_storage_t = typename to_arg_storage<T>::type;
}

namespace mimicpp
{
    /**
     * \brief Generic matcher and the basic building block of most of the built-in matchers.
     * \tparam Predicate The predicate type.
     * \tparam AdditionalArgs Addition argument types.
     * \ingroup MATCHERS
     */
    template <typename Predicate, typename... AdditionalArgs>
        requires std::is_move_constructible_v<Predicate>
              && (... && std::is_move_constructible_v<AdditionalArgs>)
    class PredicateMatcher
    {
    private:
        using storage_t = std::tuple<detail::to_arg_storage_t<AdditionalArgs>...>;
        template <typename T>
        using matches_reference_t = typename detail::to_arg_storage_t<T>::matches_reference;

    public:
        [[nodiscard]]
        explicit constexpr PredicateMatcher(
            Predicate predicate,
            StringViewT fmt,
            StringViewT invertedFmt,
            std::tuple<AdditionalArgs...> additionalArgs = {})
            noexcept(
                std::is_nothrow_move_constructible_v<Predicate>
                && (... && std::is_nothrow_move_constructible_v<AdditionalArgs>))
            : m_Predicate{std::move(predicate)},
              m_FormatString{std::move(fmt)},
              m_InvertedFormatString{std::move(invertedFmt)},
              m_AdditionalArgs{std::move(additionalArgs)}
        {
        }

        template <typename First, typename... Others>
            requires std::predicate<
                const Predicate&,
                First&,
                Others&...,
                matches_reference_t<AdditionalArgs>...>
        [[nodiscard]]
        constexpr bool matches(
            First& first,
            Others&... others) const
            noexcept(
                std::is_nothrow_invocable_v<
                    const Predicate&,
                    First&,
                    Others&...,
                    matches_reference_t<AdditionalArgs>...>)
        {
            return std::apply(
                [&, this](auto&... additionalArgs) {
                    return std::invoke(
                        m_Predicate,
                        first,
                        others...,
                        additionalArgs.as_matches_arg()...);
                },
                m_AdditionalArgs);
        }

        [[nodiscard]]
        constexpr StringT describe() const
        {
            return std::apply(
                [&, this](auto&... additionalArgs) {
                    return format::vformat(
                        m_FormatString,
                        format::make_format_args(
                            std::invoke(
                                // std::make_format_args requires lvalue-refs, so let's transform rvalue-refs to const lvalue-refs
                                [](auto&& val) noexcept -> const auto& { return val; },
                                additionalArgs.as_describe_arg())...));
                },
                m_AdditionalArgs);
        }

        [[nodiscard]]
        constexpr auto operator!() const&
            requires std::is_copy_constructible_v<Predicate>
                  && std::is_copy_constructible_v<storage_t>
        {
            return make_inverted(
                m_Predicate,
                m_InvertedFormatString,
                m_FormatString,
                m_AdditionalArgs);
        }

        [[nodiscard]]
        constexpr auto operator!() &&
        {
            return make_inverted(
                std::move(m_Predicate),
                std::move(m_InvertedFormatString),
                std::move(m_FormatString),
                std::move(m_AdditionalArgs));
        }

    private:
        [[no_unique_address]] Predicate m_Predicate;
        StringViewT m_FormatString;
        StringViewT m_InvertedFormatString;
        storage_t m_AdditionalArgs;

        template <typename Fn>
        [[nodiscard]]
        static constexpr auto make_inverted(
            Fn&& fn,
            StringViewT fmt,
            StringViewT invertedFmt,
            storage_t tuple)
        {
            using NotFnT = decltype(std::not_fn(std::forward<Fn>(fn)));
            return PredicateMatcher<NotFnT, detail::to_arg_storage_t<AdditionalArgs>...>{
                std::not_fn(std::forward<Fn>(fn)),
                std::move(fmt),
                std::move(invertedFmt),
                std::move(tuple)};
        }
    };

    /**
     * \brief Matcher, which never fails.
     * \ingroup MATCHERS
     * \snippet Requirements.cpp matcher wildcard
     */
    class WildcardMatcher
    {
    public:
        static constexpr bool matches([[maybe_unused]] auto&& target) noexcept
        {
            return true;
        }

        static constexpr StringViewT describe() noexcept
        {
            return "has no constraints";
        }
    };
}

namespace mimicpp::matches
{
    /**
     * \defgroup MATCHERS matchers
     * \brief Matchers check various argument properties.
     * \details Matchers can be used to check various argument properties and are highly customizable. In general,
     * they simply compare their arguments with a pre-defined predicate, but also provide a meaningful description.
     *
     * \attention Matchers receive their arguments as possibly non-const, which is due to workaround some restrictions
     * on const qualified views. Either way, matchers should never modify any of their arguments.
     *
     * ### Matching arguments
     * In general matchers can be applied via the ``expect::arg<n>`` factory, but they can also be directly used
     * at the expect statement.
     * \snippet Requirements.cpp expect::arg
     * \snippet Requirements.cpp expect arg matcher
     *
     * \details For equality testing, there exists an even shorter syntax.
     * \snippet Requirements.cpp expect arg equal short
     *
     * \details Most of the built-in matchers support the inversion operator (``operator !``), which then tests for the opposite
     * condition.
     * \snippet Requirements.cpp matcher inverted
     *
     * ### Custom Matcher
     * Matchers are highly customizable. In fact, any type which satisfies ``matcher_for`` concept can be used.
     * There exists no base or interface type, but the ``PredicateMatcher`` servers as a convenient generic type, which
     * simply contains a predicate, a format string and optional additional arguments. But, this is just one option. If
     * you have some very specific needs, go and create your matcher from scratch.
     * \snippet Requirements.cpp matcher predicate matcher
     *
     *\{
     */

    /**
     * \brief The wildcard matcher, always matching.
     * \snippet Requirements.cpp matcher wildcard
     */
    [[maybe_unused]] inline constexpr WildcardMatcher _{};

    /**
     * \brief Tests, whether the target compares equal to the expected value.
     * \tparam T Expected type.
     * \param value Expected value.
     */
    template <typename T>
    [[nodiscard]]
    constexpr auto eq(T&& value)
    {
        return PredicateMatcher{
            std::equal_to{},
            "== {}",
            "!= {}",
            std::make_tuple(std::forward<T>(value))};
    }

    /**
     * \brief Tests, whether the target compares not equal to the expected value.
     * \tparam T Expected type.
     * \param value Expected value.
     */
    template <typename T>
    [[nodiscard]]
    constexpr auto ne(T&& value)
    {
        return PredicateMatcher{
            std::not_equal_to{},
            "!= {}",
            "== {}",
            std::make_tuple(std::forward<T>(value))};
    }

    /**
     * \brief Tests, whether the target is less than the expected value.
     * \tparam T Expected type.
     * \param value Expected value.
     */
    template <typename T>
    [[nodiscard]]
    constexpr auto lt(T&& value)
    {
        return PredicateMatcher{
            std::less{},
            "< {}",
            ">= {}",
            std::make_tuple(std::forward<T>(value))};
    }

    /**
     * \brief Tests, whether the target is less than or equal to the expected value.
     * \tparam T Expected type.
     * \param value Expected value.
     */
    template <typename T>
    [[nodiscard]]
    constexpr auto le(T&& value)
    {
        return PredicateMatcher{
            std::less_equal{},
            "<= {}",
            "> {}",
            std::make_tuple(std::forward<T>(value))};
    }

    /**
     * \brief Tests, whether the target is greater than the expected value.
     * \tparam T Expected type.
     * \param value Expected value.
     */
    template <typename T>
    [[nodiscard]]
    constexpr auto gt(T&& value)
    {
        return PredicateMatcher{
            std::greater{},
            "> {}",
            "<= {}",
            std::make_tuple(std::forward<T>(value))};
    }

    /**
     * \brief Tests, whether the target is greater than or equal to the expected value.
     * \tparam T Expected type.
     * \param value Expected value.
     */
    template <typename T>
    [[nodiscard]]
    constexpr auto ge(T&& value)
    {
        return PredicateMatcher{
            std::greater_equal{},
            ">= {}",
            "< {}",
            std::make_tuple(std::forward<T>(value))};
    }

    /**
     * \brief Tests, whether the target fulfills the given predicate.
     * \tparam UnaryPredicate Predicate type.
     * \param predicate The predicate to test.
     * \param description The formatting string.
     * \param invertedDescription The formatting string for the inversion.
     * \snippet Requirements.cpp matcher predicate
     */
    template <typename UnaryPredicate>
    [[nodiscard]]
    constexpr auto predicate(
        UnaryPredicate&& predicate,
        StringViewT description = "passes predicate",
        StringViewT invertedDescription = "fails predicate")
    {
        return PredicateMatcher{
            std::forward<UnaryPredicate>(predicate),
            std::move(description),
            std::move(invertedDescription),
        };
    }

    /**
     * \brief Tests, whether the target is the expected instance.
     * \tparam T Instance type.
     * \param instance The instance to be compared to.
     * \snippet Requirements.cpp matcher instance
     */
    template <satisfies<std::is_lvalue_reference> T>
    [[nodiscard]]
    constexpr auto instance(T&& instance) // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        return PredicateMatcher{
            [](const std::remove_cvref_t<T>& target, const auto* instancePtr) noexcept {
                return std::addressof(target) == instancePtr;
            },
            "is instance at {}",
            "is not instance at {}",
            std::make_tuple(std::addressof(instance))};
    }

    /**
     * \}
     */
}

#endif
