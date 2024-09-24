// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"
#include "TestTypes.hpp"

#include "mimic++/InterfaceMock.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

using namespace mimicpp;

#define TO_STRING_IMPL(...) #__VA_ARGS__
#define TO_STRING(...) TO_STRING_IMPL(__VA_ARGS__)

TEST_CASE(
	"MIMICPP_DETAIL_STRIP_PARENS removes outer parens, if present.",
	"[mock][mock::interface]"
)
{
	namespace Matches = Catch::Matchers;

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_STRIP_PARENS()),
		Matches::Equals(""));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_STRIP_PARENS(())),
		Matches::Equals(""));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_STRIP_PARENS((()))),
		Matches::Equals("()"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_STRIP_PARENS(Test())),
		Matches::Equals("Test()"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_STRIP_PARENS((Test()))),
		Matches::Equals("Test()"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_STRIP_PARENS(((Test())))),
		Matches::Equals("(Test())"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_STRIP_PARENS(((,Test(),)))),
		Matches::Equals("(,Test(),)"));
}

TEST_CASE(
	"MIMICPP_DETAIL_MAKE_SIGNATURE_LIST creates a list of signatures from the given arguments.",
	"[mock][mock::interface]"
)
{
	STATIC_REQUIRE(
		std::same_as<
		std::tuple<>,
		std::tuple< MIMICPP_DETAIL_MAKE_SIGNATURE_LIST() >>);

	STATIC_REQUIRE(
		std::same_as<
		std::tuple<void()>,
		std::tuple< MIMICPP_DETAIL_MAKE_SIGNATURE_LIST((void, (), )) >>);

	STATIC_REQUIRE(
		std::same_as<
		std::tuple<const int&(float&&) const noexcept,
		void()>,
		std::tuple< MIMICPP_DETAIL_MAKE_SIGNATURE_LIST(
			(const int&, (float&&), const noexcept),
			(void, (), )) >>);
}
