// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Reporter.hpp"
#include "mimic++/Mock.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

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
		.sourceLocation = std::source_location::current(),
		.finalizerDescription = "finalizer description",
		.timesDescription = "times description",
		.expectationDescriptions = {
			"first expectation description"
		}
	};

	SECTION("When all members are equal, reports compare equal.")
	{
		const ExpectationReport second{first};

		REQUIRE(first == second);
		REQUIRE(second == first);
		REQUIRE(!(first != second));
		REQUIRE(!(second!= first));
	}

	SECTION("When source-location differs, reports do not compare equal.")
	{
		ExpectationReport second{first};
		second.sourceLocation = GENERATE(
			as<std::optional<std::source_location>>{},
			std::nullopt,
			std::source_location::current());

		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
		REQUIRE(first != second);
		REQUIRE(second!= first);
	}

	SECTION("When finalizer description differs, reports do not compare equal.")
	{
		ExpectationReport second{first};
		second.finalizerDescription = GENERATE(
			as<std::optional<StringT>>{},
			std::nullopt,
			"other finalizer description");

		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
		REQUIRE(first != second);
		REQUIRE(second!= first);
	}

	SECTION("When times description differs, reports do not compare equal.")
	{
		ExpectationReport second{first};
		second.timesDescription = GENERATE(
			as<std::optional<StringT>>{},
			std::nullopt,
			"other times description");

		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
		REQUIRE(first != second);
		REQUIRE(second!= first);
	}

	SECTION("When expectation descriptions differ, reports do not compare equal.")
	{
		ExpectationReport second{first};
		second.expectationDescriptions = GENERATE(
			(std::vector<std::optional<StringT>>{}),
			(std::vector<std::optional<StringT>>{"other expectation description"}),
			(std::vector<std::optional<StringT>>{"expectation description", "other expectation description"}));

		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
		REQUIRE(first != second);
		REQUIRE(second!= first);
	}
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

	static const std::vector<sequence::rating> firstPriorities{
		{42, sequence::Tag{1337}}
	};

	const ReportT first{
		.isApplicable = true,
		.ratings = firstPriorities,
		.description = "Hello, World!"
	};

	const auto [expectedEquality, second] = GENERATE(
		(table<bool, ReportT>({
			{false, {true, firstPriorities, "not equal"}},
			{false, {true, firstPriorities, std::nullopt}},
			{false, {false, std::nullopt, "Hello, World!"}},
			{true, {true, firstPriorities, "Hello, World!"}}
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
		.sourceLocation = std::source_location::current(),
		.finalizeReport = {"finalize description"},
		.timesReport = {true, {}, "times description"},
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

	SECTION("When source-location differs, reports do not compare equal.")
	{
		MatchReport second{first};
		second.sourceLocation = GENERATE(
			as<std::optional<std::source_location>>{},
			std::nullopt,
			std::source_location::current());

		REQUIRE(!(first == second));
		REQUIRE(!(second == first));
		REQUIRE(first != second);
		REQUIRE(second!= first);
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

		second.timesReport = {true, {}, "other times description"};

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

TEST_CASE(
	"DefaultReporter throws exceptions on expectation violations.",
	"[reporting]"
)
{
	DefaultReporter reporter{};

	const CallReport callReport{
		.returnTypeIndex = typeid(void),
		.fromLoc = std::source_location::current()
	};

	SECTION("When none matches are reported, UnmatchedCallT is thrown.")
	{
		REQUIRE_THROWS_AS(
			reporter.report_no_matches(
				callReport,
				{
				MatchReport{.timesReport = {true}, .expectationReports = {{false}}}
				}),
			UnmatchedCallT);
	}

	SECTION("When inapplicable matches are reported, UnmatchedCallT is thrown.")
	{
		REQUIRE_THROWS_AS(
			reporter.report_inapplicable_matches(
				callReport,
				{MatchReport{.timesReport = {false}}}),
			UnmatchedCallT);
	}

	SECTION("When match is reported, nothing is done.")
	{
		REQUIRE_NOTHROW(
			reporter.report_full_match(
				callReport,
				MatchReport{.timesReport = {true}}));
	}

	SECTION("When unfulfilled expectation is reported.")
	{
		SECTION("And when there exists no uncaught exception, UnfulfilledExpectationT is thrown.")
		{
			REQUIRE_THROWS_AS(
				reporter.report_unfulfilled_expectation({}),
				UnfulfilledExpectationT);
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

			const auto runTest = [&]
			{
				helper h{reporter};
				throw 42;
			};

			REQUIRE_THROWS_AS(
				runTest(),
				int);
		}
	}

	SECTION("When error is reported")
	{
		SECTION("And when there exists no uncaught exception, Error is thrown.")
		{
			REQUIRE_THROWS_AS(
				reporter.report_error({"Test"}),
				Error<>);
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

			const auto runTest = [&]
			{
				helper h{reporter};
				throw 42;
			};

			REQUIRE_THROWS_AS(
				runTest(),
				int);
		}
	}

	SECTION("When unhandled exception is reported, nothing is done.")
	{
		REQUIRE_NOTHROW(
			reporter.report_unhandled_exception(
				callReport,
				{},
				std::make_exception_ptr(std::runtime_error{"Test"})));
	}
}

TEST_CASE(
	"stringify_match_report converts the match report to text representation.",
	"[report]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("When report denotes a full match.")
	{
		SECTION("Without any requirements.")
		{
			const MatchReport report{
				.sourceLocation = std::source_location::current(),
				.finalizeReport = {},
				.timesReport = {true, {}, "finalize description"},
				.expectationReports = {}
			};

			REQUIRE_THAT(
				stringify_match_report(report),
				Matches::Matches(
					"Matched expectation: \\{\n"
					"from: .+\\[\\d+:\\d+\\], .+\n"
					"\\}\n"));
		}

		SECTION("When contains requirements.")
		{
			const MatchReport report{
				.sourceLocation = std::source_location::current(),
				.finalizeReport = {},
				.timesReport = {true, {}, "finalize description"},
				.expectationReports = {
					{true, "Requirement1 description"},
					{true, "Requirement2 description"}
				}
			};

			REQUIRE_THAT(
				stringify_match_report(report),
				Matches::Matches(
					"Matched expectation: \\{\n"
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
			const MatchReport report{
				.sourceLocation = std::source_location::current(),
				.finalizeReport = {},
				.timesReport = {false, {}, "finalize description"},
				.expectationReports = {}
			};

			REQUIRE_THAT(
				stringify_match_report(report),
				Matches::Matches(
					"Inapplicable, but otherwise matched expectation: \\{\n"
					"reason: finalize description\n"
					"from: .+\\[\\d+:\\d+\\], .+\n"
					"\\}\n"));
		}

		SECTION("When contains requirements.")
		{
			const MatchReport report{
				.sourceLocation = std::source_location::current(),
				.finalizeReport = {},
				.timesReport = {false, {}, "finalize description"},
				.expectationReports = {
					{true, "Requirement1 description"},
					{true, "Requirement2 description"}
				}
			};

			REQUIRE_THAT(
				stringify_match_report(report),
				Matches::Matches(
					"Inapplicable, but otherwise matched expectation: \\{\n"
					"reason: finalize description\n"
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
				.sourceLocation = std::source_location::current(),
				.finalizeReport = {},
				.timesReport = {true, {}, "finalize description"},
				.expectationReports = {
					{false, "Requirement1 description"},
					{false, "Requirement2 description"}
				}
			};

			REQUIRE_THAT(
				stringify_match_report(report),
				Matches::Matches(
					"Unmatched expectation: \\{\n"
					"from: .+\\[\\d+:\\d+\\], .+\n"
					"failed:\n"
					"\tRequirement1 description,\n"
					"\tRequirement2 description,\n"
					"\\}\n"));
		}

		SECTION("When contains only mixed requirements.")
		{
			const MatchReport report{
				.sourceLocation = std::source_location::current(),
				.finalizeReport = {},
				.timesReport = {true, {}, "finalize description"},
				.expectationReports = {
					{true, "Requirement1 description"},
					{false, "Requirement2 description"}
				}
			};

			REQUIRE_THAT(
				stringify_match_report(report),
				Matches::Matches(
					"Unmatched expectation: \\{\n"
					"from: .+\\[\\d+:\\d+\\], .+\n"
					"failed:\n"
					"\tRequirement2 description,\n"
					"passed:\n"
					"\tRequirement1 description,\n"
					"\\}\n"));
		}
	}

	SECTION("When source location is empty, that information is omitted.")
	{
		const MatchReport report{
			.sourceLocation = std::nullopt,
			.finalizeReport = {},
			.timesReport = {true, {}, "finalize description"},
			.expectationReports = {}
		};

		REQUIRE_THAT(
			stringify_match_report(report),
			Matches::Matches(
				"Matched expectation: \\{\n"
				"\\}\n"));
	}
}

TEST_CASE(
	"stringify_call_report converts the call report to text representation.",
	"[report]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("When report without arguments is given.")
	{
		const CallReport report{
			.returnTypeIndex = typeid(void),
			.argDetails = {},
			.fromLoc = std::source_location::current(),
			.fromCategory = ValueCategory::any,
			.fromConstness = Constness::any
		};

		REQUIRE_THAT(
			stringify_call_report(report),
			Matches::Matches(
				"call from .+\\[\\d+:\\d+\\], .+\n"
				"constness: any\n"
				"value category: any\n"
				"return type: (v|void)\n"));
	}

	SECTION("When report with arguments is given.")
	{
		const CallReport report{
			.returnTypeIndex = typeid(int),
			.argDetails = {{.typeIndex = typeid(double), .stateString = "4.2"}},
			.fromLoc = std::source_location::current(),
			.fromCategory = ValueCategory::lvalue,
			.fromConstness = Constness::as_const
		};

		REQUIRE_THAT(
			stringify_call_report(report),
			Matches::Matches(
				"call from .+\\[\\d+:\\d+\\], .+\n"
				"constness: const\n"
				"value category: lvalue\n"
				"return type: (i|int)\n"
				"args:\n"
				"\targ\\[0\\]: \\{\n"
				"\t\ttype: (d|double),\n"
				"\t\tvalue: 4.2\n"
				"\t\\},\n"));
	}
}

TEST_CASE(
	"stringify_expectation_report converts the match report to text representation.",
	"[report]"
)
{
	namespace Matches = Catch::Matchers;

	ExpectationReport report{
		.sourceLocation = std::source_location::current(),
		.finalizerDescription = "finalizer description",
		.timesDescription = "times description",
		.expectationDescriptions = {
			"expectation1 description"
		}
	};

	SECTION("When full report is given.")
	{
		REQUIRE_THAT(
			stringify_expectation_report(std::as_const(report)),
			Matches::Matches(
				"Expectation report:\n"
				"from: .+\\[\\d+:\\d+\\], .+\n"
				"times: times description\n"
				"expects:\n"
				"\texpectation1 description,\n"
				"finally: finalizer description\n"));
	}

	SECTION("When times description is missing.")
	{
		report.timesDescription.reset();

		REQUIRE_THAT(
			stringify_expectation_report(std::as_const(report)),
			Matches::Matches(
				"Expectation report:\n"
				"from: .+\\[\\d+:\\d+\\], .+\n"
				"expects:\n"
				"\texpectation1 description,\n"
				"finally: finalizer description\n"));
	}

	SECTION("When finalizer description is missing.")
	{
		report.finalizerDescription.reset();

		REQUIRE_THAT(
			stringify_expectation_report(std::as_const(report)),
			Matches::Matches(
				"Expectation report:\n"
				"from: .+\\[\\d+:\\d+\\], .+\n"
				"times: times description\n"
				"expects:\n"

				"\texpectation1 description,\n"));
	}

	SECTION("When expectation contains only empty descriptions.")
	{
		report.expectationDescriptions[0].reset();

		REQUIRE_THAT(
			stringify_expectation_report(std::as_const(report)),
			Matches::Matches(
				"Expectation report:\n"
				"from: .+\\[\\d+:\\d+\\], .+\n"
				"times: times description\n"
				"finally: finalizer description\n"));
	}

	SECTION("When expectation contains no descriptions.")
	{
		report.expectationDescriptions.clear();

		REQUIRE_THAT(
			stringify_expectation_report(std::as_const(report)),
			Matches::Matches(
				"Expectation report:\n"
				"from: .+\\[\\d+:\\d+\\], .+\n"
				"times: times description\n"
				"finally: finalizer description\n"));
	}

	SECTION("When expectation contains mixed descriptions.")
	{
		report.expectationDescriptions.emplace_back(std::nullopt);

		REQUIRE_THAT(
			stringify_expectation_report(std::as_const(report)),
			Matches::Matches(
				"Expectation report:\n"
				"from: .+\\[\\d+:\\d+\\], .+\n"
				"times: times description\n"
				"expects:\n"
				"\texpectation1 description,\n"
				"finally: finalizer description\n"));
	}

	SECTION("When expectatin contains no source-location.")
	{
		report.sourceLocation.reset();

		REQUIRE_THAT(
			stringify_expectation_report(std::as_const(report)),
			Matches::Equals(
				"Expectation report:\n"
				"times: times description\n"
				"expects:\n"
				"\texpectation1 description,\n"
				"finally: finalizer description\n"));
	}
}

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
	"[report]"
)
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
						| expect::at_most(1);	// prevent unfulfilled reporting

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
		constexpr auto action = []
		{
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
		constexpr auto action = []
		{
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

		const auto action = []
		{
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
						| expect::at_most(1)
						| ThrowOnMatches<StdException>{};

			REQUIRE_NOTHROW(mock());
			REQUIRE_THAT(
				g_WarningMessage.value(),
				matches::StartsWith("Unhandled exception: what: An std::exception type."));
		}

		SECTION("When an unknown exception is thrown.")
		{
			SCOPED_EXP mock.expect_call()
						| expect::at_most(1)
						| ThrowOnMatches<TestException>{};

			REQUIRE_NOTHROW(mock());
			REQUIRE_THAT(
				g_WarningMessage.value(),
				matches::StartsWith("Unhandled exception: Unknown exception type."));
		}
	}
}
