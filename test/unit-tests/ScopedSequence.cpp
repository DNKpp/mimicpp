//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ScopedSequence.hpp"
#include "mimic++/Mock.hpp"

#include "TestReporter.hpp"

using namespace mimicpp;

namespace
{
    using TestScopedSequence = BasicScopedSequence<sequence::detail::LazyStrategy{}>;
}

TEST_CASE(
    "ScopedSequence is a non-copyable but movable type.",
    "[sequence]")
{
    STATIC_REQUIRE(!std::is_copy_constructible_v<TestScopedSequence>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<TestScopedSequence>);

    STATIC_REQUIRE(!std::is_nothrow_destructible_v<TestScopedSequence>);

    STATIC_REQUIRE(std::is_default_constructible_v<TestScopedSequence>);
    STATIC_REQUIRE(std::is_move_constructible_v<TestScopedSequence>);
    STATIC_REQUIRE(std::is_move_assignable_v<TestScopedSequence>);
}

TEST_CASE(
    "ScopedSequence checks attached expectations during destruction.",
    "[sequence]")
{
    ScopedReporter reporter{};
    Mock<void()> mock{};

    SECTION("When no expectations are attached, nothing is checked.")
    {
        TestScopedSequence sequence{};
    }

    SECTION("When single expectation is attached.")
    {
        TestScopedSequence sequence{};
        sequence += mock.expect_call();

        mock();
    }

    SECTION("When multiple expectations are attached.")
    {
        TestScopedSequence sequence{};
        sequence += mock.expect_call();
        sequence += mock.expect_call();

        mock();

        sequence += mock.expect_call();

        mock();
        mock();
    }

    CHECK_THAT(
        reporter.unfulfilled_expectations(),
        Catch::Matchers::IsEmpty());
}

TEST_CASE(
    "ScopedSequence reports unfulfilled expectations during destruction.",
    "[sequence]")
{
    ScopedReporter reporter{};
    Mock<void()> mock{};

    SECTION("When a single expectation is attached.")
    {
        std::vector<util::SourceLocation> locations{};

        {
            TestScopedSequence sequence{};
            sequence += mock.expect_call();

            std::ranges::transform(
                sequence.expectations(),
                std::back_inserter(locations),
                &ScopedExpectation::from);
        }

        REQUIRE_THAT(
            reporter.unfulfilled_expectations(),
            Catch::Matchers::SizeIs(1u));
        auto const& report = reporter.unfulfilled_expectations().front();
        CHECK(locations.front() == report.from);
    }

    SECTION("When multiple expectations are attached, the first unfulfilled is reported.")
    {
        std::optional<TestScopedSequence> sequence{std::in_place};
        *sequence += mock.expect_call();
        *sequence += mock.expect_call();
        *sequence += mock.expect_call();

        std::vector<util::SourceLocation> locations{};
        std::ranges::transform(
            sequence->expectations(),
            std::back_inserter(locations),
            &ScopedExpectation::from);
        REQUIRE_THAT(
            locations,
            Catch::Matchers::SizeIs(3));

        auto const calls = GENERATE(0u, 1u, 2u);
        for ([[maybe_unused]] auto const i : std::views::iota(0u, calls))
        {
            mock();
        }

        sequence.reset();

        REQUIRE_THAT(
            reporter.unfulfilled_expectations(),
            Catch::Matchers::SizeIs(locations.size() - calls));
        auto const& report = reporter.unfulfilled_expectations().front();
        CHECK(locations[calls] == report.from);
    }
}

TEST_CASE(
    "Expectations can manually attached to BasicScopedSequences.",
    "[sequence]")
{
    Mock<void()> mock{};

    TestScopedSequence sequence{};
    sequence += mock.expect_call();

    ScopedExpectation expectation = mock.expect_call()
                                and expect::in_sequence(sequence);

    ScopedExpectation freestandingExpectation = mock.expect_call();
    REQUIRE_THAT(
        sequence.expectations(),
        Catch::Matchers::SizeIs(1u));

    mock();
    CHECK(freestandingExpectation.is_satisfied());
    CHECK(!expectation.is_satisfied());
    CHECK(!sequence.expectations().front().is_satisfied());

    mock();
    CHECK(freestandingExpectation.is_satisfied());
    CHECK(!expectation.is_satisfied());
    CHECK(sequence.expectations().front().is_satisfied());

    mock();
    CHECK(freestandingExpectation.is_satisfied());
    CHECK(expectation.is_satisfied());
    CHECK(sequence.expectations().front().is_satisfied());
}

TEMPLATE_TEST_CASE(
    "ScopedSequences capture their construction location.",
    "[sequence]",
    LazyScopedSequence,
    GreedyScopedSequence)
{
    constexpr util::SourceLocation before{};
    TestType const sequence{};
    constexpr util::SourceLocation after{};

    CHECK_THAT(
        std::string{sequence.from().file_name()},
        Catch::Matchers::Equals(std::string{before.file_name()})
            && Catch::Matchers::Equals(std::string{after.file_name()}));
    CHECK_THAT(
        std::string{sequence.from().function_name()},
        Catch::Matchers::StartsWith(std::string{before.function_name()})
            && Catch::Matchers::StartsWith(std::string{after.function_name()}));
    CHECK(before.line() < sequence.from().line());
    CHECK(sequence.from().line() < after.line());
}
