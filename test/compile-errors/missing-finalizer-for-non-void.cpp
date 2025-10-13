//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/mimic++.hpp"

/*
<begin-expected-compile-error>
For non-void return types, a finalize-policy must be specified\.
See: https://dnkpp\.github\.io/mimicpp
<end-expected-compile-error>
*/

void check()
{
    mimicpp::Mock<int()> mock{};
    SCOPED_EXP mock.expect_call();
}
