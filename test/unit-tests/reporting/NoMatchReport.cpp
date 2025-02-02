//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/NoMatchReport.hpp"

using namespace mimicpp;

TEST_CASE(
    "reporting::NoMatchReport is equality-comparable.",
    "[reporting]")
{
    reporting::NoMatchReport const report{
        .expectationReport = {.info = {std::source_location::current(), "Test"}},
        .requirementOutcomes = {{true}}};

    SECTION("Compares equal, when both sides are equal.")
    {
        reporting::NoMatchReport const other = report;

        REQUIRE(other == report);
        REQUIRE(report == other);
        REQUIRE_FALSE(other != report);
        REQUIRE_FALSE(report != other);
    }

    SECTION("Compares unequal, when expectation reports differ.")
    {
        reporting::NoMatchReport other{report};
        other.expectationReport.info = {std::source_location::current(), "Test2"};

        REQUIRE_FALSE(other == report);
        REQUIRE_FALSE(report == other);
        REQUIRE(other != report);
        REQUIRE(report != other);
    }

    SECTION("Compares unequal, when outcomes differ.")
    {
        reporting::NoMatchReport other{report};
        other.requirementOutcomes.outcomes = {false};

        REQUIRE_FALSE(other == report);
        REQUIRE_FALSE(report == other);
        REQUIRE(other != report);
        REQUIRE(report != other);
    }
}
