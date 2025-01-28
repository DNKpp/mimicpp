//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/TypeReport.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE(
    "TypeReport contains info about the specified type.",
    "[reporting]",
    std::string,
    std::string const,
    std::string&,
    std::string&&,
    std::string*,
    std::string[],
    std::string**)
{
    using T = TestType;

    reporting::TypeReport const report = reporting::TypeReport::make<T>();
    REQUIRE_THAT(
        report.name(),
        Catch::Matchers::Equals(print_type<T>()));
}

TEST_CASE(
    "TypeReport is equality-comparable.",
    "[reporting]")
{
    reporting::TypeReport const report = reporting::TypeReport::make<std::string>();

    SECTION("Two reports compare equal, if they both are constructed for the same type.")
    {
        reporting::TypeReport const other = reporting::TypeReport::make<std::string>();
        REQUIRE(report == other);
        REQUIRE_FALSE(report != other);
    }

    SECTION("Two reports compare unequal, if they both are constructed for the same type but with different qualifications.")
    {
        reporting::TypeReport const constOther = reporting::TypeReport::make<std::string const>();
        REQUIRE_FALSE(report == constOther);
        REQUIRE(report != constOther);

        reporting::TypeReport const refOther = reporting::TypeReport::make<std::string&>();
        REQUIRE_FALSE(report == refOther);
        REQUIRE(report != refOther);
    }

    SECTION("Two reports compare unequal, if they both are constructed for different type.")
    {
        reporting::TypeReport const other = reporting::TypeReport::make<int>();
        REQUIRE_FALSE(report == other);
        REQUIRE(report != other);
    }
}
