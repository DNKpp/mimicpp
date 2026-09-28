//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/expectation/Collector.hpp"
#include "mimic++/Mock.hpp"

#include "TestReporter.hpp"

using namespace mimicpp;

TEST_CASE(
    "expectation::Collector is a non-copyable but movable type.",
    "[expectation][expectation::collector]")
{
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<expectation::Collector>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<expectation::Collector>);

    STATIC_REQUIRE_FALSE(std::is_nothrow_destructible_v<expectation::Collector>);

    STATIC_REQUIRE(std::is_default_constructible_v<expectation::Collector>);
    STATIC_REQUIRE(std::is_move_constructible_v<expectation::Collector>);
    STATIC_REQUIRE(std::is_move_assignable_v<expectation::Collector>);
}

TEST_CASE(
    "expectation::Collector checks attached expectations during destruction.",
    "[expectation][expectation::collector]")
{
    ScopedReporter reporter{};
    Mock<void()> mock{};

    SECTION("When no expectations are attached, nothing is checked.")
    {
        expectation::Collector collector{};
    }

    SECTION("When a single expectation is attached.")
    {
        expectation::Collector collector{};
        collector += mock.expect_call();

        mock();
    }

    SECTION("When multiple expectations are attached.")
    {
        expectation::Collector collector{};
        collector += mock.expect_call();
        collector += mock.expect_call()
                 and expect::twice();

        mock();

        collector += mock.expect_call()
                 and expect::never();

        mock();
        mock();
    }

    CHECK_THAT(
        reporter.unfulfilled_expectations(),
        Catch::Matchers::IsEmpty());
}

TEST_CASE(
    "expectation::Collector::attach is an alias to the operator+=",
    "[expectation][expectation::collector]")
{
    ScopedReporter reporter{};
    Mock<void()> mock{};

    expectation::Collector collector{};
    collector.attach(
        mock.expect_call()
        and expect::never());

    CHECK_THAT(
        reporter.unfulfilled_expectations(),
        Catch::Matchers::IsEmpty());
}

TEST_CASE(
    "expectation::Collector reports unfulfilled expectations during destruction.",
    "[expectation][expectation::collector]")
{
    ScopedReporter reporter{};

    SECTION("When a single expectation is attached.")
    {
        Mock<void()> mock{};
        std::vector<util::SourceLocation> locations{};

        {
            expectation::Collector collector{};
            collector += mock.expect_call();

            std::ranges::transform(
                collector.expectations(),
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
        Mock<void(int)> mock{};
        std::optional<expectation::Collector> collector{std::in_place};
        *collector += mock.expect_call(0);
        *collector += mock.expect_call(1);
        *collector += mock.expect_call(2);

        std::vector<util::SourceLocation> locations{};
        std::ranges::transform(
            collector->expectations(),
            std::back_inserter(locations),
            &ScopedExpectation::from);
        REQUIRE_THAT(
            locations,
            Catch::Matchers::SizeIs(3));

        auto const calls = GENERATE(0u, 1u, 2u);
        CAPTURE(calls);
        for ([[maybe_unused]] auto const i : std::views::iota(0u, calls))
        {
            mock(i);
        }

        collector.reset();

        REQUIRE_THAT(
            reporter.unfulfilled_expectations(),
            Catch::Matchers::SizeIs(locations.size() - calls));
        auto const& report = reporter.unfulfilled_expectations().front();
        CHECK(locations[calls] == report.from);
    }
}

TEST_CASE(
    "expectation::Collector captures the expectation construction location.",
    "[expectation][expectation::collector]")
{
    ScopedReporter reporter{};
    Mock<void()> mock{};
    expectation::Collector collector{};
    constexpr util::SourceLocation before{};
    collector += mock.expect_call() and expect::never();
    constexpr util::SourceLocation after{};

    auto const& exp = collector.expectations().front();

    CHECK_THAT(
        std::string{exp.from().file_name()},
        Catch::Matchers::Equals(std::string{before.file_name()})
            && Catch::Matchers::Equals(std::string{after.file_name()}));
    CHECK_THAT(
        std::string{exp.from().function_name()},
        Catch::Matchers::StartsWith(std::string{before.function_name()})
            && Catch::Matchers::StartsWith(std::string{after.function_name()}));
    CHECK(before.line() < exp.from().line());
    CHECK(exp.from().line() < after.line());
}
