//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/Format.hpp"

using namespace mimicpp;

namespace
{
    class NonPrintable
    {
    };
}

TEMPLATE_TEST_CASE_SIG(
    "detail::formattable determines whether the given type has a format::formatter specialization.",
    "[print]",
    ((bool expected, typename T, typename Char), expected, T, Char),
    (true, int, char),
    (true, const int, char),
    (true, int&, char),
    (true, const int&, char),
    (true, int, wchar_t),
    (false, NonPrintable, char))
{
    STATIC_REQUIRE(expected == format::detail::formattable<T, Char>);
}
