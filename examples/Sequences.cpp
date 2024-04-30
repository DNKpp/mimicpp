// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/Sequence.hpp"

#include "../test/TestReporter.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE(
	"Sequences allow strong ordering between two or more expectations.",
	"[example][example::sequence]"
)
{
	//! [sequence]
	namespace matches = mimicpp::matches;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void(int)> mock{};

	mimicpp::Sequence sequence{};
	SCOPED_EXP mock.expect_call(matches::ne(0))
		| expect::in_sequence(sequence);
	SCOPED_EXP mock.expect_call(matches::le(42))
		| expect::in_sequence(sequence);

	// a call with arg != 0 is expected before a call with arg <= 42
	mock(42); // matches the first expectation
	mock(0); // matches the second expectation

	// note: when we change the call order, the sequence would be violated
	//! [sequence]
}

TEST_CASE(
	"Sequences can be used for multiple mocks.",
	"[example][example::sequence]"
)
{
	//! [sequence multiple mocks]
	using mimicpp::matches::_;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void(int)> mock1{};
	mimicpp::Mock<void(int)> mock2{};

	mimicpp::Sequence sequence{};
	SCOPED_EXP mock2.expect_call(_)	// mock2 must go first
		| expect::in_sequence(sequence);
	SCOPED_EXP mock1.expect_call(_)	// mock1 must go second
		| expect::in_sequence(sequence);

	mock2(42);
	mock1(1337);
	//! [sequence multiple mocks]
}

TEST_CASE(
	"Sequenced and non-sequenced expectations can be mixed.",
	"[example][example::sequence]"
)
{
	//! [sequence mixed]
	using mimicpp::matches::_;
	namespace expect = mimicpp::expect;

	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(42); // (1)

	SCOPED_EXP mock.expect_call(1337); // (2)

	mimicpp::Sequence sequence{};
	SCOPED_EXP mock.expect_call(1337)  // (3)
		| expect::in_sequence(sequence);

	SCOPED_EXP mock.expect_call(1337); // (4)

	SCOPED_EXP mock.expect_call(42)  // (5)
		| expect::in_sequence(sequence);

	mock(42); // matches (1), because (5) is the second element of the sequence
	mock(1337); // matches (4), because it's the "youngest" available alternative
	mock(1337); // matches (3), because it's "younger" than (2). So, the first element of the sequence got matched.
	mock(42); // matches (5). The sequence is now fulfilled.
	mock(1337); // finally matches (2)
	//! [sequence mixed]
}
