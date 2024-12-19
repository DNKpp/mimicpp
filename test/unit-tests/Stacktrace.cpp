// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Stacktrace.hpp"

#include <ranges>

using namespace mimicpp;

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE

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

/*
 * From here on, the CustomBackend will be registered.
 */
namespace
{
    class CustomBackend
    {
    };
}

template <>
struct custom::stacktrace_backend<register_tag>
{
    using type = CustomBackend;
};

/*TEST_CASE(
    "current_stacktrace prefers custom registrations.",
    "[stacktrace]")
{
    const auto before = std::source_location::current();
    const mimicpp::cpptrace::Backend cur = current_stacktrace();
    const auto after = std::source_location::current();

    REQUIRE_THAT(
        cur.data().frames.front().filename,
        Catch::Matchers::Equals(before.file_name()));
    const auto line = cur.data().frames.front().line;
    REQUIRE(line.has_value());
    REQUIRE(std::cmp_less(before.line(), line.value()));
    REQUIRE(std::cmp_less(line.value(), after.line()));
}*/
