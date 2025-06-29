//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/matchers/RangeMatchers.hpp"

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
    "A terse syntax for the matches::eq matcher is available.",
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
    "matches::type can be used, to disambiguate similar overloads.",
    "[example][example::requirements]")
{
    //! [matcher type]
    namespace matches = mimicpp::matches;
    mimicpp::Mock<void(int&&), void(int const&)> mock{};

    SECTION("Selecting void(int&&)")
    {
        MIMICPP_SCOPED_EXPECTATION mock.expect_call(matches::type<int&&>);

        mock(42);
    }

    SECTION("Selecting void(int const&)")
    {
        MIMICPP_SCOPED_EXPECTATION mock.expect_call(matches::type<int const&>);

        int constexpr i{42};
        mock(i);
    }
    //! [matcher type]
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
        and expect::arg<0>(matches::gt(42))
        and expect::arg<0>(matches::lt(1338));
    mock(1337);
    //! [expect::arg]
}

TEST_CASE(
    "Both requirement types can be combined.",
    "[example][example::requirements]")
{
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(matches::le(1337))
        and expect::arg<0>(matches::gt(42));
    mock(1337);
}

TEST_CASE(
    "A single requirement can encompass multiple arguments.",
    "[example][example::requirements]")
{
    //! [expect::args]
    namespace matches = mimicpp::matches;
    using matches::_;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(int, int)> mock{};

    SCOPED_EXP mock.expect_call(_, _)
        and expect::args<0, 1>(matches::predicate(std::less{})); // this requires 0th arg < 1st arg
    mock(42, 1337);

    SCOPED_EXP mock.expect_call(_, _)
        and expect::args<1, 0>(matches::predicate(std::less{})); // the index order can be freely changed
    mock(1337, 42);

    SCOPED_EXP mock.expect_call(_, _)
        and expect::args<1, 1>(matches::predicate(std::equal_to{})); // the same index may appear more than once
    mock(42, 42);
    //! [expect::args]
}

TEST_CASE(
    "A single requirement can encompass all arguments.",
    "[example][example::requirements]")
{
    //! [expect::all_args]
    namespace matches = mimicpp::matches;
    using matches::_;
    namespace expect = mimicpp::expect;

    const std::string str{"Hello, World!"};
    mimicpp::Mock<void(const char*, std::size_t)> mock{};

    SCOPED_EXP mock.expect_call(_, _)
        and expect::all_args(matches::predicate(
            [&](const char* data, std::size_t length) { return str == std::string_view{data, length}; }));
    mock(str.data(), str.size());
    //! [expect::all_args]
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
    "Ranges can be checked with various constraints.",
    "[example][example::requirements]")
{
    //! [matcher range]
    using mimicpp::matches::_;
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;

    mimicpp::Mock<void(std::span<int>)> mock{};

    SCOPED_EXP mock.expect_call(!matches::range::is_empty())                // The range argument shall not be empty...
        and expect::arg<0>(matches::range::has_size(2))                     // ... and contain exactly 2 elements.
        and expect::all_args(matches::range::is_sorted())                   // With just one argument, expect::arg<0> is equivalent to expect::all_args.
        and expect::arg<0>(matches::range::any_element(matches::eq(1337))); // At least one element is equal to 1337.

    std::vector collection{42, 1337};
    mock(collection);
    //! [matcher range]
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
