// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Sequence.hpp"
#include "mimic++/Expectation.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "TestReporter.hpp"

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
	detail::Sequence sequence{};

	SECTION("When sequence contains one id, that id can be consumed.")
	{
		const SequenceId id = sequence.add();

		REQUIRE(sequence.is_consumable(id));
		REQUIRE_NOTHROW(sequence.consume(id));

		SECTION("Even multiple times in a row.")
		{
			for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
			{
				REQUIRE(sequence.is_consumable(id));
				REQUIRE_NOTHROW(sequence.consume(id));
			}

			REQUIRE(sequence.is_consumable(id));
		}
	}

	SECTION("When sequence contains multiple ids.")
	{
		const std::vector ids{
			sequence.add(),
			sequence.add(),
			sequence.add()
		};

		SECTION("Sequence can consume ids.")
		{
			REQUIRE(sequence.is_consumable(ids[0]));
			REQUIRE(sequence.is_consumable(ids[1]));
			REQUIRE(sequence.is_consumable(ids[2]));
			REQUIRE_NOTHROW(sequence.consume(ids[0]));

			SECTION("And then the same id is consumable again.")
			{
				for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
				{
					REQUIRE(sequence.is_consumable(ids[0]));
					REQUIRE(sequence.is_consumable(ids[1]));
					REQUIRE(sequence.is_consumable(ids[2]));
					REQUIRE_NOTHROW(sequence.consume(ids[0]));
				}

				REQUIRE(sequence.is_consumable(ids[0]));
				REQUIRE(sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence.consume(ids[1]));
			}

			SECTION("And then the next id is consumable.")
			{
				REQUIRE(sequence.is_consumable(ids[0]));
				REQUIRE(sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence.consume(ids[1]));

				SECTION("And then the same id is consumable again.")
				{
					for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
					{
						REQUIRE(!sequence.is_consumable(ids[0]));
						REQUIRE(sequence.is_consumable(ids[1]));
						REQUIRE(sequence.is_consumable(ids[2]));
						REQUIRE_NOTHROW(sequence.consume(ids[1]));
					}

					REQUIRE(!sequence.is_consumable(ids[0]));
					REQUIRE(sequence.is_consumable(ids[1]));
					REQUIRE(sequence.is_consumable(ids[2]));
					REQUIRE_NOTHROW(sequence.consume(ids[2]));
				}

				SECTION("And then last id is consumable.")
				{
					REQUIRE(!sequence.is_consumable(ids[0]));
					REQUIRE(sequence.is_consumable(ids[1]));
					REQUIRE(sequence.is_consumable(ids[2]));
					REQUIRE_NOTHROW(sequence.consume(ids[2]));

					SECTION("And again...")
					{
						for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
						{
							REQUIRE(!sequence.is_consumable(ids[0]));
							REQUIRE(!sequence.is_consumable(ids[1]));
							REQUIRE(sequence.is_consumable(ids[2]));
							REQUIRE_NOTHROW(sequence.consume(ids[2]));
						}

						REQUIRE(!sequence.is_consumable(ids[0]));
						REQUIRE(!sequence.is_consumable(ids[1]));
						REQUIRE(sequence.is_consumable(ids[2]));
					}
				}
			}
		}

		SECTION("Individual ids can be skipped.")
		{
			SECTION("When first id is skipped.")
			{
				REQUIRE(sequence.is_consumable(ids[0]));
				REQUIRE(sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence.consume(ids[1]));

				REQUIRE(!sequence.is_consumable(ids[0]));
				REQUIRE(sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
			}

			SECTION("When second id is skipped.")
			{
				REQUIRE(sequence.is_consumable(ids[0]));
				REQUIRE(sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence.consume(ids[0]));

				REQUIRE(sequence.is_consumable(ids[0]));
				REQUIRE(sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence.consume(ids[2]));

				REQUIRE(!sequence.is_consumable(ids[0]));
				REQUIRE(!sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
			}

			SECTION("When first and second ids are skipped.")
			{
				REQUIRE(sequence.is_consumable(ids[0]));
				REQUIRE(sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence.consume(ids[2]));

				REQUIRE(!sequence.is_consumable(ids[0]));
				REQUIRE(!sequence.is_consumable(ids[1]));
				REQUIRE(sequence.is_consumable(ids[2]));
			}
		}
	}
}

TEST_CASE(
	"expectation_policies::Sequence checks whether the given call::Info occurs in sequence.",
	"[expectation][expectation::policy][sequence]"
)
{
	namespace Matches = Catch::Matchers;

	using SignatureT = void();
	using CallInfoT = call::info_for_signature_t<SignatureT>;
	using PolicyT = expectation_policies::Sequence;
	STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

	Sequence sequence{};
	PolicyT policy = expect::in_sequence(sequence);
	SECTION("Policy is always satisfied.")
	{
		REQUIRE(policy.is_satisfied());
	}

	const CallInfoT call{
		.args = {},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	SECTION("When sequence has just a single expectation, that expectation always matches.")
	{
		for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
		{
			const call::SubMatchResult result = policy.matches(call);

			REQUIRE(result.matched);
			REQUIRE(result.msg);
			REQUIRE_THAT(
				*result.msg,
				Matches::Equals(" is in sequence"));

			REQUIRE_NOTHROW(policy.consume(call));
		}
	}

	SECTION("When sequence has multiple expectations, the order matters.")
	{
		PolicyT policy2 = expect::in_sequence(sequence);

		SECTION("When nothing is consumed yet, both policies matches.")
		{
			const call::SubMatchResult result1 = policy.matches(call);
			REQUIRE(result1.matched);
			REQUIRE(result1.msg);
			REQUIRE_THAT(
				*result1.msg,
				Matches::Equals(" is in sequence"));

			const call::SubMatchResult result2 = policy2.matches(call);
			REQUIRE(result2.matched);
			REQUIRE(result2.msg);
			REQUIRE_THAT(
				*result2.msg,
				Matches::Equals(" is in sequence"));
		}

		SECTION("When first policy is consumed, both policies still matches.")
		{
			REQUIRE_NOTHROW(policy.consume(call));

			const call::SubMatchResult result1 = policy.matches(call);
			REQUIRE(result1.matched);
			REQUIRE(result1.msg);
			REQUIRE_THAT(
				*result1.msg,
				Matches::Equals(" is in sequence"));

			const call::SubMatchResult result2 = policy2.matches(call);
			REQUIRE(result2.matched);
			REQUIRE(result2.msg);
			REQUIRE_THAT(
				*result2.msg,
				Matches::Equals(" is in sequence"));
		}

		SECTION("When second policy is consumed, only second one matches.")
		{
			REQUIRE_NOTHROW(policy2.consume(call));

			const call::SubMatchResult result1 = policy.matches(call);
			REQUIRE(!result1.matched);
			REQUIRE(result1.msg);
			REQUIRE_THAT(
				*result1.msg,
				Matches::Equals(" is not in sequence"));

			const call::SubMatchResult result2 = policy2.matches(call);
			REQUIRE(result2.matched);
			REQUIRE(result2.msg);
			REQUIRE_THAT(
				*result2.msg,
				Matches::Equals(" is in sequence"));
		}
	}
}
