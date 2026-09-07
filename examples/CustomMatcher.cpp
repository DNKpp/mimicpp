//          Copyright Dominic (DNKpp) Koepke 2024-2026.
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
    constexpr auto Contains(int const expectedElement)
    {
        return mimicpp::make_generic_matcher(
            // This is the actual predicate.
            // The first argument is the context of the matcher, which holds the attached expectations
            // (in this case the single `expectedElement`).
            // `argument` is the actual argument from the currently processed function call.
            // Note that it's not necessary to spell-out the actual type of the context. Usually `auto& ctx` works just fine.
            [](mimicpp::MatchEvaluationContext<int>& ctx, auto const& argument) {
                auto const& [element] = ctx.expectations(); // `expectations` returns a tuple, which need to decompose.
                ctx.capture(argument);                      // Optionally, users can capture any variable and
                                                            // the matcher will add that to the description if the predicate fails.
                return std::ranges::find(argument, element) != std::ranges::end(argument);
            },
            "contains element {}", // This is the description format-string.
            expectedElement        // this captures the provided expectation
        );
    }

    //! [matcher custom contains definition]
}

TEST_CASE(
    "Custom matchers can be easily composed with the generic mimicpp::GenericMatcher.",
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
        return mimicpp::make_generic_matcher(
            // This is the actual predicate.
            // The first argument is the context of the matcher, which holds the attached expectations
            // (in this case the raw "pattern" and the pre-processed regex).
            // `input` is the actual argument from the currently processed function call.
            [](auto& ctx, std::ranges::range auto&& input) {
                auto const& [regex, rawPattern] = ctx.expectations();
                return std::regex_match(
                    std::ranges::begin(input),
                    std::ranges::end(input),
                    regex);
            },
            "matches regex {1}", // Note the explicit positional argument
            // The GenericMatchers accepts arbitrary additional data.
            // Each such captured value will internally be to the format-string.
            // Note: It is allowed to store more elements than actually referenced by the format-strings.
            // The formatter will consume the arguments in the stored order.
            std::regex{pattern},
            std::move(pattern));
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
        return mimicpp::make_generic_matcher(
            // This is the actual predicate.
            // The last two arguments are the input from the call.
            [](auto& ctx, int const firstArg, int const secondArg) {
                auto const& [sum] = ctx.expectations();
                ctx.capture(firstArg);
                ctx.capture(secondArg);
                return sum == firstArg + secondArg;
            },
            "matches sum {}", // Note the explicit positional argument
            expectedSum);
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
        // The only requirement for a matcher is a single `matches` function, returning a `mimicpp::expectation::MatchResult`.
        // The result either denotes a successful or a failed match and may optionally carry a textual description,
        // which will be used for reporting purposes (e.g. when a call does not match any expectation).
        [[nodiscard]]
        static mimicpp::expectation::MatchResult matches(int const input)
        {
            if (0 == input % 2)
            {
                return mimicpp::expectation::MatchSuccess{
                    .description = mimicpp::format::format("{} is an even number.", input)
                };
            }

            return mimicpp::expectation::MatchFailure{
                .description = mimicpp::format::format("{} is not an even number.", input)
            };
        }
    };

    // Let's see, whether we actually satisfy all constraints.
    // This checks, that `IsEvenMatcher` is a matcher for a single `int` argument.
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
    // The `matcher_for` concept does only require the absolute minimal feature-set (a single `matches` function).
    // So, in this case `!IsEvenMatcher{}` will not work.
    //! [matcher custom standalone usage]
}

namespace
{
    //! [matcher custom legacy definition]
    // Discouraged: This is the legacy matcher interface, which is still supported for backwards-compatibility.
    // It requires a `matches` function returning a plain `bool` and a separate `describe` function, providing the
    // textual description. Prefer the single-`matches`-function interface shown above instead.
    class IsEvenMatcherLegacy
    {
    public:
        [[nodiscard]]
        constexpr bool matches(int const input) const
        {
            return 0 == input % 2;
        }

        [[nodiscard]]
        constexpr std::string_view describe() const
        {
            return "is an even number.";
        }
    };

    // This still satisfies the `matcher_for` concept, as `matches` and `describe` are detected and combined
    // into an equivalent `mimicpp::expectation::MatchResult` behind the scenes.
    static_assert(mimicpp::matcher_for<IsEvenMatcherLegacy, int>);
    //! [matcher custom legacy definition]
}

TEST_CASE(
    "Legacy matchers with separate matches and describe functions are still supported, but discouraged.",
    "[example][example::matcher]")
{
    //! [matcher custom legacy usage]
    mimicpp::Mock<void(int)> mock{};

    // This expects the input to be an even number.
    SCOPED_EXP mock.expect_call(IsEvenMatcherLegacy{});

    mock(42);
    //! [matcher custom legacy usage]
}

