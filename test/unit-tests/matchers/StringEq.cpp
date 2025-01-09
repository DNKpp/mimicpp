//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/StringMatchers.hpp"

#include <array>

namespace matches = mimicpp::matches;
namespace Matches = Catch::Matchers;

TEST_CASE(
    "matches::str::eq supports empty strings.",
    "[matcher][matcher::str]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* patter = "";
        constexpr auto* mismatch = " ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is equal to \"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to \"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

    SECTION("For wchar_t-strings.")
    {
        constexpr auto* patter = L"";
        constexpr auto* mismatch = L" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is equal to L\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to L\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

    SECTION("For char8_t-strings.")
    {
        constexpr auto* patter = u8"";
        constexpr auto* mismatch = u8" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is equal to u8\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to u8\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

    SECTION("For char16_t-strings.")
    {
        constexpr auto* patter = u"";
        constexpr auto* mismatch = u" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is equal to u\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to u\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

    SECTION("For char32_t-strings.")
    {
        constexpr auto* patter = U"";
        constexpr auto* mismatch = U" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is equal to U\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to U\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }
}

namespace
{
    template <typename Source>
    void generic_str_eq_test(
        Source&& pattern,
        std::string descriptionPartExpectation,
        auto match,
        auto mismatch)
    {
        const auto matcher = matches::str::eq(pattern);

        REQUIRE_THAT(
            matcher.describe(),
            Matches::Equals("is equal to " + descriptionPartExpectation));

        SECTION("When target is equal, they match.")
        {
            REQUIRE(matcher.matches(match));
        }

        SECTION("When target is not equal, they do not match.")
        {
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("Matcher can be inverted.")
        {
            const auto invertedMatcher = !matches::str::eq(pattern);

            REQUIRE_THAT(
                invertedMatcher.describe(),
                Matches::Equals("is not equal to " + descriptionPartExpectation));

            SECTION("When target is equal, they do not match.")
            {
                REQUIRE(!invertedMatcher.matches(match));
            }

            SECTION("When target is not equal, they do match.")
            {
                REQUIRE(invertedMatcher.matches(mismatch));
            }
        }
    }
}

TEMPLATE_TEST_CASE(
    "matches::str::eq matches when target char-string compares equal to the stored one.",
    "[matcher][matcher::str]",
    const char*,
    const char (&)[14],
    std::string,
    std::string_view)
{
    constexpr char pattern[] = "Hello, World!";
    generic_str_eq_test(
        static_cast<TestType>(pattern),
        "\"Hello, World!\"",
        std::string{"Hello, World!"},
        std::string{"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
    "matches::str::eq matches when target wchar_t-string compares equal to the stored one.",
    "[matcher][matcher::str]",
    const wchar_t*,
    const wchar_t (&)[14],
    std::wstring,
    std::wstring_view)
{
    constexpr wchar_t pattern[] = L"Hello, World!";
    generic_str_eq_test(
        static_cast<TestType>(pattern),
        "L\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        std::wstring{L"Hello, World!"},
        std::wstring{L"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
    "matches::str::eq matches when target char8_t-string compares equal to the stored one.",
    "[matcher][matcher::str]",
    const char8_t*,
    const char8_t (&)[14],
    std::u8string,
    std::u8string_view)
{
    constexpr char8_t pattern[] = u8"Hello, World!";
    generic_str_eq_test<TestType>(
        static_cast<TestType>(pattern),
        "u8\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        std::u8string{u8"Hello, World!"},
        std::u8string{u8"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
    "matches::str::eq matches when target char16_t-string compares equal to the stored one.",
    "[matcher][matcher::str]",
    const char16_t*,
    const char16_t (&)[14],
    std::u16string,
    std::u16string_view)
{
    constexpr char16_t pattern[] = u"Hello, World!";
    generic_str_eq_test<TestType>(
        static_cast<TestType>(pattern),
        "u\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        std::u16string{u"Hello, World!"},
        std::u16string{u"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
    "matches::str::eq matches when target char32_t-string compares equal to the stored one.",
    "[matcher][matcher::str]",
    const char32_t*,
    const char32_t (&)[14],
    std::u32string,
    std::u32string_view)
{
    constexpr char32_t pattern[] = U"Hello, World!";
    generic_str_eq_test<TestType>(
        static_cast<TestType>(pattern),
        "U\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        std::u32string{U"Hello, World!"},
        std::u32string{U"Hello, WOrld!"});
}

TEST_CASE(
    "matches::str::eq case-insensitive overload supports empty strings.",
    "[matcher][matcher::str]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* patter = "";
        constexpr auto* mismatch = " ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively equal to \"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively not equal to \"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER

    SECTION("For wchar_t-strings.")
    {
        constexpr auto* patter = L"";
        constexpr auto* mismatch = L" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively equal to L\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively not equal to L\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

    SECTION("For char8_t-strings.")
    {
        constexpr auto* patter = u8"";
        constexpr auto* mismatch = u8" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively equal to u8\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively not equal to u8\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

    SECTION("For char16_t-strings.")
    {
        constexpr auto* patter = u"";
        constexpr auto* mismatch = u" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively equal to u\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively not equal to u\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

    SECTION("For char32_t-strings.")
    {
        constexpr auto* patter = U"";
        constexpr auto* mismatch = U" ";

        SECTION("For plain matchers.")
        {
            const auto matcher = matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is equal to U\"\""));
            REQUIRE(matcher.matches(patter));
            REQUIRE(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            const auto matcher = !matches::str::eq(patter);
            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to U\"\""));
            REQUIRE(!matcher.matches(patter));
            REQUIRE(matcher.matches(mismatch));
        }
    }

#endif
}

namespace
{
    void generic_str_eq_no_case_test(
        auto&& pattern,
        std::string descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("Plain matcher.")
        {
            const auto matcher = matches::str::eq(pattern, mimicpp::case_insensitive);

            REQUIRE_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively equal to " + descriptionPartExpectation));

            SECTION("When target is equal, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(matcher.matches(match));
            }

            SECTION("When target is not equal, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            const auto invertedMatcher = !matches::str::eq(pattern, mimicpp::case_insensitive);

            REQUIRE_THAT(
                invertedMatcher.describe(),
                Matches::Equals("is case-insensitively not equal to " + descriptionPartExpectation));

            SECTION("When target is equal, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                REQUIRE(!invertedMatcher.matches(match));
            }

            SECTION("When target is not equal, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                REQUIRE(invertedMatcher.matches(mismatch));
            }
        }
    }
}

TEMPLATE_TEST_CASE(
    "matches::str::eq supports case-insensitive comparison for char-strings.",
    "[matcher][matcher::str]",
    const char*,
    const char (&)[14],
    std::string,
    std::string_view)
{
    static constexpr std::array matches = std::to_array<const char*>(
        {"Hello, World!",
         "hello, World!",
         "Hello, world!",
         "HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<const char*>(
        {" hello, world!",
         "hello, world! ",
         "hello,world!"});

    constexpr char pattern[] = "Hello, World!";
    generic_str_eq_no_case_test(
        static_cast<TestType>(pattern),
        "\"Hello, World!\"",
        matches,
        mismatches);
}

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER

TEMPLATE_TEST_CASE(
    "matches::str::eq supports case-insensitive comparison for wchar_t-strings.",
    "[matcher][matcher::str]",
    const wchar_t*,
    const wchar_t (&)[14],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<const wchar_t*>(
        {L"Hello, World!",
         L"hello, World!",
         L"Hello, world!",
         L"HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<const wchar_t*>(
        {L" hello, world!",
         L"hello, world! ",
         L"hello,world!"});

    constexpr wchar_t pattern[] = L"Hello, World!";
    generic_str_eq_no_case_test(
        static_cast<TestType>(pattern),
        "L\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::eq supports case-insensitive comparison for char8_t-strings.",
    "[matcher][matcher::str]",
    const char8_t*,
    const char8_t (&)[14],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<const char8_t*>(
        {u8"Hello, World!",
         u8"hello, World!",
         u8"Hello, world!",
         u8"HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<const char8_t*>(
        {u8" hello, world!",
         u8"hello, world! ",
         u8"hello,world!"});

    constexpr char8_t pattern[] = u8"Hello, World!";
    generic_str_eq_no_case_test(
        static_cast<TestType>(pattern),
        "u8\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::eq supports case-insensitive comparison for char16_t-strings.",
    "[matcher][matcher::str]",
    const char16_t*,
    const char16_t (&)[14],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<const char16_t*>(
        {u"Hello, World!",
         u"hello, World!",
         u"Hello, world!",
         u"HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<const char16_t*>(
        {u" hello, world!",
         u"hello, world! ",
         u"hello,world!"});

    constexpr char16_t pattern[] = u"Hello, World!";
    generic_str_eq_no_case_test(
        static_cast<TestType>(pattern),
        "u\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

TEMPLATE_TEST_CASE(
    "matches::str::eq supports case-insensitive comparison for char32_t-strings.",
    "[matcher][matcher::str]",
    const char32_t*,
    const char32_t (&)[14],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<const char32_t*>(
        {U"Hello, World!",
         U"hello, World!",
         U"Hello, world!",
         U"HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<const char32_t*>(
        {U" hello, world!",
         U"hello, world! ",
         U"hello,world!"});

    constexpr char32_t pattern[] = U"Hello, World!";
    generic_str_eq_no_case_test(
        static_cast<TestType>(pattern),
        "U\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
        matches,
        mismatches);
}

#endif
