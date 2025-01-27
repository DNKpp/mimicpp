//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/StringMatchers.hpp"

#include <array>

namespace matches = mimicpp::matches;
namespace Matches = Catch::Matchers;

TEST_CASE(
    "matches::str::contains supports empty strings.",
    "[matcher][matcher::str]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* patter = "";
        constexpr auto* alternativeMatch = " ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains \"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains not \"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For wchar_t-strings.")
    {
        constexpr auto* patter = L"";
        constexpr auto* alternativeMatch = L" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains L\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains not L\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char8_t-strings.")
    {
        constexpr auto* patter = u8"";
        constexpr auto* alternativeMatch = u8" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains u8\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains not u8\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char16_t-strings.")
    {
        constexpr auto* patter = u"";
        constexpr auto* alternativeMatch = u" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains u\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains not u\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char32_t-strings.")
    {
        constexpr auto* patter = U"";
        constexpr auto* alternativeMatch = U" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains U\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains not U\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }
}

namespace
{
    void generic_str_contains_test(
        auto&& pattern,
        std::string descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            const auto matcher = matches::str::contains(pattern);

            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("contains " + descriptionPartExpectation));

            SECTION("When pattern is part of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(matcher.matches(match));
            }

            SECTION("When pattern is part of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            const auto invertedMatcher = !matches::str::contains(pattern);

            REQUIRE_THAT(
                invertedMatcher.describe(),
                Matches::Equals("contains not " + descriptionPartExpectation));

            SECTION("When pattern is part of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is part of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(invertedMatcher.matches(mismatch));
            }
        }
    }
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given char-string pattern is part of target.",
    "[matcher][matcher::str]",
    const char*,
    const char (&)[7],
    std::string,
    std::string_view)
{
    static constexpr std::array matches = std::to_array<const char*>(
        {"lo, Wo",
         "Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char*>(
        {"Hello, W",
         "o, World!",
         "lo, wo"});

    constexpr char pattern[] = "lo, Wo";
    generic_str_contains_test(
        static_cast<TestType>(pattern),
        "\"lo, Wo\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given wchar_t-string pattern is part of target.",
    "[matcher][matcher::str]",
    const wchar_t*,
    const wchar_t (&)[7],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<const wchar_t*>(
        {L"lo, Wo",
         L"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const wchar_t*>(
        {L"Hello, W",
         L"o, World!",
         L"lo, wo"});

    constexpr wchar_t pattern[] = L"lo, Wo";
    generic_str_contains_test(
        static_cast<TestType>(pattern),
        "L\"0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given char8_t-string pattern is part of target.",
    "[matcher][matcher::str]",
    const char8_t*,
    const char8_t (&)[7],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<const char8_t*>(
        {u8"lo, Wo",
         u8"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char8_t*>(
        {u8"Hello, W",
         u8"o, World!",
         u8"lo, wo"});

    constexpr char8_t pattern[] = u8"lo, Wo";
    generic_str_contains_test(
        static_cast<TestType>(pattern),
        "u8\"0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given char16_t-string pattern is part of target.",
    "[matcher][matcher::str]",
    const char16_t*,
    const char16_t (&)[7],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<const char16_t*>(
        {u"lo, Wo",
         u"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char16_t*>(
        {u"Hello, W",
         u"o, World!",
         u"lo, wo"});

    constexpr char16_t pattern[] = u"lo, Wo";
    generic_str_contains_test(
        static_cast<TestType>(pattern),
        "u\"0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given char32_t-string pattern is part of target.",
    "[matcher][matcher::str]",
    const char32_t*,
    const char32_t (&)[7],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<const char32_t*>(
        {U"lo, Wo",
         U"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char32_t*>(
        {U"Hello, W",
         U"o, World!",
         U"lo, wo"});

    constexpr char32_t pattern[] = U"lo, Wo";
    generic_str_contains_test(
        static_cast<TestType>(pattern),
        "U\"0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f\"",
        matches,
        mismatches);
}

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
            const auto matcher = matches::str::contains(pattern, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains \"\""));
            REQUIRE(matcher.matches(pattern));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(pattern, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains not \"\""));
            REQUIRE(!matcher.matches(pattern));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER

    SECTION("For wchar_t-strings.")
    {
        constexpr auto* patter = L"";
        constexpr auto* alternativeMatch = L" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains L\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains not L\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char8_t-strings.")
    {
        constexpr auto* patter = u8"";
        constexpr auto* alternativeMatch = u8" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains u8\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains not u8\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char16_t-strings.")
    {
        constexpr auto* patter = u"";
        constexpr auto* alternativeMatch = u" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains u\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains not u\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

    SECTION("For char32_t-strings.")
    {
        constexpr auto* patter = U"";
        constexpr auto* alternativeMatch = U" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::contains(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains U\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::contains(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains not U\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

#endif
}

namespace
{
    void generic_str_contains_case_insensitive_test(
        auto&& pattern,
        std::string descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            const auto matcher = matches::str::contains(pattern, mimicpp::case_insensitive);

            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively contains " + descriptionPartExpectation));

            SECTION("When pattern is part of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(matcher.matches(match));
            }

            SECTION("When pattern is part of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            const auto invertedMatcher = !matches::str::contains(pattern, mimicpp::case_insensitive);

            REQUIRE_THAT(
                invertedMatcher.describe(),
                Matches::Equals("case-insensitively contains not " + descriptionPartExpectation));

            SECTION("When pattern is part of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is part of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(invertedMatcher.matches(mismatch));
            }
        }
    }
}

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

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER

TEMPLATE_TEST_CASE(
    "matches::str::contains matches when given wchar_t-string pattern is case-insensitively part of target.",
    "[matcher][matcher::str]",
    const wchar_t*,
    const wchar_t (&)[7],
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
    const char8_t*,
    const char8_t (&)[7],
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
    const char16_t*,
    const char16_t (&)[7],
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
    const char32_t*,
    const char32_t (&)[7],
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

#endif
