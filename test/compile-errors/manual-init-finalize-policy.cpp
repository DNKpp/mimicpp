//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/mimic++.hpp"

/*
<begin-expected-compile-error>
Explicitly specifying the `policies::InitFinalize` is disallowed\.
<end-expected-compile-error>
*/

void check()
{
    mimicpp::Mock<void()> mock{};
    SCOPED_EXP mock.expect_call()
        and mimicpp::expectation::policies::InitFinalize{};
}
