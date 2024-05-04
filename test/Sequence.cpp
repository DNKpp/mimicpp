// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"

#include "mimic++/Sequence.hpp"
#include "mimic++/Expectation.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace mimicpp;

TEST_CASE(
	"detail::Sequence is default constructible, but immobile.",
	"[sequence]"
)
{
	STATIC_REQUIRE(std::is_default_constructible_v<detail::Sequence>);

	STATIC_REQUIRE(!std::is_copy_constructible_v<detail::Sequence>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<detail::Sequence>);
	STATIC_REQUIRE(!std::is_move_constructible_v<detail::Sequence>);
	STATIC_REQUIRE(!std::is_move_assignable_v<detail::Sequence>);
}

TEST_CASE(
	"Sequence is default constructible, but immobile.",
	"[sequence]"
)
{
	STATIC_REQUIRE(std::is_default_constructible_v<Sequence>);

	STATIC_REQUIRE(!std::is_copy_constructible_v<Sequence>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<Sequence>);
	STATIC_REQUIRE(!std::is_move_constructible_v<Sequence>);
	STATIC_REQUIRE(!std::is_move_assignable_v<Sequence>);
}

TEST_CASE(
	"expectation_policies::Sequence is non-copyable but movable.",
	"[sequence]"
)
{
	STATIC_REQUIRE(!std::is_copy_constructible_v<expectation_policies::Sequence>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<expectation_policies::Sequence>);

	STATIC_REQUIRE(std::is_move_constructible_v<expectation_policies::Sequence>);
	STATIC_REQUIRE(std::is_move_assignable_v<expectation_policies::Sequence>);
}

TEST_CASE(
	"detail::Sequence supports an arbitrary id amount.",
	"[sequence]"
)
{
	namespace Matches = Catch::Matchers;

	ScopedReporter reporter{};
	std::optional<detail::Sequence> sequence{std::in_place};

	SECTION("When attempted to add a new id with zero or negative count, an std::invalid_argument is thrown.")
	{
		REQUIRE_THROWS_AS(
			sequence->add(0),
			std::invalid_argument);
	}

	SECTION("When sequence contains one id, that id must be fully consumed.")
	{
		const std::size_t count = GENERATE(1u, 2u, 3u);
		const SequenceId id = sequence->add(count);

		for ([[maybe_unused]] const auto i : std::views::iota(0u, count - 1u))
		{
			REQUIRE(sequence->is_consumable(id));
			REQUIRE(!sequence->is_saturated(id));
			REQUIRE_NOTHROW(sequence->consume(id));
		}
		REQUIRE(sequence->is_consumable(id));
		REQUIRE(!sequence->is_saturated(id));

		SECTION("When id is not fully consumed, an error is reported.")
		{
			sequence.reset();
			REQUIRE_THAT(
				reporter.errors(),
				Matches::SizeIs(1));
			REQUIRE_THAT(
				reporter.errors().front(),
				Matches::Equals("Unfulfilled sequence. 0 out of 1 expectation(s) where fully consumed."));
		}

		SECTION("When id is fully consumed, nothing is reported.")
		{
			REQUIRE_NOTHROW(sequence->consume(id));
			REQUIRE(!sequence->is_consumable(id));
			REQUIRE(sequence->is_saturated(id));

			sequence.reset();
			REQUIRE_THAT(
				reporter.errors(),
				Matches::IsEmpty());
		}
	}

	SECTION("When sequence contains multiple ids.")
	{
		const std::vector ids{
			sequence->add(3u),
			sequence->add(2u),
			sequence->add(1u)
		};

		SECTION("Sequence can consume first id.")
		{
			for ([[maybe_unused]] const auto i : std::views::iota(0u, 3u))
			{
				REQUIRE(sequence->is_consumable(ids[0]));
				REQUIRE(!sequence->is_saturated(ids[0]));

				REQUIRE(!sequence->is_consumable(ids[1]));
				REQUIRE(!sequence->is_saturated(ids[1]));

				REQUIRE(!sequence->is_consumable(ids[2]));
				REQUIRE(!sequence->is_saturated(ids[2]));

				REQUIRE_NOTHROW(sequence->consume(ids[0]));
			}

			SECTION("When sequence is destroyed, an error is reported.")
			{
				sequence.reset();

				REQUIRE_THAT(
					reporter.errors(),
					Matches::SizeIs(1));
				REQUIRE_THAT(
					reporter.errors().front(),
					Matches::Equals("Unfulfilled sequence. 1 out of 3 expectation(s) where fully consumed."));
			}

			SECTION("And then the next id is consumable.")
			{
				for ([[maybe_unused]] const auto i : std::views::iota(0u, 2u))
				{
					REQUIRE(!sequence->is_consumable(ids[0]));
					REQUIRE(sequence->is_saturated(ids[0]));

					REQUIRE(sequence->is_consumable(ids[1]));
					REQUIRE(!sequence->is_saturated(ids[1]));

					REQUIRE(!sequence->is_consumable(ids[2]));
					REQUIRE(!sequence->is_saturated(ids[2]));

					REQUIRE_NOTHROW(sequence->consume(ids[1]));
				}

				SECTION("When sequence is destroyed, an error is reported.")
				{
					sequence.reset();

					REQUIRE_THAT(
						reporter.errors(),
						Matches::SizeIs(1));
					REQUIRE_THAT(
						reporter.errors().front(),
						Matches::Equals("Unfulfilled sequence. 2 out of 3 expectation(s) where fully consumed."));
				}

				SECTION("And then last id is consumable.")
				{
					REQUIRE(!sequence->is_consumable(ids[0]));
					REQUIRE(sequence->is_saturated(ids[0]));

					REQUIRE(!sequence->is_consumable(ids[1]));
					REQUIRE(sequence->is_saturated(ids[1]));

					REQUIRE(sequence->is_consumable(ids[2]));
					REQUIRE(!sequence->is_saturated(ids[2]));

					REQUIRE_NOTHROW(sequence->consume(ids[2]));

					SECTION("When sequence is destroyed, no error is reported.")
					{
						sequence.reset();

						REQUIRE_THAT(
							reporter.errors(),
							Matches::IsEmpty());
					}

					SECTION("And then, nothing is consumable.")
					{
						REQUIRE(!sequence->is_consumable(ids[0]));
						REQUIRE(sequence->is_saturated(ids[0]));

						REQUIRE(!sequence->is_consumable(ids[1]));
						REQUIRE(sequence->is_saturated(ids[1]));

						REQUIRE(!sequence->is_consumable(ids[2]));
						REQUIRE(sequence->is_saturated(ids[2]));
					}
				}
			}
		}
	}
}

TEST_CASE(
	"expectation_policies::Sequence checks whether the given call::Info occurs in sequence.",
	"[expectation][expectation::policy][expectation::factories][sequence]"
)
{
	namespace Matches = Catch::Matchers;

	using PolicyT = expectation_policies::Sequence;
	STATIC_REQUIRE(times_policy<PolicyT>);

	Sequence sequence{};

	const StringT applicableText = "applicable: Sequence element expects further matches.";
	const StringT saturatedText = "inapplicable: Sequence element is already saturated.";
	const StringT inapplicableText = "inapplicable: Sequence element is not the current element.";

	SECTION("When sequence contains just a single expectation.")
	{
		const auto count = GENERATE(range(1, 5));
		PolicyT policy = expect::in_sequence(sequence, count);

		for ([[maybe_unused]] const int i : std::views::iota(0, count))
		{
			REQUIRE(!policy.is_satisfied());
			REQUIRE(policy.is_applicable());
			REQUIRE_THAT(
				policy.describe_state(),
				Matches::Equals(applicableText));
			REQUIRE_NOTHROW(policy.consume());
		}

		REQUIRE(policy.is_satisfied());
		REQUIRE(!policy.is_applicable());
		REQUIRE_THAT(
			policy.describe_state(),
			Matches::Equals(saturatedText));
	}

	SECTION("When sequence has multiple expectations, the order matters.")
	{
		PolicyT policy1 = expect::in_sequence(sequence);
		const auto count2 = GENERATE(range(1, 5));
		PolicyT policy2 = expect::in_sequence(sequence, count2);

		SECTION("When first expection is satisfied, then the second one becomes applicable.")
		{
			REQUIRE(!policy1.is_satisfied());
			REQUIRE(policy1.is_applicable());
			REQUIRE_THAT(
				policy1.describe_state(),
				Matches::Equals(applicableText));

			REQUIRE(!policy2.is_satisfied());
			REQUIRE(!policy2.is_applicable());
			REQUIRE_THAT(
				policy2.describe_state(),
				Matches::Equals(inapplicableText));

			REQUIRE_NOTHROW(policy1.consume());

			for ([[maybe_unused]] const int i : std::views::iota(0, count2))
			{
				REQUIRE(policy1.is_satisfied());
				REQUIRE(!policy1.is_applicable());
				REQUIRE_THAT(
					policy1.describe_state(),
					Matches::Equals(saturatedText));

				REQUIRE(!policy2.is_satisfied());
				REQUIRE(policy2.is_applicable());
				REQUIRE_THAT(
					policy2.describe_state(),
					Matches::Equals(applicableText));

				REQUIRE_NOTHROW(policy2.consume());
			}

			REQUIRE(policy1.is_satisfied());
			REQUIRE(!policy1.is_applicable());
			REQUIRE_THAT(
				policy1.describe_state(),
				Matches::Equals(saturatedText));

			REQUIRE(policy2.is_satisfied());
			REQUIRE(!policy2.is_applicable());
			REQUIRE_THAT(
				policy2.describe_state(),
				Matches::Equals(saturatedText));
		}
	}
}

TEST_CASE(
	"An expectation can be part of multiple sequences.",
	"[expectation][expectation::policy][expectation::factories][sequence]"
)
{
	namespace Matches = Catch::Matchers;

	using PolicyT = expectation_policies::Sequence;

	SECTION("When multiple sequences are given.")
	{
		Sequence sequence1{};
		Sequence sequence2{};

		SECTION("When the first expectation is the prefix of multiple sequences.")
		{
			PolicyT policy1 = expect::in_sequences({sequence1, sequence2});
			PolicyT policy2 = expect::in_sequences({sequence2});

			REQUIRE(!policy1.is_satisfied());
			REQUIRE(policy1.is_applicable());

			REQUIRE(!policy2.is_satisfied());
			REQUIRE(!policy2.is_applicable());

			policy1.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE(!policy1.is_applicable());

			REQUIRE(!policy2.is_satisfied());
			REQUIRE(policy2.is_applicable());

			policy2.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE(!policy1.is_applicable());

			REQUIRE(policy2.is_satisfied());
			REQUIRE(!policy2.is_applicable());
		}

		SECTION("When an expectation waits for multiple sequences.")
		{
			PolicyT policy1 = expect::in_sequences({sequence1});
			PolicyT policy2 = expect::in_sequences({sequence2});
			PolicyT policy3 = expect::in_sequences({sequence1, sequence2});

			REQUIRE(!policy1.is_satisfied());
			REQUIRE(policy1.is_applicable());

			REQUIRE(!policy2.is_satisfied());
			REQUIRE(policy2.is_applicable());

			REQUIRE(!policy3.is_satisfied());
			REQUIRE(!policy3.is_applicable());

			policy1.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE(!policy1.is_applicable());

			REQUIRE(!policy2.is_satisfied());
			REQUIRE(policy2.is_applicable());

			REQUIRE(!policy3.is_satisfied());
			REQUIRE(!policy3.is_applicable());

			policy2.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE(!policy1.is_applicable());

			REQUIRE(policy2.is_satisfied());
			REQUIRE(!policy2.is_applicable());

			REQUIRE(!policy3.is_satisfied());
			REQUIRE(policy3.is_applicable());

			policy3.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE(!policy1.is_applicable());

			REQUIRE(policy2.is_satisfied());
			REQUIRE(!policy2.is_applicable());

			REQUIRE(policy3.is_satisfied());
			REQUIRE(!policy3.is_applicable());
		}
	}
}
