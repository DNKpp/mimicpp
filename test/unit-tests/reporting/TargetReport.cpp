//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/TargetReport.hpp"

using namespace mimicpp;

TEST_CASE(
    "reporting::TargetReport is equality comparable.",
    "[reporting]")
{
    reporting::TargetReport const target{
        .name = "Test mock",
        .overloadReport = reporting::TypeReport::make<void()>()};

    SECTION("Compares equal, when both sides are equal.")
    {
        reporting::TargetReport const other{target};

        REQUIRE(other == target);
        REQUIRE(target == other);
        REQUIRE_FALSE(other != target);
        REQUIRE_FALSE(target != other);
    }

    SECTION("Compares unequal, when name differs.")
    {
        reporting::TargetReport other{target};
        other.name = "Other test mock";

        REQUIRE_FALSE(other == target);
        REQUIRE_FALSE(target == other);
        REQUIRE(other != target);
        REQUIRE(target != other);
    }

    SECTION("Compares unequal, when name differs.")
    {
        reporting::TargetReport other{target};
        other.overloadReport = reporting::TypeReport::make<void() const>();

        REQUIRE_FALSE(other == target);
        REQUIRE_FALSE(target == other);
        REQUIRE(other != target);
        REQUIRE(target != other);
    }
}
