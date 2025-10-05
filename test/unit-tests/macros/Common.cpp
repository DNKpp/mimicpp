//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/macros/Common.hpp"

TEST_CASE(
    "MIMICPP_DETAIL_STRIP_PARENS removes outer parens, if present.",
    "[macro][detail]")
{
    namespace Matches = Catch::Matchers;

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_STRIP_PARENS()),
        Matches::Equals(""));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_STRIP_PARENS(())),
        Matches::Equals(""));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_STRIP_PARENS((()))),
        Matches::Equals("()"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_STRIP_PARENS(Test())),
        Matches::Equals("Test()"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_STRIP_PARENS((Test()))),
        Matches::Equals("Test()"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_STRIP_PARENS(((Test())))),
        Matches::Equals("(Test())"));

    // clang-format off
    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_STRIP_PARENS(((,Test(),)))),
        Matches::Equals("(,Test(),)"));
    // clang-format on
}
