//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/mimic++.hpp"

/*
<begin-expected-compile-error>
Only one finalize-policy may be specified per expectation\.
See: https://dnkpp\.github\.io/mimicpp/
<end-expected-compile-error>
*/

void check()
{
    mimicpp::Mock<void()> mock{};
    SCOPED_EXP mock.expect_call()
        and mimicpp::finally::throws(1u)
        and mimicpp::finally::throws(42u);
}
