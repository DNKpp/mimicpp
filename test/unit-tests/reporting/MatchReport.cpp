//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/MatchReport.hpp"

using namespace mimicpp;
using reporting::control_state_t;
using reporting::MatchReport;
using reporting::state_applicable;
using reporting::state_inapplicable;
using reporting::state_saturated;

namespace
{
    [[nodiscard]]
    detail::expectation_info make_common_expectation_info(const std::source_location& loc = std::source_location::current())
    {
        return detail::expectation_info{
            .sourceLocation = loc,
            .mockName = "Mock-Name"};
    }
}

TEST_CASE(
    "MatchReport::Finalize is equality comparable."
    "[reporting]")
{
    using ReportT = MatchReport::Finalize;

    const ReportT first{
        .description = "Hello, World!"};

    const auto [expectedEquality, second] = GENERATE(
        (table<bool, ReportT>({
            {false,     {"not equal"}},
            {false,    {std::nullopt}},
            { true, {"Hello, World!"}}
    })));

    REQUIRE(expectedEquality == (first == second));
    REQUIRE(expectedEquality == (second == first));
    REQUIRE(expectedEquality == !(first != second));
    REQUIRE(expectedEquality == !(second != first));
}

TEST_CASE(
    "MatchReport::Expectation is equality comparable."
    "[reporting]")
{
    using ReportT = MatchReport::Expectation;

    const ReportT first{
        .isMatching = true,
        .description = "Hello, World!"};

    const auto [expectedEquality, second] = GENERATE(
        (table<bool, ReportT>({
            {false,      {true, "not equal"}},
            {false,     {true, std::nullopt}},
            {false, {false, "Hello, World!"}},
            { true,  {true, "Hello, World!"}}
    })));

    REQUIRE(expectedEquality == (first == second));
    REQUIRE(expectedEquality == (second == first));
    REQUIRE(expectedEquality == !(first != second));
    REQUIRE(expectedEquality == !(second != first));
}

TEST_CASE(
    "MatchReport is equality comparable.",
    "[reporting]")
{
    const MatchReport first{
        .expectationInfo = make_common_expectation_info(),
        .finalizeReport = {"finalize description"},
        .controlReport = state_applicable{1, 1, 0},
        .expectationReports = {
                           {true, "expectation description"}}
    };

    SECTION("When both sides are equal, they compare equal.")
    {
        const MatchReport second{first};

        REQUIRE(first == second);
        REQUIRE(second == first);
        REQUIRE(!(first != second));
        REQUIRE(!(second != first));
    }

    SECTION("When expectation_info differs, reports do not compare equal.")
    {
        MatchReport second{first};
        second.expectationInfo.mockName = "other mock-name";

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("When finalize report differs, they do not compare equal.")
    {
        MatchReport second{first};

        second.finalizeReport = {"other finalize description"};

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
    }

    SECTION("When times report differs, they do not compare equal.")
    {
        MatchReport second{first};

        second.controlReport = state_inapplicable{0, 1, 0};

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
    }

    SECTION("When expectation reports differ, they do not compare equal.")
    {
        MatchReport second{first};

        using ExpectationT = MatchReport::Expectation;
        second.expectationReports = GENERATE(
            std::vector<ExpectationT>{},
            std::vector{
                (ExpectationT{true, "other expectation description"})},
            std::vector{
                (ExpectationT{true, "expectation description"}),
                (ExpectationT{false, "other expectation description"})});

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
    }
}

TEST_CASE(
    "evaluate_match_report determines the outcome of a match report.",
    "[reporting][detail]")
{
    using ExpectationReportT = MatchReport::Expectation;

    SECTION("When any policy doesn't match => MatchResult::none is returned.")
    {
        const MatchReport report{
            .controlReport = GENERATE(
                as<control_state_t>{},
                (state_applicable{0, 1, 0}),
                (state_inapplicable{0, 1, 0, {}, {sequence::Tag{1337}}}),
                (state_saturated{0, 1, 1})),
            .expectationReports = GENERATE(
                (std::vector<ExpectationReportT>{{false}}),
                (std::vector<ExpectationReportT>{{true}, {false}}),
                (std::vector<ExpectationReportT>{{false}, {true}}))};

        REQUIRE(MatchResult::none == evaluate_match_report(report));
    }

    SECTION("When all policy match but times is inapplicable => MatchResult::inapplicable is returned.")
    {
        const MatchReport report{
            .controlReport = GENERATE(
                as<control_state_t>{},
                (state_inapplicable{0, 1, 0, {}, {sequence::Tag{1337}}}),
                (state_saturated{0, 1, 1})),
            .expectationReports = GENERATE(
                (std::vector<ExpectationReportT>{}),
                (std::vector<ExpectationReportT>{{true}}),
                (std::vector<ExpectationReportT>{{true}, {true}}))};

        REQUIRE(MatchResult::inapplicable == evaluate_match_report(report));
    }

    SECTION("When all policy match and times is applicable => MatchResult::full is returned.")
    {
        const MatchReport report{
            .controlReport = state_applicable{0, 1, 0},
            .expectationReports = GENERATE(
                (std::vector<ExpectationReportT>{}
              ),
                (std::vector<ExpectationReportT>{{true}}
              ),
                (std::vector<ExpectationReportT>{{true}, {true}}
              ))
        };

        REQUIRE(MatchResult::full == evaluate_match_report(report));
    }
}

TEST_CASE(
    "MatchReport can be printed.",
    "[report][print]")
{
    namespace Matches = Catch::Matchers;

    SECTION("When report denotes a full match.")
    {
        SECTION("Without any requirements.")
        {
            const MatchReport report{
                .expectationInfo = make_common_expectation_info(),
                .finalizeReport = {},
                .controlReport = state_applicable{0, 1, 0},
                .expectationReports = {}
            };

            REQUIRE_THAT(
                print(report),
                Matches::Matches(
                    "Matched expectation: \\{\n"
                    "mock: Mock-Name\n"
                    "from: .+\\[\\d+:\\d+\\], .+\n"
                    "\\}\n"));
        }

        SECTION("When contains requirements.")
        {
            const MatchReport report{
                .expectationInfo = make_common_expectation_info(),
                .finalizeReport = {},
                .controlReport = state_applicable{0, 1, 0},
                .expectationReports = {
                                   {true, "Requirement1 description"},
                                   {true, "Requirement2 description"}}
            };

            REQUIRE_THAT(
                print(report),
                Matches::Matches(
                    "Matched expectation: \\{\n"
                    "mock: Mock-Name\n"
                    "from: .+\\[\\d+:\\d+\\], .+\n"
                    "passed:\n"
                    "\tRequirement1 description,\n"
                    "\tRequirement2 description,\n"
                    "\\}\n"));
        }
    }

    SECTION("When report denotes an inapplicable match.")
    {
        SECTION("Without any requirements.")
        {
            SECTION("Is saturated.")
            {
                const MatchReport report{
                    .expectationInfo = make_common_expectation_info(),
                    .finalizeReport = {},
                    .controlReport = state_saturated{0, 42, 42},
                    .expectationReports = {}
                };

                REQUIRE_THAT(
                    print(report),
                    Matches::Matches(
                        "Inapplicable, but otherwise matched expectation: \\{\n"
                        "mock: Mock-Name\n"
                        "reason: already saturated \\(matched 42 times\\)\n"
                        "from: .+\\[\\d+:\\d+\\], .+\n"
                        "\\}\n"));
            }

            SECTION("Is inapplicable.")
            {
                const MatchReport report{
                    .expectationInfo = make_common_expectation_info(),
                    .finalizeReport = {},
                    .controlReport = state_inapplicable{
                                       .min = 0,
                                       .max = 42,
                                       .count = 5,
                                       .sequenceRatings = {
                            sequence::rating{0, sequence::Tag{123}}},
                                       .inapplicableSequences = {sequence::Tag{1337}, sequence::Tag{1338}}},
                    .expectationReports = {}
                };

                REQUIRE_THAT(
                    print(report),
                    Matches::Matches(
                        "Inapplicable, but otherwise matched expectation: \\{\n"
                        "mock: Mock-Name\n"
                        "reason: accepts further matches \\(matched 5 out of 42 times\\),\n"
                        "\tbut is not the current element of 2 sequence\\(s\\) \\(3 total\\).\n"
                        "from: .+\\[\\d+:\\d+\\], .+\n"
                        "\\}\n"));
            }
        }

        SECTION("When contains requirements.")
        {
            const MatchReport report{
                .expectationInfo = make_common_expectation_info(),
                .finalizeReport = {},
                .controlReport = state_saturated{0, 42, 42},
                .expectationReports = {
                                   {true, "Requirement1 description"},
                                   {true, "Requirement2 description"}}
            };

            REQUIRE_THAT(
                print(report),
                Matches::Matches(
                    "Inapplicable, but otherwise matched expectation: \\{\n"
                    "mock: Mock-Name\n"
                    "reason: already saturated \\(matched 42 times\\)\n"
                    "from: .+\\[\\d+:\\d+\\], .+\n"
                    "passed:\n"
                    "\tRequirement1 description,\n"
                    "\tRequirement2 description,\n"
                    "\\}\n"));
        }
    }

    SECTION("When report denotes an unmatched report.")
    {
        SECTION("When contains only failed requirements.")
        {
            const MatchReport report{
                .expectationInfo = make_common_expectation_info(),
                .finalizeReport = {},
                .controlReport = state_applicable{0, 1, 0},
                .expectationReports = {
                                   {false, "Requirement1 description"},
                                   {false, "Requirement2 description"}}
            };

            REQUIRE_THAT(
                print(report),
                Matches::Matches(
                    "Unmatched expectation: \\{\n"
                    "mock: Mock-Name\n"
                    "from: .+\\[\\d+:\\d+\\], .+\n"
                    "failed:\n"
                    "\tRequirement1 description,\n"
                    "\tRequirement2 description,\n"
                    "\\}\n"));
        }

        SECTION("When contains only mixed requirements.")
        {
            const MatchReport report{
                .expectationInfo = make_common_expectation_info(),
                .finalizeReport = {},
                .controlReport = state_applicable{0, 1, 0},
                .expectationReports = {
                                   {true, "Requirement1 description"},
                                   {false, "Requirement2 description"}}
            };

            REQUIRE_THAT(
                print(report),
                Matches::Matches(
                    "Unmatched expectation: \\{\n"
                    "mock: Mock-Name\n"
                    "from: .+\\[\\d+:\\d+\\], .+\n"
                    "failed:\n"
                    "\tRequirement2 description,\n"
                    "passed:\n"
                    "\tRequirement1 description,\n"
                    "\\}\n"));
        }
    }
}
