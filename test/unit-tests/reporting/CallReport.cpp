//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/CallReport.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;
using namespace reporting;

TEST_CASE(
    "CallReport::Arg is equality comparable.",
    "[reporting]")
{
    using ArgT = CallReport::Arg;

    const ArgT first{
        .typeInfo = reporting::TypeReport::make<int>(),
        .stateString = "42"};

    const auto [expectedEquality, second] = GENERATE(
        (table<bool, ArgT>({
            {false, {reporting::TypeReport::make<int>(), "1337"}},
            {false, {reporting::TypeReport::make<short>(), "42"}},
            { true,   {reporting::TypeReport::make<int>(), "42"}}
    })));

    REQUIRE(expectedEquality == (first == second));
    REQUIRE(expectedEquality == (second == first));
    REQUIRE(expectedEquality == !(first != second));
    REQUIRE(expectedEquality == !(second != first));
}

TEST_CASE(
    "CallReport is equality comparable.",
    "[reporting]")
{
    const CallReport first{
        .returnTypeInfo = TypeReport::make<std::string>(),
        .argDetails = {{.typeInfo = TypeReport::make<int>(), .stateString = "42"}},
        .fromLoc = std::source_location::current(),
        .stacktrace = stacktrace::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("When both sides are equal, they compare equal.")
    {
        const CallReport second{first};

        REQUIRE(first == second);
        REQUIRE(second == first);
        REQUIRE(!(first != second));
        REQUIRE(!(second != first));
    }

    SECTION("When return type differs, they compare not equal.")
    {
        CallReport second{first};

        second.returnTypeInfo = GENERATE(TypeReport::make<void>(), TypeReport::make<std::string_view>());

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE(!(first == second));
        REQUIRE(!(second == first));
    }

    SECTION("When category differs, they compare not equal.")
    {
        CallReport second{first};

        second.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue);

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE(!(first == second));
        REQUIRE(!(second == first));
    }

    SECTION("When constness differs, they compare not equal.")
    {
        CallReport second{first};

        second.fromConstness = GENERATE(Constness::as_const, Constness::non_const);

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE(!(first == second));
        REQUIRE(!(second == first));
    }

    SECTION("When source location differs, they compare not equal.")
    {
        CallReport second{first};

        second.fromLoc = std::source_location::current();

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE(!(first == second));
        REQUIRE(!(second == first));
    }

    SECTION("When arg-infos differ, they compare not equal.")
    {
        CallReport second{first};

        using ArgT = CallReport::Arg;
        second.argDetails = GENERATE(
            std::vector<ArgT>{},
            std::vector{
                (ArgT{.typeInfo = TypeReport::make<int>(), .stateString = "1337"})},
            std::vector{
                (ArgT{.typeInfo = TypeReport::make<int>(), .stateString = "42"}),
                (ArgT{.typeInfo = TypeReport::make<int>(), .stateString = "1337"})});

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE(!(first == second));
        REQUIRE(!(second == first));
    }

#ifdef MIMICPP_DETAIL_WORKING_STACKTRACE_BACKEND
    SECTION("When stacktrace differs, they compare not equal.")
    {
        CallReport second{first};
        second.stacktrace = stacktrace::current();

        REQUIRE(first != second);
        REQUIRE(second != first);
        REQUIRE(!(first == second));
        REQUIRE(!(second == first));
    }
#endif
}

TEST_CASE(
    "make_call_report generates report from call::Info."
    "[reporting]")
{
    SECTION("When call info has void return type.")
    {
        const call::Info<void> info{
            .args = {},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers)),
            .fromSourceLocation = std::source_location::current(),
            .stacktrace = stacktrace::current()};

        const CallReport report = make_call_report(info);

        const CallReport expected{
            .returnTypeInfo = TypeReport::make<void>(),
            .argDetails = {},
            .fromLoc = info.fromSourceLocation,
            .stacktrace = info.stacktrace,
            .fromCategory = info.fromCategory,
            .fromConstness = info.fromConstness};
        REQUIRE(report == expected);
    }

    SECTION("When call info has non-void return type.")
    {
        const call::Info<int> info{
            .args = {},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers)),
            .fromSourceLocation = std::source_location::current(),
            .stacktrace = stacktrace::current()};

        const CallReport report = make_call_report(info);

        const CallReport expected{
            .returnTypeInfo = TypeReport::make<int>(),
            .argDetails = {},
            .fromLoc = info.fromSourceLocation,
            .stacktrace = info.stacktrace,
            .fromCategory = info.fromCategory,
            .fromConstness = info.fromConstness};
        REQUIRE(report == expected);
    }

    SECTION("When call info has arbitrary args.")
    {
        const int arg0{1337};
        double arg1{4.2};
        std::string arg2{"Hello, World!"};
        const call::Info<void, const int&, double, std::string> info{
            .args = {std::ref(arg0), std::ref(arg1), std::ref(arg2)},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers)),
            .fromSourceLocation = std::source_location::current(),
            .stacktrace = stacktrace::current()
        };

        const CallReport report = make_call_report(info);

        using ArgT = CallReport::Arg;
        const CallReport expected{
            .returnTypeInfo = TypeReport::make<void>(),
            .argDetails = {
                           ArgT{TypeReport::make<int const&>(), "1337"},
                           ArgT{TypeReport::make<double>(), "4.2"},
                           ArgT{TypeReport::make<std::string>(), "\"Hello, World!\""}},
            .fromLoc = info.fromSourceLocation,
            .stacktrace = info.stacktrace,
            .fromCategory = info.fromCategory,
            .fromConstness = info.fromConstness
        };
        REQUIRE(report == expected);
    }
}

TEST_CASE(
    "CallReport can be printed.",
    "[report][print]")
{
    namespace Matches = Catch::Matchers;

    SECTION("When report without arguments is given.")
    {
        const CallReport report{
            .returnTypeInfo = TypeReport::make<void>(),
            .argDetails = {},
            .fromLoc = std::source_location::current(),
            .stacktrace = stacktrace::current(),
            .fromCategory = ValueCategory::any,
            .fromConstness = Constness::any};

        REQUIRE_THAT(
            print(report),
            Matches::Matches(
                "call from .+\\[\\d+(:\\d+)?\\], .+\n"
                "constness: any\n"
                "value category: any\n"
                "return type: (v|void)\n"));
    }

    SECTION("When report with arguments is given.")
    {
        const CallReport report{
            .returnTypeInfo = TypeReport::make<int>(),
            .argDetails = {{.typeInfo = TypeReport::make<double>(), .stateString = "4.2"}},
            .fromLoc = std::source_location::current(),
            .stacktrace = stacktrace::current(),
            .fromCategory = ValueCategory::lvalue,
            .fromConstness = Constness::as_const};

        REQUIRE_THAT(
            print(report),
            Matches::Matches(
                "call from .+\\[\\d+(:\\d+)?\\], .+\n"
                "constness: const\n"
                "value category: lvalue\n"
                "return type: (i|int)\n"
                "args:\n"
                "\targ\\[0\\]: \\{\n"
                "\t\ttype: (d|double),\n"
                "\t\tvalue: 4.2\n"
                "\t\\},\n"));
    }
}
