//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
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
    "matcher::PredicateMatcher is a generic matcher.",
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
    STATIC_REQUIRE(matcher_for<AnyT, const std::string&>);

    constexpr int value{42};
    REQUIRE(matches::_.matches(value));
    REQUIRE_THAT(
        StringT{matches::_.describe()},
        Matches::Equals("has no constraints"));
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

    REQUIRE_THAT(
        matcher.describe(),
        Catch::Matchers::Matches("is instance at 0x[\\dAaBbCcDdEeFf]{1,16}"));

    SECTION("When target is the instance.")
    {
        REQUIRE(matcher.matches(instance));
    }

    SECTION("When target is not the instance.")
    {
        constexpr int target{};
        REQUIRE(!matcher.matches(target));
    }

    SECTION("Matcher can be inverted.")
    {
        const auto invertedMatcher = !matches::instance(instance);

        REQUIRE_THAT(
            invertedMatcher.describe(),
            Catch::Matchers::Matches("is not instance at 0x[\\dAaBbCcDdEeFf]{1,16}"));

        SECTION("When target is not the instance.")
        {
            constexpr int target{};
            REQUIRE(!matcher.matches(target));
        }

        SECTION("When target is the instance.")
        {
            REQUIRE(matcher.matches(instance));
        }
    }
}
