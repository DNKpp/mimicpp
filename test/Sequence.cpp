// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Sequence.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "TestReporter.hpp"

using namespace mimicpp;

TEST_CASE(
	"Sequences are default constructible, but immobile.",
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
	"Sequences support an id arbitrary amount.",
	"[sequence]"
)
{
	namespace Matches = Catch::Matchers;

	ScopedReporter reporter{};
	std::optional<Sequence> sequence{std::in_place};

	SECTION("When sequence contains one id, that id can be consumed.")
	{
		const SequenceId id = sequence->add();

		REQUIRE(sequence->is_consumable(id));
		REQUIRE_NOTHROW(sequence->consume(id));

		SECTION("Even multiple times in a row.")
		{
			for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
			{
				REQUIRE(sequence->is_consumable(id));
				REQUIRE_NOTHROW(sequence->consume(id));
			}

			REQUIRE(sequence->is_consumable(id));
		}
	}

	SECTION("When sequence contains multiple ids.")
	{
		const std::vector ids{
			sequence->add(),
			sequence->add(),
			sequence->add()
		};

		SECTION("Sequence can consume ids.")
		{
			REQUIRE(sequence->is_consumable(ids[0]));
			REQUIRE(sequence->is_consumable(ids[1]));
			REQUIRE(sequence->is_consumable(ids[2]));
			REQUIRE_NOTHROW(sequence->consume(ids[0]));

			SECTION("And then the same id is consumable again.")
			{
				for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
				{
					REQUIRE(sequence->is_consumable(ids[0]));
					REQUIRE(sequence->is_consumable(ids[1]));
					REQUIRE(sequence->is_consumable(ids[2]));
					REQUIRE_NOTHROW(sequence->consume(ids[0]));
				}

				REQUIRE(sequence->is_consumable(ids[0]));
				REQUIRE(sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence->consume(ids[1]));
			}

			SECTION("And then the next id is consumable.")
			{
				REQUIRE(sequence->is_consumable(ids[0]));
				REQUIRE(sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence->consume(ids[1]));

				SECTION("And then the same id is consumable again.")
				{
					for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
					{
						REQUIRE(!sequence->is_consumable(ids[0]));
						REQUIRE(sequence->is_consumable(ids[1]));
						REQUIRE(sequence->is_consumable(ids[2]));
						REQUIRE_NOTHROW(sequence->consume(ids[1]));
					}

					REQUIRE(!sequence->is_consumable(ids[0]));
					REQUIRE(sequence->is_consumable(ids[1]));
					REQUIRE(sequence->is_consumable(ids[2]));
					REQUIRE_NOTHROW(sequence->consume(ids[2]));
				}

				SECTION("And then last id is consumable.")
				{
					REQUIRE(!sequence->is_consumable(ids[0]));
					REQUIRE(sequence->is_consumable(ids[1]));
					REQUIRE(sequence->is_consumable(ids[2]));
					REQUIRE_NOTHROW(sequence->consume(ids[2]));

					SECTION("And again...")
					{
						for ([[maybe_unused]] const int i : std::views::iota(0, GENERATE(1, 5)))
						{
							REQUIRE(!sequence->is_consumable(ids[0]));
							REQUIRE(!sequence->is_consumable(ids[1]));
							REQUIRE(sequence->is_consumable(ids[2]));
							REQUIRE_NOTHROW(sequence->consume(ids[2]));
						}

						REQUIRE(!sequence->is_consumable(ids[0]));
						REQUIRE(!sequence->is_consumable(ids[1]));
						REQUIRE(sequence->is_consumable(ids[2]));
					}
				}
			}
		}

		SECTION("Individual ids can be skipped.")
		{
			SECTION("When first id is skipped.")
			{
				REQUIRE(sequence->is_consumable(ids[0]));
				REQUIRE(sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence->consume(ids[1]));

				REQUIRE(!sequence->is_consumable(ids[0]));
				REQUIRE(sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
			}

			SECTION("When second id is skipped.")
			{
				REQUIRE(sequence->is_consumable(ids[0]));
				REQUIRE(sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence->consume(ids[0]));

				REQUIRE(sequence->is_consumable(ids[0]));
				REQUIRE(sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence->consume(ids[2]));

				REQUIRE(!sequence->is_consumable(ids[0]));
				REQUIRE(!sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
			}

			SECTION("When first and second ids are skipped.")
			{
				REQUIRE(sequence->is_consumable(ids[0]));
				REQUIRE(sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
				REQUIRE_NOTHROW(sequence->consume(ids[2]));

				REQUIRE(!sequence->is_consumable(ids[0]));
				REQUIRE(!sequence->is_consumable(ids[1]));
				REQUIRE(sequence->is_consumable(ids[2]));
			}
		}
	}
}
