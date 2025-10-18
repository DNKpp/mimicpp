//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/Stacktrace.hpp"

using namespace mimicpp;

TEST_CASE(
    TEST_CASE_PREFIX " - The active backend is the expected one.",
    "[stacktrace]")
{
    using ExpectedBackend = EXPECTED_BACKEND_TYPE;
    STATIC_CHECK(std::same_as<ExpectedBackend, util::stacktrace::find_backend::type>);
}

#ifndef NDEBUG

TEST_CASE(
    TEST_CASE_PREFIX " - The active backend is fully functional.",
    "[stacktrace]")
{
    util::Stacktrace const stacktrace = GENERATE(
        util::stacktrace::current(),
        util::stacktrace::current(0),
        util::stacktrace::current(0, 3));
    CAPTURE(stacktrace);
    REQUIRE_FALSE(stacktrace.empty());
    REQUIRE(0 < stacktrace.size());

    // To keep my sanity, I relax the source-file and line requirement to just not failing,
    // because on macOS and boost::stacktrace nothing useful comes back.
    CHECK_THAT(
        stacktrace.description(0),
        !Catch::Matchers::IsEmpty());
    CHECK_NOTHROW(stacktrace.source_file(0));
    CHECK_NOTHROW(stacktrace.source_line(0));

    for (std::size_t const i : std::views::iota(1u, stacktrace.size()))
    {
        CAPTURE(i);
        CHECK_NOTHROW(stacktrace.description(i));
        CHECK_NOTHROW(stacktrace.source_file(i));
        CHECK_NOTHROW(stacktrace.source_line(i));
    }
}

TEST_CASE(
    TEST_CASE_PREFIX " - stacktrace::current's first frame is the from the source function.",
    "[stacktrace]")
{
    auto const super = [] { return util::stacktrace::current(); }();
    CAPTURE(super);
    REQUIRE(4u < super.size());

    auto const stacktrace = util::stacktrace::current();
    CAPTURE(stacktrace);

    // Comparing the current top-lvl frame is quite inconsistent with boost::stacktrace on msvc,
    // so let's head a few layers up.
    REQUIRE(3u < stacktrace.size());
    CHECK(super.source_line(4u) == stacktrace.source_line(3u));
    CHECK_THAT(
        stacktrace.source_file(3u),
        Catch::Matchers::Equals(super.source_file(4u)));
    CHECK_THAT(
        stacktrace.description(3u),
        Catch::Matchers::Equals(super.description(4u)));
}

TEST_CASE(
    TEST_CASE_PREFIX " - stacktrace::current supports skip.",
    "[stacktrace]")
{
    auto const full = util::stacktrace::current();
    CAPTURE(full);
    REQUIRE(3u < full.size());

    auto const stacktrace = util::stacktrace::current(3u);
    CAPTURE(stacktrace);

    REQUIRE_FALSE(stacktrace.empty());
    CHECK(full.source_line(3u) == stacktrace.source_line(0u));
    CHECK_THAT(
        stacktrace.source_file(0u),
        Catch::Matchers::ContainsSubstring(std::string{full.source_file(3u)}));
    CHECK_THAT(
        stacktrace.description(0u),
        Catch::Matchers::ContainsSubstring(std::string{full.description(3u)}));
}

TEST_CASE(
    TEST_CASE_PREFIX " - stacktrace::current supports max.",
    "[stacktrace]")
{
    auto const full = util::stacktrace::current();
    CAPTURE(full);
    REQUIRE(3u < full.size());

    auto const stacktrace = util::stacktrace::current(3u, 1u);
    CAPTURE(stacktrace);

    REQUIRE(1u == stacktrace.size());
    CHECK(full.source_line(3u) == stacktrace.source_line(0u));
    CHECK_THAT(
        stacktrace.source_file(0u),
        Catch::Matchers::ContainsSubstring(std::string{full.source_file(3u)}));
    CHECK_THAT(
        stacktrace.description(0u),
        Catch::Matchers::ContainsSubstring(std::string{full.description(3u)}));
}

#endif