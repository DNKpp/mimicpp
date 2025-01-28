//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/GlobalReporter.hpp"

#include "SuppressionMacros.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;
using reporting::CallReport;
using reporting::ExpectationReport;
using reporting::MatchReport;
using reporting::TypeReport;

namespace
{
    class ReporterMock
        : public reporting::IReporter
    {
    public:
        MAKE_MOCK2(report_no_matches, void(CallReport, std::vector<MatchReport>), override);
        MAKE_MOCK2(report_inapplicable_matches, void(CallReport, std::vector<MatchReport>), override);
        MAKE_MOCK2(report_full_match, void(CallReport, MatchReport), noexcept override);
        MAKE_MOCK1(report_unfulfilled_expectation, void(ExpectationReport), override);
        MAKE_MOCK1(report_error, void(StringT), override);
        MAKE_MOCK3(report_unhandled_exception, void(CallReport, ExpectationReport, std::exception_ptr), override);
    };
}

TEST_CASE(
    "install_reporter removes the previous reporter and installs a new one.",
    "[reporting]")
{
    reporting::install_reporter<trompeloeil::deathwatched<ReporterMock>>();

    {
        auto& prevReporter = dynamic_cast<trompeloeil::deathwatched<ReporterMock>&>(*reporting::detail::get_reporter());
        REQUIRE_DESTRUCTION(prevReporter);
        reporting::install_reporter<ReporterMock>();
    }
}

namespace
{
    class TestException
    {
    };
}

// required for the REQUIRE_THROWS_AS tests
START_WARNING_SUPPRESSION
SUPPRESS_UNREACHABLE_CODE // on msvc, that must be set before the actual test-case

    TEST_CASE(
        "free report functions forward to the currently installed reporter.",
        "[reporting][detail]")
{
    reporting::install_reporter<ReporterMock>();
    auto& reporter = dynamic_cast<ReporterMock&>(*reporting::detail::get_reporter());

    const CallReport callReport{
        .returnTypeInfo = TypeReport::make<void>(),
        .fromLoc = std::source_location::current()};

    const std::vector<MatchReport> matchReports{
        {.finalizeReport = {"match1"}},
        {.finalizeReport = {"match2"}}};

    const ExpectationReport expectationReport{};

    SECTION("When report_no_matches() is called.")
    {
        REQUIRE_CALL(reporter, report_no_matches(callReport, matchReports))
            .THROW(TestException{});

        REQUIRE_THROWS_AS(
            reporting::detail::report_no_matches(
                callReport,
                matchReports),
            TestException);
    }

    SECTION("When report_inapplicable_matches() is called.")
    {
        REQUIRE_CALL(reporter, report_inapplicable_matches(callReport, matchReports))
            .THROW(TestException{});

        REQUIRE_THROWS_AS(
            reporting::detail::report_inapplicable_matches(
                callReport,
                matchReports),
            TestException);
    }

    SECTION("When report_full_match() is called.")
    {
        REQUIRE_CALL(reporter, report_full_match(callReport, matchReports.front()));

        reporting::detail::report_full_match(
            callReport,
            matchReports.front());
    }

    SECTION("When report_unfulfilled_expectation() is called.")
    {
        REQUIRE_CALL(reporter, report_unfulfilled_expectation(expectationReport));

        reporting::detail::report_unfulfilled_expectation(
            expectationReport);
    }

    SECTION("When report_error() is called.")
    {
        const StringT error{"Error!"};
        REQUIRE_CALL(reporter, report_error(error));

        reporting::detail::report_error(error);
    }

    SECTION("When report_unhandled_exception() is called.")
    {
        const std::exception_ptr exception = std::make_exception_ptr(TestException{});
        REQUIRE_CALL(reporter, report_unhandled_exception(callReport, expectationReport, exception));

        reporting::detail::report_unhandled_exception(
            callReport,
            expectationReport,
            exception);
    }
}

STOP_WARNING_SUPPRESSION


