//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/Stacktrace.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

#include "../SuppressionMacros.hpp"
#include "../TestTypes.hpp"

#include <ranges> // std::views::*

using namespace mimicpp;

#if MIMICPP_CONFIG_USE_CXX23_STACKTRACE

TEST_CASE(
    "std::stacktrace is the installed stacktrace-backend.",
    "[stacktrace]")
{
    STATIC_REQUIRE(std::same_as<std::stacktrace, stacktrace::InstalledBackend>);
    STATIC_REQUIRE(stacktrace::backend<stacktrace::InstalledBackend>);
}

#endif

#if MIMICPP_CONFIG_USE_CPPTRACE

TEST_CASE(
    "cpptrace::stacktrace is the installed stacktrace-backend.",
    "[stacktrace]")
{
    STATIC_REQUIRE(std::same_as<cpptrace::stacktrace, stacktrace::InstalledBackend>);
    STATIC_REQUIRE(stacktrace::backend<stacktrace::InstalledBackend>);
}

#endif

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "stacktrace::current retrieves the current stacktrace.",
    "[stacktrace]")
{
    util::SourceLocation constexpr before{};
    util::Stacktrace const cur = util::stacktrace::current();
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
    void compare_traces(std::size_t const fullSkip, std::size_t const otherSkip, util::Stacktrace const& full, util::Stacktrace const& other)
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
    util::Stacktrace const full = util::stacktrace::current();
    REQUIRE(!full.empty());

    SECTION("When skip == 0")
    {
        util::Stacktrace const other = util::stacktrace::current();
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
        util::Stacktrace const partial = util::stacktrace::current(1);
        CHECK(!partial.empty());
        CHECK(full.size() == partial.size() + 1u);

        compare_traces(1u, 0u, full, partial);
    }

    SECTION("When skip == 2.")
    {
        util::Stacktrace const partial = util::stacktrace::current(2);
        CHECK(!partial.empty());
        CHECK(full.size() == partial.size() + 2u);

        compare_traces(2u, 0u, full, partial);
    }

    SECTION("When skip is very high.")
    {
        util::Stacktrace const partial = util::stacktrace::current(1337);
        REQUIRE(partial.empty());
    }
}

TEST_CASE(
    "stacktrace::current supports setting a maximum depth.",
    "[stacktrace]")
{
    util::Stacktrace const full = util::stacktrace::current();
    REQUIRE(!full.empty());

    SECTION("When max == 0")
    {
        util::Stacktrace const other = util::stacktrace::current(0u, 0u);
        CHECK(other.empty());
    }

    SECTION("When max == 1.")
    {
        util::Stacktrace const partial = util::stacktrace::current(1u, 1u);
        CHECK(!partial.empty());
        CHECK(1u == partial.size());

        compare_traces(1u, 0u, full, partial);
    }

    SECTION("When max == 2.")
    {
        util::Stacktrace const partial = util::stacktrace::current(1u, 2u);
        CHECK(!partial.empty());
        CHECK(2u == partial.size());

        compare_traces(1u, 0u, full, partial);
    }

    SECTION("When max is very high.")
    {
        util::Stacktrace const partial = util::stacktrace::current(1u, 1337);
        REQUIRE(full.size() == partial.size() + 1u);
    }
}

namespace
{
    struct scoped_base_skip
    {
    public:
        ~scoped_base_skip()
        {
            settings::stacktrace_base_skip() = m_OriginalValue;
        }

        explicit scoped_base_skip(std::size_t const value)
            : m_OriginalValue{settings::stacktrace_base_skip().exchange(value)}
        {
        }

        scoped_base_skip(scoped_base_skip const&) = delete;
        scoped_base_skip& operator=(scoped_base_skip const&) = delete;
        scoped_base_skip(scoped_base_skip&&) = delete;
        scoped_base_skip& operator=(scoped_base_skip&&) = delete;

    private:
        std::size_t m_OriginalValue;
    };
}

TEST_CASE(
    "stacktrace::current acknowledges settings::stacktrace_base_skip.",
    "[stacktrace]")
{
    util::Stacktrace const full = util::stacktrace::current();
    REQUIRE(2u < full.size());

    scoped_base_skip const guard{1u};

    SECTION("When til::stacktrace::current() is used.")
    {
        util::Stacktrace const partial = util::stacktrace::current();
        REQUIRE(full.size() == partial.size() + 1u);
        CHECK(full.source_line(1u) == partial.source_line(0u));
        CHECK_THAT(
            partial.description(0u),
            Catch::Matchers::Equals(full.description(1u)));
        CHECK_THAT(
            partial.source_file(0u),
            Catch::Matchers::Equals(full.source_file(1u)));
    }

    SECTION("When til::stacktrace::current(skip) is used.")
    {
        util::Stacktrace const partial = util::stacktrace::current(0u);
        REQUIRE(full.size() == partial.size() + 1u);
        CHECK(full.source_line(1u) == partial.source_line(0u));
        CHECK_THAT(
            partial.description(0u),
            Catch::Matchers::Equals(full.description(1u)));
        CHECK_THAT(
            partial.source_file(0u),
            Catch::Matchers::Equals(full.source_file(1u)));
    }

    SECTION("When til::stacktrace::current(skip, max) is used.")
    {
        util::Stacktrace const partial = util::stacktrace::current(0u, 42u);
        REQUIRE(full.size() == partial.size() + 1u);
        CHECK(full.source_line(1u) == partial.source_line(0u));
        CHECK_THAT(
            partial.description(0u),
            Catch::Matchers::Equals(full.description(1u)));
        CHECK_THAT(
            partial.source_file(0u),
            Catch::Matchers::Equals(full.source_file(1u)));
    }
}

#endif

TEST_CASE(
    "stacktrace::backend_traits<stacktrace::NullBackend>::current() generates a new stacktrace::NullBackend.",
    "[stacktrace]")
{
    using Traits = util::stacktrace::backend_traits<util::stacktrace::NullBackend>;

    util::stacktrace::NullBackend const& stacktrace = GENERATE(
        Traits::current(42),
        Traits::current(1337, 42));

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
    bool equal_entries(util::Stacktrace const& original, util::Stacktrace const& test)
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
    util::Stacktrace const source{util::stacktrace::current(0)};

    SECTION("When copy-constructing.")
    {
        util::Stacktrace const copy{source};

        CHECK(copy.empty() == source.empty());
        CHECK(copy.size() == source.size());
        CHECK(equal_entries(source, copy));
    }

    SECTION("When copy-assigning.")
    {
        util::Stacktrace copy{util::stacktrace::current(0)};
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
    util::Stacktrace source{util::stacktrace::current(0)};
    util::Stacktrace const copy{source};

    SECTION("When move-constructing.")
    {
        util::Stacktrace const current{std::move(source)};

        CHECK(copy.empty() == current.empty());
        CHECK(copy.size() == current.size());
        CHECK(equal_entries(copy, current));
    }

    SECTION("When move-assigning.")
    {
        util::Stacktrace current{util::stacktrace::current(0)};
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
        util::Stacktrace const stacktrace{util::stacktrace::current(0u, 0u)};
        CHECK_THAT(
            mimicpp::print(stacktrace),
            Catch::Matchers::Equals("empty"));
    }

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

    util::Stacktrace stacktrace = util::stacktrace::current();
    REQUIRE(!stacktrace.empty());

    // the std::regex on windows is too complex, so we limit it
    std::size_t constexpr maxLength{6u};
    auto const size = std::min(maxLength, stacktrace.size());
    auto const skip = stacktrace.size() - size;
    stacktrace = util::stacktrace::current(skip);
    REQUIRE(size == stacktrace.size());

    std::string const pattern = format::format(
        R"((?:#\d+ )" // always starts with the entry index
        "`"
        R"((?:\/?)"                                // may begin with a /
        R"((?:(?:\d|\w|_|-|\+|\*|\.)+(?:\\|\/))*)" // arbitrary times `dir/`
        R"((?:\d|\w|_|-|\+|\*|\.)+)?)"             // file name; sometimes there is no file, so the whole path may be empty
        R"((?::\d+`, `.*`\n)){{{}}})",             // other stuff
        size);
    CHECK_THAT(
        mimicpp::print(stacktrace),
        Catch::Matchers::Matches(pattern));

#endif
}
