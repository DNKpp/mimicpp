//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/StringMatchers.hpp"

#ifndef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER
    #error "Unicode string-matcher feature must be active."
#endif

namespace matches = mimicpp::matches;

TEST_CASE(
    "matches::str::starts_with case-insensitively overload supports empty strings.",
    "[matcher][matcher::str]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* patter = "";
        constexpr auto* alternativeMatch = " ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts with \"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts not with \"\""));
            CHECK(!matcher.matches(patter));
            CHECK(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For wchar_t-strings.")
    {
        constexpr auto* patter = L"";
        constexpr auto* alternativeMatch = L" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts with L\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts not with L\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char8_t-strings.")
    {
        constexpr auto* patter = u8"";
        constexpr auto* alternativeMatch = u8" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts with u8\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts not with u8\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char16_t-strings.")
    {
        constexpr auto* patter = u"";
        constexpr auto* alternativeMatch = u" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts with u\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts not with u\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char32_t-strings.")
    {
        constexpr auto* patter = U"";
        constexpr auto* alternativeMatch = U" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts with U\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts not with U\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(!matcher.matches(alternativeMatch));
        }
    }
}

namespace
{
    void generic_str_starts_with_case_insensitive_test(
        auto&& pattern,
        std::string const& descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            auto const matcher = matches::str::starts_with(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts with " + descriptionPartExpectation));

            SECTION("When pattern is prefix of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                CHECK(matcher.matches(match));
            }

            SECTION("When pattern is not prefix of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                CHECK(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            auto const invertedMatcher = !matches::str::starts_with(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                invertedMatcher.describe(),
                Catch::Matchers::Equals("case-insensitively starts not with " + descriptionPartExpectation));

            SECTION("When pattern is prefix of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                CHECK(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is not prefix of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                CHECK(invertedMatcher.matches(mismatch));
            }
        }
    }
}

// NOLINTBEGIN(*-avoid-c-arrays)

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given wchar_t-string pattern is case-insensitively prefix of target.",
    "[matcher][matcher::str]",
    wchar_t const*,
    wchar_t const (&)[8],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<wchar_t const*>(
        {L"Hello, ",
         L"hello, ",
         L"hElLO, ",
         L"hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<wchar_t const*>(
        {L" Hello, ",
         L"Hello,",
         L"ello, "});

    constexpr wchar_t pattern[] = L"Hello, ";
    generic_str_starts_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "L\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given char8_t-string pattern is case-insensitively prefix of target.",
    "[matcher][matcher::str]",
    char8_t const*,
    char8_t const (&)[8],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<char8_t const*>(
        {u8"Hello, ",
         u8"hello, ",
         u8"hElLO, ",
         u8"hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<char8_t const*>(
        {u8" Hello, ",
         u8"Hello,",
         u8"ello, "});

    constexpr char8_t pattern[] = u8"Hello, ";
    generic_str_starts_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "u8\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given char16_t-string pattern is case-insensitively prefix of target.",
    "[matcher][matcher::str]",
    char16_t const*,
    char16_t const (&)[8],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<char16_t const*>(
        {u"Hello, ",
         u"hello, ",
         u"hElLO, ",
         u"hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<char16_t const*>(
        {u" Hello, ",
         u"Hello,",
         u"ello, "});

    constexpr char16_t pattern[] = u"Hello, ";
    generic_str_starts_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "u\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given char32_t-string pattern is case-insensitively prefix of target.",
    "[matcher][matcher::str]",
    char32_t const*,
    char32_t const (&)[8],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<char32_t const*>(
        {U"Hello, ",
         U"hello, ",
         U"hElLO, ",
         U"hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<char32_t const*>(
        {U" Hello, ",
         U"Hello,",
         U"ello, "});

    constexpr char32_t pattern[] = U"Hello, ";
    generic_str_starts_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "U\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20\"",
        matches,
        mismatches);
}

// NOLINTEND(*-avoid-c-arrays)
