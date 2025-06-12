//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/SourceLocation.hpp"

using namespace mimicpp;

TEST_CASE(
    "util::SourceLocation is default constructible.",
    "[util]")
{
    constexpr auto before = std::source_location::current();
    constexpr util::SourceLocation loc{};
    constexpr auto after = std::source_location::current();

    CHECK_THAT(
        loc.file_name(),
        Catch::Matchers::Equals(before.file_name()));
    CHECK_THAT(
        loc.function_name(),
        Catch::Matchers::Equals(before.function_name()));
    CHECK(before.line() < loc.line());
    CHECK(loc.line() < after.line());
    CHECK(0 < loc.column());
}

TEST_CASE(
    "util::SourceLocation can be constructed with a particular source-location.",
    "[util]")
{
    constexpr auto source = std::source_location::current();
    constexpr util::SourceLocation loc{source};

    CHECK_THAT(
        loc.file_name(),
        Catch::Matchers::Equals(source.file_name()));
    CHECK_THAT(
        loc.function_name(),
        Catch::Matchers::Equals(source.function_name()));
    CHECK(source.line() == loc.line());
    CHECK(source.column() == loc.column());
}

TEST_CASE(
    "util::SourceLocation is equality-comparable.")
{
    SECTION("Compares equal, when both sides denote the same source-location.")
    {
        constexpr util::SourceLocation loc{};
        constexpr util::SourceLocation other{loc};

        CHECK(loc == other);
        CHECK(other == loc);
        CHECK_FALSE(loc != other);
        CHECK_FALSE(other != loc);
    }

    SECTION("Compares unequal, when line differs.")
    {
        constexpr util::SourceLocation loc{};
        constexpr util::SourceLocation other{};
        REQUIRE(other.line() != loc.line());

        CHECK_FALSE(loc == other);
        CHECK_FALSE(other == loc);
        CHECK(loc != other);
        CHECK(other != loc);
    }

    SECTION("Compares unequal, when column differs.")
    {
        constexpr util::SourceLocation loc{}, other{};
        REQUIRE(other.line() == loc.line());
        REQUIRE(other.column() != loc.column());

        CHECK_FALSE(loc == other);
        CHECK_FALSE(other == loc);
        CHECK(loc != other);
        CHECK(other != loc);
    }

    SECTION("Compares unequal, when function differs.")
    {
        constexpr util::SourceLocation loc{};
        constexpr auto other = std::invoke(
            [] {
                return std::source_location::current();
            });

        CHECK_FALSE(loc == other);
        CHECK_FALSE(other == loc);
        CHECK(loc != other);
        CHECK(other != loc);
    }

    SECTION("Compares unequal, when source-file differs.")
    {
        // trust me, bro
    }
}
