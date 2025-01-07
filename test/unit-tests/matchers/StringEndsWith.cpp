//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Matcher.hpp"

#include <array>

namespace matches = mimicpp::matches;
namespace Matches = Catch::Matchers;

TEST_CASE(
    "matches::str::ends_with supports empty strings.",
    "[matcher][matcher::str]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* patter = "";
        constexpr auto* alternativeMatch = " ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends with \"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends not with \"\""));
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
            const auto matcher = matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends with L\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends not with L\"\""));
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
            const auto matcher = matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends with u8\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends not with u8\"\""));
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
            const auto matcher = matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends with u\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends not with u\"\""));
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
            const auto matcher = matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends with U\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends not with U\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }
}

namespace
{
    void generic_str_ends_with_test(
        auto&& pattern,
        std::string descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            const auto matcher = matches::str::ends_with(pattern);

            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("ends with " + descriptionPartExpectation));

            SECTION("When pattern is postfix of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(matcher.matches(match));
            }

            SECTION("When pattern is not postfix of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            const auto invertedMatcher = !matches::str::ends_with(pattern);

            REQUIRE_THAT(
                invertedMatcher.describe(),
                Matches::Equals("ends not with " + descriptionPartExpectation));

            SECTION("When pattern is postfix of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is not postfix of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(invertedMatcher.matches(mismatch));
            }
        }
    }
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given char-string pattern is postfix of target.",
    "[matcher][matcher::str]",
    const char*,
    const char (&)[9],
    std::string,
    std::string_view)
{
    static constexpr std::array matches = std::to_array<const char*>(
        {", World!",
         "Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char*>(
        {", World! ",
         " World!",
         ", worlD!",
         ", World"});

    constexpr char pattern[] = ", World!";
    generic_str_ends_with_test(
        static_cast<TestType>(pattern),
        "\", World!\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given wchar_t-string pattern is postfix of target.",
    "[matcher][matcher::str]",
    const wchar_t*,
    const wchar_t (&)[9],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<const wchar_t*>(
        {L", World!",
         L"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const wchar_t*>(
        {L", World! ",
         L" World!",
         L", worlD!",
         L", World"});

    constexpr wchar_t pattern[] = L", World!";
    generic_str_ends_with_test(
        static_cast<TestType>(pattern),
        "L\"0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given char8_t-string pattern is postfix of target.",
    "[matcher][matcher::str]",
    const char8_t*,
    const char8_t (&)[9],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<const char8_t*>(
        {u8", World!",
         u8"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char8_t*>(
        {u8", World! ",
         u8" World!",
         u8", worlD!",
         u8", World"});

    constexpr char8_t pattern[] = u8", World!";
    generic_str_ends_with_test(
        static_cast<TestType>(pattern),
        "u8\"0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given char16_t-string pattern is postfix of target.",
    "[matcher][matcher::str]",
    const char16_t*,
    const char16_t (&)[9],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<const char16_t*>(
        {u", World!",
         u"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char16_t*>(
        {u", World! ",
         u" World!",
         u", worlD!",
         u", World"});

    constexpr char16_t pattern[] = u", World!";
    generic_str_ends_with_test(
        static_cast<TestType>(pattern),
        "u\"0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given char32_t-string pattern is postfix of target.",
    "[matcher][matcher::str]",
    const char32_t*,
    const char32_t (&)[9],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<const char32_t*>(
        {U", World!",
         U"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char32_t*>(
        {U", World! ",
         U" World!",
         U", worlD!",
         U", World"});

    constexpr char32_t pattern[] = U", World!";
    generic_str_ends_with_test(
        static_cast<TestType>(pattern),
        "U\"0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

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
            const auto matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends with \"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends not with \"\""));
            REQUIRE(!matcher.matches(patter));
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
            const auto matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends with L\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends not with L\"\""));
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
            const auto matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends with u8\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends not with u8\"\""));
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
            const auto matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends with u\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends not with u\"\""));
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
            const auto matcher = matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends with U\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::ends_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends not with U\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

#endif
}

namespace
{
    void generic_str_ends_with_case_insensitive_test(
        auto&& pattern,
        std::string descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            const auto matcher = matches::str::ends_with(pattern, mimicpp::case_insensitive);

            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively ends with " + descriptionPartExpectation));

            SECTION("When pattern is postfix of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(matcher.matches(match));
            }

            SECTION("When pattern is not postfix of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            const auto invertedMatcher = !matches::str::ends_with(pattern, mimicpp::case_insensitive);

            REQUIRE_THAT(
                invertedMatcher.describe(),
                Matches::Equals("case-insensitively ends not with " + descriptionPartExpectation));

            SECTION("When pattern is postfix of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is not postfix of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(invertedMatcher.matches(mismatch));
            }
        }
    }
}

TEMPLATE_TEST_CASE(
    "matches::str::ends_with matches when given char-string pattern is case-insensitively postfix of target.",
    "[matcher][matcher::str]",
    const char*,
    const char (&)[9],
    std::string,
    std::string_view)
{
    static constexpr std::array matches = std::to_array<const char*>(
        {", World!",
         ", WorlD!",
         "Hello, World!"
         "Hello, WoRlD!"});

    static constexpr std::array mismatches = std::to_array<const char*>(
        {", World! ",
         " World!",
         ", World"});

    constexpr char pattern[] = ", World!";
    generic_str_ends_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "\", World!\"",
        matches,
        mismatches);
}

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER

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

#endif
