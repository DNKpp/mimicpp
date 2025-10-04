//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/StaticString.hpp"

using namespace mimicpp;

#define MAKE_STATIC_STRING(text) util::StaticString{#text};

TEST_CASE(
    "util::StaticString can hold compile-time usable strings.",
    "[util]")
{
    SECTION("Empty string is supported.")
    {
        constexpr util::StaticString text{""};

        CHECK_THAT(
            text,
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            text,
            Catch::Matchers::SizeIs(0u));
        CHECK_THAT(
            text,
            Catch::Matchers::RangeEquals(std::vector<char>{}));
    }

    SECTION("When arbitrary string is given.")
    {
        constexpr util::StaticString text{"Hello, World!"};

        CHECK_THAT(
            (std::string{text.begin(), text.end()}),
            Catch::Matchers::Equals("Hello, World!"));
        CHECK_THAT(
            text,
            !Catch::Matchers::IsEmpty());
        CHECK_THAT(
            text,
            Catch::Matchers::SizeIs(std::ranges::size("Hello, World!") - 1u));
    }

    SECTION("When empty macro arg is converted.")
    {
        constexpr util::StaticString<char, 0u> text = MAKE_STATIC_STRING();

        CHECK_THAT(
            text,
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            text,
            Catch::Matchers::SizeIs(0u));
        CHECK_THAT(
            text,
            Catch::Matchers::RangeEquals(std::vector<char>{}));
    }

    SECTION("When arbitrary macro arg is converted.")
    {
        constexpr util::StaticString text = MAKE_STATIC_STRING(test_input);

        CHECK_THAT(
            (std::string{text.begin(), text.end()}),
            Catch::Matchers::Equals("test_input"));
        CHECK_THAT(
            text,
            !Catch::Matchers::IsEmpty());
        CHECK_THAT(
            text,
            Catch::Matchers::SizeIs(10u));
    }

    SECTION("StaticString is fully usable in compile-time context.")
    {
        STATIC_CHECK(std::ranges::equal(util::StaticString{"Hello, World!"}, std::string_view{"Hello, World!"}));
    }
}
