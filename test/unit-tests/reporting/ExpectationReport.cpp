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
    const reporting::state_inapplicable first{
        .min = 43,
        .max = 44,
        .count = 42,
        .sequenceRatings = {
                            sequence::rating{1337, sequence::Tag{1338}},
                            sequence::rating{1339, sequence::Tag{1340}}                     },
        .inapplicableSequences = {                        sequence::Tag{1341}, sequence::Tag{1342}}
    };

    reporting::state_inapplicable second{first};

    SECTION("Compare equal, when all members are equal.")
    {
        REQUIRE(first == second);
        REQUIRE(second == first);
        REQUIRE_FALSE(first != second);
        REQUIRE_FALSE(second != first);
    }

    SECTION("Compare not equal, when min differs.")
    {
        second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("Compare not equal, when max differs.")
    {
        second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("Compare not equal, when count differs.")
    {
        second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
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

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("Compare not equal, when inapplicableSequences differs.")
    {
        second.inapplicableSequences = GENERATE(
            std::vector<sequence::Tag>{},
            std::vector{sequence::Tag{1342}},
            std::vector{sequence::Tag{1341}},
            (std::vector{
                sequence::Tag{1342},
                sequence::Tag{1341}}));

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }
}

TEST_CASE(
    "reporting::state_applicable is equality comparable.",
    "[reporting]")
{
    const reporting::state_applicable first{
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
        REQUIRE(first == second);
        REQUIRE(second == first);
        REQUIRE_FALSE(first != second);
        REQUIRE_FALSE(second != first);
    }

    SECTION("Compare not equal, when min differs.")
    {
        second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("Compare not equal, when max differs.")
    {
        second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("Compare not equal, when count differs.")
    {
        second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
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

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }
}

TEST_CASE(
    "reporting::state_saturated is equality comparable.",
    "[reporting]")
{
    const reporting::state_saturated first{
        .min = 43,
        .max = 44,
        .count = 42,
        .sequences = {
                      sequence::Tag{1337},
                      sequence::Tag{1338}}
    };

    reporting::state_saturated second{first};

    SECTION("Compare equal, when all members are equal.")
    {
        REQUIRE(first == second);
        REQUIRE(second == first);
        REQUIRE_FALSE(first != second);
        REQUIRE_FALSE(second != first);
    }

    SECTION("Compare not equal, when min differs.")
    {
        second.min = GENERATE(std::numeric_limits<int>::min(), 0, 42, 44, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("Compare not equal, when max differs.")
    {
        second.max = GENERATE(std::numeric_limits<int>::min(), 0, 43, 45, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("Compare not equal, when count differs.")
    {
        second.count = GENERATE(std::numeric_limits<int>::min(), 0, 41, 43, std::numeric_limits<int>::max());

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("Compare not equal, when inapplicableSequences differs.")
    {
        second.sequences = GENERATE(
            std::vector<sequence::Tag>{},
            std::vector{sequence::Tag{1337}},
            std::vector{sequence::Tag{1338}},
            (std::vector{
                sequence::Tag{1338},
                sequence::Tag{1337}}));

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }
}

TEST_CASE(
    "reporting::detail::print_times_state converts the times state to text.",
    "[reporting][detail]")
{
    namespace Matches = Catch::Matchers;

    SECTION("When current == max.")
    {
        const auto [min, max] = GENERATE(
            (table<std::size_t, std::size_t>)({
                {0,  1},
                {0,  2},
                {3, 42},
        }));

        StringStreamT ss{};
        reporting::detail::print_times_state(
            std::ostreambuf_iterator{ss},
            max,
            min,
            max);

        REQUIRE_THAT(
            ss.str(),
            Matches::StartsWith("already saturated (matched "));
    }

    SECTION("When min <= current < max.")
    {
        const auto [current, min, max] = GENERATE(
            (table<std::size_t, std::size_t, std::size_t>)({
                { 0, 0,  1},
                { 1, 0,  2},
                {21, 3, 42},
        }));

        StringStreamT ss{};
        reporting::detail::print_times_state(
            std::ostreambuf_iterator{ss},
            current,
            min,
            max);

        REQUIRE_THAT(
            ss.str(),
            Matches::Matches("accepts further matches \\(matched \\d+ out of \\d+ times\\)"));
    }

    SECTION("When current < min.")
    {
        const auto [current, min, max] = GENERATE(
            (table<std::size_t, std::size_t, std::size_t>)({
                {0, 1,  2},
                {1, 2,  5},
                {2, 3, 42},
        }));

        StringStreamT ss{};
        reporting::detail::print_times_state(
            std::ostreambuf_iterator{ss},
            current,
            min,
            max);

        REQUIRE_THAT(
            ss.str(),
            Matches::StartsWith("matched "));
    }
}

TEST_CASE(
    "reporting::detail::control_state_printer converts control states to text.",
    "[reporting][detail]")
{
    namespace Matches = Catch::Matchers;

    SECTION("When state_applicable is given.")
    {
        SECTION("Sequence text is omitted, when not attached to a sequence.")
        {
            const reporting::state_applicable state{
                .min = 42,
                .max = 1337,
                .count = 256,
                .sequenceRatings = {}};

            StringStreamT ss{};
            std::invoke(
                reporting::detail::control_state_printer{},
                std::ostreambuf_iterator{ss},
                state);

            REQUIRE_THAT(
                ss.str(),
                Matches::Equals("accepts further matches (matched 256 out of 1337 times)"));
        }

        SECTION("Otherwise sequence text is added.")
        {
            const reporting::state_applicable state{
                .min = 42,
                .max = 1337,
                .count = 256,
                .sequenceRatings = {
                    sequence::rating{0, sequence::Tag{1337}}}};

            StringStreamT ss{};
            std::invoke(
                reporting::detail::control_state_printer{},
                std::ostreambuf_iterator{ss},
                state);

            REQUIRE_THAT(
                ss.str(),
                Matches::Equals(
                    "accepts further matches (matched 256 out of 1337 times),\n"
                    "\tand is the current element of 1 sequence(s)."));
        }
    }

    SECTION("When state_inapplicable is given.")
    {
        const reporting::state_inapplicable state{
            .min = 42,
            .max = 1337,
            .count = 256,
            .sequenceRatings = {
                                sequence::rating{0, sequence::Tag{1337}}},
            .inapplicableSequences = {sequence::Tag{1338}, sequence::Tag{1339}}
        };

        StringStreamT ss{};
        std::invoke(
            reporting::detail::control_state_printer{},
            std::ostreambuf_iterator{ss},
            state);

        REQUIRE_THAT(
            ss.str(),
            Matches::Equals(
                "accepts further matches (matched 256 out of 1337 times),\n"
                "\tbut is not the current element of 2 sequence(s) (3 total)."));
    }

    SECTION("When state_saturated is given.")
    {
        SECTION("Sequence text is omitted, when not attached to a sequence.")
        {
            const reporting::state_saturated state{
                .min = 42,
                .max = 1337,
                .count = 1337};

            StringStreamT ss{};
            std::invoke(
                reporting::detail::control_state_printer{},
                std::ostreambuf_iterator{ss},
                state);

            REQUIRE_THAT(
                ss.str(),
                Matches::Equals("already saturated (matched 1337 times)"));
        }

        SECTION("Otherwise sequence text is added.")
        {
            const reporting::state_saturated state{
                .min = 42,
                .max = 1337,
                .count = 1337,
                .sequences = {
                    sequence::Tag{1337}}};

            StringStreamT ss{};
            std::invoke(
                reporting::detail::control_state_printer{},
                std::ostreambuf_iterator{ss},
                state);

            REQUIRE_THAT(
                ss.str(),
                Matches::Equals(
                    "already saturated (matched 1337 times),\n"
                    "\tand is part of 1 sequence(s)."));
        }
    }
}

TEST_CASE(
    "reporting::ExpectationReport is equality comparable.",
    "[reporting]")
{
    const reporting::ExpectationReport first{
        .info = detail::expectation_info{
                                         .sourceLocation = std::source_location::current(),
                                         .mockName = "Mock-Name"},
        .controlReport = reporting::state_applicable{0, 1, 0},
        .finalizerDescription = "finalizer description",
        .requirementDescriptions = {"first expectation description"}
    };

    SECTION("When all members are equal, reports compare equal.")
    {
        const reporting::ExpectationReport second{first};

        REQUIRE(first == second);
        REQUIRE(second == first);
        REQUIRE_FALSE(first != second);
        REQUIRE_FALSE(second != first);
    }

    SECTION("When expectation_info differs, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.info.mockName = "other mock-name";

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("When finalizer description differs, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.finalizerDescription = GENERATE(
            as<std::optional<StringT>>{},
            std::nullopt,
            "other finalizer description");

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("When times description differs, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.controlReport = GENERATE(
            as<reporting::control_state_t>{},
            reporting::state_applicable{0, 2, 0},
            reporting::state_inapplicable{0, 2, 0, {}, std::vector<sequence::Tag>{1337}},
            reporting::state_saturated{1, 1, 2});

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }

    SECTION("When expectation descriptions differ, reports do not compare equal.")
    {
        reporting::ExpectationReport second{first};
        second.requirementDescriptions = GENERATE(
            (std::vector<std::optional<StringT>>{}),
            (std::vector<std::optional<StringT>>{"other expectation description"}),
            (std::vector<std::optional<StringT>>{"expectation description", "other expectation description"}));

        REQUIRE_FALSE(first == second);
        REQUIRE_FALSE(second == first);
        REQUIRE(first != second);
        REQUIRE(second != first);
    }
}
