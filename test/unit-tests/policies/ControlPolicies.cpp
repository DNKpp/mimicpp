//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/policies/ControlPolicies.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;
using reporting::control_state_t;
using reporting::state_applicable;
using reporting::state_inapplicable;
using reporting::state_saturated;

TEST_CASE(
    "TimesConfig has exactly 1 as limit by default.",
    "[detail][expectation][expectation::control]")
{
    constexpr detail::TimesConfig config{};

    REQUIRE(1 == config.min());
    REQUIRE(1 == config.max());
}

TEST_CASE(
    "The limits of TimesConfig can be modified.",
    "[detail][expectation][expectation::control]")
{
    int const min = GENERATE(range(0, 5));
    int const max = min + GENERATE(range(0, 5));

    detail::TimesConfig const config{min, max};

    REQUIRE(min == std::as_const(config).min());
    REQUIRE(max == std::as_const(config).max());
}

TEST_CASE(
    "TimesConfig when invalid limits are given.",
    "[detail][expectation][expectation::control]")
{
    SECTION("When max < min.")
    {
        int const max = GENERATE(range(0, 5));
        int const min = max + GENERATE(range(1, 5));
        CAPTURE(min, max);

        REQUIRE_THROWS_AS(
            (detail::TimesConfig{min, max}),
            std::invalid_argument);
    }

    SECTION("When min and/or max < 0.")
    {
        int const min = GENERATE(std::numeric_limits<int>::min(), -42, -1);
        int const max = min + GENERATE(range(0, 5));
        CAPTURE(min, max);

        REQUIRE_THROWS_AS(
            (detail::TimesConfig{min, max}),
            std::invalid_argument);
    }
}

TEST_CASE(
    "ControlPolicy can be constructed from TimesConfig and default SequenceConfig.",
    "[expectation][expectation::control]")
{
    int const min = GENERATE(range(0, 5));
    int const max = min + GENERATE(range(0, 5));
    CAPTURE(min, max);

    ControlPolicy<> policy{
        {},
        detail::TimesConfig{min, max},
        sequence::detail::Config<>{}
    };

    REQUIRE(0u == policy.sequenceCount);

    for (auto const i : std::views::iota(0, min))
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

    for (auto const i : std::views::iota(min, max))
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
    using TestSequence = sequence::detail::BasicSequenceInterface<
        sequence::Id,
        FakeSequenceStrategy{}>;
}

TEST_CASE(
    "ControlPolicy can be constructed from SequenceConfig and default TimesConfig.",
    "[expectation][expectation::control]")
{
    namespace Matches = Catch::Matchers;

    ScopedReporter reporter{};
    constexpr util::SourceLocation from{};

    SECTION("When single sequence is provided.")
    {
        std::optional<TestSequence> sequence{std::in_place};
        REQUIRE(sequence);
        std::optional policy{
            ControlPolicy{
                          from,
                          detail::TimesConfig{},
                          expect::in_sequence(*sequence)}
        };

        REQUIRE(1u == policy->sequenceCount);

        REQUIRE(from == sequence->head_from());
        REQUIRE(!std::as_const(*policy).is_satisfied());
        REQUIRE_THAT(
            std::as_const(*policy).state(),
            variant_equals(
                state_applicable{
                    .min = 1,
                    .max = 1,
                    .count = 0,
                    .sequenceRatings = {
                        sequence::rating{0, sequence->tag()}}}));

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
                        .sequences = {reporting::make_sequence_report(*sequence)}}));
            REQUIRE_FALSE(sequence->head_from());
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
        std::optional<TestSequence> firstSequence{std::in_place};
        std::optional<TestSequence> secondSequence{std::in_place};
        std::optional policy{
            ControlPolicy{
                          from,
                          detail::TimesConfig{},
                          expect::in_sequences(*firstSequence, *secondSequence)}
        };

        REQUIRE(2u == policy->sequenceCount);
        REQUIRE(from == firstSequence->head_from());
        REQUIRE(from == secondSequence->head_from());

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
                                        sequence::rating{0, secondSequence->tag()}}
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
                        .sequences = {reporting::make_sequence_report(*firstSequence), reporting::make_sequence_report(*secondSequence)}
            }));
            REQUIRE_FALSE(firstSequence->head_from());
            REQUIRE_FALSE(secondSequence->head_from());
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
    "[expectation][expectation::control]")
{
    namespace Matches = Catch::Matchers;

    ScopedReporter reporter{};

    {
        int const min = GENERATE(range(0, 5));
        int const max = min + GENERATE(range(0, 5));
        CAPTURE(min, max);

        TestSequence sequence{};
        constexpr util::SourceLocation from{};
        ControlPolicy policy{
            from,
            expect::times(min, max),
            expect::in_sequence(sequence)};

        REQUIRE(1u == policy.sequenceCount);

        for (auto const i : std::views::iota(0, min))
        {
            REQUIRE(from == sequence.head_from());
            REQUIRE(!std::as_const(policy).is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy).state(),
                variant_equals(
                    state_applicable{
                        .min = min,
                        .max = max,
                        .count = i,
                        .sequenceRatings = {
                            sequence::rating{0, sequence.tag()}}}));

            REQUIRE_NOTHROW(policy.consume());
        }

        REQUIRE(std::as_const(policy).is_satisfied());

        SECTION("Does not report, when satisfied.")
        {
        }

        SECTION("Can be consumed until saturated.")
        {
            for (auto const i : std::views::iota(min, max))
            {
                REQUIRE(from == sequence.head_from());
                REQUIRE(std::as_const(policy).is_satisfied());
                REQUIRE_THAT(
                    std::as_const(policy).state(),
                    variant_equals(
                        state_applicable{
                            .min = min,
                            .max = max,
                            .count = i,
                            .sequenceRatings = {
                                sequence::rating{0, sequence.tag()}}}));

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
                        .sequences = {reporting::make_sequence_report(sequence)}}));
            REQUIRE_FALSE(sequence.head_from());
        }
    }

    REQUIRE_THAT(
        reporter.errors(),
        Matches::IsEmpty());
}

TEST_CASE(
    "ControlPolicy checks whether the given call occurs in sequence.",
    "[expectation][expectation::control][sequence]")
{
    namespace Matches = Catch::Matchers;

    TestSequence sequence{};

    SECTION("When sequence contains just a single expectation.")
    {
        auto const count = GENERATE(range(1, 5));
        ControlPolicy policy{
            {},
            expect::times(count),
            expect::in_sequence(sequence)};

        REQUIRE(1u == policy.sequenceCount);

        for (int const i : std::views::iota(0, count))
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
                            sequence::rating{0, sequence.tag()}}}));

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
                    .sequences = {reporting::make_sequence_report(sequence)}}));
    }

    SECTION("When sequence has multiple expectations, the order matters.")
    {
        ControlPolicy policy1{
            {},
            expect::once(),
            expect::in_sequence(sequence)};

        REQUIRE(1u == policy1.sequenceCount);

        auto const count2 = GENERATE(range(1, 5));
        ControlPolicy policy2{
            {},
            expect::times(count2),
            expect::in_sequence(sequence)};

        REQUIRE(1u == policy2.sequenceCount);

        SECTION("When first expectation is satisfied, then the second one becomes applicable.")
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
                            sequence::rating{0, sequence.tag()}}}));

            REQUIRE(!std::as_const(policy2).is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_inapplicable{
                        .min = count2,
                        .max = count2,
                        .count = 0,
                        .inapplicableSequences = {reporting::make_sequence_report(sequence)}}));

            REQUIRE_NOTHROW(policy1.consume());

            for (int const i : std::views::iota(0, count2))
            {
                REQUIRE(policy1.is_satisfied());
                REQUIRE_THAT(
                    std::as_const(policy1).state(),
                    variant_equals(
                        state_saturated{
                            .min = 1,
                            .max = 1,
                            .count = 1,
                            .sequences = {reporting::make_sequence_report(sequence)}}));

                REQUIRE(!std::as_const(policy2).is_satisfied());
                REQUIRE_THAT(
                    std::as_const(policy2).state(),
                    variant_equals(
                        state_applicable{
                            .min = count2,
                            .max = count2,
                            .count = i,
                            .sequenceRatings = {
                                sequence::rating{1, sequence.tag()}}}));

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
                        .sequences = {reporting::make_sequence_report(sequence)}}));

            REQUIRE(policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_saturated{
                        .min = count2,
                        .max = count2,
                        .count = count2,
                        .sequences = {reporting::make_sequence_report(sequence)}}));
        }
    }
}

TEST_CASE(
    "An expectation can be part of multiple sequences.",
    "[expectation][expectation::control][sequence]")
{
    namespace Matches = Catch::Matchers;

    SECTION("When multiple sequences are given.")
    {
        TestSequence sequence1{};
        TestSequence sequence2{};

        SECTION("When the first expectation is the prefix of multiple sequences.")
        {
            ControlPolicy policy1{
                {},
                expect::once(),
                expect::in_sequences(sequence1, sequence2)};
            REQUIRE(2u == policy1.sequenceCount);

            ControlPolicy policy2{
                {},
                expect::once(),
                expect::in_sequence(sequence2)};
            REQUIRE(1u == policy2.sequenceCount);

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
                        .inapplicableSequences = {reporting::make_sequence_report(sequence2)}}));

            policy1.consume();

            REQUIRE(policy1.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy1).state(),
                variant_equals(
                    state_saturated{
                        .min = 1,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence1), reporting::make_sequence_report(sequence2)}
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
                            sequence::rating{1, sequence2.tag()}}}));

            policy2.consume();

            REQUIRE(policy1.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy1).state(),
                variant_equals(
                    state_saturated{
                        .min = 1,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence1), reporting::make_sequence_report(sequence2)}
            }));

            REQUIRE(policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_saturated{
                        .min = 1,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence2)}}));
        }

        SECTION("When an expectation waits for multiple sequences.")
        {
            ControlPolicy policy1{
                {},
                expect::once(),
                expect::in_sequence(sequence1)};
            REQUIRE(1u == policy1.sequenceCount);

            ControlPolicy policy2{
                {},
                expect::once(),
                expect::in_sequence(sequence2)};
            REQUIRE(1u == policy2.sequenceCount);

            ControlPolicy policy3{
                {},
                expect::once(),
                expect::in_sequences(sequence1, sequence2)};
            REQUIRE(2u == policy3.sequenceCount);

            REQUIRE(!policy1.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy1).state(),
                variant_equals(
                    state_applicable{
                        .min = 1,
                        .max = 1,
                        .count = 0,
                        .sequenceRatings = {
                            sequence::rating{0, sequence1.tag()}}}));

            REQUIRE(!policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_applicable{
                        .min = 1,
                        .max = 1,
                        .count = 0,
                        .sequenceRatings = {
                            sequence::rating{0, sequence2.tag()}}}));

            REQUIRE(!policy3.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy3).state(),
                variant_equals(
                    state_inapplicable{
                        .min = 1,
                        .max = 1,
                        .count = 0,
                        .inapplicableSequences = {reporting::make_sequence_report(sequence1), reporting::make_sequence_report(sequence2)}
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
                        .sequences = {reporting::make_sequence_report(sequence1)}}));

            REQUIRE(!policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_applicable{
                        .min = 1,
                        .max = 1,
                        .count = 0,
                        .sequenceRatings = {
                            sequence::rating{0, sequence2.tag()}}}));

            REQUIRE(!policy3.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy3).state(),
                variant_equals(
                    state_inapplicable{
                        .min = 1,
                        .max = 1,
                        .count = 0,
                        .sequences = {
                            sequence::rating{1, sequence1.tag()}},
                        .inapplicableSequences = {reporting::make_sequence_report(sequence2)}}));

            policy2.consume();

            REQUIRE(policy1.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy1).state(),
                variant_equals(
                    state_saturated{
                        .min = 1,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence1)}}));

            REQUIRE(policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_saturated{
                        .min = 1,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence2)}}));

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
                                            sequence::rating{1, sequence2.tag()}}
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
                        .sequences = {reporting::make_sequence_report(sequence1)}}));

            REQUIRE(policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_saturated{
                        .min = 1,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence2)}}));

            REQUIRE(policy3.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy3).state(),
                variant_equals(
                    state_saturated{
                        .min = 1,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence1), reporting::make_sequence_report(sequence2)}
            }));
        }
    }
}

TEST_CASE(
    "ControlPolicy supports arbitrary times limits combined with arbitrary sequences.",
    "[expectation][expectation::control][sequence]")
{
    SECTION("LazySequence supports skippable expectations.")
    {
        LazySequence sequence{};
        ControlPolicy policy1{
            {},
            expect::at_most(1),
            expect::in_sequence(sequence)};
        CHECK(1u == policy1.sequenceCount);
        CHECK(policy1.is_satisfied());
        CHECK_THAT(
            std::as_const(policy1).state(),
            variant_equals(
                state_applicable{
                    .min = 0,
                    .max = 1,
                    .count = 0,
                    .sequenceRatings = {
                        sequence::rating{std::numeric_limits<int>::max(), sequence.tag()}}}));

        ControlPolicy policy2{
            {},
            expect::at_most(1),
            expect::in_sequence(sequence)};
        CHECK(1u == policy2.sequenceCount);
        CHECK(policy2.is_satisfied());
        CHECK_THAT(
            std::as_const(policy2).state(),
            variant_equals(
                state_applicable{
                    .min = 0,
                    .max = 1,
                    .count = 0,
                    .sequenceRatings = {
                        sequence::rating{std::numeric_limits<int>::max() - 1, sequence.tag()}}}));

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
                        .sequences = {reporting::make_sequence_report(sequence)}}));

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
                            sequence::rating{std::numeric_limits<int>::max() - 1, sequence.tag()}}}));
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
                        .inapplicableSequences = {reporting::make_sequence_report(sequence)}}));

            REQUIRE(policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_saturated{
                        .min = 0,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence)}}));
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
                        .sequences = {reporting::make_sequence_report(sequence)}}));
            REQUIRE(!policy1.is_applicable());

            REQUIRE(policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_saturated{
                        .min = 0,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence)}}));
        }
    }

    SECTION("GreedySequence supports skippable expectations.")
    {
        GreedySequence sequence{};
        ControlPolicy policy1{
            {},
            expect::at_most(1),
            expect::in_sequence(sequence)};
        CHECK(1u == policy1.sequenceCount);
        CHECK(policy1.is_satisfied());
        CHECK_THAT(
            std::as_const(policy1).state(),
            variant_equals(
                state_applicable{
                    .min = 0,
                    .max = 1,
                    .count = 0,
                    .sequenceRatings = {
                        sequence::rating{0, sequence.tag()}}}));

        ControlPolicy policy2{
            {},
            expect::at_most(1),
            expect::in_sequence(sequence)};
        CHECK(1u == policy2.sequenceCount);
        CHECK(policy2.is_satisfied());
        CHECK_THAT(
            std::as_const(policy2).state(),
            variant_equals(
                state_applicable{
                    .min = 0,
                    .max = 1,
                    .count = 0,
                    .sequenceRatings = {
                        sequence::rating{1, sequence.tag()}}}));

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
                        .sequences = {reporting::make_sequence_report(sequence)}}));

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
                            sequence::rating{1, sequence.tag()}}}));
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
                        .inapplicableSequences = {reporting::make_sequence_report(sequence)}}));

            REQUIRE(policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_saturated{
                        .min = 0,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence)}}));
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
                        .sequences = {reporting::make_sequence_report(sequence)}}));

            REQUIRE(policy2.is_satisfied());
            REQUIRE_THAT(
                std::as_const(policy2).state(),
                variant_equals(
                    state_saturated{
                        .min = 0,
                        .max = 1,
                        .count = 1,
                        .sequences = {reporting::make_sequence_report(sequence)}}));
        }
    }
}

TEST_CASE(
    "expect::times and similar factories with limits create TimesConfig.",
    "[expectation][expectation::factories]")
{
    SECTION("times with binary limits.")
    {
        int const min = GENERATE(range(0, 5));
        int const max = min + GENERATE(range(0, 5));
        CAPTURE(min, max);

        detail::TimesConfig const config = expect::times(min, max);

        REQUIRE(min == config.min());
        REQUIRE(max == config.max());
    }

    SECTION("times with binary limits.")
    {
        int const exactly = GENERATE(range(0, 5));
        CAPTURE(exactly);

        detail::TimesConfig const config = expect::times(exactly);

        REQUIRE(exactly == config.min());
        REQUIRE(exactly == config.max());
    }

    SECTION("at_most")
    {
        int const limit = GENERATE(range(0, 5));
        CAPTURE(limit);

        detail::TimesConfig const config = expect::at_most(limit);

        REQUIRE(0 == config.min());
        REQUIRE(limit == config.max());
    }

    SECTION("at_least")
    {
        int const limit = GENERATE(range(0, 5));
        CAPTURE(limit);

        detail::TimesConfig const config = expect::at_least(limit);

        REQUIRE(limit == config.min());
        REQUIRE(std::numeric_limits<int>::max() == config.max());
    }

    SECTION("never")
    {
        constexpr detail::TimesConfig config = expect::never();

        REQUIRE(0 == config.min());
        REQUIRE(0 == config.max());
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

    SECTION("any_times")
    {
        constexpr detail::TimesConfig config = expect::any_times();

        REQUIRE(0 == config.min());
        REQUIRE(std::numeric_limits<int>::max() == config.max());
    }
}

TEST_CASE(
    "expect::times throws, when invalid limits are provided.",
    "[expectation][expectation::factories]")
{
    SECTION("For binary overload.")
    {
        SECTION("Min < 0")
        {
            int const min = GENERATE(std::numeric_limits<int>::min(), -1);
            CAPTURE(min);
            REQUIRE_THROWS_AS(
                expect::times(min, 42),
                std::invalid_argument);
        }

        SECTION("Max < 0")
        {
            int const max = GENERATE(std::numeric_limits<int>::min(), -1);
            CAPTURE(max);
            REQUIRE_THROWS_AS(
                expect::times(42, max),
                std::invalid_argument);
        }

        SECTION("Max < Min")
        {
            int const max = GENERATE(range(0, 5));
            int const min = max + GENERATE(range(1, 5));
            CAPTURE(min, max);
            REQUIRE_THROWS_AS(
                expect::times(min, max),
                std::invalid_argument);
        }
    }

    SECTION("For unary overload.")
    {
        int const exactly = GENERATE(std::numeric_limits<int>::min(), -1);
        CAPTURE(exactly);
        REQUIRE_THROWS_AS(
            expect::times(exactly),
            std::invalid_argument);
    }
}

TEST_CASE(
    "expect::at_least throws, when invalid limit is given.",
    "[expectation][expectation::factories]")
{
    int const limit = GENERATE(std::numeric_limits<int>::min(), -1);
    CAPTURE(limit);
    REQUIRE_THROWS_AS(
        expect::times(limit),
        std::invalid_argument);
}

TEST_CASE(
    "expect::at_most throws, when invalid limit is given.",
    "[expectation][expectation::factories]")
{
    int const limit = GENERATE(std::numeric_limits<int>::min(), -1);
    CAPTURE(limit);
    REQUIRE_THROWS_AS(
        expect::times(limit),
        std::invalid_argument);
}
