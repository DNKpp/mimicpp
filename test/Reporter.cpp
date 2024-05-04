// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Reporter.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace mimicpp;

TEST_CASE(
	"CallReport::Arg is equality comparable.",
	"[reporting]"
)
{
	using ArgT = CallReport::Arg;

	const ArgT first{
		.typeIndex = typeid(int),
		.stateString = "42"
	};

	const auto [expectedEquality, second] = GENERATE(
		(table<bool, ArgT>({
			{false, {typeid(int), "1337"}},
			{false, {typeid(short), "42"}},
			{true, {typeid(int), "42"}}
			})));

	REQUIRE(expectedEquality == (first == second));
	REQUIRE(expectedEquality == (second == first));
	REQUIRE(expectedEquality == !(first != second));
	REQUIRE(expectedEquality == !(second!= first));
}

TEST_CASE(
	"CallReport is equality comparable.",
	"[reporting]"
)
{
	const CallReport first{
		.returnTypeIndex = typeid(std::string),
		.argDetails = {
			{
				.typeIndex = typeid(int),
				.stateString = "42"
			}
		},
		.fromLoc = std::source_location::current(),
		.fromCategory = ValueCategory::any,
		.fromConstness = Constness::any
	};

	SECTION("When both sides are equal, they compare equal.")
	{
		const CallReport second{first};

		REQUIRE(first == second);
		REQUIRE(second == first);
		REQUIRE(!(first != second));
		REQUIRE(!(second != first));
	}

	SECTION("When return type differs, they compare not equal.")
	{
		CallReport second{first};

		second.returnTypeIndex = GENERATE(as<std::type_index>{}, typeid(void), typeid(std::string_view));

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
	}

	SECTION("When category differs, they compare not equal.")
	{
		CallReport second{first};

		second.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue);

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
	}

	SECTION("When constness differs, they compare not equal.")
	{
		CallReport second{first};

		second.fromConstness = GENERATE(Constness::as_const, Constness::non_const);

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
	}

	SECTION("When source location differs, they compare not equal.")
	{
		CallReport second{first};

		second.fromLoc = std::source_location::current();

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
	}

	SECTION("When source location differs, they compare not equal.")
	{
		CallReport second{first};

		using ArgT = CallReport::Arg;
		second.argDetails = GENERATE(
			std::vector<ArgT>{},
			std::vector{
			(ArgT{.typeIndex = typeid(int), .stateString = "1337"})
			},
			std::vector{
			(ArgT{.typeIndex = typeid(int), .stateString = "42"}),
			(ArgT{.typeIndex = typeid(int), .stateString = "1337"})
			});

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
	}
}

TEST_CASE(
	"make_call_report generates report from call info."
	"[reporting]"
)
{
	SECTION("When call info has void return type.")
	{
		const call::Info<void> info{
			.args = {},
			.fromCategory = GENERATE(ValueCategory::any, ValueCategory::lvalue, ValueCategory::rvalue),
			.fromConstness = GENERATE(Constness::any, Constness::as_const, Constness::non_const),
			.fromSourceLocation = std::source_location::current()
		};

		const CallReport report = make_call_report(info);

		REQUIRE(
			report ==
			CallReport{
			.returnTypeIndex = typeid(void),
			.argDetails = {},
			.fromLoc = info.fromSourceLocation,
			.fromCategory = info.fromCategory,
			.fromConstness = info.fromConstness
			});
	}

	SECTION("When call info has non-void return type.")
	{
		const call::Info<int> info{
			.args = {},
			.fromCategory = GENERATE(ValueCategory::any, ValueCategory::lvalue, ValueCategory::rvalue),
			.fromConstness = GENERATE(Constness::any, Constness::as_const, Constness::non_const),
			.fromSourceLocation = std::source_location::current()
		};

		const CallReport report = make_call_report(info);

		REQUIRE(
			report ==
			CallReport{
			.returnTypeIndex = typeid(int),
			.argDetails = {},
			.fromLoc = info.fromSourceLocation,
			.fromCategory = info.fromCategory,
			.fromConstness = info.fromConstness
			});
	}

	SECTION("When call info has arbitrary args.")
	{
		const int arg0{1337};
		double arg1{4.2};
		std::string arg2{"Hello, World!"};
		const call::Info<void, const int&, double, std::string> info{
			.args = {std::ref(arg0), std::ref(arg1), std::ref(arg2)},
			.fromCategory = GENERATE(ValueCategory::any, ValueCategory::lvalue, ValueCategory::rvalue),
			.fromConstness = GENERATE(Constness::any, Constness::as_const, Constness::non_const),
			.fromSourceLocation = std::source_location::current()
		};

		const CallReport report = make_call_report(info);

		using ArgT = CallReport::Arg;
		REQUIRE(
			report ==
			CallReport{
			.returnTypeIndex = typeid(void),
			.argDetails = {
			(ArgT{typeid(const int&), "1337"}),
			(ArgT{typeid(double), "4.2"}),
			(ArgT{typeid(std::string), "\"Hello, World!\""})
			},
			.fromLoc = info.fromSourceLocation,
			.fromCategory = info.fromCategory,
			.fromConstness = info.fromConstness
			});
	}
}

TEST_CASE(
	"ExpectationReport is equality comparable.",
	"[reporting]"
)
{
	const ExpectationReport first{
		.description = "Hello, World!"
	};

	const auto [expectedEquality, second] = GENERATE(
		(table<bool, ExpectationReport>({
			{false, {"not equal"}},
			{true, {"Hello, World!"}}
			})));

	REQUIRE(expectedEquality == (first == second));
	REQUIRE(expectedEquality == (second == first));
	REQUIRE(expectedEquality == !(first != second));
	REQUIRE(expectedEquality == !(second!= first));
}

TEST_CASE(
	"MatchReport::Finalize is equality comparable."
	"[reporting]"
)
{
	using ReportT = MatchReport::Finalize;

	const ReportT first{
		.description = "Hello, World!"
	};

	const auto [expectedEquality, second] = GENERATE(
		(table<bool, ReportT>({
			{false, {"not equal"}},
			{false, {std::nullopt}},
			{true, {"Hello, World!"}}
			})));

	REQUIRE(expectedEquality == (first == second));
	REQUIRE(expectedEquality == (second == first));
	REQUIRE(expectedEquality == !(first != second));
	REQUIRE(expectedEquality == !(second!= first));
}

TEST_CASE(
	"MatchReport::Times is equality comparable."
	"[reporting]"
)
{
	using ReportT = MatchReport::Times;

	const ReportT first{
		.isApplicable = true,
		.description = "Hello, World!"
	};

	const auto [expectedEquality, second] = GENERATE(
		(table<bool, ReportT>({
			{false, {true, "not equal"}},
			{false, {true, std::nullopt}},
			{false, {false, "Hello, World!"}},
			{true, {true, "Hello, World!"}}
			})));

	REQUIRE(expectedEquality == (first == second));
	REQUIRE(expectedEquality == (second == first));
	REQUIRE(expectedEquality == !(first != second));
	REQUIRE(expectedEquality == !(second!= first));
}

TEST_CASE(
	"MatchReport::Expectation is equality comparable."
	"[reporting]"
)
{
	using ReportT = MatchReport::Expectation;

	const ReportT first{
		.isMatching = true,
		.description = "Hello, World!"
	};

	const auto [expectedEquality, second] = GENERATE(
		(table<bool, ReportT>({
			{false, {true, "not equal"}},
			{false, {true, std::nullopt}},
			{false, {false, "Hello, World!"}},
			{true, {true, "Hello, World!"}}
			})));

	REQUIRE(expectedEquality == (first == second));
	REQUIRE(expectedEquality == (second == first));
	REQUIRE(expectedEquality == !(first != second));
	REQUIRE(expectedEquality == !(second!= first));
}

TEST_CASE(
	"MatchReport is equality comparable.",
	"[reporting]"
)
{
	const MatchReport first{
		.finalizeReport = {"finalize description"},
		.timesReport = {true, "times description"},
		.expectationReports = {
			{true, "expectation description"}
		}
	};

	SECTION("When both sides are equal, they compare equal.")
	{
		const MatchReport second{first};

		REQUIRE(first == second);
		REQUIRE(second == first);
		REQUIRE(!(first != second));
		REQUIRE(!(second != first));
	}

	SECTION("When finalize report differs, they do not compare equal.")
	{
		MatchReport second{first};

		second.finalizeReport = {"other finalize description"};

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
	}

	SECTION("When times report differs, they do not compare equal.")
	{
		MatchReport second{first};

		second.timesReport = {true, "other times description"};

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
	}

	SECTION("When expectation reports differ, they do not compare equal.")
	{
		MatchReport second{first};

		using ExpectationT = MatchReport::Expectation;
		second.expectationReports = GENERATE(
			std::vector<ExpectationT>{},
			std::vector{
			(ExpectationT{true, "other expectation description"})
			},
			std::vector{
			(ExpectationT{true, "expectation description"}),
			(ExpectationT{false, "other expectation description"})
			});

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
	}
}

namespace
{
	class ReporterMock
		: public IReporter
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
	"[reporting]"
)
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

TEST_CASE(
	"free report functions forward to the currently installed reporter.",
	"[reporting]"
)
{
	install_reporter<ReporterMock>();
	auto& reporter = dynamic_cast<ReporterMock&>(*detail::get_reporter());

	const CallReport callReport{
		.returnTypeIndex = typeid(void),
		.fromLoc = std::source_location::current()
	};

	const std::vector<MatchReport> matchReports{
		{.finalizeReport = {"match1"}},
		{.finalizeReport = {"match2"}}
	};

	const ExpectationReport expectationReport{
		.description = "ExpectationReport"
	};

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

TEST_CASE(
	"evaluate_match_report determines the outcome of a match report.",
	"[reporting]"
)
{
	using ExpectationReportT = MatchReport::Expectation;

	SECTION("When any policy doesn't match => MatchResult::none is returned.")
	{
		const MatchReport report{
			.timesReport = {GENERATE(true, false)},
			.expectationReports = GENERATE(
				(std::vector<ExpectationReportT>{{false}}),
				(std::vector<ExpectationReportT>{{true}, {false}}),
				(std::vector<ExpectationReportT>{{false}, {true}}))
		};

		REQUIRE(MatchResult::none == evaluate_match_report(report));
	}

	SECTION("When all policy match but times is inapplicable => MatchResult::inapplicable is returned.")
	{
		const MatchReport report{
			.timesReport = {false},
			.expectationReports = GENERATE(
				(std::vector<ExpectationReportT>{}),
				(std::vector<ExpectationReportT>{{true}}),
				(std::vector<ExpectationReportT>{{true}, {true}}))
		};

		REQUIRE(MatchResult::inapplicable == evaluate_match_report(report));
	}

	SECTION("When all policy match and times is applicable => MatchResult::full is returned.")
	{
		const MatchReport report{
			.timesReport = {true},
			.expectationReports = GENERATE(
				(std::vector<ExpectationReportT>{}),
				(std::vector<ExpectationReportT>{{true}}),
				(std::vector<ExpectationReportT>{{true}, {true}}))
		};

		REQUIRE(MatchResult::full == evaluate_match_report(report));
	}
}