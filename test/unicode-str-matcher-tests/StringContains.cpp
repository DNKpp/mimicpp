//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/StringMatchers.hpp"

namespace matches = mimicpp::matches;

TEST_CASE(
    "matches::str::contains case-insensitive overload supports empty strings.",
    "[matcher][matcher::str]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* pattern = "";
        constexpr auto* alternativeMatch = " ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::contains(pattern, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains \"\""));
            CHECK(matcher.matches(pattern));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::contains(pattern, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains not \"\""));
            CHECK(!matcher.matches(pattern));
            CHECK(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For wchar_t-strings.")
    {
        constexpr auto* patter = L"";
        constexpr auto* alternativeMatch = L" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::contains(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains L\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::contains(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains not L\"\""));
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
            auto const matcher = matches::str::contains(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains u8\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::contains(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains not u8\"\""));
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
            auto const matcher = matches::str::contains(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains u\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::contains(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains not u\"\""));
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
            auto const matcher = matches::str::contains(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains U\"\""));
            CHECK(matcher.matches(patter));
            CHECK(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::contains(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains not U\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(!matcher.matches(alternativeMatch));
        }
    }
}

namespace
{
    void generic_str_contains_case_insensitive_test(
        auto&& pattern,
        std::string const& descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            auto const matcher = matches::str::contains(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains " + descriptionPartExpectation));

            SECTION("When pattern is part of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                CHECK(matcher.matches(match));
            }

            SECTION("When pattern is part of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                CHECK(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            auto const invertedMatcher = !matches::str::contains(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                invertedMatcher.describe(),
                Catch::Matchers::Equals("case-insensitively contains not " + descriptionPartExpectation));

            SECTION("When pattern is part of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                CHECK(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is part of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                CHECK(invertedMatcher.matches(mismatch));
            }
        }
    }
}

// NOLINTBEGIN(*-avoid-c-arrays)

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given char-string pattern is case-insensitively part of target.",
    "[matcher][matcher::str]",
    const char*,
    const char (&)[7],
    std::string,
    std::string_view)
{
    static constexpr std::array matches = std::to_array<const char*>(
        {"lo, Wo",
         "Lo, Wo",
         "lo, wo",
         "HelLO, wOrld!"});

    static constexpr std::array mismatches = std::to_array<const char*>(
        {"Hello, W",
         "o, World!"});

    constexpr char pattern[] = "lo, Wo";
    generic_str_contains_case_insensitive_test(
        static_cast<TestType>(pattern),
        "\"lo, Wo\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given wchar_t-string pattern is case-insensitively part of target.",
    "[matcher][matcher::str]",
    wchar_t const*,
    wchar_t const (&)[7],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<const wchar_t*>(
        {L"lo, Wo",
         L"Lo, Wo",
         L"lo, wo",
         L"HelLO, wOrld!"});

    static constexpr std::array mismatches = std::to_array<const wchar_t*>(
        {L"Hello, W",
         L"o, World!"});

    constexpr wchar_t pattern[] = L"lo, Wo";
    generic_str_contains_case_insensitive_test(
        static_cast<TestType>(pattern),
        "L\"0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given char8_t-string pattern is case-insensitively part of target.",
    "[matcher][matcher::str]",
    char8_t const*,
    char8_t const (&)[7],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<const char8_t*>(
        {u8"lo, Wo",
         u8"Lo, Wo",
         u8"lo, wo",
         u8"HelLO, wOrld!"});

    static constexpr std::array mismatches = std::to_array<const char8_t*>(
        {u8"Hello, W",
         u8"o, World!"});

    constexpr char8_t pattern[] = u8"lo, Wo";
    generic_str_contains_case_insensitive_test(
        static_cast<TestType>(pattern),
        "u8\"0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given char16_t-string pattern is case-insensitively part of target.",
    "[matcher][matcher::str]",
    char16_t const*,
    char16_t const (&)[7],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<const char16_t*>(
        {u"lo, Wo",
         u"Lo, Wo",
         u"lo, wo",
         u"HelLO, wOrld!"});

    static constexpr std::array mismatches = std::to_array<const char16_t*>(
        {u"Hello, W",
         u"o, World!"});

    constexpr char16_t pattern[] = u"lo, Wo";
    generic_str_contains_case_insensitive_test(
        static_cast<TestType>(pattern),
        "u\"0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given char32_t-string pattern is case-insensitively part of target.",
    "[matcher][matcher::str]",
    char32_t const*,
    char32_t const (&)[7],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<const char32_t*>(
        {U"lo, Wo",
         U"Lo, Wo",
         U"lo, wo",
         U"HelLO, wOrld!"});

    static constexpr std::array mismatches = std::to_array<const char32_t*>(
        {U"Hello, W",
         U"o, World!"});

    constexpr char32_t pattern[] = U"lo, Wo";
    generic_str_contains_case_insensitive_test(
        static_cast<TestType>(pattern),
        "U\"0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f\"",
        matches,
        mismatches);
}

// NOLINTEND(*-avoid-c-arrays)
