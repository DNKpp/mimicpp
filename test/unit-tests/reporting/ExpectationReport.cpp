//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/ExpectationReport.hpp"

using namespace mimicpp;

TEST_CASE(
    "reporting::state_inapplicable is equality comparable.",
    "[reporting]")
{
    reporting::state_inapplicable const first{
        .min = 43,
        .max = 44,
        .count = 42,
        .sequences = {
                      sequence::rating{1337, sequence::Tag{1338}},
                      sequence::rating{1339, sequence::Tag{1340}}                       },
        .inapplicableSequences = {                      {sequence::Tag{1341}}, {sequence::Tag{1342}}}
    };

    reporting::state_inapplicable second{first};

    SECTION("Compare equal, when all members are equal.")
    {
        CHECK(first == second);
        CHECK(second == first);
        CHECK_FALSE(first != second);
        CHECK_FALSE(second != first);
    }

    SECTION("Compare not equal, when min differs.")
    {
        second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when max differs.")
    {
        second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when count differs.")
    {
        second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when sequenceRatings differs.")
    {
        second.sequences = GENERATE(
            std::vector<sequence::rating>{
        },
            (std::vector{
                sequence::rating{1337, sequence::Tag{1338}}}),
            (std::vector{
                sequence::rating{1339, sequence::Tag{1340}}}),
            (std::vector{
                sequence::rating{1339, sequence::Tag{1340}},
                sequence::rating{1337, sequence::Tag{1338}}}));

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when inapplicableSequences differs.")
    {
        using SeqReport = reporting::SequenceReport;
        second.inapplicableSequences = GENERATE(
            std::vector<SeqReport>{},
            std::vector{SeqReport{sequence::Tag{1342}}},
            std::vector{SeqReport{sequence::Tag{1341}}},
            (std::vector{
                SeqReport{sequence::Tag{1342}},
                SeqReport{sequence::Tag{1341}}}));

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }
}

TEST_CASE(
    "reporting::state_applicable is equality comparable.",
    "[reporting]")
{
    reporting::state_applicable const first{
        .min = 43,
        .max = 44,
        .count = 42,
        .sequenceRatings = {
                            sequence::rating{1337, sequence::Tag{1338}},
                            sequence::rating{1339, sequence::Tag{1340}}}
    };

    reporting::state_applicable second{first};

    SECTION("Compare equal, when all members are equal.")
    {
        CHECK(first == second);
        CHECK(second == first);
        CHECK_FALSE(first != second);
        CHECK_FALSE(second != first);
    }

    SECTION("Compare not equal, when min differs.")
    {
        second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when max differs.")
    {
        second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when count differs.")
    {
        second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when sequenceRatings differs.")
    {
        second.sequenceRatings = GENERATE(
            std::vector<sequence::rating>{
        },
            (std::vector{
                sequence::rating{1337, sequence::Tag{1338}}}),
            (std::vector{
                sequence::rating{1339, sequence::Tag{1340}}}),
            (std::vector{
                sequence::rating{1339, sequence::Tag{1340}},
                sequence::rating{1337, sequence::Tag{1338}}}));

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }
}

TEST_CASE(
    "reporting::state_saturated is equality comparable.",
    "[reporting]")
{
    reporting::state_saturated const first{
        .min = 43,
        .max = 44,
        .count = 42,
        .sequences = {{sequence::Tag{1337}}, {sequence::Tag{1338}}}
    };

    reporting::state_saturated second{first};

    SECTION("Compare equal, when all members are equal.")
    {
        CHECK(first == second);
        CHECK(second == first);
        CHECK_FALSE(first != second);
        CHECK_FALSE(second != first);
    }

    SECTION("Compare not equal, when min differs.")
    {
        second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when max differs.")
    {
        second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when count differs.")
    {
        second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("Compare not equal, when inapplicableSequences differs.")
    {
        using SeqReport = reporting::SequenceReport;
        second.sequences = GENERATE(
            std::vector<SeqReport>{},
            std::vector{SeqReport{sequence::Tag{1337}}},
            std::vector{SeqReport{sequence::Tag{1338}}},
            (std::vector{
                SeqReport{sequence::Tag{1338}},
                SeqReport{sequence::Tag{1337}}}));

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }
}

TEST_CASE(
    "reporting::ExpectationReport is equality comparable.",
    "[reporting]")
{
    reporting::ExpectationReport const first{
        .from = {},
        .target = {"Mock-Name", reporting::TypeReport::make<void()>()},
        .controlReport = reporting::state_applicable{0, 1, 0},
        .finalizerDescription = "finalizer description",
        .requirementDescriptions = {"first expectation description"}
    };

    SECTION("When all members are equal, reports compare equal.")
    {
        const reporting::ExpectationReport second{first};

        CHECK(first == second);
        CHECK(second == first);
        CHECK_FALSE(first != second);
        CHECK_FALSE(second != first);
    }

    SECTION("When SourceLocation differs, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.from = {};

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("When TargetReport differs, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.target.name = "other mock-name";

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("When finalizer description differs, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.finalizerDescription = GENERATE(
            as<std::optional<StringT>>{},
            std::nullopt,
            "other finalizer description");

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("When times description differs, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.controlReport = GENERATE(
            as<reporting::control_state_t>{},
            reporting::state_applicable{0, 2, 0},
            reporting::state_inapplicable{0, 2, 0, {}, {{sequence::Tag{1337}}}},
            reporting::state_saturated{1, 1, 2});

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }

    SECTION("When expectation descriptions differ, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.requirementDescriptions = GENERATE(
            (std::vector<std::optional<StringT>>{}),
            (std::vector<std::optional<StringT>>{"other expectation description"}),
            (std::vector<std::optional<StringT>>{"expectation description", "other expectation description"}));

        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
        CHECK(first != second);
        CHECK(second != first);
    }
}

TEST_CASE(
    "reporting::RequirementOutcomes is equality-comparable.",
    "[reporting]")
{
    reporting::RequirementOutcomes const outcomes{
        .outcomes = {true}};

    SECTION("Compares equal, when both sides are equal.")
    {
        reporting::RequirementOutcomes const other = outcomes;

        CHECK(other == outcomes);
        CHECK(outcomes == other);
        CHECK_FALSE(other != outcomes);
        CHECK_FALSE(outcomes != other);
    }

    SECTION("Compares unequal, when both sides have different sizes.")
    {
        reporting::RequirementOutcomes other{
            .outcomes = GENERATE(
                std::vector<bool>{},
                std::vector{true, false},
                std::vector{false, true})};

        CHECK_FALSE(other == outcomes);
        CHECK_FALSE(outcomes == other);
        CHECK(other != outcomes);
        CHECK(outcomes != other);
    }

    SECTION("Compares unequal, when both sides have same sizes but different elements.")
    {
        reporting::RequirementOutcomes other{
            .outcomes = {false}};

        CHECK_FALSE(other == outcomes);
        CHECK_FALSE(outcomes == other);
        CHECK(other != outcomes);
        CHECK(outcomes != other);
    }
}
