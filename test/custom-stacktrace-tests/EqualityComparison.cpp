//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "CustomStacktrace.hpp"

using namespace mimicpp;

using traits_t = util::stacktrace::backend_traits<CustomBackend>;

TEST_CASE(
    "Two stacktraces with different sizes always compare unequal.",
    "[stacktrace]")
{
    using trompeloeil::_;

    const std::shared_ptr firstInner = std::make_shared<CustomBackend::Inner>();
    const std::shared_ptr secondInner = std::make_shared<CustomBackend::Inner>();
    util::Stacktrace first{CustomBackend{firstInner}};
    util::Stacktrace second{CustomBackend{secondInner}};

    const auto [firstSize, secondSize] = GENERATE(
        (table<std::size_t, std::size_t>({
            { 0u,    1u},
            { 1u,    2u},
            {42u, 1337u}
    })));

    REQUIRE_CALL(firstInner->sizeMock, Invoke())
        .RETURN(firstSize);
    REQUIRE_CALL(secondInner->sizeMock, Invoke())
        .RETURN(secondSize);

    SECTION("first == second")
    {
        REQUIRE_FALSE(first == second);
    }

    SECTION("second == first")
    {
        REQUIRE_FALSE(first == second);
    }

    SECTION("first != second")
    {
        REQUIRE(first != second);
    }

    SECTION("second != first")
    {
        REQUIRE(second != first);
    }
}

namespace
{
    struct entry
    {
        std::string description{};
        std::string source{};
        std::size_t line{};
    };
}

TEST_CASE(
    "Two stacktraces with equal sizes are compared entry-wise",
    "[stacktrace]")
{
    using trompeloeil::_;

    const std::shared_ptr firstInner = std::make_shared<CustomBackend::Inner>();
    const std::shared_ptr secondInner = std::make_shared<CustomBackend::Inner>();
    util::Stacktrace first{CustomBackend{firstInner}};
    util::Stacktrace second{CustomBackend{secondInner}};

    SECTION("When both are empty.")
    {
        REQUIRE_CALL(firstInner->sizeMock, Invoke())
            .TIMES(AT_LEAST(1u))
            .RETURN(0u);
        REQUIRE_CALL(secondInner->sizeMock, Invoke())
            .TIMES(AT_LEAST(1u))
            .RETURN(0u);

        SECTION("first == second")
        {
            REQUIRE(first == second);
        }

        SECTION("second == first")
        {
            REQUIRE(first == second);
        }

        SECTION("first != second")
        {
            REQUIRE_FALSE(first != second);
        }

        SECTION("second != first")
        {
            REQUIRE_FALSE(second != first);
        }
    }

    SECTION("When both sides have size of 1.")
    {
        REQUIRE_CALL(firstInner->sizeMock, Invoke())
            .TIMES(AT_LEAST(1u))
            .RETURN(1u);
        REQUIRE_CALL(secondInner->sizeMock, Invoke())
            .TIMES(AT_LEAST(1u))
            .RETURN(1u);

        auto&& [expected, firstEntry, secondEntry] = GENERATE(
            (table<bool, entry, entry>)({
                { true,  {"Description", "Source.cpp", 1337u},  {"Description", "Source.cpp", 1337u}},
                {false, {"Description1", "Source.cpp", 1337u}, {"Description2", "Source.cpp", 1337u}},
                {false, {"Description", "Source1.cpp", 1337u}, {"Description", "Source2.cpp", 1337u}},
                {false,    {"Description", "Source.cpp", 42u},  {"Description", "Source.cpp", 1337u}},
        }));

        REQUIRE_CALL(firstInner->descriptionMock, Invoke(0u))
            .TIMES(AT_MOST(1u))
            .RETURN(firstEntry.description);
        REQUIRE_CALL(firstInner->sourceMock, Invoke(0u))
            .TIMES(AT_MOST(1u))
            .RETURN(firstEntry.source);
        REQUIRE_CALL(firstInner->lineMock, Invoke(0u))
            .TIMES(AT_MOST(1u))
            .RETURN(firstEntry.line);

        REQUIRE_CALL(secondInner->descriptionMock, Invoke(0u))
            .TIMES(AT_MOST(1u))
            .RETURN(secondEntry.description);
        REQUIRE_CALL(secondInner->sourceMock, Invoke(0u))
            .TIMES(AT_MOST(1u))
            .RETURN(secondEntry.source);
        REQUIRE_CALL(secondInner->lineMock, Invoke(0u))
            .TIMES(AT_MOST(1u))
            .RETURN(secondEntry.line);

        SECTION("first == second")
        {
            REQUIRE(expected == (first == second));
        }

        SECTION("second == first")
        {
            REQUIRE(expected == (first == second));
        }

        SECTION("first != second")
        {
            REQUIRE_FALSE(expected == (first != second));
        }

        SECTION("second != first")
        {
            REQUIRE_FALSE(expected == (second != first));
        }
    }

    SECTION("When both sides have size of 2.")
    {
        REQUIRE_CALL(firstInner->sizeMock, Invoke())
            .TIMES(AT_LEAST(1u))
            .RETURN(2u);
        REQUIRE_CALL(secondInner->sizeMock, Invoke())
            .TIMES(AT_LEAST(1u))
            .RETURN(2u);

        const entry topEntry{"TopDescription", "TopSource.cpp", 42u};
        REQUIRE_CALL(firstInner->descriptionMock, Invoke(0u))
            .RETURN(topEntry.description);
        REQUIRE_CALL(firstInner->sourceMock, Invoke(0u))
            .RETURN(topEntry.source);
        REQUIRE_CALL(firstInner->lineMock, Invoke(0u))
            .RETURN(topEntry.line);

        REQUIRE_CALL(secondInner->descriptionMock, Invoke(0u))
            .RETURN(topEntry.description);
        REQUIRE_CALL(secondInner->sourceMock, Invoke(0u))
            .RETURN(topEntry.source);
        REQUIRE_CALL(secondInner->lineMock, Invoke(0u))
            .RETURN(topEntry.line);

        auto&& [expected, firstEntry, secondEntry] = GENERATE(
            (table<bool, entry, entry>)({
                { true,  {"Description", "Source.cpp", 1337u},  {"Description", "Source.cpp", 1337u}},
                {false, {"Description1", "Source.cpp", 1337u}, {"Description2", "Source.cpp", 1337u}},
                {false, {"Description", "Source1.cpp", 1337u}, {"Description", "Source2.cpp", 1337u}},
                {false,    {"Description", "Source.cpp", 42u},  {"Description", "Source.cpp", 1337u}},
        }));

        REQUIRE_CALL(firstInner->descriptionMock, Invoke(1u))
            .TIMES(AT_MOST(1u))
            .RETURN(firstEntry.description);
        REQUIRE_CALL(firstInner->sourceMock, Invoke(1u))
            .TIMES(AT_MOST(1u))
            .RETURN(firstEntry.source);
        REQUIRE_CALL(firstInner->lineMock, Invoke(1u))
            .TIMES(AT_MOST(1u))
            .RETURN(firstEntry.line);

        REQUIRE_CALL(secondInner->descriptionMock, Invoke(1u))
            .TIMES(AT_MOST(1u))
            .RETURN(secondEntry.description);
        REQUIRE_CALL(secondInner->sourceMock, Invoke(1u))
            .TIMES(AT_MOST(1u))
            .RETURN(secondEntry.source);
        REQUIRE_CALL(secondInner->lineMock, Invoke(1u))
            .TIMES(AT_MOST(1u))
            .RETURN(secondEntry.line);

        SECTION("first == second")
        {
            REQUIRE(expected == (first == second));
        }

        SECTION("second == first")
        {
            REQUIRE(expected == (second == first));
        }

        SECTION("first != second")
        {
            REQUIRE_FALSE(expected == (first != second));
        }

        SECTION("second != first")
        {
            REQUIRE_FALSE(expected == (second != first));
        }
    }
}
