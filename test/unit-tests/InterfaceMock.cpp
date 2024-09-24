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

TEST_CASE(
	"MIMICPP_DETAIL_MAKE_OVERLOADED_MOCK creates a mock from a list of signatures.",
	"[mock][mock::interface]"
)
{
	SECTION("Just void()")
	{
		MIMICPP_DETAIL_MAKE_OVERLOADED_MOCK(
			mock,
			( void() ));

		REQUIRE(std::invocable<decltype(mock)>);
	}

	SECTION("Just float&(int&&)")
	{
		MIMICPP_DETAIL_MAKE_OVERLOADED_MOCK(
			mock,
			( float&(int&&) ));

		REQUIRE(std::invocable<decltype(mock), int&&>);
		REQUIRE(std::same_as<float&, std::invoke_result_t<decltype(mock), int&&>>);
	}
}

TEST_CASE(
	"MIMICPP_DETAIL_MAKE_PARAM_LIST creates the param list for the given types.",
	"[mock][mock::interface]"
)
{
	namespace Matches = Catch::Matchers;

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST()),
		Matches::Equals(""));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST(int)),
		Matches::Equals("int arg_i"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST(const int&, int&&)),
		Matches::Equals("const int& arg_i, int&& arg_ii"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST(int, int)),
		Matches::Equals("int arg_i, int arg_ii"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST((std::tuple<int, float>))),
		Matches::Equals("std::tuple<int, float> arg_i"));
}

TEST_CASE(
	"MIMICPP_DETAIL_FORWARD_ARGS a list of forwarded arguments.",
	"[mock][mock::interface]"
)
{
	namespace Matches = Catch::Matchers;

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_FORWARD_ARGS()),
		Matches::Equals(""));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_FORWARD_ARGS(int)),
		Matches::Equals(", ::std::forward<::std::add_rvalue_reference_t< int>>(arg_i)"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_FORWARD_ARGS(const int&, int&&)),
		Matches::Equals(
			", ::std::forward<::std::add_rvalue_reference_t< const int&>>(arg_i),"
			" ::std::forward<::std::add_rvalue_reference_t< int&&>>(arg_ii)"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_FORWARD_ARGS(int, int)),
		Matches::Equals(", ::std::forward<::std::add_rvalue_reference_t< int>>(arg_i),"
			" ::std::forward<::std::add_rvalue_reference_t< int>>(arg_ii)"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_DETAIL_FORWARD_ARGS((std::tuple<int, float>))),
		Matches::Equals(", ::std::forward<::std::add_rvalue_reference_t< std::tuple<int, float>>>(arg_i)"));
}

TEST_CASE(
	"MIMICPP_ADD_OVERLOAD prepares the necessary overload infos.",
	"[mock][mock::interface]"
)
{
	namespace Matches = Catch::Matchers;

	REQUIRE_THAT(
		TO_STRING(MIMICPP_ADD_OVERLOAD(void, ())),
		Matches::Equals("(void, (),, (), ())"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_ADD_OVERLOAD(const int&&, (const std::string&, int&&), const noexcept)),
		Matches::Equals(
			"(const int&&, (const std::string&, int&&), const noexcept, "
			"(const std::string& arg_i, int&& arg_ii), "
			"(, ::std::forward<::std::add_rvalue_reference_t< const std::string&>>(arg_i), "
			"::std::forward<::std::add_rvalue_reference_t< int&&>>(arg_ii)))"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_ADD_OVERLOAD((std::tuple<int, float>), ())),
		Matches::Equals("((std::tuple<int, float>), (),, (), ())"));

	REQUIRE_THAT(
		TO_STRING(MIMICPP_ADD_OVERLOAD(void, ((std::tuple<int, float>)))),
		Matches::Equals(
			"(void, ((std::tuple<int, float>)),, "
			"(std::tuple<int, float> arg_i), "
			"(, ::std::forward<::std::add_rvalue_reference_t< std::tuple<int, float>>>(arg_i)))"));
}
