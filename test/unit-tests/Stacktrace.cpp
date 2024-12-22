// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Stacktrace.hpp"

#include "SuppressionMacros.hpp"
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
        first.data().frames,
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

TEST_CASE(
    "current_stacktrace supports skipping of the top elements.",
    "[stacktrace]")
{
    const Stacktrace full = current_stacktrace();
    CHECK(!full.empty());

    const auto compare = [&](const std::size_t skip, const Stacktrace& other) {
        return std::ranges::all_of(
            std::views::iota(0u, other.size()),
            [&](const std::size_t i) {
                return full.description(i + skip) == other.description(i)
                    && full.source_file(i + skip) == other.source_file(i)
                    && full.source_line(i + skip) == other.source_line(i);
            });
    };

    SECTION("When skip == 0")
    {
        const Stacktrace other = current_stacktrace();
        CHECK(full.size() == other.size());

        REQUIRE( // everything except the top element must be equal
            std::ranges::all_of(
                std::views::iota(1u, other.size()),
                [&](const std::size_t i) {
                    return full.description(i) == other.description(i)
                        && full.source_file(i) == other.source_file(i)
                        && full.source_line(i) == other.source_line(i);
                }));
        // of the top elements description may differ
        REQUIRE_THAT(
            other.source_file(0),
            Catch::Matchers::Equals(full.source_file(0)));
        REQUIRE(full.source_line(0) < other.source_line(0));
    }

    SECTION("When skip == 1.")
    {
        const Stacktrace partial = current_stacktrace(1);
        CHECK(!partial.empty());
        CHECK(full.size() == partial.size() + 1u);

        REQUIRE(compare(1u, partial));
    }

    SECTION("When skip == 2.")
    {
        const Stacktrace partial = current_stacktrace(2);
        CHECK(!partial.empty());
        CHECK(full.size() == partial.size() + 2u);

        REQUIRE(compare(2u, partial));
    }

    SECTION("When skip is very high.")
    {
        const Stacktrace partial = current_stacktrace(1337);
        REQUIRE(partial.empty());
    }
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

namespace
{
    [[nodiscard]]
    bool equal_entries(const Stacktrace& original, const Stacktrace& test)
    {
#ifdef MIMICPP_DETAIL_WORKING_STACKTRACE_BACKEND
        return std::ranges::all_of(
            std::views::iota(0u, original.size()),
            [&](const std::size_t index) {
                return original.description(index) == test.description(index)
                    && original.source_file(index) == test.source_file(index)
                    && original.source_line(index) == test.source_line(index);
            });
#else
        // we have an EmptyStacktraceBackend
        const std::size_t index = GENERATE(0, 1, 42);
        REQUIRE_THROWS(original.description(index));
        REQUIRE_THROWS(original.source_file(index));
        REQUIRE_THROWS(original.source_line(index));
        REQUIRE_THROWS(test.description(index));
        REQUIRE_THROWS(test.source_file(index));
        REQUIRE_THROWS(test.source_line(index));
        return true;
#endif
    }
}

TEST_CASE(
    "Stacktrace is copyable.",
    "[stacktrace]")
{
    // explicitly prevent a custom backend.
    using traits_t = stacktrace_traits<find_stacktrace_backend<register_tag>::type>;
    const Stacktrace source{traits_t::current(0)};

    SECTION("When copy-constructing.")
    {
        const Stacktrace copy{source};

        REQUIRE(copy.empty() == source.empty());
        REQUIRE(copy.size() == source.size());
        REQUIRE(equal_entries(source, copy));
    }

    SECTION("When copy-assigning.")
    {
        Stacktrace copy{traits_t::current(0)};
        copy = source;

        REQUIRE(copy.empty() == source.empty());
        REQUIRE(copy.size() == source.size());
        REQUIRE(equal_entries(source, copy));
    }
}

TEST_CASE(
    "Stacktrace is movable.",
    "[stacktrace]")
{
    // explicitly prevent a custom backend.
    using traits_t = stacktrace_traits<find_stacktrace_backend<register_tag>::type>;
    Stacktrace source{traits_t::current(0)};
    const Stacktrace copy{source};

    SECTION("When move-constructing.")
    {
        const Stacktrace current{std::move(source)};

        REQUIRE(copy.empty() == current.empty());
        REQUIRE(copy.size() == current.size());
        REQUIRE(equal_entries(copy, current));
    }

    SECTION("When move-assigning.")
    {
        Stacktrace current{traits_t::current(0)};
        current = std::move(source);

        REQUIRE(copy.empty() == current.empty());
        REQUIRE(copy.size() == current.size());
        REQUIRE(equal_entries(copy, current));
    }

    SECTION("When self move-assigning.")
    {
        START_WARNING_SUPPRESSION
        SUPPRESS_SELF_MOVE
        source = std::move(source);
        STOP_WARNING_SUPPRESSION

        REQUIRE(copy.empty() == source.empty());
        REQUIRE(copy.size() == source.size());
        REQUIRE(equal_entries(copy, source));
    }
}

TEST_CASE(
    "Stacktrace is printable.",
    "[print][stacktrace]")
{
    SECTION("Empty stacktraces have special treatment.")
    {
        const Stacktrace stacktrace{EmptyStacktraceBackend{}};
        REQUIRE_THAT(
            mimicpp::print(stacktrace),
            Catch::Matchers::Equals("empty"));
    }

#ifdef MIMICPP_DETAIL_WORKING_STACKTRACE_BACKEND

    Stacktrace stacktrace = current_stacktrace();
    CHECK(!stacktrace.empty());

    // the std::regex on windows is too complex, so we limit it
    constexpr std::size_t maxLength{6u};
    const auto size = std::min(maxLength, stacktrace.size());
    const auto skip = stacktrace.size() - size;
    stacktrace = current_stacktrace(skip);
    CHECK(size == stacktrace.size());

    const std::string pattern = format::format(
        "(.+?\\[\\d+\\], .+?\\n){{{}}}"
        ".+?\\[\\d+\\], .*?\\n", // on linux, the description of the lowest entry may be empty.
        size - 1u);
    REQUIRE_THAT(
        mimicpp::print(stacktrace),
        Catch::Matchers::Matches(pattern));

#endif
}
