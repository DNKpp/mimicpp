// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"

#include "../test/TestReporter.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE(
	"Requirements can be specified for each argument.",
	"[example][example::requirements]"
)
{
	namespace matches = mimicpp::matches;

	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(matches::eq(42));
	mock(42);
}

TEST_CASE(
	"A simplified syntax for the matches::eq matcher is available.",
	"[example][example::requirements]"
)
{
	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(42);
	mock(42);
}

TEST_CASE(
	"The wildcard matcher matches everything.",
	"[example][example::requirements]"
)
{
	using mimicpp::matches::_;

	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(_);
	mock(1337);
}

TEST_CASE(
	"Requirements can be specified later via expect::arg<n>.",
	"[example][example::requirements]"
)
{
	using mimicpp::matches::_;
	namespace matches = mimicpp::matches;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(_)
				| expect::arg<0>(matches::gt(42));
	mock(1337);
}

TEST_CASE(
	"Both requirement types can be combined.",
	"[example][example::requirements]"
)
{
	using mimicpp::matches::le;
	namespace matches = mimicpp::matches;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(le(1337))
				| expect::arg<0>(matches::gt(42));
	mock(1337);
}

TEST_CASE(
	"Most requirements can be inverted.",
	"[example][example::requirements]"
)
{
	using mimicpp::matches::_;
	namespace matches = mimicpp::matches;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(_)	// in fact, the _ is the only built-in matcher, which isn't invertible
				| expect::arg<0>(!matches::le(42));	// note the !
	mock(1337);
}

TEST_CASE(
	"Custom predicates can be checked.",
	"[example][example::requirements]"
)
{
	using mimicpp::matches::_;
	namespace matches = mimicpp::matches;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void(int)> mock{};

	constexpr auto isOdd = [](int val) { return 0 != val % 2; };

	SCOPED_EXP mock.expect_call(_)
				| expect::arg<0>(matches::predicate(isOdd));
	mock(1337);
}

TEST_CASE(
	"Ranges can be checked.",
	"[example][example::requirements]"
)
{
	using mimicpp::matches::_;
	namespace matches = mimicpp::matches;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void(std::span<int>)> mock{};

	SCOPED_EXP mock.expect_call(_)
				| expect::arg<0>(matches::range::is_sorted());

	std::vector collection{42, 1337};
	mock(collection);
}

TEST_CASE(
	"Custom matchers can be easily composed via the generic PredicateMatcher.",
	"[example][example::requirements]"
)
{
	using mimicpp::matches::_;
	namespace matches = mimicpp::matches;
	namespace expect = mimicpp::expect;

	mimicpp::PredicateMatcher containsMatcher{
		// provide a test predicate
		[](const auto& target, const auto& element) // the left most param is the argument to be checked
		{
			return std::ranges::find(target, element) != std::ranges::end(target);
		},
		// specify a descriptive format message, which will be applied to std::format.
		"range {} contains the element {}",
		// capture additional data, which will be forwarded to both, the predicate and the description
		std::tuple{42}
	};

	mimicpp::Mock<void(std::span<int>)> mock{};

	SCOPED_EXP mock.expect_call(_)
				| expect::arg<0>(containsMatcher);

	std::vector collection{42, 1337};
	mock(collection);
}
