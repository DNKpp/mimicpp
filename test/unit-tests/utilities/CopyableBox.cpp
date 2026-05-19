//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/CopyableBox.hpp"
#include "../SuppressionMacros.hpp"

using namespace mimicpp;

TEST_CASE(
    "util::CopyableBox is a copyable type.",
    "[utility]")
{
    STATIC_CHECK(std::is_default_constructible_v<util::CopyableBox<int>>);
    STATIC_CHECK(std::is_nothrow_destructible_v<util::CopyableBox<int>>);
    STATIC_CHECK(std::is_copy_constructible_v<util::CopyableBox<int>>);
    STATIC_CHECK(std::is_copy_assignable_v<util::CopyableBox<int>>);
    STATIC_CHECK(std::is_nothrow_move_constructible_v<util::CopyableBox<int>>);
    STATIC_CHECK(std::is_nothrow_move_assignable_v<util::CopyableBox<int>>);
}

TEST_CASE(
    "util::CopyableBox can be default constructed.",
    "[utility]")
{
    util::CopyableBox<int> source{};
    CHECK(0 == *source);
    CHECK(0 == *std::as_const(source));
}

TEST_CASE(
    "util::CopyableBox supports copy operations.",
    "[utility]")
{
    util::CopyableBox source{42};

    SECTION("When copy-ctor is used.")
    {
        util::CopyableBox const other{source};
        CHECK(42 == *other);
        CHECK(source);
    }

    SECTION("When copy-assignment is used.")
    {
        util::CopyableBox<int> other{};
        other = source;

        CHECK(42 == *other);
    }

    SECTION("When self copy-assigned.")
    {
        START_WARNING_SUPPRESSION
        SUPPRESS_SELF_ASSIGN
        source = source;
        STOP_WARNING_SUPPRESSION

        CHECK(42 == *source);
    }
}

TEST_CASE(
    "util::CopyableBox supports move operations.",
    "[utility]")
{
    util::CopyableBox source{42};

    SECTION("When move-ctor is used.")
    {
        util::CopyableBox const other{std::move(source)};
        CHECK(42 == *other);
        CHECK(!source);
    }

    SECTION("When move-assignment is used.")
    {
        util::CopyableBox<int> other{};
        other = std::move(source);

        CHECK(42 == *other);
    }

    SECTION("When self move-assigned.")
    {
        START_WARNING_SUPPRESSION
        SUPPRESS_SELF_MOVE
        source = std::move(source);
        STOP_WARNING_SUPPRESSION

        CHECK(42 == *source);
    }
}

TEST_CASE(
    "util::CopyableBox can be equality compared.",
    "[utility]")
{
    auto const [expected, firstValue, secondValue] = GENERATE((table<bool, int, int>)({
        {true,  42, 42  },
        {false, 42, 1337}
    }));

    util::CopyableBox const first{firstValue};
    util::CopyableBox const second{secondValue};

    CHECK(expected == (first == second));
    CHECK(expected == (second == first));
}

TEST_CASE(
    "util::CopyableBox supports operator->.",
    "[utility]")
{
    util::CopyableBox const first{42};
    util::CopyableBox const second{42};

    CHECK(first.operator->() == &*first);
    CHECK(std::as_const(first).operator->() == &*first);
    CHECK(second.operator->() == &*second);
    CHECK(std::as_const(second).operator->() == &*second);
    CHECK(first.operator->() != second.operator->());
}
