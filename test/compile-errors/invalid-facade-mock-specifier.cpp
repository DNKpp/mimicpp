//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/mimic++.hpp"

/*
<begin-expected-compile-error>
unsupported specifier[;] allowed are `const`, `&`, `&&`, `noexcept`, `override` and `final`\.
<end-expected-compile-error>
*/

void check()
{
    class Foo
    {
        MIMICPP_MAKE_MEMBER_MOCK(foo, void, (), !invalid223);
    };
}
