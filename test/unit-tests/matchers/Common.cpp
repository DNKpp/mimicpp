//          Copyright Dominic (DNKpp) Koepke 2024 - 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/Common.hpp"
#include "mimic++/utilities/TypeList.hpp"

using namespace mimicpp;

namespace
{
    class LegacyCommonMatcher
    {
        MAKE_CONST_MOCK1(matches, bool(const int&));
        MAKE_CONST_MOCK0(describe, StringViewT());
    };

    class LegacyCustomMatcher
    {
        MAKE_CONST_MOCK1(my_matches, bool(const int&));
        MAKE_CONST_MOCK0(my_describe, StringViewT());
    };

    class LegacyMixed1Matcher
    {
        MAKE_CONST_MOCK1(matches, bool(const int&));
        MAKE_CONST_MOCK0(my_describe, StringViewT());
    };

    class LegacyMixed2Matcher
    {
        MAKE_CONST_MOCK1(my_matches, bool(const int&));
        MAKE_CONST_MOCK0(describe, StringViewT());
    };

    class LegacyCommonVariadicMatcher
    {
        MAKE_CONST_MOCK(matches, auto(const int&, const double&)->bool);
        MAKE_CONST_MOCK(matches, auto(const int&, const double&, const std::string&)->bool);

        MAKE_CONST_MOCK0(describe, StringViewT());
    };

    class LegacyCustomVariadicMatcher
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
struct custom::matcher_traits<LegacyCustomMatcher>
{
    [[nodiscard]]
    static bool matches(const LegacyCustomMatcher& matcher, const int& value)
    {
        return matcher.my_matches(value);
    }

    [[nodiscard]]
    static StringViewT describe(const LegacyCustomMatcher& matcher)
    {
        return matcher.my_describe();
    }
};

template <>
struct custom::matcher_traits<LegacyMixed1Matcher>
{
    [[nodiscard]]
    static StringViewT describe(const LegacyMixed1Matcher& matcher)
    {
        return matcher.my_describe();
    }
};

template <>
struct custom::matcher_traits<LegacyMixed2Matcher>
{
    [[nodiscard]]
    static bool matches(const LegacyMixed2Matcher& matcher, const int& value)
    {
        return matcher.my_matches(value);
    }
};

template <>
struct custom::matcher_traits<LegacyCustomVariadicMatcher>
{
    [[nodiscard]]
    static bool matches(const LegacyCustomVariadicMatcher& matcher, const int& first, const double& second)
    {
        return matcher.my_matches2(first, second);
    }

    [[nodiscard]]
    static bool matches(const LegacyCustomVariadicMatcher& matcher, const int& first, const double& second, const std::string& third)
    {
        return matcher.my_matches3(first, second, third);
    }

    [[nodiscard]]
    static StringViewT describe(const LegacyCustomVariadicMatcher& matcher)
    {
        return matcher.my_describe();
    }
};

TEST_CASE(
    "detail::matches_hook::matches chooses correct implementation for legacy matchers.",
    "[matcher][matcher::detail]")
{
    using trompeloeil::_;

    auto const result = GENERATE(true, false);
    CAPTURE(result);
    int value = 42;

    auto const CheckOutcome = [=](matcher::MatchResult const& outcome) {
        CHECKED_IF(result)
        {
            CHECK(std::holds_alternative<matcher::MatchSuccess>(outcome));
        }
        CHECKED_ELSE(result)
        {
            CHECK(std::holds_alternative<matcher::MatchFailure>(outcome));
        }
    };

    SECTION("For member matches.")
    {
        SECTION("For pure common matchers.")
        {
            LegacyCommonMatcher matcher{};
            REQUIRE_CALL(matcher, describe())
                .RETURN("description");
            REQUIRE_CALL(matcher, matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            CheckOutcome(detail::matches_hook::matches(matcher, value));
        }

        SECTION("For mixed matchers.")
        {
            LegacyMixed1Matcher matcher{};
            REQUIRE_CALL(matcher, my_describe())
                .RETURN("description");
            REQUIRE_CALL(matcher, matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            CheckOutcome(detail::matches_hook::matches(matcher, value));
        }

        SECTION("For common variadic matchers.")
        {
            constexpr double second{1337.};
            const std::string third{"Test"};

            LegacyCommonVariadicMatcher matcher{};

            SECTION("For two arguments.")
            {
                REQUIRE_CALL(matcher, describe())
                    .RETURN("description");
                REQUIRE_CALL(matcher, matches(_, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .RETURN(result);

                CheckOutcome(detail::matches_hook::matches(matcher, value, second));
            }

            SECTION("For three arguments.")
            {
                REQUIRE_CALL(matcher, describe())
                    .RETURN("description");
                REQUIRE_CALL(matcher, matches(_, _, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .LR_WITH(&_3 == &third)
                    .RETURN(result);

                CheckOutcome(detail::matches_hook::matches(matcher, value, second, third));
            }
        }
    }

    SECTION("For custom matches.")
    {
        SECTION("For pure custom matchers.")
        {
            LegacyCustomMatcher matcher{};
            REQUIRE_CALL(matcher, my_describe())
                .RETURN("description");
            REQUIRE_CALL(matcher, my_matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            CheckOutcome(detail::matches_hook::matches(matcher, value));
        }

        SECTION("For mixed matchers.")
        {
            LegacyMixed2Matcher matcher{};
            REQUIRE_CALL(matcher, describe())
                .RETURN("description");
            REQUIRE_CALL(matcher, my_matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            CheckOutcome(detail::matches_hook::matches(matcher, value));
        }

        SECTION("For custom variadic matchers.")
        {
            constexpr double second{1337.};
            const std::string third{"Test"};

            LegacyCustomVariadicMatcher matcher{};

            SECTION("For two arguments.")
            {
                REQUIRE_CALL(matcher, my_describe())
                    .RETURN("description");
                REQUIRE_CALL(matcher, my_matches2(_, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .RETURN(result);

                CheckOutcome(detail::matches_hook::matches(matcher, value, second));
            }

            SECTION("For three arguments.")
            {
                REQUIRE_CALL(matcher, my_describe())
                    .RETURN("description");
                REQUIRE_CALL(matcher, my_matches3(_, _, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .LR_WITH(&_3 == &third)
                    .RETURN(result);

                CheckOutcome(detail::matches_hook::matches(matcher, value, second, third));
            }
        }
    }
}

namespace
{
    class CommonMatcher
    {
        MAKE_CONST_MOCK1(matches, matcher::MatchResult(const int&));
    };

    class CustomMatcher
    {
        MAKE_CONST_MOCK1(my_matches, matcher::MatchResult(const int&));
    };

    class CommonVariadicMatcher
    {
        MAKE_CONST_MOCK(matches, auto(const int&, const double&)->matcher::MatchResult);
        MAKE_CONST_MOCK(matches, auto(const int&, const double&, const std::string&)->matcher::MatchResult);
    };

    class CustomVariadicMatcher
    {
        MAKE_CONST_MOCK(my_matches2, auto(const int&, const double&)->matcher::MatchResult);
        MAKE_CONST_MOCK(my_matches3, auto(const int&, const double&, const std::string&)->matcher::MatchResult);
    };
}

template <>
struct custom::matcher_traits<CustomMatcher>
{
    [[nodiscard]]
    static matcher::MatchResult matches(const CustomMatcher& matcher, const int& value)
    {
        return matcher.my_matches(value);
    }
};

template <>
struct custom::matcher_traits<CustomVariadicMatcher>
{
    [[nodiscard]]
    static matcher::MatchResult matches(const CustomVariadicMatcher& matcher, const int& first, const double& second)
    {
        return matcher.my_matches2(first, second);
    }

    [[nodiscard]]
    static matcher::MatchResult matches(const CustomVariadicMatcher& matcher, const int& first, const double& second, const std::string& third)
    {
        return matcher.my_matches3(first, second, third);
    }
};

TEST_CASE(
    "detail::matches_hook::matches chooses correct implementation.",
    "[matcher][matcher::detail]")
{
    using trompeloeil::_;

    auto const result = GENERATE(
        as<matcher::MatchResult>{},
        matcher::MatchSuccess{},
        matcher::MatchFailure{.description = []{ return StringT{"Hello, World"}; }});
    CAPTURE(result);
    int value = 42;

    auto const CheckOutcome = [=](matcher::MatchResult const& outcome) {
        std::visit(
            []<typename Lhs, typename Rhs>(Lhs const& lhs, Rhs const& rhs) {
                CHECK(std::same_as<Lhs, Rhs>);

                if constexpr (std::same_as<Lhs, matcher::MatchFailure>
                    && std::same_as<Rhs, matcher::MatchFailure>)
                {
                    CHECK(lhs.description() == rhs.description());
                }
            },
            result,
            outcome);
    };

    SECTION("For member matches.")
    {
        SECTION("For unary common matchers.")
        {
            CommonMatcher matcher{};
            REQUIRE_CALL(matcher, matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            CheckOutcome(detail::matches_hook::matches(matcher, value));
        }

        SECTION("For variadic common matchers.")
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

                CheckOutcome(detail::matches_hook::matches(matcher, value, second));
            }

            SECTION("For three arguments.")
            {
                REQUIRE_CALL(matcher, matches(_, _, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .LR_WITH(&_3 == &third)
                    .RETURN(result);

                CheckOutcome(detail::matches_hook::matches(matcher, value, second, third));
            }
        }
    }

    SECTION("For custom matches.")
    {
        SECTION("For unary custom matchers.")
        {
            CustomMatcher matcher{};
            REQUIRE_CALL(matcher, my_matches(_))
                .LR_WITH(&_1 == &value)
                .RETURN(result);

            CheckOutcome(detail::matches_hook::matches(matcher, value));
        }

        SECTION("For variadic custom matchers.")
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

                CheckOutcome(detail::matches_hook::matches(matcher, value, second));
            }

            SECTION("For three arguments.")
            {
                REQUIRE_CALL(matcher, my_matches3(_, _, _))
                    .LR_WITH(&_1 == &value)
                    .LR_WITH(&_2 == &second)
                    .LR_WITH(&_3 == &third)
                    .RETURN(result);

                CheckOutcome(detail::matches_hook::matches(matcher, value, second, third));
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
            LegacyCommonMatcher matcher{};
            REQUIRE_CALL(matcher, describe())
                .RETURN(result);

            auto const description = detail::describe_hook::describe(matcher);
            REQUIRE_THAT(
                StringT{description},
                Matches::Equals(StringT{result}));
        }

        SECTION("For mixed matchers.")
        {
            LegacyMixed2Matcher matcher{};
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
            LegacyCustomMatcher matcher{};
            REQUIRE_CALL(matcher, my_describe())
                .RETURN(result);

            auto const description = detail::describe_hook::describe(matcher);
            REQUIRE_THAT(
                StringT{description},
                Matches::Equals(StringT{result}));
        }

        SECTION("For mixed matchers.")
        {
            LegacyMixed1Matcher matcher{};
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
    (std::ignore, LegacyCommonMatcher, int),
    (std::ignore, LegacyCustomMatcher, int),
    (std::ignore, LegacyMixed1Matcher, int),
    (std::ignore, LegacyMixed2Matcher, int),
    (std::ignore, CommonNullDescribeMatcher, int),
    (std::ignore, LegacyCommonVariadicMatcher, int, double),
    (std::ignore, LegacyCommonVariadicMatcher, int, double, std::string),
    (std::ignore, LegacyCustomVariadicMatcher, int, double),
    (std::ignore, LegacyCustomVariadicMatcher, int, double, std::string))
{
    STATIC_CHECK(matcher_for<Matcher, First, Others...>);
    STATIC_CHECK(matcher_for<Matcher, First const, Others const...>);
    STATIC_CHECK(matcher_for<Matcher, First&, Others&...>);
    STATIC_CHECK(matcher_for<Matcher, First const&, Others const&...>);
    STATIC_CHECK(matcher_for<Matcher, First&&, Others&&...>);
    STATIC_CHECK(matcher_for<Matcher, First const&&, Others const&&...>);
}

namespace
{
    template <typename... AcceptedTypes>
    class BasicAcceptingMatcher
    {
    public:
        template <typename... Ts>
        using is_accepting = std::bool_constant<(... && std::is_same_v<AcceptedTypes, Ts>)>;

        static constexpr bool matches(auto&&...) noexcept
        {
            return true;
        }

        static constexpr std::nullopt_t describe() noexcept
        {
            return std::nullopt;
        }
    };
}

TEMPLATE_TEST_CASE_SIG(
    "matcher_for acknowledges the optional is_accepting trait-property.",
    "[matcher]",
    ((bool expected, typename Matcher, typename First, typename... Others), expected, Matcher, First, Others...),
    (true, BasicAcceptingMatcher<int>, int),
    (true, BasicAcceptingMatcher<int&>, int&),
    (true, BasicAcceptingMatcher<int const&>, int const&),
    (true, BasicAcceptingMatcher<int&&>, int&&),
    (true, BasicAcceptingMatcher<int const&&>, int const&&),

    (false, BasicAcceptingMatcher<int>, int&),
    (false, BasicAcceptingMatcher<int const&>, int&),
    (false, BasicAcceptingMatcher<int&&>, int&),
    (false, BasicAcceptingMatcher<int const&&>, int&),

    (false, BasicAcceptingMatcher<int>, int const&),
    (false, BasicAcceptingMatcher<int&>, int const&),
    (false, BasicAcceptingMatcher<int&&>, int const&),
    (false, BasicAcceptingMatcher<int const&&>, int const&),

    (false, BasicAcceptingMatcher<int>, int&&),
    (false, BasicAcceptingMatcher<int&>, int&&),
    (false, BasicAcceptingMatcher<int const&>, int&&),
    (false, BasicAcceptingMatcher<int const&&>, int&&),

    (false, BasicAcceptingMatcher<int>, int const&&),
    (false, BasicAcceptingMatcher<int&>, int const&&),
    (false, BasicAcceptingMatcher<int const&>, int const&&),
    (false, BasicAcceptingMatcher<int&&>, int const&&),

    (true, BasicAcceptingMatcher<int const&, std::string>, int const&, std::string),
    (false, BasicAcceptingMatcher<int const&, std::string>, int&, std::string),
    (false, BasicAcceptingMatcher<int const&, std::string>, std::string, int const&))
{
    STATIC_CHECK(expected == matcher_for<Matcher, First, Others...>);
}
