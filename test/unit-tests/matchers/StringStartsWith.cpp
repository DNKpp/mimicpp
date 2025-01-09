//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/StringMatchers.hpp"

#include <array>

namespace matches = mimicpp::matches;
namespace Matches = Catch::Matchers;

TEST_CASE(
    "matches::str::starts_with supports empty strings.",
    "[matcher][matcher::str]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* patter = "";
        constexpr auto* alternativeMatch = " ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts with \"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts not with \"\""));
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
            const auto matcher = matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts with L\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts not with L\"\""));
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
            const auto matcher = matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts with u8\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts not with u8\"\""));
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
            const auto matcher = matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts with u\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts not with u\"\""));
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
            const auto matcher = matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts with U\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts not with U\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }
}

namespace
{
    void generic_str_starts_with_test(
        auto&& pattern,
        std::string descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            const auto matcher = matches::str::starts_with(pattern);

            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("starts with " + descriptionPartExpectation));

            SECTION("When pattern is prefix of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(matcher.matches(match));
            }

            SECTION("When pattern is not prefix of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            const auto invertedMatcher = !matches::str::starts_with(pattern);

            REQUIRE_THAT(
                invertedMatcher.describe(),
                Matches::Equals("starts not with " + descriptionPartExpectation));

            SECTION("When pattern is prefix of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is not prefix of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(invertedMatcher.matches(mismatch));
            }
        }
    }
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given char-string pattern is prefix of target.",
    "[matcher][matcher::str]",
    const char*,
    const char (&)[8],
    std::string,
    std::string_view)
{
    static constexpr std::array matches = std::to_array<const char*>(
        {"Hello, ",
         "Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char*>(
        {" Hello, ",
         "Hello,",
         "hello, ",
         "ello, "});

    constexpr char pattern[] = "Hello, ";
    generic_str_starts_with_test(
        static_cast<TestType>(pattern),
        "\"Hello, \"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given wchar_t-string pattern is prefix of target.",
    "[matcher][matcher::str]",
    const wchar_t*,
    const wchar_t (&)[8],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<const wchar_t*>(
        {L"Hello, ",
         L"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const wchar_t*>(
        {L" Hello, ",
         L"Hello,",
         L"hello, ",
         L"ello, "});

    constexpr wchar_t pattern[] = L"Hello, ";
    generic_str_starts_with_test(
        static_cast<TestType>(pattern),
        "L\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given char8_t-string pattern is prefix of target.",
    "[matcher][matcher::str]",
    const char8_t*,
    const char8_t (&)[8],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<const char8_t*>(
        {u8"Hello, ",
         u8"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char8_t*>(
        {u8" Hello, ",
         u8"Hello,",
         u8"hello, ",
         u8"ello, "});

    constexpr char8_t pattern[] = u8"Hello, ";
    generic_str_starts_with_test(
        static_cast<TestType>(pattern),
        "u8\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given char16_t-string pattern is prefix of target.",
    "[matcher][matcher::str]",
    const char16_t*,
    const char16_t (&)[8],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<const char16_t*>(
        {u"Hello, ",
         u"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char16_t*>(
        {u" Hello, ",
         u"Hello,",
         u"hello, ",
         u"ello, "});

    constexpr char16_t pattern[] = u"Hello, ";
    generic_str_starts_with_test(
        static_cast<TestType>(pattern),
        "u\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given char32_t-string pattern is prefix of target.",
    "[matcher][matcher::str]",
    const char32_t*,
    const char32_t (&)[8],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<const char32_t*>(
        {U"Hello, ",
         U"Hello, World!"});

    static constexpr std::array mismatches = std::to_array<const char32_t*>(
        {U" Hello, ",
         U"Hello,",
         U"hello, ",
         U"ello, "});

    constexpr char32_t pattern[] = U"Hello, ";
    generic_str_starts_with_test(
        static_cast<TestType>(pattern),
        "U\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20\"",
        matches,
        mismatches);
}

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
            const auto matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts with \"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts not with \"\""));
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
            const auto matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts with L\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts not with L\"\""));
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
            const auto matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts with u8\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts not with u8\"\""));
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
            const auto matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts with u\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts not with u\"\""));
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
            const auto matcher = matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts with U\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(matcher.matches(alternativeMatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::starts_with(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts not with U\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(!matcher.matches(alternativeMatch));
        }
    }

#endif
}

namespace
{
    void generic_str_starts_with_case_insensitive_test(
        auto&& pattern,
        std::string descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("When plain matcher is used.")
        {
            const auto matcher = matches::str::starts_with(pattern, mimicpp::case_insensitive);

            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("case-insensitively starts with " + descriptionPartExpectation));

            SECTION("When pattern is prefix of target, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(matcher.matches(match));
            }

            SECTION("When pattern is not prefix of target, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            const auto invertedMatcher = !matches::str::starts_with(pattern, mimicpp::case_insensitive);

            REQUIRE_THAT(
                invertedMatcher.describe(),
                Matches::Equals("case-insensitively starts not with " + descriptionPartExpectation));

            SECTION("When pattern is prefix of target, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(!invertedMatcher.matches(match));
            }

            SECTION("When pattern is not prefix of target, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(invertedMatcher.matches(mismatch));
            }
        }
    }
}

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given char-string pattern is case-insensitively prefix of target.",
    "[matcher][matcher::str]",
    const char*,
    const char (&)[8],
    std::string,
    std::string_view)
{
    static constexpr std::array matches = std::to_array<const char*>(
        {"Hello, ",
         "hello, ",
         "hElLO, ",
         "hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<const char*>(
        {" Hello, ",
         "Hello,",
         "ello, "});

    constexpr char pattern[] = "Hello, ";
    generic_str_starts_with_case_insensitive_test(
        static_cast<TestType>(pattern),
        "\"Hello, \"",
        matches,
        mismatches);
}

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER

TEMPLATE_TEST_CASE(
    "matches::str::starts_with matches when given wchar_t-string pattern is case-insensitively prefix of target.",
    "[matcher][matcher::str]",
    const wchar_t*,
    const wchar_t (&)[8],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<const wchar_t*>(
        {L"Hello, ",
         L"hello, ",
         L"hElLO, ",
         L"hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<const wchar_t*>(
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
    const char8_t*,
    const char8_t (&)[8],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<const char8_t*>(
        {u8"Hello, ",
         u8"hello, ",
         u8"hElLO, ",
         u8"hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<const char8_t*>(
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
    const char16_t*,
    const char16_t (&)[8],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<const char16_t*>(
        {u"Hello, ",
         u"hello, ",
         u"hElLO, ",
         u"hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<const char16_t*>(
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
    const char32_t*,
    const char32_t (&)[8],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<const char32_t*>(
        {U"Hello, ",
         U"hello, ",
         U"hElLO, ",
         U"hElLO, World!"});

    static constexpr std::array mismatches = std::to_array<const char32_t*>(
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

#endif
