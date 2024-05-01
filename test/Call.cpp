// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Call.hpp"
#include "mimic++/Printer.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace mimicpp;

// Todo: test call::detail::evaluate_sub_match_results

TEST_CASE(
	"call::Info is equality comparable.",
	"[call]"
)
{
	using CallInfoT = call::Info<void, const int>;

	constexpr auto loc1 = std::source_location::current();
	constexpr auto loc2 = std::source_location::current();

	// ReSharper disable once CppVariableCanBeMadeConstexpr
	static int value1{42};
	// ReSharper disable once CppVariableCanBeMadeConstexpr
	static int value2{42};

	const CallInfoT info{
		.args = {std::ref(value1)},
		.fromCategory = ValueCategory::any,
		.fromConstness = Constness::any,
		.fromSourceLocation = loc1
	};

	const auto& [expected, other] = GENERATE_REF(
		(table<bool, CallInfoT>)({
			{false, CallInfoT{std::tuple{std::ref(value2)}, ValueCategory::any, Constness::any, loc1}},
			{false, CallInfoT{std::tuple{std::ref(value1)}, ValueCategory::lvalue, Constness::any, loc1}},
			{false, CallInfoT{std::tuple{std::ref(value1)}, ValueCategory::any, Constness::as_const, loc1}},
			{false, CallInfoT{std::tuple{std::ref(value1)}, ValueCategory::any, Constness::any, loc2}},
			{true, CallInfoT{std::tuple{std::ref(value1)}, ValueCategory::any, Constness::any, loc1}}
			}));

	REQUIRE(expected == (info == other));
	REQUIRE(expected == (other == info));
	REQUIRE(expected == !(info != other));
	REQUIRE(expected == !(other != info));
}

TEST_CASE(
	"call::MatchCategory is formattable.",
	"[call]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("When valid category is given.")
	{
		const auto [expected, category] = GENERATE(
			(table<StringT, call::MatchCategory>)({
				{"no match", call::MatchCategory::no},
				{"non applicable match", call::MatchCategory::non_applicable},
				{"full match", call::MatchCategory::ok},
			}));

		REQUIRE_THAT(
			format::format("{}", category),
			Matches::Equals(expected));
	}

	SECTION("When an invalid category is given, std::invalid_argument is thrown.")
	{
		REQUIRE_THROWS_AS(
			format::format("{}", call::MatchCategory{42}),
			std::invalid_argument);
	}
}
