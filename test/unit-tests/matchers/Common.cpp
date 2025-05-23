//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/Common.hpp"

using namespace mimicpp;

namespace
{
    class CommonMatcher
    {
        MAKE_CONST_MOCK1(matches, bool(const int&));
        MAKE_CONST_MOCK0(describe, StringViewT());
    };

    class CustomMatcher
    {
        MAKE_CONST_MOCK1(my_matches, bool(const int&));
        MAKE_CONST_MOCK0(my_describe, StringViewT());
    };

    class Mixed1Matcher
    {
        MAKE_CONST_MOCK1(matches, bool(const int&));
        MAKE_CONST_MOCK0(my_describe, StringViewT());
    };

    class Mixed2Matcher
    {
        MAKE_CONST_MOCK1(my_matches, bool(const int&));
        MAKE_CONST_MOCK0(describe, StringViewT());
    };

    class CommonVariadicMatcher
    {
        MAKE_CONST_MOCK(matches, auto(const int&, const double&)->bool);
        MAKE_CONST_MOCK(matches, auto(const int&, const double&, const std::string&)->bool);

        MAKE_CONST_MOCK0(describe, StringViewT());
    };

    class CustomVariadicMatcher
    {
        MAKE_CONST_MOCK(my_matches2, auto(const int&, const double&)->bool);
        MAKE_CONST_MOCK(my_matches3, auto(const int&, const double&, const std::string&)->bool);

        MAKE_CONST_MOCK0(my_describe, StringViewT());
    };

    class CommonNullDescribeMatcher
    {
        MAKE_CONST_MOCK1(matches, bool(const int&));
        MAKE_CONST_MOCK0(describe, std::nullopt_t());
    };
}

template <>
struct custom::matcher_traits<CustomMatcher>
{
    [[nodiscard]]
    static bool matches(const CustomMatcher& matcher, const int& value)
    {
        return matcher.my_matches(value);
    }

    [[nodiscard]]
    static StringViewT describe(const CustomMatcher& matcher)
    {
        return matcher.my_describe();
    }
};

template <>
struct custom::matcher_traits<Mixed1Matcher>
{
    [[nodiscard]]
    static StringViewT describe(const Mixed1Matcher& matcher)
    {
        return matcher.my_describe();
    }
};

template <>
struct custom::matcher_traits<Mixed2Matcher>
{
    [[nodiscard]]
    static bool matches(const Mixed2Matcher& matcher, const int& value)
    {
        return matcher.my_matches(value);
    }
};

template <>
struct custom::matcher_traits<CustomVariadicMatcher>
{
    [[nodiscard]]
    static bool matches(const CustomVariadicMatcher& matcher, const int& first, const double& second)
    {
        return matcher.my_matches2(first, second);
    }

    [[nodiscard]]
    static bool matches(const CustomVariadicMatcher& matcher, const int& first, const double& second, const std::string& third)
    {
        return matcher.my_matches3(first, second, third);
    }

    [[nodiscard]]
    static StringViewT describe(const CustomVariadicMatcher& matcher)
    {
        return matcher.my_describe();
    }
};

TEST_CASE(
    "detail::matches_hook::matches chooses correct implementation.",
    "[matcher][matcher::detail]")
{
    using trompeloeil::_;

    const bool result = GENERATE(true, false);
    int value = 42;

    SECTION("For member matches.")
    {
        SECTION("For pure common matchers.")
        {
            CommonMatcher matcher{};
            REQUIRE_CALL(matcher, matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            REQUIRE(result == detail::matches_hook::matches(matcher, value));
        }

        SECTION("For mixed matchers.")
        {
            Mixed1Matcher matcher{};
            REQUIRE_CALL(matcher, matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            REQUIRE(result == detail::matches_hook::matches(matcher, value));
        }

        SECTION("For common variadic matchers.")
        {
            constexpr double second{1337.};
            const std::string third{"Test"};

            CommonVariadicMatcher matcher{};

            SECTION("For two arguments.")
            {
                REQUIRE_CALL(matcher, matches(_, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .RETURN(result);

                REQUIRE(result == detail::matches_hook::matches(matcher, value, second));
            }

            SECTION("For three arguments.")
            {
                REQUIRE_CALL(matcher, matches(_, _, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .LR_WITH(&_3 == &third)
                    .RETURN(result);

                REQUIRE(result == detail::matches_hook::matches(matcher, value, second, third));
            }
        }
    }

    SECTION("For custom matches.")
    {
        SECTION("For pure custom matchers.")
        {
            CustomMatcher matcher{};
            REQUIRE_CALL(matcher, my_matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            REQUIRE(result == detail::matches_hook::matches(matcher, value));
        }

        SECTION("For mixed matchers.")
        {
            Mixed2Matcher matcher{};
            REQUIRE_CALL(matcher, my_matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            REQUIRE(result == detail::matches_hook::matches(matcher, value));
        }

        SECTION("For custom variadic matchers.")
        {
            constexpr double second{1337.};
            const std::string third{"Test"};

            CustomVariadicMatcher matcher{};

            SECTION("For two arguments.")
            {
                REQUIRE_CALL(matcher, my_matches2(_, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .RETURN(result);

                REQUIRE(result == detail::matches_hook::matches(matcher, value, second));
            }

            SECTION("For three arguments.")
            {
                REQUIRE_CALL(matcher, my_matches3(_, _, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .LR_WITH(&_3 == &third)
                    .RETURN(result);

                REQUIRE(result == detail::matches_hook::matches(matcher, value, second, third));
            }
        }
    }
}

TEST_CASE(
    "detail::describe_hook::describe chooses correct implementation.",
    "[matcher][matcher::detail]")
{
    namespace Matches = Catch::Matchers;

    const StringViewT result = GENERATE("Hello, World!", " Hello, 2, World! ");

    SECTION("For member describe.")
    {
        SECTION("For pure common matchers.")
        {
            CommonMatcher matcher{};
            REQUIRE_CALL(matcher, describe())
                .RETURN(result);

            auto const description = detail::describe_hook::describe(matcher);
            REQUIRE_THAT(
                StringT{description},
                Matches::Equals(StringT{result}));
        }

        SECTION("For mixed matchers.")
        {
            Mixed2Matcher matcher{};
            REQUIRE_CALL(matcher, describe())
                .RETURN(result);

            auto const description = detail::describe_hook::describe(matcher);
            REQUIRE_THAT(
                StringT{description},
                Matches::Equals(StringT{result}));
        }
    }

    SECTION("For custom describe.")
    {
        SECTION("For pure custom matchers.")
        {
            CustomMatcher matcher{};
            REQUIRE_CALL(matcher, my_describe())
                .RETURN(result);

            auto const description = detail::describe_hook::describe(matcher);
            REQUIRE_THAT(
                StringT{description},
                Matches::Equals(StringT{result}));
        }

        SECTION("For mixed matchers.")
        {
            Mixed1Matcher matcher{};
            REQUIRE_CALL(matcher, my_describe())
                .RETURN(result);

            auto const description = detail::describe_hook::describe(matcher);
            REQUIRE_THAT(
                StringT{description},
                Matches::Equals(StringT{result}));
        }
    }

    SECTION("When matcher doesn't have a description.")
    {
        CommonNullDescribeMatcher matcher{};
        REQUIRE_CALL(matcher, describe())
            .RETURN(std::nullopt);

        auto const description = detail::describe_hook::describe(matcher);
        REQUIRE(std::optional<StringT>{} == description);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "Given types satisfy mimicpp::matcher_for concept.",
    "[matcher]",
    ((auto dummy, typename Matcher, typename First, typename... Others), dummy, Matcher, First, Others...),
    (std::ignore, CommonMatcher, int),
    (std::ignore, CustomMatcher, int),
    (std::ignore, Mixed1Matcher, int),
    (std::ignore, Mixed2Matcher, int),
    (std::ignore, CommonNullDescribeMatcher, int),
    (std::ignore, CommonVariadicMatcher, int, double),
    (std::ignore, CommonVariadicMatcher, int, double, std::string),
    (std::ignore, CustomVariadicMatcher, int, double),
    (std::ignore, CustomVariadicMatcher, int, double, std::string))
{
    STATIC_REQUIRE(matcher_for<Matcher, First, Others...>);
    STATIC_REQUIRE(matcher_for<Matcher, const First, const Others...>);
    STATIC_REQUIRE(matcher_for<Matcher, First&, Others&...>);
    STATIC_REQUIRE(matcher_for<Matcher, const First&, const Others&...>);
    STATIC_REQUIRE(matcher_for<Matcher, First&&, Others&&...>);
    STATIC_REQUIRE(matcher_for<Matcher, const First&&, const Others&&...>);
}
