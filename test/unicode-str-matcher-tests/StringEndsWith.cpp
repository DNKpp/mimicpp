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
    "matches::str::ends_with case-insensitively overload supports empty strings.",
    "[matcher][matcher::str]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* patter = "";
        constexpr auto* alternativeMatch = " ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends with \"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends not with \"\""));
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
            auto const matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends with L\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends not with L\"\""));
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
            auto const matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends with u8\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends not with u8\"\""));
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
            auto const matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends with u\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends not with u\"\""));
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
            auto const matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends with U\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends not with U\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(!matcher.matches(alternativeMatch));
        }
    }
}

namespace
{
    void generic_str_ends_with_case_insensitive_test(
        auto&& pattern,
        std::string const& descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            auto const matcher = matches::str::ends_with(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends with " + descriptionPartExpectation));

            SECTION("When pattern is postfix of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                CHECK(matcher.matches(match));
            }

            SECTION("When pattern is not postfix of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                CHECK(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            auto const invertedMatcher = !matches::str::ends_with(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                invertedMatcher.describe(),
                Catch::Matchers::Equals("case-insensitively ends not with " + descriptionPartExpectation));

            SECTION("When pattern is postfix of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                CHECK(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is not postfix of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                CHECK(invertedMatcher.matches(mismatch));
            }
        }
    }
}

// NOLINTBEGIN(*-avoid-c-arrays)

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given wchar_t-string pattern is case-insensitively postfix of target.",
    "[matcher][matcher::str]",
    const wchar_t*,
    const wchar_t (&)[9],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<const wchar_t*>(
        {L", World!",
         L", WorlD!",
         L"Hello, World!"
         L"Hello, WoRlD!"});

    static constexpr std::array mismatches = std::to_array<const wchar_t*>(
        {L", World! ",
         L" World!",
         L", World"});

    constexpr wchar_t pattern[] = L", World!";
    generic_str_ends_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "L\"0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given char8_t-string pattern is case-insensitively postfix of target.",
    "[matcher][matcher::str]",
    const char8_t*,
    const char8_t (&)[9],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<const char8_t*>(
        {u8", World!",
         u8", WorlD!",
         u8"Hello, World!"
         u8"Hello, WoRlD!"});

    static constexpr std::array mismatches = std::to_array<const char8_t*>(
        {u8", World! ",
         u8" World!",
         u8", World"});

    constexpr char8_t pattern[] = u8", World!";
    generic_str_ends_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "u8\"0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given char16_t-string pattern is case-insensitively postfix of target.",
    "[matcher][matcher::str]",
    const char16_t*,
    const char16_t (&)[9],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<const char16_t*>(
        {u", World!",
         u", WorlD!",
         u"Hello, World!"
         u"Hello, WoRlD!"});

    static constexpr std::array mismatches = std::to_array<const char16_t*>(
        {u", World! ",
         u" World!",
         u", World"});

    constexpr char16_t pattern[] = u", World!";
    generic_str_ends_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "u\"0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given char32_t-string pattern is case-insensitively postfix of target.",
    "[matcher][matcher::str]",
    const char32_t*,
    const char32_t (&)[9],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<const char32_t*>(
        {U", World!",
         U", WorlD!",
         U"Hello, World!"
         U"Hello, WoRlD!"});

    static constexpr std::array mismatches = std::to_array<const char32_t*>(
        {U", World! ",
         U" World!",
         U", World"});

    constexpr char32_t pattern[] = U", World!";
    generic_str_ends_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "U\"0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

// NOLINTEND(*-avoid-c-arrays)
