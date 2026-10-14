//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/UnwrapRef.hpp"

using namespace mimicpp;

TEST_CASE(
    "util::unwrap_ref forwards plain lvalue-references as-is.",
    "[util][util::unwrap_ref]")
{
    SECTION("Mutable lvalue.")
    {
        int value{42};

        int& result = util::unwrap_ref(value);

        CHECK(&result == &value);
    }

    SECTION("Const lvalue.")
    {
        constexpr int value{42};

        int const& result = util::unwrap_ref(value);

        CHECK(&result == &value);
    }
}

TEST_CASE(
    "util::unwrap_ref unwraps std::reference_wrapper and returns the referenced object.",
    "[util][util::unwrap_ref]")
{
    SECTION("Mutable referenced object.")
    {
        int value{42};
        std::reference_wrapper const ref{value};

        int& result = util::unwrap_ref(ref);

        CHECK(&result == &value);
    }

    SECTION("Const referenced object.")
    {
        constexpr int value{42};
        std::reference_wrapper const ref{value};

        int const& result = util::unwrap_ref(ref);

        CHECK(&result == &value);
    }

    SECTION("Also works, when the reference_wrapper itself is a prvalue.")
    {
        int value{42};

        int& result = util::unwrap_ref(std::ref(value));

        CHECK(&result == &value);
    }
}

TEMPLATE_TEST_CASE(
    "util::unwrap_ref does not accept rvalue-references, other than std::reference_wrapper.",
    "[util][util::unwrap_ref]",
    int)
{
    using T = TestType;
    STATIC_REQUIRE(!requires { util::unwrap_ref(T{}); });
    STATIC_REQUIRE(requires { {util::unwrap_ref(std::declval<T&>())} -> std::same_as<T&>; });
    STATIC_REQUIRE(requires { {util::unwrap_ref(std::declval<std::reference_wrapper<T>>())} -> std::same_as<T&>; });
}

