// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ControlPolicy.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>

#include "TestReporter.hpp"

using namespace mimicpp;

TEST_CASE(
	"TimesConfig has exactly 1 as limit by default.",
	"[expectation][expectation::control]"
)
{
	constexpr TimesConfig config{};

	REQUIRE(1 == config.min());
	REQUIRE(1 == config.max());
}

TEST_CASE(
	"The limits of TimesConfig can be modified.",
	"[expectation][expectation::control]"
)
{
	TimesConfig config{};

	const int min = GENERATE(range(0, 5));
	const int max = min + GENERATE(range(0, 5));

	config.set_limits(min, max);

	REQUIRE(min == std::as_const(config).min());
	REQUIRE(max == std::as_const(config).max());
}

TEST_CASE(
	"TimesConfig when invalid limits are given.",
	"[expectation][expectation::control]"
)
{
	TimesConfig config{};

	SECTION("When max > min.")
	{
		const int max = GENERATE(range(0, 5));
		const int min = max + GENERATE(range(1, 5));

		REQUIRE_THROWS_AS(
			config.set_limits(min, max),
			std::invalid_argument);
	}

	SECTION("When min and/or max < 0.")
	{
		const int min = GENERATE(std::numeric_limits<int>::min(), -42, -1);
		const int max = min + GENERATE(range(0, 5));

		REQUIRE_THROWS_AS(
			config.set_limits(min, max),
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
		std::invoke(
			[=]
			{
				TimesConfig config{};
				config.set_limits(min, max);
				return config;
			}),
		sequence::detail::SequenceConfig<>{}
	};

	for ([[maybe_unused]] auto i : std::views::iota(0, min))
	{
		REQUIRE(!std::as_const(policy).is_satisfied());
		REQUIRE(std::as_const(policy).is_applicable());
		REQUIRE(std::ranges::empty(policy.priorities()));

		REQUIRE_NOTHROW(policy.consume());
	}

	REQUIRE(std::as_const(policy).is_satisfied());

	for ([[maybe_unused]] auto i : std::views::iota(min, max))
	{
		REQUIRE(std::as_const(policy).is_satisfied());
		REQUIRE(std::as_const(policy).is_applicable());
		REQUIRE(std::ranges::empty(policy.priorities()));

		REQUIRE_NOTHROW(policy.consume());
	}

	REQUIRE(std::as_const(policy).is_satisfied());
	REQUIRE(!std::as_const(policy).is_applicable());
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
		std::optional<SequenceT> sequence{std::in_place};
		std::optional policy{
			ControlPolicy{
				TimesConfig{},
				expect::in_sequence(*sequence)
			}
		};

		REQUIRE(1 == std::ranges::ssize(std::as_const(*policy).priorities()));
		REQUIRE(std::as_const(*policy).priorities()[0].tag == sequence->tag());
		REQUIRE(std::as_const(*policy).priorities()[0].priority);

		REQUIRE(!std::as_const(*policy).is_satisfied());
		REQUIRE(std::as_const(*policy).is_applicable());

		SECTION("Is satisfied, when consumed once.")
		{
			REQUIRE_NOTHROW(policy->consume());

			REQUIRE(std::as_const(*policy).is_satisfied());
			REQUIRE(!std::as_const(*policy).is_applicable());
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
		std::optional<SequenceT> firstSequence{std::in_place};
		std::optional<SequenceT> secondSequence{std::in_place};
		std::optional policy{
			ControlPolicy{
				TimesConfig{},
				expect::in_sequences(
					*firstSequence,
					*secondSequence)
			}
		};

		REQUIRE(2 == std::ranges::ssize(std::as_const(*policy).priorities()));
		REQUIRE(std::as_const(*policy).priorities()[0].tag == firstSequence->tag());
		REQUIRE(std::as_const(*policy).priorities()[0].priority);
		REQUIRE(std::as_const(*policy).priorities()[1].tag == secondSequence->tag());
		REQUIRE(std::as_const(*policy).priorities()[1].priority);

		REQUIRE(!std::as_const(*policy).is_satisfied());
		REQUIRE(std::as_const(*policy).is_applicable());

		SECTION("Is satisfied, when consumed once.")
		{
			REQUIRE_NOTHROW(policy->consume());

			REQUIRE(std::as_const(*policy).is_satisfied());
			REQUIRE(!std::as_const(*policy).is_applicable());
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

		SequenceT sequence{};
		ControlPolicy policy{
			expect::times(min, max),
			expect::in_sequence(sequence)
		};

		for ([[maybe_unused]] auto i : std::views::iota(0, min))
		{
			REQUIRE(!std::as_const(policy).is_satisfied());
			REQUIRE(std::as_const(policy).is_applicable());

			REQUIRE(1 == std::ranges::ssize(std::as_const(policy).priorities()));
			REQUIRE(std::as_const(policy).priorities()[0].tag == sequence.tag());
			REQUIRE(std::as_const(policy).priorities()[0].priority);

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
				REQUIRE(std::as_const(policy).is_applicable());

				REQUIRE(1 == std::ranges::ssize(std::as_const(policy).priorities()));
				REQUIRE(std::as_const(policy).priorities()[0].tag == sequence.tag());
				REQUIRE(std::as_const(policy).priorities()[0].priority);

				REQUIRE_NOTHROW(policy.consume());
			}

			REQUIRE(std::as_const(policy).is_satisfied());
			REQUIRE(!std::as_const(policy).is_applicable());
		}
	}

	REQUIRE_THAT(
		reporter.errors(),
		Matches::IsEmpty());
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

		const TimesConfig config = expect::times(min, max);

		REQUIRE(min == config.min());
		REQUIRE(max == config.max());
	}

	SECTION("times with binary limits.")
	{
		const int exactly = GENERATE(range(0, 5));

		const TimesConfig config = expect::times(exactly);

		REQUIRE(exactly == config.min());
		REQUIRE(exactly == config.max());
	}

	SECTION("at_most")
	{
		const int limit = GENERATE(range(0, 5));

		const TimesConfig config = expect::at_most(limit);

		REQUIRE(0 == config.min());
		REQUIRE(limit == config.max());
	}

	SECTION("at_least")
	{
		const int limit = GENERATE(range(0, 5));

		const TimesConfig config = expect::at_least(limit);

		REQUIRE(limit == config.min());
		REQUIRE(std::numeric_limits<int>::max() == config.max());
	}

	SECTION("once")
	{
		constexpr TimesConfig config = expect::once();

		REQUIRE(1 == config.min());
		REQUIRE(1 == config.max());
	}

	SECTION("twice")
	{
		constexpr TimesConfig config = expect::twice();

		REQUIRE(2 == config.min());
		REQUIRE(2 == config.max());
	}
}

TEST_CASE(
	"mimicpp::expect::times throws, when invalid limits are provided.",
	"[expectation][expectation::factories]"
)
{
	const std::size_t max = GENERATE(range(0u, 5u));
	const std::size_t min = max + GENERATE(range(1u, 5u));

	REQUIRE_THROWS_AS(
		expect::times(min, max),
		std::invalid_argument);
}
