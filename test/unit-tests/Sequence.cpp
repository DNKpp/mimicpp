// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"
#include "TestTypes.hpp"

#include "mimic++/Sequence.hpp"
#include "mimic++/Expectation.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace mimicpp;

namespace
{
	using sequence::Id;
}

TEST_CASE(
	"detail::BasicSequence is default constructible, but immobile.",
	"[sequence]"
)
{
	using TestSequenceT = sequence::detail::BasicSequence<Id, FakeSequenceStrategy{}>;

	STATIC_REQUIRE(std::is_default_constructible_v<TestSequenceT>);

	STATIC_REQUIRE(!std::is_copy_constructible_v<TestSequenceT>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<TestSequenceT>);
	STATIC_REQUIRE(!std::is_move_constructible_v<TestSequenceT>);
	STATIC_REQUIRE(!std::is_move_assignable_v<TestSequenceT>);
}

TEMPLATE_TEST_CASE(
	"Sequence interfaces are default constructible, but immobile.",
	"[sequence]",
	LazySequence,
	GreedySequence,
	SequenceT
)
{
	STATIC_REQUIRE(std::is_default_constructible_v<TestType>);

	STATIC_REQUIRE(!std::is_copy_constructible_v<TestType>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<TestType>);
	STATIC_REQUIRE(!std::is_move_constructible_v<TestType>);
	STATIC_REQUIRE(!std::is_move_assignable_v<TestType>);
}

TEST_CASE(
	"detail::BasicSequence::add throws, when all Ids are in use.",
	"[sequence]"
)
{
	enum class ShortSequenceId
		: std::int8_t
	{
	};

	sequence::detail::BasicSequence<
		ShortSequenceId,
		FakeSequenceStrategy{}> seq{};

	for ([[maybe_unused]] const auto i : std::views::iota(
			0,
			int{std::numeric_limits<std::int8_t>::max()} + 1))
	{
		seq.set_satisfied(seq.add());
	}

	REQUIRE_THROWS_AS(
		seq.add(),
		std::runtime_error);
}

TEST_CASE(
	"detail::BasicSequence supports an arbitrary id amount.",
	"[sequence]"
)
{
	namespace Matches = Catch::Matchers;

	using TestSequenceT = sequence::detail::BasicSequence<Id, FakeSequenceStrategy{}>;

	ScopedReporter reporter{};
	std::optional<TestSequenceT> sequence{std::in_place};

	SECTION("When Sequence contains zero elements.")
	{
		REQUIRE_NOTHROW(sequence.reset());
	}

	static constexpr std::array consumeStateActions = std::to_array(
		{
			+[](TestSequenceT&, const Id) { assert(true); },
			+[](TestSequenceT& seq, const Id v) { seq.consume(v); }
		});

	SECTION("When sequence contains one id, that id must be satisfied.")
	{
		const Id id = sequence->add();
		REQUIRE(sequence->is_consumable(id));

		std::invoke(
			GENERATE(from_range(consumeStateActions)),
			*sequence,
			id);

		SECTION("Reports error, when id is unsatisfied.")
		{
			sequence.reset();
			REQUIRE_THAT(
				reporter.errors().front(),
				Matches::Equals("Unfulfilled sequence. 0 out of 1 expectation(s) are satisfied."));
		}

		SECTION("Reports nothing, when id is satisfied.")
		{
			sequence->set_satisfied(id);

			REQUIRE(sequence->is_consumable(id));

			sequence.reset();
			REQUIRE_THAT(
				reporter.errors(),
				Matches::IsEmpty());
		}

		SECTION("Reports nothing, when id is saturated.")
		{
			sequence->set_saturated(id);

			REQUIRE(!sequence->is_consumable(id));

			sequence.reset();
			REQUIRE_THAT(
				reporter.errors(),
				Matches::IsEmpty());
		}
	}

	SECTION("When sequence contains multiple ids.")
	{
		static constexpr std::array alterStateActions = std::to_array(
			{
				+[](TestSequenceT&, const Id) { assert(true); },
				+[](TestSequenceT& seq, const Id v) { seq.set_satisfied(v); },
				+[](TestSequenceT& seq, const Id v) { seq.set_saturated(v); }
			});

		const std::vector ids{
			sequence->add(),
			sequence->add(),
			sequence->add()
		};

		SECTION("In initial state, only the first one is consumable.")
		{
			REQUIRE(sequence->is_consumable(ids[0]));
			REQUIRE(sequence->priority_of(ids[0]));

			std::invoke(
				GENERATE(from_range(alterStateActions)),
				*sequence,
				ids[1]);
			REQUIRE(!sequence->is_consumable(ids[1]));
			REQUIRE(!sequence->priority_of(ids[1]));

			std::invoke(
				GENERATE(from_range(alterStateActions)),
				*sequence,
				ids[2]);
			REQUIRE(!sequence->is_consumable(ids[2]));
			REQUIRE(!sequence->priority_of(ids[2]));

			REQUIRE_NOTHROW(sequence->consume(ids[0]));

			sequence.reset();
			REQUIRE_THAT(
				reporter.errors().front(),
				Matches::Equals("Unfulfilled sequence. 0 out of 3 expectation(s) are satisfied."));
		}

		SECTION("If first is either satisfied or saturated, the second one becomes consumable.")
		{
			std::invoke(
				GENERATE(from_range(consumeStateActions)),
				*sequence,
				ids[0]);

			const auto secondStateAction = GENERATE(from_range(alterStateActions));

			SECTION("When first is satisfied.")
			{
				sequence->set_satisfied(ids[0]);

				REQUIRE(sequence->is_consumable(ids[0]));
				REQUIRE(sequence->priority_of(ids[0]));
			}

			SECTION("When first is saturated.")
			{
				sequence->set_saturated(ids[0]);

				REQUIRE(!sequence->is_consumable(ids[0]));
				REQUIRE(!sequence->priority_of(ids[0]));
			}

			REQUIRE(sequence->is_consumable(ids[1]));
			REQUIRE(sequence->priority_of(ids[1]));

			std::invoke(
				secondStateAction,
				*sequence,
				ids[2]);
			REQUIRE(!sequence->is_consumable(ids[2]));
			REQUIRE(!sequence->priority_of(ids[2]));

			sequence.reset();
			REQUIRE_THAT(
				reporter.errors().front(),
				Matches::Equals("Unfulfilled sequence. 1 out of 3 expectation(s) are satisfied."));
		}

		SECTION("If first and second are either satisfied or saturated, the last one becomes consumable.")
		{
			std::invoke(
				GENERATE(from_range(alterStateActions | std::views::drop(1))),
				*sequence,
				ids[0]);

			std::invoke(
				GENERATE(from_range(alterStateActions | std::views::drop(1))),
				*sequence,
				ids[1]);

			REQUIRE(sequence->is_consumable(ids[2]));
			REQUIRE(sequence->priority_of(ids[2]));

			// jumps directly from 0 to 2
			sequence->consume(ids[2]);

			SECTION("Reports error, when last is unfulfilled.")
			{
				sequence.reset();
				REQUIRE_THAT(
					reporter.errors().front(),
					Matches::Equals("Unfulfilled sequence. 2 out of 3 expectation(s) are satisfied."));
			}

			SECTION("Succeeds, when last is either satisfied or saturated.")
			{
				std::invoke(
					GENERATE(from_range(alterStateActions | std::views::drop(1))),
					*sequence,
					ids[2]);

				sequence.reset();
				REQUIRE_THAT(
					reporter.errors(),
					Matches::IsEmpty());
			}
		}
	}
}

TEST_CASE(
	"detail::BasicSequence::tag returns its opaque address.",
	"[sequence]"
)
{
	const sequence::detail::BasicSequence<Id, FakeSequenceStrategy{}> sequence{};
	const sequence::Tag tag = sequence.tag();

	REQUIRE(to_underlying(tag) == std::bit_cast<std::ptrdiff_t>(std::addressof(sequence)));
}

TEST_CASE(
	"detail::LazyStrategy prefers elements near cursor.",
	"[sequence]"
)
{
	const auto [expected, id, cursor] = GENERATE(
		(table<int, Id, int>({
			{std::numeric_limits<int>::max(), Id{0}, 0},
			{std::numeric_limits<int>::max(), Id{1}, 1},
			{std::numeric_limits<int>::max() - 1, Id{1}, 0},
			{std::numeric_limits<int>::max() - 2, Id{2}, 0},
			{std::numeric_limits<int>::max() - 1, Id{2}, 1}
			})));

	REQUIRE(expected == std::invoke(sequence::detail::LazyStrategy{}, id, cursor));
}

TEST_CASE(
	"detail::GreedyStrategy prefers elements far from cursor.",
	"[sequence]"
)
{
	const auto [expected, id, cursor] = GENERATE(
		(table<int, Id, int>({
			{0, Id{0}, 0},
			{0, Id{1}, 1},
			{1, Id{1}, 0},
			{2, Id{2}, 0},
			{1, Id{2}, 1}
			})));

	REQUIRE(expected == std::invoke(sequence::detail::GreedyStrategy{}, id, cursor));
}

TEST_CASE(
	"detail::has_better_rating determines, whether lhs shall be preferred over rhs.",
	"[detail][sequence]"
)
{
	using sequence::rating;
	using sequence::detail::has_better_rating;
	constexpr std::array sequence_tags = std::to_array<>(
		{
			sequence::Tag{1},
			sequence::Tag{2},
			sequence::Tag{3},
		});

	SECTION("Lhs with zero ratings is always preferred.")
	{
		const std::vector<rating> lhs{};

		std::vector<rating> rhs{};
		REQUIRE(has_better_rating(lhs, rhs));

		rhs.emplace_back(std::numeric_limits<int>::max(), sequence_tags[0]);
		REQUIRE(has_better_rating(lhs, rhs));

		rhs.emplace_back(0, sequence_tags[1]);
		REQUIRE(has_better_rating(lhs, rhs));
	}

	SECTION("When lhs has single rating.")
	{
		const std::vector<rating> lhs{
			{42, sequence_tags[1]}
		};

		std::vector<rating> rhs{};
		REQUIRE(has_better_rating(lhs, rhs));

		rhs = lhs;
		REQUIRE(has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.front().priority = 41;
		REQUIRE(has_better_rating(lhs, rhs));
		REQUIRE(!has_better_rating(rhs, lhs));

		rhs.front().priority = 43;
		REQUIRE(!has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.emplace(rhs.begin(), std::numeric_limits<int>::max(), sequence_tags[0]);
		REQUIRE(!has_better_rating(lhs, rhs));
	}

	SECTION("When lhs has two ratings.")
	{
		const std::vector<rating> lhs{
			{42, sequence_tags[1]},
			{1337, sequence_tags[0]}
		};

		std::vector rhs = lhs;
		REQUIRE(has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.front().priority += 1;
		REQUIRE(has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs = lhs;
		rhs.back().priority += 1;
		REQUIRE(has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.front().priority += 1;
		REQUIRE(!has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.emplace(rhs.begin() + 1, 0, sequence_tags[2]);
		REQUIRE(!has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));
	}

	SECTION("When lhs has three ratings.")
	{
		const std::vector<rating> lhs{
			{42, sequence_tags[1]},
			{25, sequence_tags[2]},
			{1337, sequence_tags[0]}
		};

		std::vector<rating> rhs{};
		REQUIRE(has_better_rating(lhs, rhs));

		rhs.emplace_back(1338, sequence_tags[0]);
		REQUIRE(has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.emplace_back(43, sequence_tags[1]);
		REQUIRE(has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.emplace_back(26, sequence_tags[2]);
		REQUIRE(!has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.front().priority -= 2;
		REQUIRE(!has_better_rating(lhs, rhs));
		REQUIRE(has_better_rating(rhs, lhs));

		rhs.back().priority -= 2;
		REQUIRE(has_better_rating(lhs, rhs));
		REQUIRE(!has_better_rating(rhs, lhs));
	}
}

TEST_CASE(
	"expect::in_sequence creates detail::SequenceConfig.",
	"[expectation::factories][sequence]"
)
{
	SequenceT sequence{};

	const sequence::detail::Config config = expect::in_sequence(sequence);

	REQUIRE(std::get<0>(config.sequences())->tag() == sequence.tag());
}

TEST_CASE(
	"expect::in_sequences creates detail::SequenceConfig.",
	"[expectation::factories][sequence]"
)
{
	LazySequence firstSequence{};
	GreedySequence secondSequence{};
	SequenceT thirdSequence{};

	SECTION("Throws, when duplicates are given.")
	{
		REQUIRE_THROWS_AS(
			expect::in_sequences(
				firstSequence,
				firstSequence),
			std::invalid_argument);

		REQUIRE_THROWS_AS(
			expect::in_sequences(
				firstSequence,
				secondSequence,
				firstSequence),
			std::invalid_argument);
	}

	SECTION("When two sequences are given.")
	{
		const sequence::detail::Config config = expect::in_sequences(
			firstSequence,
			secondSequence);

		REQUIRE(std::get<0>(config.sequences())->tag() == firstSequence.tag());
		REQUIRE(std::get<1>(config.sequences())->tag() == secondSequence.tag());
	}

	SECTION("When three sequences are given.")
	{
		const sequence::detail::Config config = expect::in_sequences(
			firstSequence,
			thirdSequence,
			secondSequence);

		REQUIRE(std::get<0>(config.sequences())->tag() == firstSequence.tag());
		REQUIRE(std::get<1>(config.sequences())->tag() == thirdSequence.tag());
		REQUIRE(std::get<2>(config.sequences())->tag() == secondSequence.tag());
	}
}

TEST_CASE(
	"detail::SequenceConfig::concat combines two config.",
	"[sequence]")
{
	const sequence::detail::Config<> firstConfig{};

	SECTION("Concat two empty sequences is pointless but possible.")
	{
		[[maybe_unused]] const sequence::detail::Config<> result = firstConfig.concat(
			sequence::detail::Config<>{});
	}

	SECTION("Concat appends the right side.")
	{
		SequenceT firstSequence{};
		const sequence::detail::Config firstResult = firstConfig.concat(
			expect::in_sequence(firstSequence));

		REQUIRE(std::get<0>(firstResult.sequences())->tag() == firstSequence.tag());

		SequenceT secondSequence{};
		sequence::detail::Config secondResult = firstResult.concat(
			expect::in_sequence(secondSequence));
		REQUIRE(std::get<0>(secondResult.sequences())->tag() == firstSequence.tag());
		REQUIRE(std::get<1>(secondResult.sequences())->tag() == secondSequence.tag());

		SECTION("Throws, when contains duplicates.")
		{
			REQUIRE_THROWS_AS(
				secondResult.concat(
					expect::in_sequence(firstSequence)),
				std::invalid_argument);
			REQUIRE_THROWS_AS(
				secondResult.concat(
					expect::in_sequence(secondSequence)),
				std::invalid_argument);
		}

		SECTION("Can be arbitrarily continued.")
		{
			SequenceT thirdSequence{};
			sequence::detail::Config thirdResult = secondResult.concat(
				expect::in_sequence(thirdSequence));
			REQUIRE(std::get<0>(thirdResult.sequences())->tag() == firstSequence.tag());
			REQUIRE(std::get<1>(thirdResult.sequences())->tag() == secondSequence.tag());
			REQUIRE(std::get<2>(thirdResult.sequences())->tag() == thirdSequence.tag());
		}
	}
}
