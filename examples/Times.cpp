// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ExpectationPolicies.hpp"
#include "mimic++/Mock.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE(
	"once() requires the expectation to be matched excatly once.",
	"[example][example::times]"
)
{
	//! [once]
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void()> mock{};
	SCOPED_EXP mock.expect_call()
		| expect::once();

	mock();
	//! [once]
}

TEST_CASE(
	"twice() requires the expectation to be matched excatly twice.",
	"[example][example::times]"
)
{
	//! [twice]
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void()> mock{};
	SCOPED_EXP mock.expect_call()
		| expect::twice();

	mock();	// not enough
	mock();	// fine!
	//! [twice]
}

TEST_CASE(
	"at_least() requires the expectation to be matched at least the specified times.",
	"[example][example::times]"
)
{
	SECTION("With runtime arguments.")
	{
		//! [at_least rt]
		namespace expect = mimicpp::expect;

		mimicpp::Mock<void()> mock{};
		SCOPED_EXP mock.expect_call()
					| expect::at_least(2u);

		mock();	// not enough
		mock();	// fine
		mock();	// also fine!
		//! [at_least rt]
	}

	SECTION("With compile-time arguments.")
	{
		//! [at_least compile-time]
		namespace expect = mimicpp::expect;

		mimicpp::Mock<void()> mock{};
		SCOPED_EXP mock.expect_call()
					| expect::at_least<2u>();

		mock();	// not enough
		mock();	// fine
		mock();	// also fine!
		//! [at_least compile-time]
	}
}

TEST_CASE(
	"at_most() requires the expectation to be matched at most the specified times.",
	"[example][example::times]"
)
{
	SECTION("With runtime arguments.")
	{
		//! [at_most rt]
		namespace expect = mimicpp::expect;

		mimicpp::Mock<void()> mock{};
		SCOPED_EXP mock.expect_call()
					| expect::at_most(2u);

		mock();	// fine
		mock();	// fine but exhausted!
		//! [at_most rt]
	}

	SECTION("With compile-time arguments.")
	{
		//! [at_most compile-time]
		namespace expect = mimicpp::expect;

		mimicpp::Mock<void()> mock{};
		SCOPED_EXP mock.expect_call()
					| expect::at_most<2u>();

		mock();	// fine
		mock();	// fine but exhausted!
		//! [at_most compile-time]
	}
}

TEST_CASE(
	"times() supports a single limit.",
	"[example][example::times]"
)
{
	SECTION("With runtime arguments.")
	{
		//! [times single rt]
		namespace expect = mimicpp::expect;

		mimicpp::Mock<void()> mock{};
		SCOPED_EXP mock.expect_call()
					| expect::times(2u);	// equivalent to twice()

		mock();	// not enough
		mock();	//fine
		//! [times single rt]
	}

	SECTION("With compile-time arguments.")
	{
		//! [times single compile-time]
		namespace expect = mimicpp::expect;

		mimicpp::Mock<void()> mock{};
		SCOPED_EXP mock.expect_call()
					| expect::times<2u>();	// equivalent to twice()

		mock();	// not enough
		mock();	//fine
		//! [times single compile-time]
	}
}

TEST_CASE(
	"times() supports an upper and lower limit.",
	"[example][example::times]"
)
{
	SECTION("With runtime arguments.")
	{
		//! [times rt]
		namespace expect = mimicpp::expect;

		mimicpp::Mock<void()> mock{};
		SCOPED_EXP mock.expect_call()
					| expect::times(1u, 3u);	// between 1 and 3 matches 

		mock();	// fine
		mock();	// fine
		mock();	// fine (exhausted)
		//! [times rt]
	}

	SECTION("With compile-time arguments.")
	{
		//! [times compile-time]
		namespace expect = mimicpp::expect;

		mimicpp::Mock<void()> mock{};
		SCOPED_EXP mock.expect_call()
					| expect::times<1u, 3u>();	// between 1 and 3 matches 

		mock();	// fine
		mock();	// fine
		mock();	// fine (exhausted)
		//! [times compile-time]
	}
}
