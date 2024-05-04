// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Reporter.hpp"

#include <catch2/catch_test_macros.hpp>
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
