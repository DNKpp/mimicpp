// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"

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
	class FakeStrategy
	{
	public:
		[[nodiscard, maybe_unused]]
		constexpr int operator ()(const auto id, [[maybe_unused]] const int cursor) const noexcept
		{
			return static_cast<int>(id);
		}
	};

	using sequence::detail::SequenceId;
}

TEST_CASE(
	"detail::BasicSequence is default constructible, but immobile.",
	"[sequence]"
)
{
	using SequenceT = sequence::detail::BasicSequence<SequenceId, FakeStrategy{}>;

	STATIC_REQUIRE(std::is_default_constructible_v<SequenceT>);

	STATIC_REQUIRE(!std::is_copy_constructible_v<SequenceT>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<SequenceT>);
	STATIC_REQUIRE(!std::is_move_constructible_v<SequenceT>);
	STATIC_REQUIRE(!std::is_move_assignable_v<SequenceT>);
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
		FakeStrategy{}> seq{};

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

	using SequenceT = sequence::detail::BasicSequence<SequenceId, FakeStrategy{}>;

	ScopedReporter reporter{};
	std::optional<SequenceT> sequence{std::in_place};

	SECTION("When Sequence contains zero elements.")
	{
		REQUIRE_NOTHROW(sequence.reset());
	}

	static constexpr std::array consumeStateActions = std::to_array(
		{
			+[](SequenceT&, const SequenceId) { assert(true); },
			+[](SequenceT& seq, const SequenceId v) { seq.consume(v); }
		});

	SECTION("When sequence contains one id, that id must be satisfied.")
	{
		const SequenceId id = sequence->add();
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
				+[](SequenceT&, const SequenceId) { assert(true); },
				+[](SequenceT& seq, const SequenceId v) { seq.set_satisfied(v); },
				+[](SequenceT& seq, const SequenceId v) { seq.set_saturated(v); }
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
	"sequence"
)
{
	const sequence::detail::BasicSequence<SequenceId, FakeStrategy{}> sequence{};
	const sequence::SequenceTag tag = sequence.tag();

	REQUIRE(to_underlying(tag) == reinterpret_cast<std::ptrdiff_t>(std::addressof(sequence)));
}

TEST_CASE(
	"detail::LazyStrategy prefers elements near cursor.",
	"[sequence]"
)
{
	const auto [expected, id, cursor] = GENERATE(
		(table<int, SequenceId, int>({
			{std::numeric_limits<int>::max(), SequenceId{0}, 0},
			{std::numeric_limits<int>::max(), SequenceId{1}, 1},
			{std::numeric_limits<int>::max() - 1, SequenceId{1}, 0},
			{std::numeric_limits<int>::max() - 2, SequenceId{2}, 0},
			{std::numeric_limits<int>::max() - 1, SequenceId{2}, 1}
		})));

	REQUIRE(expected == std::invoke(sequence::detail::LazyStrategy{}, id, cursor));
}

TEST_CASE(
	"detail::GreedyStrategy prefers elements far from cursor.",
	"[sequence]"
)
{
	const auto [expected, id, cursor] = GENERATE(
		(table<int, SequenceId, int>({
			{0, SequenceId{0}, 0},
			{0, SequenceId{1}, 1},
			{1, SequenceId{1}, 0},
			{2, SequenceId{2}, 0},
			{1, SequenceId{2}, 1}
		})));

	REQUIRE(expected == std::invoke(sequence::detail::GreedyStrategy{}, id, cursor));
}

TEST_CASE(
	"detail::has_better_rating determines, whether lhs shall be preferred over rhs.",
	"[sequence]"
)
{
	using detail::sequence_rating;
	using detail::has_better_rating;
	constexpr std::array sequence_tags = std::to_array<>({
		sequence::SequenceTag{1},
		sequence::SequenceTag{2},
		sequence::SequenceTag{3},
	});

	SECTION("Lhs with zero ratings is always preferred.")
	{
		const std::vector<sequence_rating> lhs{};

		std::vector<sequence_rating> rhs{};
		REQUIRE(has_better_rating(lhs, rhs));

		rhs.emplace_back(std::numeric_limits<int>::max(), sequence_tags[0]);
		REQUIRE(has_better_rating(lhs, rhs));

		rhs.emplace_back(0, sequence_tags[1]);
		REQUIRE(has_better_rating(lhs, rhs));
	}

	SECTION("When lhs has single rating.")
	{
		const std::vector<sequence_rating> lhs{
			{42, sequence_tags[1]}
		};

		std::vector<sequence_rating> rhs{};
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
		const std::vector<sequence_rating> lhs{
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
		const std::vector<sequence_rating> lhs{
			{42, sequence_tags[1]},
			{25, sequence_tags[2]},
			{1337, sequence_tags[0]}
		};

		std::vector<sequence_rating> rhs{};
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

	const sequence::detail::SequenceConfig config = expect::in_sequence(sequence);

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

	SECTION("Throwsn, when duplicates are given.")
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
		const sequence::detail::SequenceConfig config = expect::in_sequences(
			firstSequence,
			secondSequence);

		REQUIRE(std::get<0>(config.sequences())->tag() == firstSequence.tag());
		REQUIRE(std::get<1>(config.sequences())->tag() == secondSequence.tag());
	}

	SECTION("When three sequences are given.")
	{
		const sequence::detail::SequenceConfig config = expect::in_sequences(
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
	const sequence::detail::SequenceConfig<> firstConfig{};

	SECTION("Concat two empty sequences is pointlesss but possible.")
	{
		[[maybe_unused]] const sequence::detail::SequenceConfig<> result = firstConfig.concat(
			sequence::detail::SequenceConfig<>{});
	}

	SECTION("Concat appends the right side.")
	{
		SequenceT firstSequence{};
		const sequence::detail::SequenceConfig firstResult = firstConfig.concat(
			expect::in_sequence(firstSequence));

		REQUIRE(std::get<0>(firstResult.sequences())->tag() == firstSequence.tag());

		SequenceT secondSequence{};
		sequence::detail::SequenceConfig secondResult = firstResult.concat(
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
			sequence::detail::SequenceConfig thirdResult = secondResult.concat(
				expect::in_sequence(thirdSequence));
			REQUIRE(std::get<0>(thirdResult.sequences())->tag() == firstSequence.tag());
			REQUIRE(std::get<1>(thirdResult.sequences())->tag() == secondSequence.tag());
			REQUIRE(std::get<2>(thirdResult.sequences())->tag() == thirdSequence.tag());
		}
	}
}
//
//TEST_CASE(
//	"expectation_policies::Sequence checks whether the given call::Info occurs in sequence.",
//	"[expectation][expectation::policy][expectation::factories][sequence]"
//)
//{
//	namespace Matches = Catch::Matchers;
//
//	using PolicyT = expectation_policies::Sequence;
//	STATIC_REQUIRE(control_policy<PolicyT>);
//
//	Sequence sequence{};
//
//	const StringT applicableText = "applicable: Sequence element expects further matches.";
//	const StringT saturatedText = "inapplicable: Sequence element is already saturated.";
//	const StringT inapplicableText = "inapplicable: Sequence element is not the current element.";
//
//	SECTION("When sequence contains just a single expectation.")
//	{
//		const auto count = GENERATE(range(1, 5));
//		PolicyT policy = expect::in_sequence(sequence, count);
//
//		for ([[maybe_unused]] const int i : std::views::iota(0, count))
//		{
//			REQUIRE(!policy.is_satisfied());
//			REQUIRE(policy.is_applicable());
//			REQUIRE_THAT(
//				policy.describe_state(),
//				Matches::Equals(applicableText));
//			REQUIRE_NOTHROW(policy.consume());
//		}
//
//		REQUIRE(policy.is_satisfied());
//		REQUIRE(!policy.is_applicable());
//		REQUIRE_THAT(
//			policy.describe_state(),
//			Matches::Equals(saturatedText));
//	}
//
//	SECTION("When sequence has multiple expectations, the order matters.")
//	{
//		PolicyT policy1 = expect::in_sequence(sequence);
//		const auto count2 = GENERATE(range(1, 5));
//		PolicyT policy2 = expect::in_sequence(sequence, count2);
//
//		SECTION("When first expection is satisfied, then the second one becomes applicable.")
//		{
//			REQUIRE(!policy1.is_satisfied());
//			REQUIRE(policy1.is_applicable());
//			REQUIRE_THAT(
//				policy1.describe_state(),
//				Matches::Equals(applicableText));
//
//			REQUIRE(!policy2.is_satisfied());
//			REQUIRE(!policy2.is_applicable());
//			REQUIRE_THAT(
//				policy2.describe_state(),
//				Matches::Equals(inapplicableText));
//
//			REQUIRE_NOTHROW(policy1.consume());
//
//			for ([[maybe_unused]] const int i : std::views::iota(0, count2))
//			{
//				REQUIRE(policy1.is_satisfied());
//				REQUIRE(!policy1.is_applicable());
//				REQUIRE_THAT(
//					policy1.describe_state(),
//					Matches::Equals(saturatedText));
//
//				REQUIRE(!policy2.is_satisfied());
//				REQUIRE(policy2.is_applicable());
//				REQUIRE_THAT(
//					policy2.describe_state(),
//					Matches::Equals(applicableText));
//
//				REQUIRE_NOTHROW(policy2.consume());
//			}
//
//			REQUIRE(policy1.is_satisfied());
//			REQUIRE(!policy1.is_applicable());
//			REQUIRE_THAT(
//				policy1.describe_state(),
//				Matches::Equals(saturatedText));
//
//			REQUIRE(policy2.is_satisfied());
//			REQUIRE(!policy2.is_applicable());
//			REQUIRE_THAT(
//				policy2.describe_state(),
//				Matches::Equals(saturatedText));
//		}
//	}
//}

//TEST_CASE(
//	"An expectation can be part of multiple sequences.",
//	"[expectation][expectation::policy][expectation::factories][sequence]"
//)
//{
//	namespace Matches = Catch::Matchers;
//
//	using PolicyT = expectation_policies::Sequence;
//
//	SECTION("When multiple sequences are given.")
//	{
//		Sequence sequence1{};
//		Sequence sequence2{};
//
//		SECTION("When the first expectation is the prefix of multiple sequences.")
//		{
//			PolicyT policy1 = expect::in_sequences({sequence1, sequence2});
//			PolicyT policy2 = expect::in_sequences({sequence2});
//
//			REQUIRE(!policy1.is_satisfied());
//			REQUIRE(policy1.is_applicable());
//
//			REQUIRE(!policy2.is_satisfied());
//			REQUIRE(!policy2.is_applicable());
//
//			policy1.consume();
//
//			REQUIRE(policy1.is_satisfied());
//			REQUIRE(!policy1.is_applicable());
//
//			REQUIRE(!policy2.is_satisfied());
//			REQUIRE(policy2.is_applicable());
//
//			policy2.consume();
//
//			REQUIRE(policy1.is_satisfied());
//			REQUIRE(!policy1.is_applicable());
//
//			REQUIRE(policy2.is_satisfied());
//			REQUIRE(!policy2.is_applicable());
//		}
//
//		SECTION("When an expectation waits for multiple sequences.")
//		{
//			PolicyT policy1 = expect::in_sequences({sequence1});
//			PolicyT policy2 = expect::in_sequences({sequence2});
//			PolicyT policy3 = expect::in_sequences({sequence1, sequence2});
//
//			REQUIRE(!policy1.is_satisfied());
//			REQUIRE(policy1.is_applicable());
//
//			REQUIRE(!policy2.is_satisfied());
//			REQUIRE(policy2.is_applicable());
//
//			REQUIRE(!policy3.is_satisfied());
//			REQUIRE(!policy3.is_applicable());
//
//			policy1.consume();
//
//			REQUIRE(policy1.is_satisfied());
//			REQUIRE(!policy1.is_applicable());
//
//			REQUIRE(!policy2.is_satisfied());
//			REQUIRE(policy2.is_applicable());
//
//			REQUIRE(!policy3.is_satisfied());
//			REQUIRE(!policy3.is_applicable());
//
//			policy2.consume();
//
//			REQUIRE(policy1.is_satisfied());
//			REQUIRE(!policy1.is_applicable());
//
//			REQUIRE(policy2.is_satisfied());
//			REQUIRE(!policy2.is_applicable());
//
//			REQUIRE(!policy3.is_satisfied());
//			REQUIRE(policy3.is_applicable());
//
//			policy3.consume();
//
//			REQUIRE(policy1.is_satisfied());
//			REQUIRE(!policy1.is_applicable());
//
//			REQUIRE(policy2.is_satisfied());
//			REQUIRE(!policy2.is_applicable());
//
//			REQUIRE(policy3.is_satisfied());
//			REQUIRE(!policy3.is_applicable());
//		}
//	}
//}
