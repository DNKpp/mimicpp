//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/GeneralMatchers.hpp"

#include <TestTypes.hpp>

using namespace mimicpp;

namespace
{
    template <typename T>
    class MatcherPredicateMock
    {
    public:
        MAKE_CONST_MOCK1(check, bool(T));

        [[nodiscard]]
        constexpr bool operator()(T value) const
        {
            return check(value);
        }
    };

    template <typename... Args>
    class VariadicMatcherPredicateMock
        : public InvocableMock<bool, Args...>
    {
    };
}

TEST_CASE(
    "matcher::PredicateMatcher is a generic legacy matcher.",
    "[matcher]")
{
    MatcherPredicateMock<int> predicate{};
    PredicateMatcher matcher{
        std::ref(predicate),
        "Hello, World!",
        "not Hello, World!"};

    SECTION("When matches() is called, argument is forwarded to the predicate.")
    {
        const bool result = GENERATE(true, false);

        REQUIRE_CALL(predicate, check(42))
            .RETURN(result);

        constexpr int value{42};
        REQUIRE(result == matcher.matches(value));
    }

    SECTION("When describe() is called.")
    {
        REQUIRE("Hello, World!" == matcher.describe());
    }
}

TEST_CASE(
    "matcher::PredicateMatcher can be negated.",
    "[matcher]")
{
    SECTION("As rvalue.")
    {
        MatcherPredicateMock<int> predicate{};
        PredicateMatcher matcher{
            std::ref(predicate),
            "Hello, World!",
            "not Hello, World!"};

        PredicateMatcher negatedMatcher = !std::move(matcher);
        STATIC_REQUIRE(matcher_for<decltype(negatedMatcher), int>);

        SECTION("When matches() is called, argument is forwarded to the predicate.")
        {
            const bool result = GENERATE(true, false);

            REQUIRE_CALL(predicate, check(42))
                .RETURN(result);

            constexpr int value{42};
            REQUIRE(result == !negatedMatcher.matches(value));
        }

        SECTION("When describe() is called.")
        {
            REQUIRE("not Hello, World!" == negatedMatcher.describe());
        }
    }

    SECTION("As const lvalue.")
    {
        MatcherPredicateMock<int> predicate{};
        const PredicateMatcher matcher{
            std::ref(predicate),
            "Hello, World!",
            "not Hello, World!"};

        PredicateMatcher negatedMatcher = !matcher;
        STATIC_REQUIRE(matcher_for<decltype(negatedMatcher), int>);

        SECTION("When matches() is called, argument is forwarded to the predicate.")
        {
            const bool result = GENERATE(true, false);

            REQUIRE_CALL(predicate, check(42))
                .RETURN(result);

            constexpr int value{42};
            REQUIRE(result == !negatedMatcher.matches(value));
        }

        SECTION("When describe() is called.")
        {
            REQUIRE("not Hello, World!" == negatedMatcher.describe());
        }

        SECTION("And original matcher is still working.")
        {
            SECTION("When matches() is called, argument is forwarded to the predicate.")
            {
                const bool result = GENERATE(true, false);

                REQUIRE_CALL(predicate, check(42))
                    .RETURN(result);

                constexpr int value{42};
                REQUIRE(result == matcher.matches(value));
            }

            SECTION("When describe() is called.")
            {
                REQUIRE("Hello, World!" == matcher.describe());
            }
        }
    }
}

TEST_CASE(
    "matcher::PredicateMatcher supports variadic arguments.",
    "[matcher]")
{
    SECTION("Two arguments.")
    {
        VariadicMatcherPredicateMock<int, const float&> predicate{};
        const PredicateMatcher matcher{
            std::ref(predicate),
            "Plain",
            "Negated"};

        SECTION("When matches() is called, argument is forwarded to the predicate.")
        {
            const bool result = GENERATE(true, false);
            REQUIRE_CALL(predicate, Invoke(1337, 42.f))
                .RETURN(result);

            constexpr int value{1337};
            constexpr float other{42.f};
            REQUIRE(result == matcher.matches(value, other));
        }
    }

    SECTION("Three arguments.")
    {
        VariadicMatcherPredicateMock<const double&, int, const float&> predicate{};
        const PredicateMatcher matcher{
            std::ref(predicate),
            "Plain",
            "Negated"};

        SECTION("When matches() is called, argument is forwarded to the predicate.")
        {
            const bool result = GENERATE(true, false);
            REQUIRE_CALL(predicate, Invoke(4242., 1337, 42.f))
                .RETURN(result);

            constexpr double first{4242.};
            constexpr int value{1337};
            constexpr float other{42.f};
            REQUIRE(result == matcher.matches(first, value, other));
        }
    }
}

TEST_CASE(
    "matches::_ matches always.",
    "[matcher]")
{
    namespace Matches = Catch::Matchers;

    using AnyT = std::remove_cvref_t<decltype(matches::_)>;
    STATIC_REQUIRE(matcher_for<AnyT, int>);
    STATIC_REQUIRE(matcher_for<AnyT, std::string const&>);

    constexpr int value{42};
    CHECK_THAT(matches::_.matches(value), variant_equals(expectation::MatchSuccess{}));
}

TEST_CASE(
    "matches::eq matches when target value compares equal to the stored one.",
    "[matcher]")
{
    constexpr auto matcher = matches::eq(42);
    REQUIRE_THAT(
        matcher.describe(),
        Catch::Matchers::Equals("== 42"));

    SECTION("When target is equal.")
    {
        constexpr int target{42};
        REQUIRE(matcher.matches(target));
    }

    SECTION("When target is not equal.")
    {
        constexpr int target{1337};
        REQUIRE(!matcher.matches(target));
    }

    SECTION("Matcher can be inverted.")
    {
        constexpr auto invertedMatcher = !matches::eq(42);

        REQUIRE_THAT(
            invertedMatcher.describe(),
            Catch::Matchers::Equals("!= 42"));

        SECTION("When target is equal.")
        {
            constexpr int target{42};
            REQUIRE(!invertedMatcher.matches(target));
        }

        SECTION("When target is not equal.")
        {
            constexpr int target{1337};
            REQUIRE(invertedMatcher.matches(target));
        }
    }
}

TEST_CASE(
    "matches::ne matches when target value does not compare equal to the stored one.",
    "[matcher]")
{
    constexpr auto matcher = matches::ne(42);

    REQUIRE_THAT(
        matcher.describe(),
        Catch::Matchers::Equals("!= 42"));

    SECTION("When target is not equal.")
    {
        constexpr int target{1337};
        REQUIRE(matcher.matches(target));
    }

    SECTION("When target is equal.")
    {
        constexpr int target{42};
        REQUIRE(!matcher.matches(target));
    }

    SECTION("Matcher can be inverted.")
    {
        constexpr auto invertedMatcher = !matches::ne(42);

        REQUIRE_THAT(
            invertedMatcher.describe(),
            Catch::Matchers::Equals("== 42"));

        SECTION("When target is not equal.")
        {
            constexpr int target{1337};
            REQUIRE(!invertedMatcher.matches(target));
        }

        SECTION("When target is equal.")
        {
            constexpr int target{42};
            REQUIRE(invertedMatcher.matches(target));
        }
    }
}

TEST_CASE(
    "matches::lt matches when target value is less than the stored one.",
    "[matcher]")
{
    constexpr auto matcher = matches::lt(42);

    REQUIRE_THAT(
        matcher.describe(),
        Catch::Matchers::Equals("< 42"));

    SECTION("When target is less.")
    {
        const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
        REQUIRE(matcher.matches(target));
    }

    SECTION("When target is not less.")
    {
        const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
        REQUIRE(!matcher.matches(target));
    }

    SECTION("Matcher can be inverted.")
    {
        constexpr auto invertedMatcher = !matches::lt(42);

        REQUIRE_THAT(
            invertedMatcher.describe(),
            Catch::Matchers::Equals(">= 42"));

        SECTION("When target is less.")
        {
            const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
            REQUIRE(!invertedMatcher.matches(target));
        }

        SECTION("When target is not less.")
        {
            const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
            REQUIRE(invertedMatcher.matches(target));
        }
    }
}

TEST_CASE(
    "matches::le matches when target value is less than or equal to the stored one.",
    "[matcher]")
{
    constexpr auto matcher = matches::le(42);

    REQUIRE_THAT(
        matcher.describe(),
        Catch::Matchers::Equals("<= 42"));

    SECTION("When target is less or equal.")
    {
        const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 42);
        REQUIRE(matcher.matches(target));
    }

    SECTION("When target is greater.")
    {
        const int target = GENERATE(43, std::numeric_limits<int>::max());
        REQUIRE(!matcher.matches(target));
    }

    SECTION("Matcher can be inverted.")
    {
        constexpr auto invertedMatcher = !matches::le(42);

        REQUIRE_THAT(
            invertedMatcher.describe(),
            Catch::Matchers::Equals("> 42"));

        SECTION("When target is less or equal.")
        {
            const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 42);
            REQUIRE(!invertedMatcher.matches(target));
        }

        SECTION("When target is greater.")
        {
            const int target = GENERATE(43, std::numeric_limits<int>::max());
            REQUIRE(invertedMatcher.matches(target));
        }
    }
}

TEST_CASE(
    "matches::gt matches when target value is greater than the stored one.",
    "[matcher]")
{
    constexpr auto matcher = matches::gt(42);

    REQUIRE_THAT(
        matcher.describe(),
        Catch::Matchers::Equals("> 42"));

    SECTION("When target is greater.")
    {
        const int target = GENERATE(43, std::numeric_limits<int>::max());
        REQUIRE(matcher.matches(target));
    }

    SECTION("When target is not greater.")
    {
        const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41, 42);
        REQUIRE(!matcher.matches(target));
    }

    SECTION("Matcher can be inverted.")
    {
        constexpr auto invertedMatcher = !matches::gt(42);

        REQUIRE_THAT(
            invertedMatcher.describe(),
            Catch::Matchers::Equals("<= 42"));

        SECTION("When target is greater.")
        {
            const int target = GENERATE(43, std::numeric_limits<int>::max());
            REQUIRE(!invertedMatcher.matches(target));
        }

        SECTION("When target is not greater.")
        {
            const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41, 42);
            REQUIRE(invertedMatcher.matches(target));
        }
    }
}

TEST_CASE(
    "matches::ge matches when target value is greater than or equal to the stored one.",
    "[matcher]")
{
    constexpr auto matcher = matches::ge(42);

    REQUIRE_THAT(
        matcher.describe(),
        Catch::Matchers::Equals(">= 42"));

    SECTION("When target is greater or equal.")
    {
        const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
        REQUIRE(matcher.matches(target));
    }

    SECTION("When target is less.")
    {
        const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
        REQUIRE(!matcher.matches(target));
    }

    SECTION("Matcher can be inverted.")
    {
        constexpr auto invertedMatcher = !matches::ge(42);

        REQUIRE_THAT(
            invertedMatcher.describe(),
            Catch::Matchers::Equals("< 42"));

        SECTION("When target is greater or equal.")
        {
            const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
            REQUIRE(!invertedMatcher.matches(target));
        }

        SECTION("When target is less.")
        {
            const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
            REQUIRE(invertedMatcher.matches(target));
        }
    }
}

TEST_CASE(
    "Ordering matches support different, but comparable types, on both sides.",
    "[matcher]")
{
    SECTION("matches::eq")
    {
        constexpr auto matcher = matches::eq(std::nullopt);

        std::optional<int> opt{};
        REQUIRE(matcher.matches(opt));

        opt = 42;
        REQUIRE_FALSE(matcher.matches(opt));
    }

    SECTION("matches::ne")
    {
        constexpr auto matcher = matches::ne(std::nullopt);

        std::optional opt{42};
        REQUIRE(matcher.matches(opt));

        opt.reset();
        REQUIRE_FALSE(matcher.matches(opt));
    }

    SECTION("matches::lt")
    {
        constexpr auto matcher = matches::lt(std::nullopt);

        std::optional opt{42};
        REQUIRE_FALSE(matcher.matches(opt));

        opt.reset();
        REQUIRE_FALSE(matcher.matches(opt));
    }

    SECTION("matches::le")
    {
        constexpr auto matcher = matches::le(std::nullopt);

        std::optional opt{42};
        REQUIRE_FALSE(matcher.matches(opt));

        opt.reset();
        REQUIRE(matcher.matches(opt));
    }

    SECTION("matches::gt")
    {
        constexpr auto matcher = matches::gt(std::nullopt);

        std::optional opt{42};
        REQUIRE(matcher.matches(opt));

        opt.reset();
        REQUIRE_FALSE(matcher.matches(opt));
    }

    SECTION("matches::ge")
    {
        constexpr auto matcher = matches::ge(std::nullopt);

        std::optional opt{42};
        REQUIRE(matcher.matches(opt));

        opt.reset();
        REQUIRE(matcher.matches(opt));
    }
}

TEST_CASE(
    "matches::predicate matches when the given predicate is satisfied.",
    "[matcher]")
{
    using trompeloeil::_;

    const int target = GENERATE(42, 43, std::numeric_limits<int>::max());

    InvocableMock<bool, const int&> predicate{};
    const auto expectedResult = GENERATE(true, false);
    REQUIRE_CALL(predicate, Invoke(_))
        .LR_WITH(&_1 == &target)
        .RETURN(expectedResult);

    const auto matcher = matches::predicate(std::ref(predicate));
    REQUIRE(expectedResult == matcher.matches(target));
    REQUIRE_THAT(
        matcher.describe(),
        Catch::Matchers::Equals("passes predicate"));

    SECTION("When matcher is inverted.")
    {
        const auto invertedMatcher = !matches::predicate(std::ref(predicate));

        REQUIRE_THAT(
            invertedMatcher.describe(),
            Catch::Matchers::Equals("fails predicate"));

        REQUIRE_CALL(predicate, Invoke(_))
            .LR_WITH(&_1 == &target)
            .RETURN(expectedResult);

        REQUIRE(expectedResult == !invertedMatcher.matches(target));
    }

    SECTION("Custom descriptions are supported.")
    {
        const auto customMatcher = matches::predicate(
            std::ref(predicate),
            "custom predicate is passed",
            "custom predicate is failed");

        REQUIRE_CALL(predicate, Invoke(_))
            .LR_WITH(&_1 == &target)
            .RETURN(expectedResult);
        REQUIRE(expectedResult == customMatcher.matches(target));
        REQUIRE_THAT(
            customMatcher.describe(),
            Catch::Matchers::Equals("custom predicate is passed"));

        REQUIRE_THAT(
            (!customMatcher).describe(),
            Catch::Matchers::Equals("custom predicate is failed"));
    }
}

TEMPLATE_TEST_CASE(
    "matches::instance does only accept lvalues.",
    "[matcher]",
    int,
    const int,
    int&&,
    const int&&)
{
    STATIC_REQUIRE(!requires { matches::instance(std::declval<TestType>()); });
}

TEST_CASE(
    "matches::instance matches when the target is the expected instance.",
    "[matcher]")
{
    int instance{42};
    const auto matcher = matches::instance(instance);

    auto withDescription = [](StringT regex) {
        return [regex = std::move(regex)](auto const& value) {
            UNSCOPED_CAPTURE(value.description);
            REQUIRE(value.description);
            CHECK_THAT(*value.description, Catch::Matchers::Matches(regex));
            return true;
        };
    };

    SECTION("When target is the instance.")
    {
        CHECK_THAT(
            matcher.matches(instance),
            variant_matches<expectation::MatchSuccess>(withDescription("is instance at 0x[\\dAaBbCcDdEeFf]{1,16}")));
    }

    SECTION("When target is not the instance.")
    {
        constexpr int target{};
        CHECK_THAT(
            matcher.matches(target),
            variant_matches<expectation::MatchFailure>(
                withDescription("is instance at 0x[\\dAaBbCcDdEeFf]{1,16}, but actually 0x[\\dAaBbCcDdEeFf]{1,16}")));
    }

    SECTION("Matcher can be inverted.")
    {
        const auto invertedMatcher = !matches::instance(instance);

        SECTION("When target is not the instance.")
        {
            constexpr int target{};
            CHECK_THAT(
                invertedMatcher.matches(target),
                variant_matches<expectation::MatchSuccess>(
                    withDescription(R"(not \(is instance at 0x[\dAaBbCcDdEeFf]{1,16}\))")));
        }

        SECTION("When target is the instance.")
        {
            CHECK_THAT(
                invertedMatcher.matches(instance),
                variant_matches<expectation::MatchFailure>(
                    withDescription(R"(not \(is instance at 0x[\dAaBbCcDdEeFf]{1,16}\), but actually 0x[\dAaBbCcDdEeFf]{1,16})")));
        }
    }
}

TEST_CASE(
    "matches::instance supports type hierarchies.",
    "[matcher]")
{
    struct base
    {
    };

    struct derived
        : public base
    {
    };

    constexpr derived object{};
    auto const matcher = matches::instance(object);

    SECTION("Matches, when same instance is provided.")
    {
        CHECK_THAT(
            matcher.matches(static_cast<base const&>(object)),
            variant_holds_alternative<expectation::MatchSuccess>());
    }

    SECTION("Does not match, when other instance is provided.")
    {
        constexpr derived other{};
        CHECK_THAT(
            matcher.matches(static_cast<base const&>(other)),
            variant_holds_alternative<expectation::MatchFailure>());
    }
}

TEST_CASE(
    "matches::type satisfies matcher_for only for the given type.",
    "[matcher]")
{
    auto constexpr matcher = matches::type<int&>;
    using Matcher = std::remove_cvref_t<decltype(matcher)>;

    SECTION("When argument is an exact match.")
    {
        STATIC_CHECK(matcher_for<Matcher, int&>);

        int i{42};
        CHECK_THAT(matcher.matches(i), variant_holds_alternative<expectation::MatchSuccess>());
    }

    SECTION("When argument is not an exact match.")
    {
        STATIC_CHECK_FALSE(matcher_for<Matcher, int>);
        STATIC_CHECK_FALSE(matcher_for<Matcher, int const&>);
        STATIC_CHECK_FALSE(matcher_for<Matcher, int&&>);
        STATIC_CHECK_FALSE(matcher_for<Matcher, int const&&>);
    }
}

TEST_CASE(
    "matcher::GenericMatcher is a matcher with a custom predicate.",
    "[matcher]")
{
    using trompeloeil::_;
    InvocableMock<bool, MatchEvaluationContext<>&, int> predicate{};

    auto matcher = make_generic_matcher(std::ref(predicate), "my matcher!");

    SECTION("When matches() is called, argument is forwarded to the predicate.")
    {
        auto const [expected, result] = GENERATE((table<expectation::MatchResult, bool>)({
            {expectation::MatchSuccess{.description = "my matcher!"}, true },
            {expectation::MatchFailure{.description = "my matcher!"}, false},
        }));
        CAPTURE(result);

        REQUIRE_CALL(predicate, Invoke(_, 42))
            .RETURN(result);

        constexpr int value{42};

        SECTION("When the matcher is used as-is.")
        {
            CHECK(expected == matcher.matches(value));
        }

        SECTION("When the matcher is used via double inversion.")
        {
            CHECK(expected == (!!matcher).matches(value));
        }
    }

    SECTION("When matches() on the inverted matcher is called.")
    {
        auto const [expected, result] = GENERATE((table<expectation::MatchResult, bool>)({
            {expectation::MatchFailure{.description = "not (my matcher!)"}, true },
            {expectation::MatchSuccess{.description = "not (my matcher!)"}, false},
        }));
        CAPTURE(result);

        REQUIRE_CALL(predicate, Invoke(_, 42))
            .RETURN(result);

        constexpr int value{42};

        SECTION("When the matcher is used inverted.")
        {
            CHECK(expected == (!matcher).matches(value));
        }

        SECTION("When the matcher is used via triple inversion.")
        {
            CHECK(expected == (!!!matcher).matches(value));
        }
    }

    SECTION("The predicate can capture arbitrary values.")
    {
        constexpr int value{1337};

        SECTION("The captured values will be printed when the predicate fails.")
        {
            SECTION("When the matcher is not inverted.")
            {
                REQUIRE_CALL(predicate, Invoke(_, value))
                    .SIDE_EFFECT(_1.capture(42))
                    .SIDE_EFFECT(_1.capture(std::string_view{"Hello, World!"}))
                    .RETURN(false);

                CHECK_THAT(
                    matcher.matches(value),
                    variant_equals(expectation::MatchFailure{.description = R"(my matcher!, but actually 42, "Hello, World!")"}));
            }

            SECTION("When the matcher is inverted.")
            {
                REQUIRE_CALL(predicate, Invoke(_, value))
                    .SIDE_EFFECT(_1.capture(42))
                    .SIDE_EFFECT(_1.capture(std::string_view{"Hello, World!"}))
                    .RETURN(true);

                CHECK_THAT(
                    (!matcher).matches(value),
                    variant_equals(expectation::MatchFailure{.description = R"(not (my matcher!), but actually 42, "Hello, World!")"}));
            }
        }

        SECTION("But not printed when the predicate succeeds.")
        {
            SECTION("When the matcher is not inverted.")
            {
                REQUIRE_CALL(predicate, Invoke(_, value))
                    .SIDE_EFFECT(_1.capture(42))
                    .SIDE_EFFECT(_1.capture(std::string_view{"Hello, World!"}))
                    .RETURN(true);

                CHECK_THAT(
                    matcher.matches(value),
                    variant_equals(expectation::MatchSuccess{.description = "my matcher!"}));
            }

            SECTION("When the matcher is inverted.")
            {
                REQUIRE_CALL(predicate, Invoke(_, value))
                    .SIDE_EFFECT(_1.capture(42))
                    .SIDE_EFFECT(_1.capture(std::string_view{"Hello, World!"}))
                    .RETURN(false);

                CHECK_THAT(
                    (!matcher).matches(value),
                    variant_equals(expectation::MatchSuccess{.description = "not (my matcher!)"}));
            }
        }
    }
}

TEST_CASE(
    "matcher::GenericMatcher can be instantiated with additional args.",
    "[matcher]")
{
    using trompeloeil::_;
    InvocableMock<bool, MatchEvaluationContext<int, std::string>&, int> predicate{};

    auto matcher = make_generic_matcher(std::ref(predicate), "my matcher with {}, {}!", 42, std::string{"Hello, World!"});

    auto const [expected, result] = GENERATE((table<expectation::MatchResult, bool>)({
        {expectation::MatchSuccess{R"(my matcher with 42, Hello, World!!)"}, true},
        {expectation::MatchFailure{R"(my matcher with 42, Hello, World!!)"}, false},
    }));

    constexpr int value{1337};
    REQUIRE_CALL(predicate, Invoke(_, value))
        .RETURN(result);

    CHECK(expected == matcher.matches(value));
}
