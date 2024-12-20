// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Stacktrace.hpp"

#include "TestTypes.hpp"

#include <ranges> // std::views::*
// ReSharper disable once CppUnusedIncludeDirective
#include <source_location>

using namespace mimicpp;

#ifdef MIMICPP_CONFIG_USE_CPPTRACE

TEST_CASE(
    "stacktrace_traits<mimicpp::cpptrace::Backend>::current() generates a new cpptrace::stacktrace.",
    "[cpptrace][stacktrace]")
{
    using BackendT = mimicpp::cpptrace::Backend;
    using traits_t = stacktrace_traits<BackendT>;

    const BackendT first = traits_t::current(0);
    const BackendT second = traits_t::current(0);

    REQUIRE_THAT(
        first,
        !Catch::Matchers::IsEmpty());
    REQUIRE_THAT(
        first.data().frames | std::views::drop(1),
        Catch::Matchers::RangeEquals(second.data().frames | std::views::drop(1)));
    REQUIRE(first.data().frames.front() != second.data().frames.front());
}

#endif

#ifdef __cpp_lib_stacktrace

TEST_CASE(
    "stacktrace_traits<std::stacktrace>::current() generates a new std::stacktrace.",
    "[stacktrace]")
{
    using BackendT = std::stacktrace;
    using traits_t = stacktrace_traits<BackendT>;

    const BackendT first = traits_t::current(0);
    const BackendT second = traits_t::current(0);

    REQUIRE_THAT(
        first,
        !Catch::Matchers::IsEmpty());
    REQUIRE_THAT(
        first | std::views::drop(1),
        Catch::Matchers::RangeEquals(second | std::views::drop(1)));
    REQUIRE(first.at(0) != second.at(0));
}

#endif

#ifdef MIMICPP_DETAIL_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "current_stacktrace retrieves the current stacktrace.",
    "[stacktrace]")
{
    const auto before = std::source_location::current();
    const Stacktrace cur = current_stacktrace();
    const auto after = std::source_location::current();

    REQUIRE(!cur.empty());
    REQUIRE_THAT(
        cur.source_file(0u),
        Catch::Matchers::Equals(before.file_name()));
    const std::size_t line = cur.source_line(0u);
    REQUIRE(std::cmp_less(before.line(), line));
    REQUIRE(std::cmp_less(line, after.line()));
}

#endif

TEST_CASE(
    "stacktrace_traits<EmptyStacktraceBackend>::current() generates a new empty stacktrace.",
    "[stacktrace]")
{
    using traits_t = stacktrace_traits<EmptyStacktraceBackend>;

    const Stacktrace stacktrace{traits_t::current(42)};

    REQUIRE(stacktrace.empty());
    REQUIRE(0u == stacktrace.size());

    const std::size_t index = GENERATE(0, 1, 42);
    REQUIRE_THROWS(stacktrace.description(index));
    REQUIRE_THROWS(stacktrace.source_file(index));
    REQUIRE_THROWS(stacktrace.source_line(index));
}
