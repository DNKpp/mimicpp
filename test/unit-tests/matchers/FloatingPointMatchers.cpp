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
