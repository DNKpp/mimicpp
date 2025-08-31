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
    constexpr util::SourceLocation before{};
    constexpr util::SourceLocation loc{};
    constexpr util::SourceLocation after{};

    CHECK_THAT(
        std::string{loc.file_name()},
        Catch::Matchers::Equals(std::string{before.file_name()}));
    CHECK_THAT(
        std::string{loc.function_name()},
        Catch::Matchers::Equals(std::string{before.function_name()}));
    CHECK(before.line() < loc.line());
    CHECK(loc.line() < after.line());
}

TEST_CASE(
    "util::SourceLocation is equality-comparable.",
    "[util]")
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

    SECTION("Compares unequal, when function differs.")
    {
        constexpr util::SourceLocation loc{};
        constexpr auto other = std::invoke(
            [] { return util::SourceLocation{}; });

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
