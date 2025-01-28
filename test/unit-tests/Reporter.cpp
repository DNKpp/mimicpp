//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Reporter.hpp"
#include "mimic++/Mock.hpp"

#include "SuppressionMacros.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

using reporting::TypeReport;

namespace
{
    class ReporterMock
        : public IReporter
    {
    public:
        MAKE_MOCK2(report_no_matches, void(reporting::CallReport, std::vector<MatchReport>), override);
        MAKE_MOCK2(report_inapplicable_matches, void(reporting::CallReport, std::vector<MatchReport>), override);
        MAKE_MOCK2(report_full_match, void(reporting::CallReport, MatchReport), noexcept override);
        MAKE_MOCK1(report_unfulfilled_expectation, void(ExpectationReport), override);
        MAKE_MOCK1(report_error, void(StringT), override);
        MAKE_MOCK3(report_unhandled_exception, void(reporting::CallReport, ExpectationReport, std::exception_ptr), override);
    };
}

TEST_CASE(
    "install_reporter removes the previous reporter and installs a new one.",
    "[reporting]")
{
    install_reporter<trompeloeil::deathwatched<ReporterMock>>();

    {
        auto& prevReporter = dynamic_cast<trompeloeil::deathwatched<ReporterMock>&>(*detail::get_reporter());
        REQUIRE_DESTRUCTION(prevReporter);
        install_reporter<ReporterMock>();
    }
}

namespace
{
    class TestException
    {
    };
}

START_WARNING_SUPPRESSION

// required for the REQUIRE_THROWS_AS tests
SUPPRESS_UNREACHABLE_CODE // on msvc, that must be set before the actual test-case

    TEST_CASE(
        "free report functions forward to the currently installed reporter.",
        "[reporting][detail]")
{
    install_reporter<ReporterMock>();
    auto& reporter = dynamic_cast<ReporterMock&>(*detail::get_reporter());

    const reporting::CallReport callReport{
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
            detail::report_no_matches(
                callReport,
                matchReports),
            TestException);
    }

    SECTION("When report_inapplicable_matches() is called.")
    {
        REQUIRE_CALL(reporter, report_inapplicable_matches(callReport, matchReports))
            .THROW(TestException{});

        REQUIRE_THROWS_AS(
            detail::report_inapplicable_matches(
                callReport,
                matchReports),
            TestException);
    }

    SECTION("When report_full_match() is called.")
    {
        REQUIRE_CALL(reporter, report_full_match(callReport, matchReports.front()));

        detail::report_full_match(
            callReport,
            matchReports.front());
    }

    SECTION("When report_unfulfilled_expectation() is called.")
    {
        REQUIRE_CALL(reporter, report_unfulfilled_expectation(expectationReport));

        detail::report_unfulfilled_expectation(
            expectationReport);
    }

    SECTION("When report_error() is called.")
    {
        const StringT error{"Error!"};
        REQUIRE_CALL(reporter, report_error(error));

        detail::report_error(error);
    }

    SECTION("When report_unhandled_exception() is called.")
    {
        const std::exception_ptr exception = std::make_exception_ptr(TestException{});
        REQUIRE_CALL(reporter, report_unhandled_exception(callReport, expectationReport, exception));

        detail::report_unhandled_exception(
            callReport,
            expectationReport,
            exception);
    }
}

STOP_WARNING_SUPPRESSION

START_WARNING_SUPPRESSION

// required for the REQUIRE_THROWS_AS tests
SUPPRESS_UNREACHABLE_CODE // on msvc, that must be set before the actual test-case

    TEST_CASE(
        "DefaultReporter throws exceptions on expectation violations.",
        "[reporting]")
{
    namespace Matches = Catch::Matchers;

    const bool enabledOStream = GENERATE(true, false);
    std::unique_ptr<StringStreamT> out{
        enabledOStream ? new StringStreamT{} : nullptr};

    DefaultReporter reporter{
        out.get()};

    const reporting::CallReport callReport{
        .returnTypeInfo = TypeReport::make<void>(),
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("When none matches are reported, UnmatchedCallT is thrown.")
    {
        REQUIRE_THROWS_AS(
            reporter.report_no_matches(
                callReport,
                {
                    (MatchReport{
                                 .controlReport = mimicpp::state_applicable{1, 1, 0},
                                 .expectationReports = {{false}}}
                    )
        }),
            UnmatchedCallT);

        CHECKED_IF(out)
        {
            REQUIRE_THAT(
                out->str(),
                Matches::StartsWith("No match for"));
        }
    }

    SECTION("When inapplicable matches are reported, UnmatchedCallT is thrown.")
    {
        REQUIRE_THROWS_AS(
            reporter.report_inapplicable_matches(
                callReport,
                {MatchReport{.controlReport = mimicpp::state_inapplicable{1, 1, 1}}}),
            UnmatchedCallT);

        CHECKED_IF(out)
        {
            REQUIRE_THAT(
                out->str(),
                Matches::StartsWith("No applicable match for"));
        }
    }

    SECTION("When match is reported, nothing is done.")
    {
        REQUIRE_NOTHROW(
            reporter.report_full_match(
                callReport,
                MatchReport{
                    .controlReport = mimicpp::state_applicable{1, 1, 0}
        }));

        CHECKED_IF(out)
        {
            REQUIRE_THAT(
                out->str(),
                Matches::IsEmpty());
        }
    }

    SECTION("When unfulfilled expectation is reported.")
    {
        SECTION("And when there exists no uncaught exception, UnfulfilledExpectationT is thrown.")
        {
            REQUIRE_THROWS_AS(
                reporter.report_unfulfilled_expectation({}),
                UnfulfilledExpectationT);

            CHECKED_IF(out)
            {
                REQUIRE_THAT(
                    out->str(),
                    Matches::StartsWith("Unfulfilled expectation:"));
            }
        }

        SECTION("And when there exists an uncaught exception, nothing is done.")
        {
            struct helper
            {
                ~helper()
                {
                    rep.report_unfulfilled_expectation({});
                }

                DefaultReporter& rep;
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
                Error<>);

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

                DefaultReporter& rep;
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

namespace
{
    struct saturated
    {
    };

    inline std::optional<StringT> g_SuccessMessage{};

    void send_success(const StringViewT msg)
    {
        if (g_SuccessMessage.has_value())
        {
            throw saturated{};
        }

        g_SuccessMessage.emplace(msg);
    }

    inline std::optional<StringT> g_WarningMessage{};

    void send_warning(const StringViewT msg)
    {
        if (g_WarningMessage.has_value())
        {
            throw saturated{};
        }

        g_WarningMessage.emplace(msg);
    }

    inline std::optional<StringT> g_FailMessage{};

    [[noreturn]]
    void send_fail(const StringViewT msg)
    {
        if (g_FailMessage.has_value())
        {
            throw saturated{};
        }

        g_FailMessage.emplace(msg);
        throw TestException{};
    }

    class ScopedReporter
    {
    public:
        ~ScopedReporter() noexcept
        {
            install_reporter<DefaultReporter>();
        }

        ScopedReporter() noexcept
        {
            install_reporter<
                BasicReporter<
                    &send_success,
                    &send_warning,
                    &send_fail>>();
        }
    };

    class StdException
        : public std::runtime_error
    {
    public:
        StdException()
            : std::runtime_error{"An std::exception type."}
        {
        }
    };

    template <typename Exception>
    class ThrowOnMatches
    {
    public:
        [[maybe_unused]]
        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        [[maybe_unused]]
        static bool matches([[maybe_unused]] const auto& info)
        {
            throw Exception{};
        }

        [[maybe_unused]]
        static constexpr std::nullopt_t describe() noexcept
        {
            return std::nullopt;
        }

        [[maybe_unused]]
        static constexpr void consume([[maybe_unused]] const auto& info) noexcept
        {
        }
    };
}

TEST_CASE(
    "BasicReporter forwards messages to the installed callbacks.",
    "[report]")
{
    namespace matches = Catch::Matchers;

    const ScopedReporter reporter{};
    g_SuccessMessage.reset();
    g_WarningMessage.reset();
    g_FailMessage.reset();

    SECTION("When success is reported.")
    {
        Mock<void(int)> mock{};

        SCOPED_EXP mock.expect_call(42);

        mock(42);
        REQUIRE_THAT(
            g_SuccessMessage.value(),
            matches::StartsWith("Found match for"));
    }

    SECTION("Sends fail, when no match can be found.")
    {
        Mock<void(int)> mock{};

        SECTION("When no expectation exists.")
        {
            REQUIRE_THROWS_AS(
                mock(1337),
                TestException);

            REQUIRE_THAT(
                g_FailMessage.value(),
                matches::StartsWith("No match for")
                    && matches::ContainsSubstring("No expectations available."));
        }

        SECTION("When non matching expectation exists.")
        {
            SCOPED_EXP mock.expect_call(42)
                && expect::at_most(1); // prevent unfulfilled reporting

            REQUIRE_THROWS_AS(
                mock(1337),
                TestException);

            REQUIRE_THAT(
                g_FailMessage.value(),
                matches::StartsWith("No match for")
                    && matches::ContainsSubstring("1 available expectation(s):"));
        }
    }

    SECTION("Sends fail, when no applicable match can be found.")
    {
        Mock<void(int)> mock{};

        SCOPED_EXP mock.expect_call(42);

        mock(42);
        REQUIRE_THROWS_AS(
            mock(42),
            TestException);

        REQUIRE_THAT(
            g_FailMessage.value(),
            matches::StartsWith("No applicable match for")
                && matches::ContainsSubstring("Tested expectations:"));
    }

    SECTION("Sends fail, when unfulfilled expectation is reported.")
    {
        constexpr auto action = [] {
            Mock<void()> mock{};
            SCOPED_EXP mock.expect_call();
        };

        REQUIRE_THROWS_AS(
            action(),
            TestException);

        REQUIRE_THAT(
            g_FailMessage.value(),
            matches::StartsWith("Unfulfilled expectation:"));
    }

    SECTION("Does not send fail, when unfulfilled expectation is reported, but an other exception already exists.")
    {
        constexpr auto action = [] {
            Mock<void()> mock{};
            SCOPED_EXP mock.expect_call();
            throw 42;
        };

        REQUIRE_THROWS_AS(
            action(),
            int);

        REQUIRE(!g_FailMessage);
    }

    SECTION("Sends fail, when generic error is reported.")
    {
        REQUIRE_THROWS_AS(
            mimicpp::detail::report_error("Hello, World!"),
            TestException);

        REQUIRE_THAT(
            g_FailMessage.value(),
            matches::Equals("Hello, World!"));
    }

    SECTION("Does not send fail, when generic error is reported, but an other exception already exists.")
    {
        struct helper
        {
            ~helper()
            {
                detail::report_error("Hello, World!");
            }
        };

        const auto action = [] {
            helper h{};
            throw 42;
        };

        REQUIRE_THROWS_AS(
            action(),
            int);

        REQUIRE(!g_FailMessage);
    }

    SECTION("Sends warning, when unhandled exception is reported.")
    {
        Mock<void()> mock{};
        SCOPED_EXP mock.expect_call(); // this will actually consume the call

        SECTION("When a std::exception is thrown.")
        {
            SCOPED_EXP mock.expect_call()
                && expect::at_most(1)
                && ThrowOnMatches<StdException>{};

            REQUIRE_NOTHROW(mock());
            REQUIRE_THAT(
                g_WarningMessage.value(),
                matches::StartsWith("Unhandled exception: what: An std::exception type."));
        }

        SECTION("When an unknown exception is thrown.")
        {
            SCOPED_EXP mock.expect_call()
                && expect::at_most(1)
                && ThrowOnMatches<TestException>{};

            REQUIRE_NOTHROW(mock());
            REQUIRE_THAT(
                g_WarningMessage.value(),
                matches::StartsWith("Unhandled exception: Unknown exception type."));
        }
    }
}
