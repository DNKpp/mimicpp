//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/matchers/GeneralMatchers.hpp"

#include <regex>

namespace
{
    //! [matcher custom contains definition]
    [[nodiscard]]
    constexpr auto Contains(int expectedElement)
    {
        return mimicpp::PredicateMatcher{
            // This is the actual predicate.
            // The first argument is the actual input and the other are the stored tuple-elements.
            [](auto const& argument, auto const& element) {
                return std::ranges::find(argument, element) != std::ranges::end(argument);
            },
            "contains element {}",     // This is the description format-string.
            "contains not element {}", // This is the description string for the negated matcher (i.e. when `!Contains` is used).
            // Capture additional data, which will be forwarded to both, the predicate and the description.
            std::make_tuple(expectedElement)};
    }

    //! [matcher custom contains definition]
}

TEST_CASE(
    "Custom matchers can be easily composed with the generic mimicpp::PredicateMatcher.",
    "[example][example::matcher]")
{
    //! [matcher custom contains usage]
    mimicpp::Mock<void(std::span<int const>)> mock{};

    std::vector const collection{42, 1337};

    SCOPED_EXP mock.expect_call(Contains(42));

    mock(collection);

    // Negating the matcher is directly supported. In this case this means:
    // "input-array does not contain the element `-42`"
    SCOPED_EXP mock.expect_call(!Contains(-42));

    mock(collection);
    //! [matcher custom contains usage]
}

namespace
{
    //! [matcher custom regex definition]
    [[nodiscard]]
    auto MatchesRegex(std::string pattern)
    {
        return mimicpp::PredicateMatcher{
            // This is the actual predicate.
            // The first argument is the actual input and the other are the stored tuple-elements.
            [](std::ranges::range auto&& input, [[maybe_unused]] std::string const& patternString, std::regex const& regex) {
                return std::regex_match(
                    std::ranges::begin(input),
                    std::ranges::end(input),
                    regex);
            },
            "matches regex {}",        // This is the description format-string.
            "does not match regex {}", // This is the description string for the negated matcher (i.e. when `!MatchesRegex` is used).
            // PredicateMatcher accepts arbitrary additional date, wrapped as std::tuple.
            // Each tuple-element will internally be applied to the predicate function and both format-strings.
            // Note: It is allowed to store more elements than actually referenced by the format-strings.
            // The formatter will consume the arguments in the stored order.
            std::make_tuple(pattern, std::regex{pattern})};
    }

    //! [matcher custom regex definition]
}

TEST_CASE(
    "mimicpp::PredicateMatcher is very flexible.",
    "[example][example::matcher]")
{
    //! [matcher custom regex usage]
    mimicpp::Mock<void(std::string const&)> mock{};

    // Let's build an expectation where the argument-string must exactly contain 4 digits.
    SCOPED_EXP mock.expect_call(MatchesRegex(R"(\d{4})"));

    mock("1337");

    // And another expectation where the argument must not match the regex (note the preceding `!`).
    SCOPED_EXP mock.expect_call(!MatchesRegex(R"(\d*)"));

    mock("Hello, World!");
    //! [matcher custom regex usage]
}

namespace
{
    //! [matcher custom variadic definition]
    [[nodiscard]]
    constexpr auto MatchesSum(int const expectedSum)
    {
        return mimicpp::PredicateMatcher{
            // This is the actual predicate.
            // The first two arguments are the input from the call and the last input is the set expectation.
            [](int const firstArg, int const secondArg, int const expected) {
                return firstArg + secondArg == expected;
            },
            "matches sum {}",        // This is the description format-string.
            "does not match sum {}", // This is the description string for the negated matcher (i.e. when `!MatchesSum` is used).
            std::make_tuple(expectedSum)};
    }

    //! [matcher custom variadic definition]
}

TEST_CASE(
    "mimicpp::PredicateMatcher supports multi-arguments.",
    "[example][example::matcher]")
{
    //! [matcher custom variadic usage]
    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;
    using matches::_;

    mimicpp::Mock<void(int, int)> mock{};

    SCOPED_EXP mock.expect_call(_, _)
        // This policy applies all arguments at a whole onto the specified matcher.
        // In this case, we expect that the sum of the two is equal to `1337`.
        and expect::all_args(MatchesSum(1337));

    mock(42, 1295);
    //! [matcher custom variadic usage]
}

namespace
{
    //! [matcher custom standalone definition]
    class IsEvenMatcher
    {
    public:
        [[nodiscard]]
        bool matches(int const input) const
        {
            return 0 == input % 2;
        }

        [[nodiscard]]
        std::string_view describe() const
        {
            return "is an even number.";
        }
    };

    // Let's see, whether we actually satisfy all constraints.
    // This checks, that `IsEvenNumber` is a matcher for a single `int` argument.
    static_assert(mimicpp::matcher_for<IsEvenMatcher, int>);
    //! [matcher custom standalone definition]
}

TEST_CASE(
    "Custom matchers can be built from the ground-up.",
    "[example][example::matcher]")
{
    //! [matcher custom standalone usage]
    mimicpp::Mock<void(int)> mock{};

    // This expects the input to be an even number.
    SCOPED_EXP mock.expect_call(IsEvenMatcher{});

    mock(42);

    // Note: It's the users responsibility to add additional feature, like negation.
    // The `matcher_for` concept does only require the absolute minimal feature-set (a `matches` and `describe` function).
    // So, in this case `!IsEvenMatcher{}` will not work.
    //! [matcher custom standalone usage]
}
