// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ControlPolicy.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

TEST_CASE(
	"TimesConfig has exactly 1 as limit by default.",
	"[detail][expectation][expectation::control]"
)
{
	constexpr detail::TimesConfig config{};

	REQUIRE(1 == config.min());
	REQUIRE(1 == config.max());
}

TEST_CASE(
	"The limits of TimesConfig can be modified.",
	"[detail][expectation][expectation::control]"
)
{
	const int min = GENERATE(range(0, 5));
	const int max = min + GENERATE(range(0, 5));

	const detail::TimesConfig config{min, max};

	REQUIRE(min == std::as_const(config).min());
	REQUIRE(max == std::as_const(config).max());
}

TEST_CASE(
	"TimesConfig when invalid limits are given.",
	"[detail][expectation][expectation::control]"
)
{
	SECTION("When max > min.")
	{
		const int max = GENERATE(range(0, 5));
		const int min = max + GENERATE(range(1, 5));

		REQUIRE_THROWS_AS(
			(detail::TimesConfig{min, max}),
			std::invalid_argument);
	}

	SECTION("When min and/or max < 0.")
	{
		const int min = GENERATE(std::numeric_limits<int>::min(), -42, -1);
		const int max = min + GENERATE(range(0, 5));

		REQUIRE_THROWS_AS(
			(detail::TimesConfig{min, max}),
			std::invalid_argument);
	}
}

TEST_CASE(
	"ControlPolicy can be constructed from TimesConfig and default SequenceConfig.",
	"[expectation][expectation::control]"
)
{
	const int min = GENERATE(range(0, 5));
	const int max = min + GENERATE(range(0, 5));

	ControlPolicy<> policy{
		detail::TimesConfig{min, max},
		sequence::detail::Config<>{}
	};

	for ([[maybe_unused]] auto i : std::views::iota(0, min))
	{
		REQUIRE(!std::as_const(policy).is_satisfied());
		REQUIRE_THAT(
			std::as_const(policy).state(),
			variant_equals(
				state_applicable{
				.min = min,
				.max = max,
				.count = i,
				}));

		REQUIRE_NOTHROW(policy.consume());
	}

	REQUIRE(std::as_const(policy).is_satisfied());

	for ([[maybe_unused]] auto i : std::views::iota(min, max))
	{
		REQUIRE(std::as_const(policy).is_satisfied());
		REQUIRE_THAT(
			std::as_const(policy).state(),
			variant_equals(
				state_applicable{
				.min = min,
				.max = max,
				.count = i,
				}));

		REQUIRE_NOTHROW(policy.consume());
	}

	REQUIRE(std::as_const(policy).is_satisfied());
	REQUIRE_THAT(
		std::as_const(policy).state(),
		variant_equals(
			state_saturated{
			.min = min,
			.max = max,
			.count = max,
			}));
}

namespace
{
	using TestSequenceT = sequence::detail::BasicSequenceInterface<
		sequence::Id,
		FakeSequenceStrategy{}>;
}

TEST_CASE(
	"ControlPolicy can be constructed from SequenceConfig and default TimesConfig.",
	"[expectation][expectation::control]"
)
{
	namespace Matches = Catch::Matchers;

	ScopedReporter reporter{};

	SECTION("When single sequence is provided.")
	{
		std::optional<TestSequenceT> sequence{std::in_place};
		std::optional policy{
			ControlPolicy{
				detail::TimesConfig{},
				expect::in_sequence(*sequence)
			}
		};

		REQUIRE(!std::as_const(*policy).is_satisfied());
		REQUIRE_THAT(
			std::as_const(*policy).state(),
			variant_equals(
				state_applicable{
				.min = 1,
				.max = 1,
				.count = 0,
				.sequenceRatings = {
				sequence::rating{0, sequence->tag()}
				}
				}));

		SECTION("Is satisfied, when consumed once.")
		{
			REQUIRE_NOTHROW(policy->consume());

			REQUIRE(std::as_const(*policy).is_satisfied());
			REQUIRE_THAT(
				std::as_const(*policy).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {sequence->tag()}
					}));
		}

		SECTION("Reports error, when unconsumed.")
		{
			sequence.reset();
			policy.reset();
			REQUIRE_THAT(
				reporter.errors(),
				Matches::SizeIs(1));
		}
	}

	SECTION("When multiples sequences are provided.")
	{
		std::optional<TestSequenceT> firstSequence{std::in_place};
		std::optional<TestSequenceT> secondSequence{std::in_place};
		std::optional policy{
			ControlPolicy{
				detail::TimesConfig{},
				expect::in_sequences(
					*firstSequence,
					*secondSequence)
			}
		};

		REQUIRE(!std::as_const(*policy).is_satisfied());
		REQUIRE_THAT(
			std::as_const(*policy).state(),
			variant_equals(
				state_applicable{
				.min = 1,
				.max = 1,
				.count = 0,
				.sequenceRatings = {
				sequence::rating{0, firstSequence->tag()},
				sequence::rating{0, secondSequence->tag()}
				}
				}));

		SECTION("Is satisfied, when consumed once.")
		{
			REQUIRE_NOTHROW(policy->consume());

			REQUIRE(std::as_const(*policy).is_satisfied());
			REQUIRE_THAT(
				std::as_const(*policy).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {
					firstSequence->tag(),
					secondSequence->tag()
					}
					}));
		}

		SECTION("Reports error, when unconsumed.")
		{
			std::size_t expectedErrorCount{1};
			SECTION("Due to first sequence.")
			{
				firstSequence.reset();
			}

			SECTION("Due to second sequence.")
			{
				secondSequence.reset();
			}

			SECTION("Due to both sequences.")
			{
				firstSequence.reset();
				secondSequence.reset();
				expectedErrorCount = 2;
			}

			policy.reset();
			REQUIRE_THAT(
				reporter.errors(),
				Matches::SizeIs(expectedErrorCount));
		}
	}
}

TEST_CASE(
	"ControlPolicy can be constructed from SequenceConfig and TimesConfig.",
	"[expectation][expectation::control]"
)
{
	namespace Matches = Catch::Matchers;

	ScopedReporter reporter{};

	{
		const int min = GENERATE(range(0, 5));
		const int max = min + GENERATE(range(0, 5));

		TestSequenceT sequence{};
		ControlPolicy policy{
			expect::times(min, max),
			expect::in_sequence(sequence)
		};

		for ([[maybe_unused]] auto i : std::views::iota(0, min))
		{
			REQUIRE(!std::as_const(policy).is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy).state(),
				variant_equals(
					state_applicable{
					.min = min,
					.max = max,
					.count = i,
					.sequenceRatings = {
					sequence::rating{0, sequence.tag()}
					}
					}));

			REQUIRE_NOTHROW(policy.consume());
		}

		REQUIRE(std::as_const(policy).is_satisfied());

		SECTION("Does not report, when satisfied.")
		{
		}

		SECTION("Can be consumed until saturated.")
		{
			for ([[maybe_unused]] auto i : std::views::iota(min, max))
			{
				REQUIRE(std::as_const(policy).is_satisfied());
				REQUIRE_THAT(
					std::as_const(policy).state(),
					variant_equals(
						state_applicable{
						.min = min,
						.max = max,
						.count = i,
						.sequenceRatings = {
						sequence::rating{0, sequence.tag()}
						}
						}));

				REQUIRE_NOTHROW(policy.consume());
			}

			REQUIRE(std::as_const(policy).is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy).state(),
				variant_equals(
					state_saturated{
					.min = min,
					.max = max,
					.count = max,
					.sequences = {sequence.tag()}
					}));
		}
	}

	REQUIRE_THAT(
		reporter.errors(),
		Matches::IsEmpty());
}

TEST_CASE(
	"both, expectation_policies::Times and expectation_policies::Times use detail::describe_times_state for their description.",
	"[expectation][detail]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("When current == max.")
	{
		const auto [min, max] = GENERATE(
			(table<std::size_t, std::size_t>)({
				{0, 1},
				{0, 2},
				{3, 42},
				}));

		const auto description = detail::describe_times_state(
			max,
			min,
			max);

		REQUIRE_THAT(
			description,
			Matches::StartsWith("inapplicable: already saturated (matched "));
	}

	SECTION("When min <= current < max.")
	{
		const auto [current, min, max] = GENERATE(
			(table<std::size_t, std::size_t, std::size_t>)({
				{0, 0, 1},
				{1, 0, 2},
				{21, 3, 42},
				}));

		const auto description = detail::describe_times_state(
			current,
			min,
			max);

		REQUIRE_THAT(
			description,
			Matches::Matches("applicable: accepts further matches \\(matched \\d+ out of \\d+ times\\)"));
	}

	SECTION("When current < min.")
	{
		const auto [current, min, max] = GENERATE(
			(table<std::size_t, std::size_t, std::size_t>)({
				{0, 1, 2},
				{1, 2, 5},
				{2, 3, 42},
				}));

		const auto description = detail::describe_times_state(
			current,
			min,
			max);

		REQUIRE_THAT(
			description,
			Matches::StartsWith("unsatisfied: matched "));
	}
}

TEST_CASE(
	"ControlPolicy checks whether the given call occurs in sequence.",
	"[expectation][expectation::control][sequence]"
)
{
	namespace Matches = Catch::Matchers;

	TestSequenceT sequence{};

	SECTION("When sequence contains just a single expectation.")
	{
		const auto count = GENERATE(range(1, 5));
		ControlPolicy policy{
			expect::times(count),
			expect::in_sequence(sequence)
		};

		for ([[maybe_unused]] const int i : std::views::iota(0, count))
		{
			REQUIRE(!std::as_const(policy).is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy).state(),
				variant_equals(
					state_applicable{
					.min = count,
					.max = count,
					.count = i,
					.sequenceRatings = {
					sequence::rating{0, sequence.tag()}
					}
					}));
			REQUIRE_THAT(
				policy.describe_state(),
				Matches::Matches("unsatisfied: matched .+ - .+ is expected\n\t.*")
				&& Matches::EndsWith("\n\tIs head from 1 out of 1 sequences."));

			REQUIRE_NOTHROW(policy.consume());
		}

		REQUIRE(std::as_const(policy).is_satisfied());
		REQUIRE_THAT(
			std::as_const(policy).state(),
			variant_equals(
				state_saturated{
				.min = count,
				.max = count,
				.count = count,
				.sequences = {sequence.tag()}
				}));
		REQUIRE_THAT(
			policy.describe_state(),
			Matches::StartsWith("inapplicable: already saturated (matched ")
			&& Matches::EndsWith(")"));
	}

	SECTION("When sequence has multiple expectations, the order matters.")
	{
		ControlPolicy policy1{
			expect::once(),
			expect::in_sequence(sequence)
		};

		const auto count2 = GENERATE(range(1, 5));
		ControlPolicy policy2{
			expect::times(count2),
			expect::in_sequence(sequence)
		};

		SECTION("When first expection is satisfied, then the second one becomes applicable.")
		{
			REQUIRE(!std::as_const(policy1).is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_applicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					sequence::rating{0, sequence.tag()}
					}
					}));
			REQUIRE_THAT(
				policy1.describe_state(),
				Matches::Equals("unsatisfied: matched never - exactly once is expected\n\tIs head from 1 out of 1 sequences."));

			REQUIRE(!std::as_const(policy2).is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_inapplicable{
					.min = count2,
					.max = count2,
					.count = 0,
					.inapplicableSequences = {sequence.tag()}
					}));
			REQUIRE_THAT(
				policy2.describe_state(),
				Matches::Matches("unsatisfied: matched never - .+ is expected\n\t.*")
				&& Matches::EndsWith("\n\tIs head from 0 out of 1 sequences."));

			REQUIRE_NOTHROW(policy1.consume());

			for ([[maybe_unused]] const int i : std::views::iota(0, count2))
			{
				REQUIRE(policy1.is_satisfied());
				REQUIRE_THAT(
					std::as_const(policy1).state(),
					variant_equals(
						state_saturated{
						.min = 1,
						.max = 1,
						.count = 1,
						.sequences = {sequence.tag()}
						}));
				REQUIRE_THAT(
					policy1.describe_state(),
					Matches::Equals("inapplicable: already saturated (matched once)"));

				REQUIRE(!std::as_const(policy2).is_satisfied());
				REQUIRE_THAT(
					std::as_const(policy2).state(),
					variant_equals(
						state_applicable{
						.min = count2,
						.max = count2,
						.count = i,
						.sequenceRatings = {
						sequence::rating{1, sequence.tag()}
						}
						}));
				REQUIRE_THAT(
					policy2.describe_state(),
					Matches::Matches("unsatisfied: matched .+ - .+ is expected\n\t.*")
					&& Matches::EndsWith("\n\tIs head from 1 out of 1 sequences."));

				REQUIRE_NOTHROW(policy2.consume());
			}

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));
			REQUIRE_THAT(
				policy1.describe_state(),
				Matches::Equals("inapplicable: already saturated (matched once)"));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_saturated{
					.min = count2,
					.max = count2,
					.count = count2,
					.sequences = {sequence.tag()}
					}));
			REQUIRE_THAT(
				policy2.describe_state(),
				Matches::StartsWith("inapplicable: already saturated (matched")
				&& Matches::EndsWith(")"));
		}
	}
}

TEST_CASE(
	"An expectation can be part of multiple sequences.",
	"[expectation][expectation::control][sequence]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("When multiple sequences are given.")
	{
		TestSequenceT sequence1{};
		TestSequenceT sequence2{};

		SECTION("When the first expectation is the prefix of multiple sequences.")
		{
			ControlPolicy policy1{
				expect::once(),
				expect::in_sequences(sequence1, sequence2)
			};
			ControlPolicy policy2{
				expect::once(),
				expect::in_sequence(sequence2)
			};

			REQUIRE(!policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_applicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					sequence::rating{0, sequence1.tag()},
					sequence::rating{0, sequence2.tag()},
					}
					}));

			REQUIRE(!policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_inapplicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.inapplicableSequences = {sequence2.tag()}
					}));

			policy1.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {
					sequence1.tag(),
					sequence2.tag()
					}
					}));

			REQUIRE(!policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_applicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					sequence::rating{1, sequence2.tag()},
					}
					}));

			policy2.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {
					sequence1.tag(),
					sequence2.tag()
					}
					}));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {sequence2.tag()}
					}));
		}

		SECTION("When an expectation waits for multiple sequences.")
		{
			ControlPolicy policy1{
				expect::once(),
				expect::in_sequence(sequence1)
			};
			ControlPolicy policy2{
				expect::once(),
				expect::in_sequence(sequence2)
			};
			ControlPolicy policy3{
				expect::once(),
				expect::in_sequences(sequence1, sequence2)
			};

			REQUIRE(!policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_applicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					sequence::rating{0, sequence1.tag()}
					}
					}));

			REQUIRE(!policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_applicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					sequence::rating{0, sequence2.tag()}
					}
					}));

			REQUIRE(!policy3.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy3).state(),
				variant_equals(
					state_inapplicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.inapplicableSequences = {
					sequence1.tag(),
					sequence2.tag()
					}
					}));

			policy1.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {sequence1.tag()}
					}));

			REQUIRE(!policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_applicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					sequence::rating{0, sequence2.tag()}
					}
					}));

			REQUIRE(!policy3.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy3).state(),
				variant_equals(
					state_inapplicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					sequence::rating{1, sequence1.tag()}
					},
					.inapplicableSequences = {
					sequence2.tag()
					}
					}));

			policy2.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {sequence1.tag()}
					}));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {sequence2.tag()}
					}));

			REQUIRE(!policy3.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy3).state(),
				variant_equals(
					state_applicable{
					.min = 1,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					sequence::rating{1, sequence1.tag()},
					sequence::rating{1, sequence2.tag()}
					}
					}));

			policy3.consume();

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {sequence1.tag()}
					}));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {sequence2.tag()}
					}));

			REQUIRE(policy3.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy3).state(),
				variant_equals(
					state_saturated{
					.min = 1,
					.max = 1,
					.count = 1,
					.sequences = {
					sequence1.tag(),
					sequence2.tag()
					}
					}));
		}
	}
}

TEST_CASE(
	"ControlPolicy supports arbitrary times limits combined with arbitrary sequences.",
	"[expectation][expectation::control][sequence]"
)
{
	SECTION("LazySequence supports skipable expectations.")
	{
		LazySequence sequence{};
		ControlPolicy policy1{
			expect::at_most(1),
			expect::in_sequence(sequence)
		};
		CHECK(policy1.is_satisfied());
		CHECK_THAT(
			std::as_const(policy1).state(),
			variant_equals(
				state_applicable{
				.min = 0,
				.max = 1,
				.count = 0,
				.sequenceRatings = {
				sequence::rating{std::numeric_limits<int>::max(), sequence.tag()}
				}
				}));

		ControlPolicy policy2{
			expect::at_most(1),
			expect::in_sequence(sequence)
		};
		CHECK(policy2.is_satisfied());
		CHECK_THAT(
			std::as_const(policy2).state(),
			variant_equals(
				state_applicable{
				.min = 0,
				.max = 1,
				.count = 0,
				.sequenceRatings = {
				sequence::rating{std::numeric_limits<int>::max() - 1, sequence.tag()}
				}
				}));

		SECTION("Succeeds, even when nothing got consumed.")
		{
		}

		SECTION("Succeeds, when only first one got consumed.")
		{
			REQUIRE_NOTHROW(policy1.consume());

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 0,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_applicable{
					.min = 0,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					// priority may change
					sequence::rating{std::numeric_limits<int>::max() - 1, sequence.tag()}
					}
					}));
		}

		SECTION("Succeeds, when only second one got consumed.")
		{
			REQUIRE_NOTHROW(policy2.consume());

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_inapplicable{
					.min = 0,
					.max = 1,
					.count = 0,
					.inapplicableSequences = {sequence.tag()}
					}));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_saturated{
					.min = 0,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));
		}

		SECTION("Succeeds, when both got consumed.")
		{
			REQUIRE_NOTHROW(policy1.consume());
			REQUIRE_NOTHROW(policy2.consume());

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 0,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));
			REQUIRE(!policy1.is_applicable());

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_saturated{
					.min = 0,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));
		}
	}

	SECTION("GreedySequence supports skipable expectations.")
	{
		GreedySequence sequence{};
		ControlPolicy policy1{
			expect::at_most(1),
			expect::in_sequence(sequence)
		};
		CHECK(policy1.is_satisfied());
		CHECK_THAT(
			std::as_const(policy1).state(),
			variant_equals(
				state_applicable{
				.min = 0,
				.max = 1,
				.count = 0,
				.sequenceRatings = {
				sequence::rating{0, sequence.tag()}
				}
				}));

		ControlPolicy policy2{
			expect::at_most(1),
			expect::in_sequence(sequence)
		};
		CHECK(policy2.is_satisfied());
		CHECK_THAT(
			std::as_const(policy2).state(),
			variant_equals(
				state_applicable{
				.min = 0,
				.max = 1,
				.count = 0,
				.sequenceRatings = {
				sequence::rating{1, sequence.tag()}
				}
				}));

		SECTION("Succeeds, even when nothing got consumed.")
		{
		}

		SECTION("Succeeds, when only first one got consumed.")
		{
			REQUIRE_NOTHROW(policy1.consume());

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 0,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_applicable{
					.min = 0,
					.max = 1,
					.count = 0,
					.sequenceRatings = {
					// priority may change
					sequence::rating{1, sequence.tag()}
					}
					}));
		}

		SECTION("Succeeds, when only second one got consumed.")
		{
			REQUIRE_NOTHROW(policy2.consume());

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_inapplicable{
					.min = 0,
					.max = 1,
					.count = 0,
					.inapplicableSequences = {sequence.tag()}
					}));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_saturated{
					.min = 0,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));
		}

		SECTION("Succeeds, when both got consumed.")
		{
			REQUIRE_NOTHROW(policy1.consume());
			REQUIRE_NOTHROW(policy2.consume());

			REQUIRE(policy1.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy1).state(),
				variant_equals(
					state_saturated{
					.min = 0,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));

			REQUIRE(policy2.is_satisfied());
			REQUIRE_THAT(
				std::as_const(policy2).state(),
				variant_equals(
					state_saturated{
					.min = 0,
					.max = 1,
					.count = 1,
					.sequences = {sequence.tag()}
					}));
		}
	}
}

TEST_CASE(
	"expect::times and similar factories with limits create TimesConfig.",
	"[expectation][expectation::factories]"
)
{
	SECTION("times with binary limits.")
	{
		const int min = GENERATE(range(0, 5));
		const int max = min + GENERATE(range(0, 5));

		const detail::TimesConfig config = expect::times(min, max);

		REQUIRE(min == config.min());
		REQUIRE(max == config.max());
	}

	SECTION("times with binary limits.")
	{
		const int exactly = GENERATE(range(0, 5));

		const detail::TimesConfig config = expect::times(exactly);

		REQUIRE(exactly == config.min());
		REQUIRE(exactly == config.max());
	}

	SECTION("at_most")
	{
		const int limit = GENERATE(range(0, 5));

		const detail::TimesConfig config = expect::at_most(limit);

		REQUIRE(0 == config.min());
		REQUIRE(limit == config.max());
	}

	SECTION("at_least")
	{
		const int limit = GENERATE(range(0, 5));

		const detail::TimesConfig config = expect::at_least(limit);

		REQUIRE(limit == config.min());
		REQUIRE(std::numeric_limits<int>::max() == config.max());
	}

	SECTION("once")
	{
		constexpr detail::TimesConfig config = expect::once();

		REQUIRE(1 == config.min());
		REQUIRE(1 == config.max());
	}

	SECTION("twice")
	{
		constexpr detail::TimesConfig config = expect::twice();

		REQUIRE(2 == config.min());
		REQUIRE(2 == config.max());
	}
}

TEST_CASE(
	"expect::times throws, when invalid limits are provided.",
	"[expectation][expectation::factories]"
)
{
	SECTION("For binary overload.")
	{
		SECTION("Min < 0")
		{
			const int min = GENERATE(std::numeric_limits<int>::min(), -1);
			REQUIRE_THROWS_AS(
				expect::times(min, 42),
				std::invalid_argument);
		}

		SECTION("Max < 0")
		{
			const int max = GENERATE(std::numeric_limits<int>::min(), -1);
			REQUIRE_THROWS_AS(
				expect::times(42, max),
				std::invalid_argument);
		}

		SECTION("Max < Min")
		{
			const int max = GENERATE(range(0, 5));
			const int min = max + GENERATE(range(1, 5));
			REQUIRE_THROWS_AS(
				expect::times(min, max),
				std::invalid_argument);
		}
	}

	SECTION("For unary overload.")
	{
		const int exactly = GENERATE(std::numeric_limits<int>::min(), -1);
		REQUIRE_THROWS_AS(
			expect::times(exactly),
			std::invalid_argument);
	}
}

TEST_CASE(
	"expect::at_least throws, when invalit limit is given.",
	"[expectation][expectation::factories]"
)
{
	const int limit = GENERATE(std::numeric_limits<int>::min(), -1);
	REQUIRE_THROWS_AS(
		expect::times(limit),
		std::invalid_argument);
}

TEST_CASE(
	"expect::at_most throws, when invalit limit is given.",
	"[expectation][expectation::factories]"
)
{
	const int limit = GENERATE(std::numeric_limits<int>::min(), -1);
	REQUIRE_THROWS_AS(
		expect::times(limit),
		std::invalid_argument);
}
