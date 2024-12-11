// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

TEST_CASE(
    "Requirements can be specified for each argument.",
    "[example][example::requirements]")
{
    //! [expect arg matcher]
    namespace matches = mimicpp::matches;

    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(matches::eq(42));
    mock(42);
    //! [expect arg matcher]
}

TEST_CASE(
    "A simplified syntax for the matches::eq matcher is available.",
    "[example][example::requirements]")
{
    //! [expect arg equal short]
    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(42);
    mock(42);
    //! [expect arg equal short]
}

TEST_CASE(
    "The wildcard matcher matches everything.",
    "[example][example::requirements]")
{
    //! [matcher wildcard]
    using mimicpp::matches::_;

    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(_);
    mock(1337);
    //! [matcher wildcard]
}

TEST_CASE(
    "Requirements can be specified later via expect::arg<n>.",
    "[example][example::requirements]")
{
    //! [expect::arg]
    using mimicpp::matches::_;
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(_)
        and expect::arg<0>(matches::gt(42));
    mock(1337);
    //! [expect::arg]
}

TEST_CASE(
    "Both requirement types can be combined.",
    "[example][example::requirements]")
{
    using mimicpp::matches::le;
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(le(1337))
        and expect::arg<0>(matches::gt(42));
    mock(1337);
}

TEST_CASE(
    "Most requirements can be inverted.",
    "[example][example::requirements]")
{
    //! [matcher inverted]
    using mimicpp::matches::_;
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(_)            // in fact, the _ is the only built-in matcher, which isn't invertible
        and expect::arg<0>(!matches::le(42)); // note the !, as this makes it an actual > test
    mock(1337);
    //! [matcher inverted]
}

TEST_CASE(
    "Custom predicates can be checked.",
    "[example][example::requirements]")
{
    //! [matcher predicate]
    using mimicpp::matches::_;
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(int)> mock{};

    constexpr auto isOdd = [](int val) { return 0 != val % 2; };

    SCOPED_EXP mock.expect_call(_)
        and expect::arg<0>(matches::predicate(isOdd));
    mock(1337);
    //! [matcher predicate]
}

TEST_CASE(
    "matches::instance checks, whether the expected instance is given.",
    "[example][example::requirements]")
{
    //! [matcher instance]
    namespace matches = mimicpp::matches;

    mimicpp::Mock<void(const int&)> mock{};

    int myInt{};
    SCOPED_EXP mock.expect_call(matches::instance(myInt));
    mock(myInt);
    //! [matcher instance]
}

TEST_CASE(
    "Ranges can be checked.",
    "[example][example::requirements]")
{
    //! [matcher range sorted]
    using mimicpp::matches::_;
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(std::span<int>)> mock{};

    SCOPED_EXP mock.expect_call(_)
        and expect::arg<0>(matches::range::is_sorted());

    std::vector collection{42, 1337};
    mock(collection);
    //! [matcher range sorted]
}

TEST_CASE(
    "Custom matchers can be easily composed via the generic PredicateMatcher.",
    "[example][example::requirements]")
{
    //! [matcher predicate matcher]
    using mimicpp::matches::_;
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::PredicateMatcher containsMatcher{
        // provide a test predicate
        [](const auto& target, const auto& element) // the left most param is the argument to be checked
        {
            return std::ranges::find(target, element) != std::ranges::end(target);
        },
        // specify a descriptive format message, which will be applied to std::format.
        "contains element {}",
        // specify the inverted message, which will also be applied to std::format, when inversion is used
        "contains not element {}",
        // capture additional data, which will be forwarded to both, the predicate and the description
        std::tuple{42}};

    mimicpp::Mock<void(std::span<int>)> mock{};

    SCOPED_EXP mock.expect_call(_)
        and expect::arg<0>(containsMatcher);

    std::vector collection{42, 1337};
    mock(collection);
    //! [matcher predicate matcher]
}

TEST_CASE(
    "Various string matchers are supported.",
    "[example][example::requirements]")
{
    //! [matcher str matcher]
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(std::string)> mock{};

    SCOPED_EXP mock.expect_call(matches::str::starts_with("Hell"))
        and expect::arg<0>(matches::str::ends_with("d!"))
        and expect::arg<0>(matches::str::contains(", Wo"));

    mock("Hello, World!");
    //! [matcher str matcher]
}
