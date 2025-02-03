//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/Concepts.hpp"

#include <type_traits>

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "satisfies determines, whether T satisfies the given trait.",
    "[utility]",
    ((bool expected, typename T, template <typename> typename Trait), expected, T, Trait),
    (false, int, std::is_floating_point),
    (false, int&, std::is_integral),
    (true, int, std::is_integral))
{
    STATIC_REQUIRE(expected == util::satisfies<T, Trait>);
}

TEMPLATE_TEST_CASE_SIG(
    "same_as_any determines, whether T is the same as any of the given types.",
    "[utility]",
    ((bool expected, typename T, typename... Others), expected, T, Others...),
    (false, int),
    (false, int, int&),
    (false, int, int&, int&),
    (false, int, int&, double, float),
    (true, int, int),
    (true, int, int&, int),
    (true, int, double, int, int&))
{
    STATIC_REQUIRE(expected == util::same_as_any<T, Others...>);
}
