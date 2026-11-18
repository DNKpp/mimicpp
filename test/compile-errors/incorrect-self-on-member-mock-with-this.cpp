//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/mimic++.hpp"

/*
<begin-expected-compile-error>
The `self_type` alias must name the type that holds the member-mock with the additional this-argument, e\.g\. `using self_type = Foo[;]` where `Foo` is the surrounding class\.
<end-expected-compile-error>
*/

void check()
{
    class Foo
    {
        using self_type = int;
        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo, void, ());
    };
}
