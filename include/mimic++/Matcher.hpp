//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHER_HPP
#define MIMICPP_MATCHER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Printer.hpp"
#include "mimic++/String.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/Utility.hpp"

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <ranges>
#include <tuple>
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
        [[maybe_unused]] const priority_tag<1>,
        const Matcher& matcher,
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
        [[maybe_unused]] const priority_tag<0>,
        const Matcher& matcher,
        T& target,
        Others&... others)
        requires requires { { matcher.matches(target, others...) } -> std::convertible_to<bool>; }
    {
        return matcher.matches(target, others...);
    }

    constexpr priority_tag<1> maxTag;

    constexpr auto matches = []<typename Matcher, typename T, typename... Others>(
                                 const Matcher& matcher,
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
        const Matcher& matcher,
        [[maybe_unused]] const priority_tag<1>)
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
        const Matcher& matcher,
        [[maybe_unused]] const priority_tag<0>)
        requires requires { { matcher.describe() } -> std::convertible_to<StringViewT>; }
    {
        return matcher.describe();
    }

    constexpr priority_tag<1> maxTag;

    constexpr auto describe = []<typename Matcher>(const Matcher& matcher) -> decltype(auto)
        requires requires { { describe_impl(matcher, maxTag) } -> std::convertible_to<StringViewT>; }
    {
        return describe_impl(matcher, maxTag);
    };
}

namespace mimicpp
{
    template <typename T, typename First, typename... Others>
    concept matcher_for = std::same_as<T, std::remove_cvref_t<T>>
                       && std::is_move_constructible_v<T>
                       && std::destructible<T>
                       && requires(const T& matcher, First& first, Others&... others) {
                              { detail::matches_hook::matches(matcher, first, others...) } -> std::convertible_to<bool>;
                              { detail::describe_hook::describe(matcher) } -> std::convertible_to<StringViewT>;
                          };

    namespace detail
    {
        template <typename Arg, typename MatchesProjection = std::identity, typename DescribeProjection = PrintFn>
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

    /**
     * \brief Generic matcher and the basic building block of most of the built-in matchers.
     * \tparam Predicate The predicate type.
     * \tparam AdditionalArgs Addition argument types.
     * \ingroup EXPECTATION_REQUIREMENT
     * \ingroup EXPECTATION_MATCHER
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
     * \ingroup EXPECTATION_REQUIREMENT
     * \ingroup EXPECTATION_MATCHER
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
     * \defgroup EXPECTATION_MATCHER matchers
     * \ingroup EXPECTATION_REQUIREMENT
     * \brief Matchers check various argument properties.
     * \details Matchers can be used to check various argument properties and are highly customizable. In general,
     * they simply compare their arguments with a pre-defined predicate, but also provide a meaningful description.
     *
     * \attention Matchers receive their arguments as possibly non-const, which is due to workaround some restrictions
     * on const qualified views. Either way, matchers should never modify any of their arguments.
     *
     * # Matching arguments
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
     * # Custom Matcher
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
            std::tuple{std::forward<T>(value)}};
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
            std::tuple{std::forward<T>(value)}};
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
            std::tuple{std::forward<T>(value)}};
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
            std::tuple{std::forward<T>(value)}};
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
            std::tuple{std::forward<T>(value)}};
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
            std::tuple{std::forward<T>(value)}};
    }

    /**
     * \brief Tests, whether the floating-point target is NaN.
     */
    [[nodiscard]]
    consteval auto NaN() noexcept
    {
        return PredicateMatcher{
            []<std::floating_point T>(const T target) noexcept {
                return std::isnan(target);
            },
            "is not a number (NaN)",
            "is a number"};
    }

    namespace detail
    {
        void check_fp_value(const std::floating_point auto value)
        {
            if (std::isnan(value)
                || std::isinf(value))
            {
                throw std::runtime_error{"Value must be not NaN and not infinity."};
            }
        }

        void check_fp_epsilon(const std::floating_point auto epsilon)
        {
            if (std::isnan(epsilon)
                || std::isinf(epsilon)
                || epsilon <= 0.)
            {
                throw std::runtime_error{"Epsilon must be not NaN, not infinity and not less or equal 0."};
            }
        }
    }

    /**
     * \brief Tests, whether the floating-point target is approximately equal to value.
     * \param value The value to compare to.
     * \param epsilon The maximum absolute difference.
     * \throws std::runtime_error When ``value`` is ``NaN`` or ``infinity``.
     * \throws std::runtime_error When ``epsilon`` is ``NaN``, ``infinity`` or not positive.
     * \return The newly created matcher.
     */
    [[nodiscard]]
    constexpr auto approx_abs(
        const std::floating_point auto value,
        const std::floating_point auto epsilon)
    {
        detail::check_fp_value(value);
        detail::check_fp_epsilon(epsilon);

        return PredicateMatcher{
            [](const std::floating_point auto target, const auto val, const auto eps) {
                return std::abs(target - val) <= eps;
                                                                                       },
            "is approximately {} +- {}",
            "is not approximately {} +- {}",
            std::tuple{value, epsilon}
        };
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
            std::tuple{std::addressof(instance)}};
    }

    /**
     * \}
     */
}

namespace mimicpp
{
    /**
     * \brief Tag type, used in string matchers.
     * \ingroup EXPECTATION_MATCHERS_STRING
     */
    struct case_insensitive_t
    {
    } constexpr case_insensitive{};
}

namespace mimicpp::matches::detail
{
    template <string String>
    [[nodiscard]]
    constexpr auto make_view(String&& str)
    {
        using traits_t = string_traits<std::remove_cvref_t<String>>;
        return traits_t::view(std::forward<String>(str));
    }

    template <case_foldable_string String>
    [[nodiscard]]
    constexpr auto make_case_folded_string(String&& str)
    {
        return std::invoke(
            string_case_fold_converter<string_char_t<String>>{},
            detail::make_view(std::forward<String>(str)));
    }

    template <string Target, string Pattern>
    constexpr void check_string_compatibility() noexcept
    {
        static_assert(
            std::same_as<
                string_char_t<Target>,
                string_char_t<Pattern>>,
            "Pattern and target string must have the same character-type.");
    }
}

namespace mimicpp::matches::str
{
    /**
     * \defgroup EXPECTATION_MATCHERS_STRING string matchers
     * \ingroup EXPECTATION_REQUIREMENT
     * \ingroup EXPECTATION_MATCHER
     * \brief String specific matchers.
     * \details These matchers are designed to work with any string- and character-type.
     * This comes with some caveats and restrictions, e.g. comparisons between strings of different character-types are not supported.
     * Any string, which satisfies the ``string`` concept is directly supported, thus it's possible to integrate your own types seamlessly.
     *
     * ## Example
     *
     * \snippet Requirements.cpp matcher str matcher
     *
     * ## Case-Insensitive Matchers
     *
     * In the following these terms are used:
     * - ``code-point`` is a logical string-element. In byte-strings these are the single characters, but in Unicode this may span multiple physical-elements.
     * - ``code-unit`` is the single physical-element of the string (i.e. the underlying character-type). Multiple ``code-units`` may build a single ``code-point``.
     *
     * All comparisons are done via iterators and, to make that consistent, ``mimic++`` requires the iterator values to be ``code-units``.
     * As this should be fine for equality-comparisons, this will quickly lead to issues when performing case-insensitive comparisons.
     * ``mimic++`` therefore converts all participating strings to their *case-folded* representation during comparisons.
     *
     * ### Case-Folding
     *
     * Case-Folding means the process of making a string independent of its case (e.g. ``a`` and ``A`` would compare equal).
     * This process is centralized to the ``string_case_fold_converter`` template, which must be specialized for the appropriate character-type.
     * The converter receives the whole string and is required to perform a consistent case-folding. Unicode defines the necessary steps here:
     * https://unicode-org.github.io/icu/userguide/transforms/casemappings.html#case-folding
     *
     * For a list of ``code-point`` mappings have a look here:
     * https://www.unicode.org/Public/UNIDATA/CaseFolding.txt
     *
     * Unfortunately, this requires a lot of work to make that work (correctly) for all existing character-types.
     * Due to this, currently only byte-strings are supported for case-insensitive comparisons.
     *
     * #### Byte-String
     *
     * Byte-Strings are element-wise lazily case-folded via ``std::toupper`` function.
     *
     * #### Strings with other character-types
     *
     * Even if ``mimic++`` does not support case-folding for other string types out of the box, users can specialize the ``string_case_fold_converter``
     * for the missing character-types and thus inject the necessary support.
     *
     * Another, but yet experimental, possibility is to enable the \ref MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER option.
     *
     *\{
     */

    /**
     * \brief Tests, whether the target string compares equal to the expected string.
     * \tparam Pattern The string type.
     * \param pattern The pattern object.
     */
    template <string Pattern>
    [[nodiscard]]
    constexpr auto eq(Pattern&& pattern)
    {
        return PredicateMatcher{
            []<string T, typename Stored>(T&& target, Stored&& stored) {
                detail::check_string_compatibility<T, Pattern>();

                return std::ranges::equal(
                    detail::make_view(std::forward<T>(target)),
                    detail::make_view(std::forward<Stored>(stored)));
            },
            "is equal to {}",
            "is not equal to {}",
            std::tuple{std::forward<Pattern>(pattern)}};
    }

    /**
     * \brief Tests, whether the target string compares case-insensitively equal to the expected string.
     * \tparam Pattern The string type.
     * \param pattern The pattern object.
     */
    template <case_foldable_string Pattern>
    [[nodiscard]]
    constexpr auto eq(Pattern&& pattern, [[maybe_unused]] const case_insensitive_t)
    {
        return PredicateMatcher{
            []<case_foldable_string T, typename Stored>(T&& target, Stored&& stored) {
                detail::check_string_compatibility<T, Pattern>();

                return std::ranges::equal(
                    detail::make_case_folded_string(std::forward<T>(target)),
                    detail::make_case_folded_string(std::forward<Stored>(stored)));
            },
            "is case-insensitively equal to {}",
            "is case-insensitively not equal to {}",
            std::tuple{std::forward<Pattern>(pattern)}};
    }

    /**
     * \brief Tests, whether the target string starts with the pattern string.
     * \tparam Pattern The string type.
     * \param pattern The pattern string.
     */
    template <string Pattern>
    [[nodiscard]]
    constexpr auto starts_with(Pattern&& pattern)
    {
        return PredicateMatcher{
            []<string T, typename Stored>(T&& target, Stored&& stored) {
                detail::check_string_compatibility<T, Pattern>();

                auto patternView = detail::make_view(std::forward<Stored>(stored));
                const auto [ignore, patternIter] = std::ranges::mismatch(
                    detail::make_view(std::forward<T>(target)),
                    patternView);

                return patternIter == std::ranges::end(patternView);
            },
            "starts with {}",
            "starts not with {}",
            std::tuple{std::forward<Pattern>(pattern)}};
    }

    /**
     * \brief Tests, whether the target string starts case-insensitively with the pattern string.
     * \tparam Pattern The string type.
     * \param pattern The pattern string.
     */
    template <string Pattern>
    [[nodiscard]]
    constexpr auto starts_with(Pattern&& pattern, [[maybe_unused]] const case_insensitive_t)
    {
        return PredicateMatcher{
            []<string T, typename Stored>(T&& target, Stored&& stored) {
                detail::check_string_compatibility<T, Pattern>();

                auto caseFoldedPattern = detail::make_case_folded_string(std::forward<Stored>(stored));
                const auto [ignore, patternIter] = std::ranges::mismatch(
                    detail::make_case_folded_string(std::forward<T>(target)),
                    caseFoldedPattern);

                return patternIter == std::ranges::end(caseFoldedPattern);
            },
            "case-insensitively starts with {}",
            "case-insensitively starts not with {}",
            std::tuple{std::forward<Pattern>(pattern)}};
    }

    /**
     * \brief Tests, whether the target string ends with the pattern string.
     * \tparam Pattern The string type.
     * \param pattern The pattern string.
     */
    template <string Pattern>
    [[nodiscard]]
    constexpr auto ends_with(Pattern&& pattern)
    {
        return PredicateMatcher{
            []<string T, typename Stored>(T&& target, Stored&& stored) {
                detail::check_string_compatibility<T, Pattern>();

                auto patternView = detail::make_view(std::forward<Stored>(stored))
                                 | std::views::reverse;
                const auto [ignore, patternIter] = std::ranges::mismatch(
                    detail::make_view(std::forward<T>(target)) | std::views::reverse,
                    patternView);

                return patternIter == std::ranges::end(patternView);
            },
            "ends with {}",
            "ends not with {}",
            std::tuple{std::forward<Pattern>(pattern)}};
    }

    /**
     * \brief Tests, whether the target string ends case-insensitively with the pattern string.
     * \tparam Pattern The string type.
     * \param pattern The pattern string.
     */
    template <string Pattern>
    [[nodiscard]]
    constexpr auto ends_with(Pattern&& pattern, [[maybe_unused]] const case_insensitive_t)
    {
        return PredicateMatcher{
            []<string T, typename Stored>(T&& target, Stored&& stored) {
                detail::check_string_compatibility<T, Pattern>();

                auto caseFoldedPattern = detail::make_case_folded_string(std::forward<Stored>(stored))
                                       | std::views::reverse;
                const auto [ignore, patternIter] = std::ranges::mismatch(
                    detail::make_case_folded_string(std::forward<T>(target)) | std::views::reverse,
                    caseFoldedPattern);

                return patternIter == std::ranges::end(caseFoldedPattern);
            },
            "case-insensitively ends with {}",
            "case-insensitively ends not with {}",
            std::tuple{std::forward<Pattern>(pattern)}};
    }

    /**
     * \brief Tests, whether the pattern string is part of the target string.
     * \tparam Pattern The string type.
     * \param pattern The pattern string.
     */
    template <string Pattern>
    [[nodiscard]]
    constexpr auto contains(Pattern&& pattern)
    {
        return PredicateMatcher{
            []<string T, typename Stored>(T&& target, Stored&& stored) {
                detail::check_string_compatibility<T, Pattern>();

                auto patternView = detail::make_view(std::forward<Stored>(stored));
                return std::ranges::empty(patternView)
                    || !std::ranges::empty(
                           std::ranges::search(
                               detail::make_view(std::forward<T>(target)),
                               std::move(patternView)));
            },
            "contains {}",
            "contains not {}",
            std::tuple{std::forward<Pattern>(pattern)}};
    }

    /**
     * \brief Tests, whether the pattern string is case-insensitively part of the target string.
     * \tparam Pattern The string type.
     * \param pattern The pattern string.
     */
    template <string Pattern>
    [[nodiscard]]
    constexpr auto contains(Pattern&& pattern, [[maybe_unused]] const case_insensitive_t)
    {
        return PredicateMatcher{
            []<string T, typename Stored>(T&& target, Stored&& stored) {
                detail::check_string_compatibility<T, Pattern>();

                auto patternView = detail::make_case_folded_string(std::forward<Stored>(stored));
                auto targetView = detail::make_case_folded_string(std::forward<T>(target));
                return std::ranges::empty(patternView)
                    || !std::ranges::empty(std::ranges::search(targetView, patternView));
            },
            "case-insensitively contains {}",
            "case-insensitively contains not {}",
            std::tuple{std::forward<Pattern>(pattern)}};
    }

    /**
     * \}
     */
}

namespace mimicpp::matches::range
{
    /**
     * \defgroup EXPECTATION_MATCHER_RANGE range matchers
     * \ingroup EXPECTATION_REQUIREMENT
     * \ingroup EXPECTATION_MATCHER
     * \brief Range specific matchers.
     * \snippet Requirements.cpp matcher range sorted
     *\{
     */

    /**
     * \brief Tests, whether the target range compares equal to the expected range, by comparing them element-wise.
     * \tparam Range Expected range type.
     * \tparam Comparator Comparator type.
     * \param expected The expected range.
     * \param comparator The comparator.
     */
    template <std::ranges::forward_range Range, typename Comparator = std::equal_to<>>
    [[nodiscard]]
    constexpr auto eq(Range&& expected, Comparator comparator = Comparator{})
    {
        return PredicateMatcher{
            [comp = std::move(comparator)]<typename Target>(Target&& target, auto& range) // NOLINT(cppcoreguidelines-missing-std-forward)
                requires std::predicate<
                             const Comparator&,
                             std::ranges::range_reference_t<Target>,
                             std::ranges::range_reference_t<Range>>
            {
                return std::ranges::equal(
                    target,
                    range,
                    std::ref(comp));
            },
            "elements are {}",
            "elements are not {}",
            std::tuple{std::views::all(std::forward<Range>(expected))}};
    }

    /**
     * \brief Tests, whether the target range is a permutation of the expected range, by comparing them element-wise.
     * \tparam Range Expected range type.
     * \tparam Comparator Comparator type.
     * \param expected The expected range.
     * \param comparator The comparator.
     */
    template <std::ranges::forward_range Range, typename Comparator = std::equal_to<>>
    [[nodiscard]]
    constexpr auto unordered_eq(Range&& expected, Comparator comparator = Comparator{})
    {
        return PredicateMatcher{
            [comp = std::move(comparator)]<typename Target>(Target&& target, auto& range) // NOLINT(cppcoreguidelines-missing-std-forward)
                requires std::predicate<
                             const Comparator&,
                             std::ranges::range_reference_t<Target>,
                             std::ranges::range_reference_t<Range>>
            {
                return std::ranges::is_permutation(
                    target,
                    range,
                    std::ref(comp));
            },
            "is a permutation of {}",
            "is not a permutation of {}",
            std::tuple{std::views::all(std::forward<Range>(expected))}};
    }

    /**
     * \brief Tests, whether the target range is sorted, by applying the relation on each adjacent elements.
     * \tparam Relation Relation type.
     * \param relation The relation.
     */
    template <typename Relation = std::ranges::less>
    [[nodiscard]]
    constexpr auto is_sorted(Relation relation = Relation{})
    {
        return PredicateMatcher{
            [rel = std::move(relation)]<typename Target>(Target&& target) // NOLINT(cppcoreguidelines-missing-std-forward)
                requires std::equivalence_relation<
                             const Relation&,
                             std::ranges::range_reference_t<Target>,
                             std::ranges::range_reference_t<Target>>
            {
                return std::ranges::is_sorted(
                    target,
                    std::ref(rel));
            },
            "is a sorted range",
            "is an unsorted range"};
    }

    /**
     * \brief Tests, whether the target range is empty.
     */
    [[nodiscard]]
    constexpr auto is_empty()
    {
        return PredicateMatcher{
            [](std::ranges::range auto&& target) {
                return std::ranges::empty(target);
            },
            "is an empty range",
            "is not an empty range"};
    }

    /**
     * \brief Tests, whether the target range has the expected size.
     * \param expected The expected size.
     */
    [[nodiscard]]
    constexpr auto has_size(const std::integral auto expected)
    {
        return PredicateMatcher{
            [](std::ranges::range auto&& target, const std::integral auto size) {
                return std::cmp_equal(
                    size,
                    std::ranges::size(target));
            },
            "has size of {}",
            "has different size than {}",
            std::tuple{expected}};
    }

    /**
     * \brief Tests, whether each element of the target range matches the specified matcher.
     * \param matcher The matcher.
     */
    template <typename Matcher>
    [[nodiscard]]
    constexpr auto each_element(Matcher&& matcher)
    {
        using MatcherT = std::remove_cvref_t<Matcher>;
        return PredicateMatcher{
            [](std::ranges::range auto&& target, const MatcherT& m) {
                return std::ranges::all_of(
                    target,
                    [&](const auto& element) { return m.matches(element); });
            },
            "each el in range: el {}",
            "not each el in range: el {}",
            std::tuple{
                mimicpp::detail::arg_storage<
                    MatcherT,
                    std::identity,
                    decltype([](const auto& m) { return mimicpp::detail::describe_hook::describe(m); })>{
                    std::forward<MatcherT>(matcher)}}};
    }

    /**
     * \brief Tests, whether any element of the target range matches the specified matcher.
     * \param matcher The matcher.
     */
    template <typename Matcher>
    [[nodiscard]]
    constexpr auto any_element(Matcher&& matcher)
    {
        using MatcherT = std::remove_cvref_t<Matcher>;
        return PredicateMatcher{
            [](std::ranges::range auto&& target, const MatcherT& m) {
                return std::ranges::any_of(
                    target,
                    [&](const auto& element) { return m.matches(element); });
            },
            "any el in range: el {}",
            "none el in range: el {}",
            std::tuple{
                mimicpp::detail::arg_storage<
                    MatcherT,
                    std::identity,
                    decltype([](const auto& m) { return mimicpp::detail::describe_hook::describe(m); })>{
                    std::forward<MatcherT>(matcher)}}};
    }

    /**
     * \}
     */
}

#endif
