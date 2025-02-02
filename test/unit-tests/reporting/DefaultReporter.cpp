//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/DefaultReporter.hpp"

#include "SuppressionMacros.hpp"

using namespace mimicpp;
using reporting::CallReport;
using reporting::ExpectationReport;
using reporting::NoMatchReport;
using reporting::RequirementOutcomes;
using reporting::TypeReport;

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

// required for the REQUIRE_THROWS_AS tests
START_WARNING_SUPPRESSION
SUPPRESS_UNREACHABLE_CODE // on msvc, that must be set before the actual test-case

    TEST_CASE(
        "DefaultReporter throws exceptions on expectation violations.",
        "[reporting]")
{
    namespace Matches = Catch::Matchers;

    auto [reporter, out] = std::invoke(
        [] {
            std::unique_ptr<StringStreamT> ss{};
            if (GENERATE(true, false))
            {
                ss = std::make_unique<StringStreamT>();
                return std::tuple{
                    reporting::DefaultReporter{*ss},
                    std::move(ss)};
            }

            return std::tuple{
                reporting::DefaultReporter{},
                std::move(ss)};
        });

    CallReport const callReport{
        .returnTypeInfo = TypeReport::make<void>(),
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("When no-match is reported, UnmatchedCallT is thrown.")
    {
        ExpectationReport const expectationReport{
            .info = make_common_expectation_info(),
            .controlReport = reporting::state_applicable{1, 1, 0},
            .requirementDescriptions = {{"expect: Invalid"}}
        };
        RequirementOutcomes const requirementOutcomes{
            .outcomes = {false}};
        std::vector noMatchReports{
            NoMatchReport{expectationReport, requirementOutcomes}
        };

        REQUIRE_THROWS_AS(
            reporter.report_no_matches(callReport, noMatchReports),
            reporting::UnmatchedCallT);

        CHECKED_IF(out)
        {
            REQUIRE_THAT(
                out->str(),
                Matches::StartsWith("Unmatched Call from ")
                    && Matches::ContainsSubstring("1 non-matching Expectation(s):\n")
                    && Matches::ContainsSubstring("#1 Expectation from ")
                    && Matches::ContainsSubstring("Due to Violation(s):\n")
                    && Matches::ContainsSubstring("  - expect: Invalid")
                    && !Matches::ContainsSubstring("With Adherence(s):"));
        }
    }

    SECTION("When inapplicable matches are reported, UnmatchedCallT is thrown.")
    {
        ExpectationReport const expectationReport{
            .info = make_common_expectation_info(),
            .controlReport = reporting::state_inapplicable{1, 1, 1},
            .requirementDescriptions = {{"expect: Valid"}}
        };

        REQUIRE_THROWS_AS(
            reporter.report_inapplicable_matches(callReport, {expectationReport}),
            reporting::UnmatchedCallT);

        CHECKED_IF(out)
        {
            REQUIRE_THAT(
                out->str(),
                Matches::StartsWith("Unmatched Call from ")
                    && Matches::ContainsSubstring("1 inapplicable but otherwise matching Expectation(s):\n")
                    && Matches::ContainsSubstring("#1 Expectation from ")
                    && Matches::ContainsSubstring("With Adherence(s):\n")
                    && Matches::ContainsSubstring("  + expect: Valid")
                    && !Matches::ContainsSubstring("Due to Violation(s):"));
        }
    }

    SECTION("When match is reported, nothing is done.")
    {
        ExpectationReport const expectationReport{
            .info = make_common_expectation_info(),
            .controlReport = reporting::state_applicable{1, 1, 0},
            .requirementDescriptions = {{"expect: Valid"}}
        };

        REQUIRE_NOTHROW(reporter.report_full_match(callReport, expectationReport));

        CHECKED_IF(out)
        {
            REQUIRE_THAT(
                out->str(),
                Matches::IsEmpty());
        }
    }

    SECTION("When unfulfilled expectation is reported.")
    {
        static ExpectationReport const expectationReport{
            .info = make_common_expectation_info(),
            .controlReport = reporting::state_applicable{1, 1, 0}
        };

        SECTION("And when there exists no uncaught exception, UnfulfilledExpectationT is thrown.")
        {
            REQUIRE_THROWS_AS(
                reporter.report_unfulfilled_expectation(expectationReport),
                reporting::UnfulfilledExpectationT);

            CHECKED_IF(out)
            {
                REQUIRE_THAT(
                    out->str(),
                    Matches::StartsWith("Unfulfilled Expectation from "));
            }
        }

        SECTION("And when there exists an uncaught exception, nothing is done.")
        {
            struct helper
            {
                ~helper()
                {
                    rep.report_unfulfilled_expectation(expectationReport);
                }

                reporting::DefaultReporter& rep;
            };

            const auto runTest = [&] {
                helper h{reporter};
                throw 42;
            };

            REQUIRE_THROWS_AS(
                runTest(),
                int);

            CHECKED_IF(out)
            {
                REQUIRE_THAT(
                    out->str(),
                    Matches::IsEmpty());
            }
        }
    }

    SECTION("When error is reported")
    {
        SECTION("And when there exists no uncaught exception, Error is thrown.")
        {
            REQUIRE_THROWS_AS(
                reporter.report_error({"Test"}),
                reporting::Error<>);

            CHECKED_IF(out)
            {
                REQUIRE_THAT(
                    out->str(),
                    Matches::StartsWith("Test"));
            }
        }

        SECTION("And when there exists an uncaught exception, nothing is done.")
        {
            struct helper
            {
                ~helper()
                {
                    rep.report_error({"Test"});
                }

                reporting::DefaultReporter& rep;
            };

            const auto runTest = [&] {
                helper h{reporter};
                throw 42;
            };

            REQUIRE_THROWS_AS(
                runTest(),
                int);

            CHECKED_IF(out)
            {
                REQUIRE_THAT(
                    out->str(),
                    Matches::IsEmpty());
            }
        }
    }

    SECTION("When unhandled exception is reported, nothing is done.")
    {
        REQUIRE_NOTHROW(
            reporter.report_unhandled_exception(
                callReport,
                {},
                std::make_exception_ptr(std::runtime_error{"Test"})));

        CHECKED_IF(out)
        {
            REQUIRE_THAT(
                out->str(),
                Matches::StartsWith("Unhandled exception:"));
        }
    }
}

STOP_WARNING_SUPPRESSION
