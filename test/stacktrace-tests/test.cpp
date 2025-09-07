//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/SourceLocation.hpp"

#include <mimic++/utilities/Stacktrace.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <concepts>
#include <ranges>

using namespace mimicpp;

TEST_CASE(
    TEST_CASE_PREFIX " - The active backend is the expected one.",
    "[stacktrace]")
{
    using ExpectedBackend = EXPECTED_BACKEND_TYPE;
    STATIC_CHECK(std::same_as<ExpectedBackend, util::stacktrace::find_backend::type>);
}

TEST_CASE(
    TEST_CASE_PREFIX " - The active backend is fully functional.",
    "[stacktrace]")
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

namespace
{
    [[nodiscard]]
    util::Stacktrace generate_stacktrace()
    {
        return util::stacktrace::current();
    }
}

TEST_CASE(
    TEST_CASE_PREFIX " - stacktrace::current's first frame is the from the source function.",
    "[stacktrace]")
{
    auto const stacktrace = generate_stacktrace();

    REQUIRE_FALSE(stacktrace.empty());
    CHECK_THAT(
        stacktrace.description(0u),
        Catch::Matchers::ContainsSubstring("generate_stacktrace"));
}

namespace
{
    [[nodiscard]]
    util::Stacktrace skip_inner_generate_stacktrace(std::size_t const skip)
    {
        return util::stacktrace::current(skip);
    }

    [[nodiscard]]
    util::Stacktrace skip_generate_stacktrace(std::size_t const skip)
    {
        return skip_inner_generate_stacktrace(skip);
    }
}

TEST_CASE(
    TEST_CASE_PREFIX " - stacktrace::current supports skip.",
    "[stacktrace]")
{
    REQUIRE(1u < skip_generate_stacktrace(0u).size());
    REQUIRE_THAT(
        skip_generate_stacktrace(0u).description(0u),
        Catch::Matchers::ContainsSubstring("skip_inner_generate_stacktrace"));

    auto const stacktrace = skip_generate_stacktrace(1u);

    REQUIRE_FALSE(stacktrace.empty());
    CHECK_THAT(
        stacktrace.description(0u),
        Catch::Matchers::ContainsSubstring("skip_generate_stacktrace"));
}

namespace
{
    [[nodiscard]]
    util::Stacktrace max_generate_stacktrace(std::size_t const max)
    {
        return util::stacktrace::current(0u, max);
    }
}

TEST_CASE(
    TEST_CASE_PREFIX " - stacktrace::current supports max.",
    "[stacktrace]")
{
    auto const stacktrace = max_generate_stacktrace(1u);

    REQUIRE(1u == stacktrace.size());
    REQUIRE_THAT(
        stacktrace.description(0u),
        Catch::Matchers::ContainsSubstring("max_generate_stacktrace"));
}
