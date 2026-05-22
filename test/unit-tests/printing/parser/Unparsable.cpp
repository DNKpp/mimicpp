//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/type/Parser.hpp"

using namespace mimicpp;

TEST_CASE(
    "parsing::parse_type returns std::nullopt if input cannot be parsed.",
    "[print][print::type]")
{
    auto const input = GENERATE(
        "const int const",
        "volatile int const volatile",
        "int const int",
        "int* int",
        "int<int>",
        "int[int]");
    CAPTURE(input);

    CHECK_FALSE(printing::type::parse_type(input));
}
