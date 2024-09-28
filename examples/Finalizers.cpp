// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"

#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

TEST_CASE(
	"Finalizers can return pre-defined values.",
	"[example][example::finalizer]"
)
{
	//! [finally::returns]
	namespace finally = mimicpp::finally;

	mimicpp::Mock<int()> mock{};

	SCOPED_EXP mock.expect_call()
				and finally::returns(42);

	REQUIRE(42 == mock());
	//! [finally::returns]
}

TEST_CASE(
	"Returning finalizers are aware of std::reference_wrapper.",
	"[example][example::finalizer]"
)
{
	//! [finally::returns std::ref]
	namespace finally = mimicpp::finally;

	mimicpp::Mock<int&()> mock{};

	int myReturn{42};
	SCOPED_EXP mock.expect_call()
				and finally::returns(std::ref(myReturn));

	REQUIRE(&myReturn == &mock());
	//! [finally::returns std::ref]
}

TEST_CASE(
	"Storing returned reference is fine, as long as the expectation is alive.",
	"[example][example::finalizer]"
)
{
	//! [finally::returns ref]
	namespace finally = mimicpp::finally;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<int&()> mock{};

	SCOPED_EXP mock.expect_call()
				and expect::twice()	// we call the mock two times
				and finally::returns(42);

	int& result = mock();
	REQUIRE(42 == result);	// fine

	result = 1337;
	REQUIRE(1337 == mock());
	//! [finally::returns ref]
}

TEST_CASE(
	"Finalizers can throw exceptions.",
	"[example][example::finalizer]"
)
{
	//! [finally::throws]
	namespace finally = mimicpp::finally;

	mimicpp::Mock<int()> mock{};

	SCOPED_EXP mock.expect_call()
				and finally::throws(std::runtime_error{"Something happened."});

	REQUIRE_THROWS_AS(
		mock(),
		std::runtime_error);
	//! [finally::throws]
}

TEST_CASE(
	"Finalizers can return the result of a function call.",
	"[example][example::finalizer]"
)
{
	//! [finally::returns_result_of]
	namespace finally = mimicpp::finally;

	mimicpp::Mock<std::string()> mock{};

	SCOPED_EXP mock.expect_call()
				and finally::returns_result_of([] { return "Hello, World!"; });

	REQUIRE("Hello, World!" == mock());
	//! [finally::returns_result_of]
}

TEST_CASE(
	"Finalizers can return any given argument.",
	"[example][example::finalizer]"
)
{
	//! [finally::returns_param]
	namespace finally = mimicpp::finally;

	mimicpp::Mock<int(int, int)> mock{};

	SCOPED_EXP mock.expect_call(1337, 42)
				and finally::returns_arg<1>();

	REQUIRE(42 == mock(1337, 42));
	//! [finally::returns_param]
}

TEST_CASE(
	"Finalizers can apply any argument to a function and then return the invocation result.",
	"[example][example::finalizer]"
)
{
	//! [finally::returns_apply_result_of]
	namespace finally = mimicpp::finally;

	mimicpp::Mock<int(int, int)> mock{};

	SCOPED_EXP mock.expect_call(1337, 42)
				and finally::returns_apply_result_of<0, 1>(std::plus{});

	REQUIRE(1379 == mock(1337, 42));
	//! [finally::returns_apply_result_of]
}

TEST_CASE(
	"Finalizers can apply all argument to a function and then return the invocation result.",
	"[example][example::finalizer]"
)
{
	//! [finally::returns_apply_all_result_of]
	namespace finally = mimicpp::finally;

	mimicpp::Mock<int(int, int)> mock{};

	SCOPED_EXP mock.expect_call(1337, 42)
				and finally::returns_apply_all_result_of(std::plus{});

	REQUIRE(1379 == mock(1337, 42));
	//! [finally::returns_apply_all_result_of]
}

TEST_CASE(
	"Custom finalizers are supported.",
	"[example][example::finalizer]"
)
{
	//! [custom finalizer]
	class MyFinalizer
	{
	public:
		int finalize_call([[maybe_unused]] const mimicpp::call::Info<int>& callInfo)
		{
			return 1337;
		}
	};

	mimicpp::Mock<int()> mock{};

	SCOPED_EXP mock.expect_call()
				and MyFinalizer{};

	REQUIRE(1337 == mock());
	//! [custom finalizer]
}
