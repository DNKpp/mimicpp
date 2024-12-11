// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ExpectationPolicies.hpp"
#include "mimic++/Mock.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE(
    "once() requires the expectation to be matched exactly once.",
    "[example][example::times]")
{
    //! [once]
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void()> mock{};
    SCOPED_EXP mock.expect_call()
        and expect::once();

    mock();
    //! [once]
}

TEST_CASE(
    "twice() requires the expectation to be matched exactly twice.",
    "[example][example::times]")
{
    //! [twice]
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void()> mock{};
    SCOPED_EXP mock.expect_call()
        and expect::twice();

    mock(); // not enough
    mock(); // fine!
            //! [twice]
}

TEST_CASE(
    "at_least() requires the expectation to be matched at least the specified times.",
    "[example][example::times]")
{
    //! [at_least]
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void()> mock{};
    SCOPED_EXP mock.expect_call()
        and expect::at_least(2u);

    mock(); // not enough
    mock(); // fine
    mock(); // also fine!
            //! [at_least]
}

TEST_CASE(
    "at_most() requires the expectation to be matched at most the specified times.",
    "[example][example::times]")
{
    //! [at_most]
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void()> mock{};
    SCOPED_EXP mock.expect_call()
        and expect::at_most(2u);

    mock(); // fine
    mock(); // fine but exhausted!
            //! [at_most]
}

TEST_CASE(
    "times() supports a single limit.",
    "[example][example::times]")
{
    //! [times single]
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void()> mock{};
    SCOPED_EXP mock.expect_call()
        and expect::times(2u); // equivalent to twice()

    mock(); // not enough
    mock(); // fine
    //! [times single]
}

TEST_CASE(
    "times() supports an upper and lower limit.",
    "[example][example::times]")
{
    //! [times]
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void()> mock{};
    SCOPED_EXP mock.expect_call()
        and expect::times(1u, 3u); // between 1 and 3 matches

    mock(); // fine
    mock(); // fine
    mock(); // fine (exhausted)
            //! [times]
}
