//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/expectation/Collector.hpp"

#include <optional>

TEST_CASE(
    "A ScopedExpectation is verified during destruction.",
    "[example][example::expectation]")
{
    //! [expectation basic]
    {
        mimicpp::Mock<void()> mock{};
        SCOPED_EXP mock.expect_call(); // This creates a `mimicpp::ScopedExpectation` with a unique name.
        mock();
    } // The expectation is checked here.
    //! [expectation basic]
}

TEST_CASE(
    "A ScopedExpectation may outlive its mock target.",
    "[example][example::expectation]")
{
    //! [expectation outlive]
    std::optional<mimicpp::ScopedExpectation> expectation{};

    {
        mimicpp::Mock<void()> mock{};
        expectation = mock.expect_call();
        mock();
    }

    expectation.reset(); // The expectation is checked here, even though the actual mock-target is already destroyed.
    //! [expectation outlive]
}

TEST_CASE(
    "mimicpp::ScopedExpectations is a convenient expectation-collector.",
    "[example][example::expectation]")
{
    //! [expectation collector]
    {
        mimicpp::ScopedExpectations collector{};

        mimicpp::Mock<void()> outer{};
        collector += outer.expect_call();

        {
            mimicpp::Mock<void()> inner{};
            collector += inner.expect_call();
            inner();
        }

        outer();
    } // All expectations are checked here.
    //! [expectation collector]
}
