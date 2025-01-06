// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Matcher.hpp"
#include "mimic++/Printer.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE(
    "matches::NaN determines, whether the given target denotes a not-a-number.",
    "[matcher]",
    float,
    double,
    long double)
{
    using T = TestType;
    const auto [expected, target] = GENERATE(
        (table<bool, T>({
            {false,                                    T{0.}},
            {false,                                   T{4.2}},
            {false,       std::numeric_limits<T>::infinity()},
            {false,      -std::numeric_limits<T>::infinity()},
            { true,  std::numeric_limits<T>::signaling_NaN()},
            { true, -std::numeric_limits<T>::signaling_NaN()},
            { true,      std::numeric_limits<T>::quiet_NaN()},
            { true,     -std::numeric_limits<T>::quiet_NaN()}
    })));
    INFO("Target: " << target);

    SECTION("When plain matcher is used.")
    {
        constexpr auto matcher = matches::NaN();

        REQUIRE_THAT(
            matcher.describe(),
            Catch::Matchers::Equals("is not a number (NaN)"));

        REQUIRE(expected == matcher.matches(target));
    }

    SECTION("When negated matcher is used.")
    {
        constexpr auto matcher = !matches::NaN();

        REQUIRE_THAT(
            matcher.describe(),
            Catch::Matchers::Equals("is a number"));

        REQUIRE(expected != matcher.matches(target));
    }
}

namespace
{
    inline const std::string fpRegex = R"(\d+(.\d+)?(e(\+|-)\d+)?)";
}

TEMPLATE_TEST_CASE(
    "matches::approx_abs determines a match when the floating-point value is within the specified epsilon range to the"
    " expected value",
    "[matcher]",
    float,
    double,
    long double)
{
    using T = TestType;
    const auto [expected, value, target, eps] = GENERATE(
        (table<bool, T, T, T>({
            { true,   T{0.},                                   T{0.}, T{1.e-7}},
            { true,   T{1.},                                   T{1.}, T{1.e-7}},
            { true,   T{1.},                                   T{0.},    T{1.}},
            { true,  T{-1.},                                   T{0.},    T{1.}},
            { true,  T{1.1},                                   T{1.},  T{0.15}},
            { true, T{-1.1},                                  T{-1.},  T{0.15}},
            {false, T{-1.1},                                   T{1.},  T{0.15}},
            {false,  T{1.1},                                  T{-1.},  T{0.15}},
            {false,  T{1.1},                                  T{-1.},  T{0.15}},
            {false,  T{4.2},      std::numeric_limits<T>::infinity(),    T{.1}},
            {false,  T{4.2},     -std::numeric_limits<T>::infinity(),    T{.1}},
            {false,  T{4.2},     std::numeric_limits<T>::quiet_NaN(), T{1337.}},
            {false,  T{4.2}, std::numeric_limits<T>::signaling_NaN(), T{1337.}},
    })));
    INFO("Expected value: " << value << " epsilon: " << eps << " target: " << target);

    SECTION("When plain matcher is used.")
    {
        const auto matcher = matches::approx_abs(value, eps);

        REQUIRE_THAT(
            matcher.describe(),
            Catch::Matchers::Matches(R"(is approximately -?)" + fpRegex + R"( \+- )" + fpRegex));

        REQUIRE(expected == matcher.matches(target));
    }

    SECTION("When negated matcher is used.")
    {
        const auto matcher = !matches::approx_abs(value, eps);

        REQUIRE_THAT(
            matcher.describe(),
            Catch::Matchers::Matches(R"(is not approximately -?)" + fpRegex + R"( \+- )" + fpRegex));

        REQUIRE(expected != matcher.matches(target));
    }
}

TEMPLATE_TEST_CASE(
    "matches::approx_abs throws std::runtime_error, when invalid value is given.",
    "[matcher]",
    float,
    double,
    long double)
{
    using T = TestType;

    const auto value = GENERATE(
        std::numeric_limits<T>::quiet_NaN(),
        -std::numeric_limits<T>::quiet_NaN(),
        std::numeric_limits<T>::signaling_NaN(),
        -std::numeric_limits<T>::signaling_NaN(),
        std::numeric_limits<T>::infinity(),
        -std::numeric_limits<T>::infinity());
    INFO("Value: " << value);

    REQUIRE_THROWS_AS(
        matches::approx_abs(value, T{4.2}),
        std::runtime_error);
}

TEMPLATE_TEST_CASE(
    "matches::approx_abs throws std::runtime_error, when invalid epsilon is given.",
    "[matcher]",
    float,
    double,
    long double)
{
    using T = TestType;

    const auto epsilon = GENERATE(
        std::numeric_limits<T>::quiet_NaN(),
        -std::numeric_limits<T>::quiet_NaN(),
        std::numeric_limits<T>::signaling_NaN(),
        -std::numeric_limits<T>::signaling_NaN(),
        std::numeric_limits<T>::infinity(),
        -std::numeric_limits<T>::infinity(),
        T{0},
        -T{0},
        -std::numeric_limits<T>::epsilon(),
        -4.2);
    INFO("Epsilon: " << epsilon);

    REQUIRE_THROWS_AS(
        matches::approx_abs(T{4.2}, epsilon),
        std::runtime_error);
}

TEMPLATE_TEST_CASE(
    "matches::approx_rel determines a match when the floating-point value is within a scaled epsilon range to the"
    " expected value",
    "[matcher]",
    float,
    double,
    long double)
{
    using T = TestType;
    const auto [expected, value, target, relEpsilon] = GENERATE(
        (table<bool, T, T, T>({
            { true,                                                        T{0.},                                   T{0.}, T{1.e-7}},
            { true,                                                        T{1.},                                   T{1.}, T{1.e-7}},
            // very close to 1., but with rounding errors
            { true, T{0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1},                                   T{1.}, T{1.e-7}},
            { true,                                                        T{1.},                                   T{0.},    T{1.}},
            { true,                                                       T{-1.},                                   T{0.},    T{1.}},
            { true,                                                       T{1.1},                                   T{1.},  T{0.15}},
            { true,                                                      T{-1.1},                                  T{-1.},  T{0.15}},
            {false,                                                      T{-1.1},                                   T{1.},  T{0.15}},
            {false,                                                       T{1.1},                                  T{-1.},  T{0.15}},
            {false,                                                       T{1.1},                                  T{-1.},  T{0.15}},
            {false,                                                       T{4.2},      std::numeric_limits<T>::infinity(),    T{.1}},
            {false,                                                       T{4.2},     -std::numeric_limits<T>::infinity(),    T{.1}},
            {false,                                                       T{4.2},     std::numeric_limits<T>::quiet_NaN(), T{1337.}},
            {false,                                                       T{4.2}, std::numeric_limits<T>::signaling_NaN(), T{1337.}},
    })));
    INFO("Expected value: " << value << " relative-epsilon: " << relEpsilon << " target: " << target);

    SECTION("When plain matcher is used.")
    {
        const auto matcher = matches::approx_rel(value, relEpsilon);

        REQUIRE_THAT(
            matcher.describe(),
            Catch::Matchers::Matches(R"(is approximately -?)" + fpRegex + R"( \+- \()" + fpRegex + R"( \* max\(\|lhs\|, \|rhs\|\)\))"));

        REQUIRE(expected == matcher.matches(target));
    }

    SECTION("When negated matcher is used.")
    {
        const auto matcher = !matches::approx_rel(value, relEpsilon);

        REQUIRE_THAT(
            matcher.describe(),
            Catch::Matchers::Matches(R"(is not approximately -?)" + fpRegex + R"( \+- \()" + fpRegex + R"( \* max\(\|lhs\|, \|rhs\|\)\))"));

        REQUIRE(expected != matcher.matches(target));
    }
}

TEMPLATE_TEST_CASE(
    "matches::approx_rel throws std::runtime_error, when invalid value is given.",
    "[matcher]",
    float,
    double,
    long double)
{
    using T = TestType;

    const auto value = GENERATE(
        std::numeric_limits<T>::quiet_NaN(),
        -std::numeric_limits<T>::quiet_NaN(),
        std::numeric_limits<T>::signaling_NaN(),
        -std::numeric_limits<T>::signaling_NaN(),
        std::numeric_limits<T>::infinity(),
        -std::numeric_limits<T>::infinity());
    INFO("Value: " << value);

    REQUIRE_THROWS_AS(
        matches::approx_rel(value, T{4.2}),
        std::runtime_error);
    REQUIRE_THROWS_AS(
        matches::approx_rel(value),
        std::runtime_error);
}

TEMPLATE_TEST_CASE(
    "matches::approx_rel throws std::runtime_error, when invalid epsilon is given.",
    "[matcher]",
    float,
    double,
    long double)
{
    using T = TestType;

    const auto epsilon = GENERATE(
        std::numeric_limits<T>::quiet_NaN(),
        -std::numeric_limits<T>::quiet_NaN(),
        std::numeric_limits<T>::signaling_NaN(),
        -std::numeric_limits<T>::signaling_NaN(),
        std::numeric_limits<T>::infinity(),
        -std::numeric_limits<T>::infinity(),
        T{0},
        -T{0},
        -std::numeric_limits<T>::epsilon(),
        -4.2);
    INFO("Epsilon: " << epsilon);

    REQUIRE_THROWS_AS(
        matches::approx_rel(T{4.2}, epsilon),
        std::runtime_error);
}
