//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Stacktrace.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

#include "SuppressionMacros.hpp"
#include "TestTypes.hpp"

#include <ranges> // std::views::*

using namespace mimicpp;

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND
    #ifdef MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE

TEST_CASE(
    "stacktrace::backend_traits<stacktrace::CpptraceBackend>::current() generates a new cpptrace::stacktrace.",
    "[cpptrace][stacktrace]")
{
    using BackendT = stacktrace::CpptraceBackend;
    using traits_t = stacktrace::backend_traits<BackendT>;

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

    #elif defined(__cpp_lib_stacktrace)

TEST_CASE(
    "stacktrace::backend_traits<std::stacktrace>::current() generates a new std::stacktrace.",
    "[stacktrace]")
{
    using BackendT = std::stacktrace;
    using traits_t = stacktrace::backend_traits<BackendT>;

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
#endif

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "stacktrace::current retrieves the current stacktrace.",
    "[stacktrace]")
{
    util::SourceLocation constexpr before{};
    Stacktrace const cur = stacktrace::current();
    REQUIRE(!cur.empty());
    util::SourceLocation constexpr after{};

    CHECK_THAT(
        cur.source_file(0u),
        Catch::Matchers::Equals(std::string{before.file_name()}));
    std::size_t const line = cur.source_line(0u);
    CHECK(before.line() < line);
    CHECK(line < after.line());
}

namespace
{
    void compare_traces(std::size_t const fullSkip, std::size_t const otherSkip, Stacktrace const& full, Stacktrace const& other)
    {
        CAPTURE(fullSkip, otherSkip);
        REQUIRE(fullSkip < full.size());
        REQUIRE(otherSkip < other.size());
        REQUIRE(other.size() - otherSkip <= full.size() - fullSkip);

        for (auto const i : std::views::iota(0u, other.size() - otherSkip))
        {
            CAPTURE(i);
            CHECK_THAT(
                full.description(i + fullSkip),
                Catch::Matchers::Equals(other.description(i + otherSkip)));
            CHECK_THAT(
                full.source_file(i + fullSkip),
                Catch::Matchers::Equals(other.source_file(i + otherSkip)));
            CHECK(full.source_line(i + fullSkip) == other.source_line(i + otherSkip));
        }
    }
}

TEST_CASE(
    "stacktrace::current supports skipping of the top elements.",
    "[stacktrace]")
{
    Stacktrace const full = stacktrace::current();
    REQUIRE(!full.empty());

    SECTION("When skip == 0")
    {
        Stacktrace const other = stacktrace::current();
        CHECK(full.size() == other.size());

        // everything except the top element must be equal
        compare_traces(1u, 1u, full, other);
        // description of the top elements description may differ
        CHECK_THAT(
            other.source_file(0),
            Catch::Matchers::Equals(full.source_file(0)));
        CHECK(full.source_line(0) < other.source_line(0));
    }

    SECTION("When skip == 1.")
    {
        Stacktrace const partial = stacktrace::current(1);
        CHECK(!partial.empty());
        CHECK(full.size() == partial.size() + 1u);

        compare_traces(1u, 0u, full, partial);
    }

    SECTION("When skip == 2.")
    {
        Stacktrace const partial = stacktrace::current(2);
        CHECK(!partial.empty());
        CHECK(full.size() == partial.size() + 2u);

        compare_traces(2u, 0u, full, partial);
    }

    SECTION("When skip is very high.")
    {
        Stacktrace const partial = stacktrace::current(1337);
        REQUIRE(partial.empty());
    }
}

TEST_CASE(
    "stacktrace::current supports setting a maximum depth.",
    "[stacktrace]")
{
    Stacktrace const full = stacktrace::current();
    REQUIRE(!full.empty());

    SECTION("When max == 0")
    {
        Stacktrace const other = stacktrace::current(0u, 0u);
        CHECK(other.empty());
    }

    SECTION("When max == 1.")
    {
        Stacktrace const partial = stacktrace::current(1u, 1u);
        CHECK(!partial.empty());
        CHECK(1u == partial.size());

        compare_traces(1u, 0u, full, partial);
    }

    SECTION("When max == 2.")
    {
        Stacktrace const partial = stacktrace::current(1u, 2u);
        CHECK(!partial.empty());
        CHECK(2u == partial.size());

        compare_traces(1u, 0u, full, partial);
    }

    SECTION("When max is very high.")
    {
        Stacktrace const partial = stacktrace::current(1u, 1337);
        REQUIRE(full.size() == partial.size() + 1u);
    }
}

#endif

TEST_CASE(
    "stacktrace::backend_traits<stacktrace::NullBackend>::current() generates a new stacktrace::NullBackend.",
    "[stacktrace]")
{
    using Traits = stacktrace::backend_traits<stacktrace::NullBackend>;

    stacktrace::NullBackend constexpr stacktrace{Traits::current(42)};

    CHECK(Traits::empty(stacktrace));
    CHECK(0u == Traits::size(stacktrace));

    std::size_t const index = GENERATE(0, 1, 42);
    CHECK_THROWS(Traits::description(stacktrace, index));
    CHECK_THROWS(Traits::source_file(stacktrace, index));
    CHECK_THROWS(Traits::source_line(stacktrace, index));
}

namespace
{
    [[nodiscard]]
    bool equal_entries(Stacktrace const& original, Stacktrace const& test)
    {
#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND
        return std::ranges::all_of(
            std::views::iota(0u, original.size()),
            [&](std::size_t const index) {
                return original.description(index) == test.description(index)
                    && original.source_file(index) == test.source_file(index)
                    && original.source_line(index) == test.source_line(index);
            });
#else
        // we have an EmptyStacktraceBackend
        std::size_t const index = GENERATE(0, 1, 42);
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
    Stacktrace const source{stacktrace::current(0)};

    SECTION("When copy-constructing.")
    {
        Stacktrace const copy{source};

        CHECK(copy.empty() == source.empty());
        CHECK(copy.size() == source.size());
        CHECK(equal_entries(source, copy));
    }

    SECTION("When copy-assigning.")
    {
        Stacktrace copy{stacktrace::current(0)};
        copy = source;

        CHECK(copy.empty() == source.empty());
        CHECK(copy.size() == source.size());
        CHECK(equal_entries(source, copy));
    }
}

TEST_CASE(
    "Stacktrace is movable.",
    "[stacktrace]")
{
    Stacktrace source{stacktrace::current(0)};
    Stacktrace const copy{source};

    SECTION("When move-constructing.")
    {
        Stacktrace const current{std::move(source)};

        CHECK(copy.empty() == current.empty());
        CHECK(copy.size() == current.size());
        CHECK(equal_entries(copy, current));
    }

    SECTION("When move-assigning.")
    {
        Stacktrace current{stacktrace::current(0)};
        current = std::move(source);

        CHECK(copy.empty() == current.empty());
        CHECK(copy.size() == current.size());
        CHECK(equal_entries(copy, current));
    }

    SECTION("When self move-assigning.")
    {
        START_WARNING_SUPPRESSION
        SUPPRESS_SELF_MOVE
        source = std::move(source);
        STOP_WARNING_SUPPRESSION

        CHECK(copy.empty() == source.empty());
        CHECK(copy.size() == source.size());
        CHECK(equal_entries(copy, source));
    }
}

TEST_CASE(
    "Stacktrace is printable.",
    "[print][stacktrace]")
{
    SECTION("Empty stacktraces have special treatment.")
    {
        Stacktrace const stacktrace{stacktrace::current(0u, 0u)};
        CHECK_THAT(
            mimicpp::print(stacktrace),
            Catch::Matchers::Equals("empty"));
    }

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

    Stacktrace stacktrace = stacktrace::current();
    REQUIRE(!stacktrace.empty());

    // the std::regex on windows is too complex, so we limit it
    std::size_t constexpr maxLength{6u};
    auto const size = std::min(maxLength, stacktrace.size());
    auto const skip = stacktrace.size() - size;
    stacktrace = stacktrace::current(skip);
    REQUIRE(size == stacktrace.size());

    std::string const pattern = format::format(
        R"((?:#\d+ )"                              // always starts with the entry index
        "`"
        R"((?:\/?)"                                // may begin with a /
        R"((?:(?:\d|\w|_|-|\+|\*|\.)+(?:\\|\/))*)" // arbitrary times `dir/`
        R"((?:\d|\w|_|-|\+|\*|\.)+)?)"             // file name; sometimes there is no file, so the whole path may be empty
        "`"
        R"((?:#L\d+, `.*`\n)){{{}}})", // other stuff
        size);
    CHECK_THAT(
        mimicpp::print(stacktrace),
        Catch::Matchers::Matches(pattern));

#endif
}
