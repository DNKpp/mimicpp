//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/CallReport.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;
using namespace reporting;

namespace
{
    template <typename Signature>
    [[nodiscard]]
    TargetReport make_common_target_report()
    {
        return TargetReport{
            .name = "Mock-Name",
            .overloadReport = TypeReport::make<Signature>()};
    }
}

TEST_CASE(
    "CallReport::Arg is equality comparable.",
    "[reporting]")
{
    using ArgT = CallReport::Arg;

    ArgT const first{
        .typeInfo = TypeReport::make<int>(),
        .stateString = "42"};

    auto const [expectedEquality, second] = GENERATE(
        (table<bool, ArgT>({
            {false, {reporting::TypeReport::make<int>(), "1337"}},
            {false, {reporting::TypeReport::make<short>(), "42"}},
            { true,   {reporting::TypeReport::make<int>(), "42"}}
    })));

    CHECK(expectedEquality == (first == second));
    CHECK(expectedEquality == (second == first));
    CHECK(expectedEquality == !(first != second));
    CHECK(expectedEquality == !(second != first));
}

TEST_CASE(
    "CallReport is equality comparable.",
    "[reporting]")
{
    const CallReport first{
        .target = make_common_target_report<std::string(int)>(),
        .returnTypeInfo = TypeReport::make<std::string>(),
        .argDetails = {{.typeInfo = TypeReport::make<int>(), .stateString = "42"}},
        .fromLoc = std::source_location::current(),
        .stacktrace = stacktrace::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("When both sides are equal, they compare equal.")
    {
        CallReport const second{first};

        CHECK(first == second);
        CHECK(second == first);
        CHECK_FALSE(first != second);
        CHECK_FALSE(second != first);
    }

    SECTION("When target differs, they compare not equal.")
    {
        CallReport second{first};

        second.target.name = "Other Mock";

        CHECK(first != second);
        CHECK(second != first);
        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
    }

    SECTION("When return type differs, they compare not equal.")
    {
        CallReport second{first};

        second.returnTypeInfo = GENERATE(TypeReport::make<void>(), TypeReport::make<std::string_view>());

        CHECK(first != second);
        CHECK(second != first);
        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
    }

    SECTION("When category differs, they compare not equal.")
    {
        CallReport second{first};

        second.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue);

        CHECK(first != second);
        CHECK(second != first);
        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
    }

    SECTION("When constness differs, they compare not equal.")
    {
        CallReport second{first};

        second.fromConstness = GENERATE(Constness::as_const, Constness::non_const);

        CHECK(first != second);
        CHECK(second != first);
        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
    }

    SECTION("When source location differs, they compare not equal.")
    {
        CallReport second{first};

        second.fromLoc = std::source_location::current();

        CHECK(first != second);
        CHECK(second != first);
        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
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

        CHECK(first != second);
        CHECK(second != first);
        CHECK_FALSE(first == second);
        CHECK_FALSE(second == first);
    }

#ifdef MIMICPP_DETAIL_WORKING_STACKTRACE_BACKEND
    SECTION("When stacktrace differs, they compare not equal.")
    {
        CallReport second{first};
        second.stacktrace = stacktrace::current();

        CHECK(first != second);
        CHECK(second != first);
        CHECK_FALSE(first == second);
        RCHECK_FALSE(second == first);
    }
#endif
}

TEST_CASE(
    "make_call_report generates report from call::Info."
    "[reporting]")
{
    SECTION("When call info has void return type.")
    {
        call::Info<void> const info{
            .args = {},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers)),
            .fromSourceLocation = std::source_location::current(),
            .stacktrace = stacktrace::current()};
        TargetReport const target = make_common_target_report<void()>();
        CallReport const report = make_call_report(target, info);

        CallReport const expected{
            .target = target,
            .returnTypeInfo = TypeReport::make<void>(),
            .argDetails = {},
            .fromLoc = info.fromSourceLocation,
            .stacktrace = info.stacktrace,
            .fromCategory = info.fromCategory,
            .fromConstness = info.fromConstness};
        CHECK(report == expected);
    }

    SECTION("When call info has non-void return type.")
    {
        call::Info<int> const info{
            .args = {},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers)),
            .fromSourceLocation = std::source_location::current(),
            .stacktrace = stacktrace::current()};
        TargetReport const target = make_common_target_report<int()>();
        CallReport const report = make_call_report(target, info);

        CallReport const expected{
            .target = target,
            .returnTypeInfo = TypeReport::make<int>(),
            .argDetails = {},
            .fromLoc = info.fromSourceLocation,
            .stacktrace = info.stacktrace,
            .fromCategory = info.fromCategory,
            .fromConstness = info.fromConstness};
        CHECK(report == expected);
    }

    SECTION("When call info has arbitrary args.")
    {
        const int arg0{1337};
        double arg1{4.2};
        std::string arg2{"Hello, World!"};
        call::Info<void, const int&, double, std::string> const info{
            .args = {std::ref(arg0), std::ref(arg1), std::ref(arg2)},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers)),
            .fromSourceLocation = std::source_location::current(),
            .stacktrace = stacktrace::current()
        };
        TargetReport const target = make_common_target_report<void(const int&, double, std::string)>();
        CallReport const report = make_call_report(target, info);

        using ArgT = CallReport::Arg;
        CallReport const expected{
            .target = target,
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
        CHECK(report == expected);
    }
}
