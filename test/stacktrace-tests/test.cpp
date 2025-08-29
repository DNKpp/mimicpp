//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <mimic++/utilities/Stacktrace.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <concepts>
#include <ranges>

using namespace mimicpp;

TEST_CASE(TEST_CASE_PREFIX " - The active backend is not stacktrace::NullBackend.")
{
    STATIC_CHECK(!std::same_as<util::stacktrace::NullBackend, util::stacktrace::find_backend::type>);
}

TEST_CASE(TEST_CASE_PREFIX " - The active backend is fully functional.")
{
    util::Stacktrace const stacktrace = GENERATE(
        util::stacktrace::current(),
        util::stacktrace::current(0),
        util::stacktrace::current(0, 3));
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
