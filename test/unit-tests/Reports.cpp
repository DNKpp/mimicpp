// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Reports.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace mimicpp;

TEST_CASE(
	"state_inapplicable is equality comparable.",
	"[reporting]"
)
{
	const state_inapplicable first{
		.min = 43,
		.max = 44,
		.count = 42,
		.sequenceRatings = {
			sequence::rating{1337, sequence::Tag{1338}},
			sequence::rating{1339, sequence::Tag{1340}}
		},
		.inapplicableSequences = {
			sequence::Tag{1341},
			sequence::Tag{1342}
		}
	};

	state_inapplicable second{first};

	SECTION("Compare equal, when all members are equal.")
	{
		REQUIRE(first == second);
		REQUIRE(second == first);
		REQUIRE_FALSE(first != second);
		REQUIRE_FALSE(second != first);
	}

	SECTION("Compare not equal, when min differs.")
	{
		second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when max differs.")
	{
		second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when count differs.")
	{
		second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when sequenceRatings differs.")
	{
		second.sequenceRatings = GENERATE(
			std::vector<sequence::rating>{},
			(std::vector{
				sequence::rating{1337, sequence::Tag{1338}}
				}),
			(std::vector{
				sequence::rating{1339, sequence::Tag{1340}}
				}),
			(std::vector{
				sequence::rating{1339, sequence::Tag{1340}},
				sequence::rating{1337, sequence::Tag{1338}}
				}));

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when inapplicableSequences differs.")
	{
		second.inapplicableSequences = GENERATE(
			std::vector<sequence::Tag>{},
			std::vector{sequence::Tag{1342}},
			std::vector{sequence::Tag{1341}},
			(std::vector{
				sequence::Tag{1342},
				sequence::Tag{1341}
				}));

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}
}

TEST_CASE(
	"state_applicable is equality comparable.",
	"[reporting]"
)
{
	const state_applicable first{
		.min = 43,
		.max = 44,
		.count = 42,
		.sequenceRatings = {
			sequence::rating{1337, sequence::Tag{1338}},
			sequence::rating{1339, sequence::Tag{1340}}
		}
	};

	state_applicable second{first};

	SECTION("Compare equal, when all members are equal.")
	{
		REQUIRE(first == second);
		REQUIRE(second == first);
		REQUIRE_FALSE(first != second);
		REQUIRE_FALSE(second != first);
	}

	SECTION("Compare not equal, when min differs.")
	{
		second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when max differs.")
	{
		second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when count differs.")
	{
		second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when sequenceRatings differs.")
	{
		second.sequenceRatings = GENERATE(
			std::vector<sequence::rating>{},
			(std::vector{
				sequence::rating{1337, sequence::Tag{1338}}
				}),
			(std::vector{
				sequence::rating{1339, sequence::Tag{1340}}
				}),
			(std::vector{
				sequence::rating{1339, sequence::Tag{1340}},
				sequence::rating{1337, sequence::Tag{1338}}
				}));

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}
}

TEST_CASE(
	"state_saturated is equality comparable.",
	"[reporting]"
)
{
	const state_saturated first{
		.min = 43,
		.max = 44,
		.count = 42,
		.sequences = {
			sequence::Tag{1337},
			sequence::Tag{1338}
		}
	};

	state_saturated second{first};

	SECTION("Compare equal, when all members are equal.")
	{
		REQUIRE(first == second);
		REQUIRE(second == first);
		REQUIRE_FALSE(first != second);
		REQUIRE_FALSE(second != first);
	}

	SECTION("Compare not equal, when min differs.")
	{
		second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when max differs.")
	{
		second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when count differs.")
	{
		second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}

	SECTION("Compare not equal, when inapplicableSequences differs.")
	{
		second.sequences = GENERATE(
			std::vector<sequence::Tag>{},
			std::vector{sequence::Tag{1337}},
			std::vector{sequence::Tag{1338}},
			(std::vector{
				sequence::Tag{1338},
				sequence::Tag{1337}
				}));

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second != first);
	}
}

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
		.controlReport = state_applicable{1, 1, 0},
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

		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
		REQUIRE(first != second);
		REQUIRE(second!= first);
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
			(ExpectationT{true, "other expectation description"})
			},
			std::vector{
			(ExpectationT{true, "expectation description"}),
			(ExpectationT{false, "other expectation description"})
			});

		REQUIRE(first != second);
		REQUIRE(second != first);
		REQUIRE_FALSE(first == second);
		REQUIRE_FALSE(second == first);
	}
}

TEST_CASE(
	"evaluate_match_report determines the outcome of a match report.",
	"[reporting][detail]"
)
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
				(std::vector<ExpectationReportT>{{false}, {true}}))
		};

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
				(std::vector<ExpectationReportT>{{true}, {true}}))
		};

		REQUIRE(MatchResult::inapplicable == evaluate_match_report(report));
	}

	SECTION("When all policy match and times is applicable => MatchResult::full is returned.")
	{
		const MatchReport report{
			.controlReport = state_applicable{0, 1, 0},
			.expectationReports = GENERATE(
				(std::vector<ExpectationReportT>{}),
				(std::vector<ExpectationReportT>{{true}}),
				(std::vector<ExpectationReportT>{{true}, {true}}))
		};

		REQUIRE(MatchResult::full == evaluate_match_report(report));
	}
}

TEST_CASE(
	"stringify_match_report converts the match report to text representation.",
	"[report][detail]"
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
				.controlReport = state_applicable{0, 1, 0},
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
				.controlReport = state_applicable{0, 1, 0},
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
			SECTION("Is saturated.")
			{
				const MatchReport report{
					.sourceLocation = std::source_location::current(),
					.finalizeReport = {},
					.controlReport = state_saturated{0, 42, 42},
					.expectationReports = {}
				};

				REQUIRE_THAT(
					stringify_match_report(report),
					Matches::Matches(
						"Inapplicable, but otherwise matched expectation: \\{\n"
						"reason: already saturated \\(matched 42 times\\)\n"
						"from: .+\\[\\d+:\\d+\\], .+\n"
						"\\}\n"));
			}

			SECTION("Is inapplicable.")
			{
				const MatchReport report{
					.sourceLocation = std::source_location::current(),
					.finalizeReport = {},
					.controlReport = state_inapplicable{
						.min = 0,
						.max = 42,
						.count = 5,
						.sequenceRatings = {
							sequence::rating{0, sequence::Tag{123}}
						},
						.inapplicableSequences = {
							sequence::Tag{1337},
							sequence::Tag{1338}
						}
					},
					.expectationReports = {}
				};

				REQUIRE_THAT(
					stringify_match_report(report),
					Matches::Matches(
						"Inapplicable, but otherwise matched expectation: \\{\n"
						"reason: accepts further matches \\(matched 5 out of 42 times\\),\n"
						"\tbut is not the current element of 2 sequence\\(s\\) \\(3 total\\).\n"
						"from: .+\\[\\d+:\\d+\\], .+\n"
						"\\}\n"));
			}
		}

		SECTION("When contains requirements.")
		{
			const MatchReport report{
				.sourceLocation = std::source_location::current(),
				.finalizeReport = {},
				.controlReport = state_saturated{0, 42, 42},
				.expectationReports = {
					{true, "Requirement1 description"},
					{true, "Requirement2 description"}
				}
			};

			REQUIRE_THAT(
				stringify_match_report(report),
				Matches::Matches(
					"Inapplicable, but otherwise matched expectation: \\{\n"
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
				.sourceLocation = std::source_location::current(),
				.finalizeReport = {},
				.controlReport = state_applicable{0, 1, 0},
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
				.controlReport = state_applicable{0, 1, 0},
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
			.controlReport = state_applicable{0, 1, 0},
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
	"[report][detail]"
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
	"[report][detail]"
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
