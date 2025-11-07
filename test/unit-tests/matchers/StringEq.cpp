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
            auto const matcher = matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is equal to \"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to \"\""));
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
            auto const matcher = matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is equal to L\"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to L\"\""));
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
            auto const matcher = matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is equal to u8\"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to u8\"\""));
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
            auto const matcher = matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is equal to u\"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to u\"\""));
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
                Matches::Equals("is equal to U\"\""));
            CHECK(matcher.matches(patter));
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("For inverted matchers.")
        {
            auto const matcher = !matches::str::eq(patter);
            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is not equal to U\"\""));
            CHECK(!matcher.matches(patter));
            CHECK(matcher.matches(mismatch));
        }
    }
}

namespace
{
    template <typename Source>
    void generic_str_eq_test(
        Source&& pattern,
        std::string const& descriptionPartExpectation,
        auto match,
        auto mismatch)
    {
        auto const matcher = matches::str::eq(pattern);

        CHECK_THAT(
            matcher.describe(),
            Matches::Equals("is equal to " + descriptionPartExpectation));

        SECTION("When target is equal, they match.")
        {
            CHECK(matcher.matches(match));
        }

        SECTION("When target is not equal, they do not match.")
        {
            CHECK(!matcher.matches(mismatch));
        }

        SECTION("Matcher can be inverted.")
        {
            auto const invertedMatcher = !matches::str::eq(pattern);

            CHECK_THAT(
                invertedMatcher.describe(),
                Matches::Equals("is not equal to " + descriptionPartExpectation));

            SECTION("When target is equal, they do not match.")
            {
                CHECK(!invertedMatcher.matches(match));
            }

            SECTION("When target is not equal, they do match.")
            {
                CHECK(invertedMatcher.matches(mismatch));
            }
        }
    }
}

// NOLINTBEGIN(*-avoid-c-arrays)

TEMPLATE_TEST_CASE(
    "matches::str::eq matches when target char-string compares equal to the stored one.",
    "[matcher][matcher::str]",
    char const*,
    char const (&)[14],
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
    wchar_t const*,
    wchar_t const (&)[14],
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
    char8_t const*,
    char8_t const (&)[14],
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
    char16_t const*,
    char16_t const (&)[14],
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
    char32_t const*,
    char32_t const (&)[14],
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

// NOLINTEND(*-avoid-c-arrays)

TEST_CASE(
    "matches::str::eq case-insensitive overload supports empty char-strings.",
    "[matcher][matcher::str]")
{
    constexpr auto* patter = "";
    constexpr auto* mismatch = " ";

    SECTION("For plain matchers.")
    {
        auto const matcher = matches::str::eq(patter, mimicpp::case_insensitive);
        CHECK_THAT(
            matcher.describe(),
            Matches::Equals("is case-insensitively equal to \"\""));
        CHECK(matcher.matches(patter));
        CHECK(!matcher.matches(mismatch));
    }

    SECTION("For inverted matchers.")
    {
        auto const matcher = !matches::str::eq(patter, mimicpp::case_insensitive);
        CHECK_THAT(
            matcher.describe(),
            Matches::Equals("is case-insensitively not equal to \"\""));
        CHECK(!matcher.matches(patter));
        CHECK(matcher.matches(mismatch));
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
            const auto matcher = matches::str::eq(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                matcher.describe(),
                Matches::Equals("is case-insensitively equal to " + descriptionPartExpectation));

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
            const auto invertedMatcher = !matches::str::eq(pattern, mimicpp::case_insensitive);

            CHECK_THAT(
                invertedMatcher.describe(),
                Matches::Equals("is case-insensitively not equal to " + descriptionPartExpectation));

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
    "matches::str::eq supports case-insensitive comparison for char-strings.",
    "[matcher][matcher::str]",
    char const*,
    char const (&)[14],
    std::string,
    std::string_view)
{
    static constexpr std::array matches = std::to_array<char const*>(
        {"Hello, World!",
         "hello, World!",
         "Hello, world!",
         "HelLo, world!"});

    static constexpr std::array mismatches = std::to_array<char const*>(
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

// NOLINTEND(*-avoid-c-arrays)
