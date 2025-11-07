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
    "matches::str::eq case-insensitive overload supports empty strings.",
    "[matcher][matcher::str][unicode]")
{
    SECTION("For char-strings.")
    {
        constexpr auto* patter = "";
        constexpr auto* mismatch = " ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::eq(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively equal to \"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively not equal to \"\""));
            CHECK(!matcher.matches(patter));
            CHECK(matcher.matches(mismatch));
        }
    }

    SECTION("For wchar_t-strings.")
    {
        constexpr auto* patter = L"";
        constexpr auto* mismatch = L" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::eq(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively equal to L\"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively not equal to L\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(matcher.matches(mismatch));
        }
    }

    SECTION("For char8_t-strings.")
    {
        constexpr auto* patter = u8"";
        constexpr auto* mismatch = u8" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::eq(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively equal to u8\"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively not equal to u8\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(matcher.matches(mismatch));
        }
    }

    SECTION("For char16_t-strings.")
    {
        constexpr auto* patter = u"";
        constexpr auto* mismatch = u" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::eq(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively equal to u\"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively not equal to u\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(matcher.matches(mismatch));
        }
    }

    SECTION("For char32_t-strings.")
    {
        constexpr auto* patter = U"";
        constexpr auto* mismatch = U" ";

        SECTION("For plain matchers.")
        {
            auto const matcher = matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is equal to U\"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is not equal to U\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(matcher.matches(mismatch));
        }
    }
}

namespace
{
    void generic_str_eq_no_case_test(
        auto&& pattern,
        std::string const& descriptionPartExpectation,
        auto&& matches,
        auto&& mismatches)
    {
        SECTION("Plain matcher.")
        {
            auto const matcher = matches::str::eq(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                matcher.describe(),
                Catch::Matchers::Equals("is case-insensitively equal to " + descriptionPartExpectation));

            SECTION("When target is equal, they match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                CHECK(matcher.matches(match));
            }

            SECTION("When target is not equal, they do not match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                CHECK(!matcher.matches(mismatch));
            }
        }

        SECTION("Matcher can be inverted.")
        {
            auto const invertedMatcher = !matches::str::eq(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                invertedMatcher.describe(),
                Catch::Matchers::Equals("is case-insensitively not equal to " + descriptionPartExpectation));

            SECTION("When target is equal, they do not match.")
            {
                auto&& match = GENERATE_REF(from_range(matches));
                CHECK(!invertedMatcher.matches(match));
            }

            SECTION("When target is not equal, they do match.")
            {
                auto&& mismatch = GENERATE_REF(from_range(mismatches));
                CHECK(invertedMatcher.matches(mismatch));
            }
        }
    }
}

// NOLINTBEGIN(*-avoid-c-arrays)

TEMPLATE_TEST_CASE(
    "matches::str::eq supports case-insensitive comparison for wchar_t-strings.",
    "[matcher][matcher::str][unicode]",
    wchar_t const*,
    wchar_t const (&)[14],
    std::wstring,
    std::wstring_view)
{
    static constexpr std::array matches = std::to_array<wchar_t const*>(
        {L"Hello, World!",
         L"hello, World!",
         L"Hello, world!",
         L"HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<wchar_t const*>(
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
    "[matcher][matcher::str][unicode]",
    char8_t const*,
    char8_t const (&)[14],
    std::u8string,
    std::u8string_view)
{
    static constexpr std::array matches = std::to_array<char8_t const*>(
        {u8"Hello, World!",
         u8"hello, World!",
         u8"Hello, world!",
         u8"HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<char8_t const*>(
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
    "[matcher][matcher::str][unicode]",
    char16_t const*,
    char16_t const (&)[14],
    std::u16string,
    std::u16string_view)
{
    static constexpr std::array matches = std::to_array<char16_t const*>(
        {u"Hello, World!",
         u"hello, World!",
         u"Hello, world!",
         u"HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<char16_t const*>(
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
    "[matcher][matcher::str][unicode]",
    char32_t const*,
    char32_t const (&)[14],
    std::u32string,
    std::u32string_view)
{
    static constexpr std::array matches = std::to_array<char32_t const*>(
        {U"Hello, World!",
         U"hello, World!",
         U"Hello, world!",
         U"HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<char32_t const*>(
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

// NOLINTEND(*-avoid-c-arrays)
