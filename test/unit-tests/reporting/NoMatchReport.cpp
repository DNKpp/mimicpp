//          Copyright Dominic (DNKpp) Koepke 2024 - 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/MatchReport.hpp"

using namespace mimicpp;

TEST_CASE(
    "reporting::MatchReport is equality-comparable.",
    "[reporting]")
{
    reporting::MatchReport const report{
        .expectationReport = {
                              .from = {},
                              .target = {.name = "Test", .overloadReport = reporting::TypeReport::make<void()>()}},
        .matchResults = {expectation::MatchSuccess{}},
    };

    SECTION("Compares equal, when both sides are equal.")
    {
        reporting::MatchReport const other = report;

        REQUIRE(other == report);
        REQUIRE(report == other);
        REQUIRE_FALSE(other != report);
        REQUIRE_FALSE(report != other);
    }

    SECTION("Compares unequal, when expectation reports differ.")
    {
        reporting::MatchReport other{report};
        other.expectationReport.from = {};

        REQUIRE_FALSE(other == report);
        REQUIRE_FALSE(report == other);
        REQUIRE(other != report);
        REQUIRE(report != other);
    }

    SECTION("Compares unequal, when outcomes differ.")
    {
        reporting::MatchReport other{report};
        other.matchResults = {expectation::MatchFailure{}};

        REQUIRE_FALSE(other == report);
        REQUIRE_FALSE(report == other);
        REQUIRE(other != report);
        REQUIRE(report != other);
    }
}
