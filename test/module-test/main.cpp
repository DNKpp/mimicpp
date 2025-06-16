//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

import mimicpp;

#include <string>

#include "mimic++/MacroExports.hpp"

TEST_CASE("mimic++ supports modules!")
{
    mimicpp::Mock<int(std::string), void()> mock{};
    mimicpp::ScopedSequence sequence{};

    SCOPED_EXP mock.expect_call(mimicpp::matches::str::eq("Hello, mimic++!"))
        and mimicpp::expect::in_sequence(sequence)
        and mimicpp::expect::times(2)
        and mimicpp::expect::arg<0>(!mimicpp::matches::range::is_empty())
        and mimicpp::then::invoke([] {})
        and mimicpp::finally::returns(42);

    CHECK(42 == mock("Hello, mimic++!"));
    CHECK(42 == mock("Hello, mimic++!"));
}
