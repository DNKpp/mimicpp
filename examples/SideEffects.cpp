// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE(
	"Side-effects can execute arbitrary actions.",
	"[example][example::side-effect]"
)
{
	namespace then = mimicpp::then;

	mimicpp::Mock<void(int)> mock{};

	std::string outText{};
	SCOPED_EXP mock.expect_call(42)
				and then::invoke([&] { outText = "Hello, mimic++!"; });

	mock(42);

	REQUIRE(outText == "Hello, mimic++!");
}

TEST_CASE(
	"Side-effects can modify call params.",
	"[example][example::side-effect]"
)
{
	namespace then = mimicpp::then;

	mimicpp::Mock<void(int&)> mock{};

	int value{42};
	SCOPED_EXP mock.expect_call(42)
				and then::apply_arg<0>([](auto& v) { v = 1337; });

	mock(value);

	REQUIRE(value == 1337);
}

TEST_CASE(
	"Side-effects can modify call params, which is observable by other side-effects.",
	"[example][example::side-effect]"
)
{
	namespace then = mimicpp::then;

	mimicpp::Mock<void(int&)> mock{};

	int value{42};
	SCOPED_EXP mock.expect_call(42)
				and then::apply_arg<0>([](auto& v) { v = 1337; })
				and then::apply_arg<0>([](auto& v) { v = (v == 1337 ? -1337 : -42); });

	mock(value);

	REQUIRE(value == -1337);
}

TEST_CASE(
	"Side-effects can apply the params in any order.",
	"[example][example::side-effect]"
)
{
	namespace then = mimicpp::then;

	mimicpp::Mock<void(int&, int, int)> mock{};

	int outResult{};
	SCOPED_EXP mock.expect_call(0, 1, 2)
				and then::apply_args<1, 2, 0>(	// note the index order:
					// outResult is invoked as left-most param, but applied as right-most
					[](int lhs, int rhs, int& out) { out = lhs + rhs; });

	mock(outResult, 1, 2);

	REQUIRE(outResult == 3);
}

TEST_CASE(
	"Side-effects can apply params multiple times.",
	"[example][example::side-effect]"
)
{
	namespace then = mimicpp::then;

	mimicpp::Mock<void(int&, int)> mock{};

	int outResult{};
	SCOPED_EXP mock.expect_call(0, 42)
				and then::apply_args<1, 1, 0>(	// note the indices: second param is applied twice
					[](int lhs, int rhs, int& out) { out = lhs + rhs; });

	mock(outResult, 42);

	REQUIRE(outResult == 84);
}
