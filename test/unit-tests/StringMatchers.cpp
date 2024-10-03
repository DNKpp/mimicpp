// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Matcher.hpp"
#include "mimic++/Printer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace matches = mimicpp::matches;

TEST_CASE(
	"matches::str::eq matches when target string compares equal to the stored one.",
	"[matcher][matcher::str]"
)
{
	const auto matcher = matches::str::eq("Hello, World!");
	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("is equal to \"Hello, World!\""));

	SECTION("When target is equal, they match.")
	{
		const std::string target{"Hello, World!"};

		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is not equal, they do not match.")
	{
		const std::string target{"Hello, WOrld!"};

		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::str::eq("Hello, World!");

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("is not equal to \"Hello, World!\""));

		SECTION("When target is equal, they do not match.")
		{
			const std::string target{"Hello, World!"};

			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is not equal, they do match.")
		{
			const std::string target{"Hello, WOrld!"};

			REQUIRE(invertedMatcher.matches(target));
		}
	}
}
