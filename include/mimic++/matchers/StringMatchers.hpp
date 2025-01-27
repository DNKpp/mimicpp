//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHERS_STRING_MATCHERS_HPP
#define MIMICPP_MATCHERS_STRING_MATCHERS_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/String.hpp"
#include "mimic++/matchers/GeneralMatchers.hpp"

#include <algorithm>
#include <concepts>
#include <ranges>
#include <tuple>
#include <utility>

namespace mimicpp
{
    /**
     * \brief Tag type, used in string matchers.
     * \ingroup MATCHERS_STRING
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
        requires std::ranges::view<String>
    [[nodiscard]]
    constexpr auto make_case_folded_string(String str)
    {
        return std::invoke(
            string_case_fold_converter<string_char_t<String>>{},
            std::move(str));
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

    struct make_view_fn
    {
        template <string T>
        [[nodiscard]]
        constexpr auto operator()(T const& str) const
        {
            return string_traits<T>::view(str);
        }
    };

    struct describe_fn
    {
        template <string T>
        [[nodiscard]]
        constexpr auto operator()(T const& str) const
        {
            return mimicpp::print(std::invoke(make_view_fn{}, str));
        }
    };

    // This specialization is needed for raw char (const)* strings, for which the matcher loses information about
    // and wrongly treats as general raw-pointer.
    template <string T>
    using string_arg_storage = mimicpp::detail::arg_storage<
        T,
        make_view_fn,
        describe_fn>;
    ;
}

namespace mimicpp::matches::str
{
    /**
     * \defgroup MATCHERS_STRING string matchers
     * \ingroup MATCHERS
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
            []<string T>(T&& target, auto const& patternView) {
                detail::check_string_compatibility<T, Pattern>();

                return std::ranges::equal(
                    detail::make_view(target),
                    patternView);
            },
            "is equal to {}",
            "is not equal to {}",
            std::make_tuple(
                detail::string_arg_storage{std::forward<Pattern>(pattern)})};
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
            []<case_foldable_string T>(T&& target, auto const& patternView) {
                detail::check_string_compatibility<T, Pattern>();

                return std::ranges::equal(
                    detail::make_case_folded_string(detail::make_view(target)),
                    detail::make_case_folded_string(patternView));
            },
            "is case-insensitively equal to {}",
            "is case-insensitively not equal to {}",
            std::make_tuple(
                detail::string_arg_storage{std::forward<Pattern>(pattern)})};
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
            []<string T>(T&& target, auto const& patternView) {
                detail::check_string_compatibility<T, Pattern>();

                auto const [ignore, patternIter] = std::ranges::mismatch(
                    detail::make_view(target),
                    patternView);

                return patternIter == std::ranges::end(patternView);
            },
            "starts with {}",
            "starts not with {}",
            std::make_tuple(
                detail::string_arg_storage{std::forward<Pattern>(pattern)})};
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
            []<case_foldable_string T>(T&& target, auto const& patternView) {
                detail::check_string_compatibility<T, Pattern>();

                auto const caseFoldedPattern = detail::make_case_folded_string(patternView);
                auto const [ignore, patternIter] = std::ranges::mismatch(
                    detail::make_case_folded_string(detail::make_view(target)),
                    caseFoldedPattern);

                return patternIter == std::ranges::end(caseFoldedPattern);
            },
            "case-insensitively starts with {}",
            "case-insensitively starts not with {}",
            std::make_tuple(
                detail::string_arg_storage{std::forward<Pattern>(pattern)})};
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
            []<string T>(T&& target, auto const& patternView) {
                detail::check_string_compatibility<T, Pattern>();

                auto reversedPattern = patternView | std::views::reverse;
                const auto [ignore, patternIter] = std::ranges::mismatch(
                    detail::make_view(target) | std::views::reverse,
                    reversedPattern);

                return patternIter == std::ranges::end(reversedPattern);
            },
            "ends with {}",
            "ends not with {}",
            std::make_tuple(
                detail::string_arg_storage{std::forward<Pattern>(pattern)})};
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
            []<case_foldable_string T>(T&& target, auto const& patternView) {
                detail::check_string_compatibility<T, Pattern>();

                auto reversedCaseFoldedPattern = detail::make_case_folded_string(patternView)
                                               | std::views::reverse;
                const auto [ignore, patternIter] = std::ranges::mismatch(
                    detail::make_case_folded_string(detail::make_view(target)) | std::views::reverse,
                    reversedCaseFoldedPattern);

                return patternIter == std::ranges::end(reversedCaseFoldedPattern);
            },
            "case-insensitively ends with {}",
            "case-insensitively ends not with {}",
            std::make_tuple(
                detail::string_arg_storage{std::forward<Pattern>(pattern)})};
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
            []<string T>(T&& target, auto const& patternView) {
                detail::check_string_compatibility<T, Pattern>();

                return std::ranges::empty(patternView)
                    || !std::ranges::empty(
                           std::ranges::search(
                               detail::make_view(target),
                               patternView));
            },
            "contains {}",
            "contains not {}",
            std::make_tuple(
                detail::string_arg_storage{std::forward<Pattern>(pattern)})};
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
            []<case_foldable_string T>(T&& target, auto const& patternView) {
                detail::check_string_compatibility<T, Pattern>();

                auto caseFoldedPattern = detail::make_case_folded_string(patternView);
                auto targetView = detail::make_case_folded_string(detail::make_view(target));
                return std::ranges::empty(caseFoldedPattern)
                    || !std::ranges::empty(std::ranges::search(targetView, caseFoldedPattern));
            },
            "case-insensitively contains {}",
            "case-insensitively contains not {}",
            std::make_tuple(
                detail::string_arg_storage{std::forward<Pattern>(pattern)})};
    }

    /**
     * \}
     */
}

#endif
