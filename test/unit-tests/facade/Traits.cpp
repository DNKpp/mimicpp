//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"
#include "mimic++/Facade.hpp"
#include "mimic++/ScopedSequence.hpp"

using namespace mimicpp;

namespace
{
    void CheckOmittedStacktraceEntry(util::SourceLocation const& before, reporting::CallReport const& report, util::SourceLocation const& after)
    {
#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND
        CHECK_THAT(
            report.stacktrace.source_file(0u),
            Catch::Matchers::Equals(std::string{before.file_name()}));
        // there is no straight-forward way to check the description
        CHECK(before.line() < report.stacktrace.source_line(0u));
        // strict < fails on some compilers
        CHECK(report.stacktrace.source_line(0u) <= after.line());
#else
        REQUIRE(report.stacktrace.empty());
#endif
    }
}

TEST_CASE(
    "facade::mock_as_member traits omits the facade functions stacktrace entry.",
    "[facade]")
{
    struct Type
    {
        MIMICPP_MAKE_FACADE_EXT(
            facade::mock_as_member,
            foo_,
            foo,
            /*linkage*/,
            void,
            ());
    };

    ScopedReporter reporter{};

    Type target{};
    ScopedExpectation const exp = target.foo_.expect_call();
    constexpr util::SourceLocation before{};
    target.foo();
    constexpr util::SourceLocation after{};

    REQUIRE_THAT(
        reporter.full_match_reports(),
        Catch::Matchers::SizeIs(1u));

    CheckOmittedStacktraceEntry(
        before,
        std::get<0>(reporter.full_match_reports().front()),
        after);
}

TEST_CASE(
    "facade::mock_as_member_with_this traits omits the facade functions stacktrace entry.",
    "[facade]")
{
    struct Type
    {
        MIMICPP_MAKE_FACADE_EXT(
            facade::mock_as_member_with_this<Type>,
            foo_,
            foo,
            /*linkage*/,
            void,
            ());
    };

    ScopedReporter reporter{};

    Type target{};
    ScopedExpectation const exp = target.foo_.expect_call(&target);
    constexpr util::SourceLocation before{};
    target.foo();
    constexpr util::SourceLocation after{};

    REQUIRE_THAT(
        reporter.full_match_reports(),
        Catch::Matchers::SizeIs(1u));

    CheckOmittedStacktraceEntry(
        before,
        std::get<0>(reporter.full_match_reports().front()),
        after);
}

TEST_CASE(
    "facade::mock_as_member traits generates an appropriate target name.",
    "[facade]")
{
    struct Type
    {
        MIMICPP_MAKE_FACADE_EXT(
            facade::mock_as_member,
            foo_,
            foo,
            /*linkage*/,
            void,
            ());
    };

    Type target{};
    ScopedExpectation const expectation = target.foo_.expect_call()
                                      and expect::never();
    REQUIRE_THAT(
        expectation.mock_name(),
        Catch::Matchers::ContainsSubstring("Type")
            && Catch::Matchers::EndsWith("::foo")
            && Catch::Matchers::Matches(R"(.+Type::foo)"));
}

TEST_CASE(
    "facade::mock_as_member_with_this traits generates an appropriate target name.",
    "[facade]")
{
    struct Type
    {
        MIMICPP_MAKE_FACADE_EXT(
            facade::mock_as_member_with_this<Type>,
            foo_,
            foo,
            /*linkage*/,
            void,
            ());
    };

    Type target{};
    ScopedExpectation const expectation = target.foo_.expect_call(&target)
                                      and expect::never();
    REQUIRE_THAT(
        expectation.mock_name(),
        Catch::Matchers::ContainsSubstring("Type")
            && Catch::Matchers::EndsWith("::foo")
            && Catch::Matchers::Matches(R"(.+Type::foo)"));
}

