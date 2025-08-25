//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "CustomStacktrace.hpp"

using namespace mimicpp;

TEST_CASE(
    "current_stacktrace prefers custom registrations, when present.",
    "[stacktrace]")
{
    using trompeloeil::_;
    using trompeloeil::gt;

    using traits = stacktrace::backend_traits<CustomBackend>;

    std::shared_ptr const inner = std::make_shared<CustomBackend::Inner>();
    REQUIRE_CALL(traits::currentMock, Invoke(_))
        .WITH(_1 > 42)
        .LR_RETURN(CustomBackend{inner});
    Stacktrace const stacktrace = stacktrace::current(42);

    SECTION("Testing empty.")
    {
        bool const empty = GENERATE(true, false);
        REQUIRE_CALL(inner->emptyMock, Invoke())
            .RETURN(empty);

        REQUIRE(empty == stacktrace.empty());
    }

    SECTION("Testing size.")
    {
        std::size_t const size = GENERATE(0u, 1u, 42u);
        REQUIRE_CALL(inner->sizeMock, Invoke())
            .RETURN(size);

        REQUIRE(size == stacktrace.size());
    }

    SECTION("Testing backend entries.")
    {
        std::size_t const at = GENERATE(0, 1, 42);

        SECTION("Testing description.")
        {
            std::string const description = GENERATE("", "Test", " Hello, World! ");
            REQUIRE_CALL(inner->descriptionMock, Invoke(at))
                .RETURN(description);

            REQUIRE_THAT(
                stacktrace.description(at),
                Catch::Matchers::Equals(description));
        }

        SECTION("Testing source_file.")
        {
            std::string const sourceFile = GENERATE("", "Test", " Hello, World! ");
            REQUIRE_CALL(inner->sourceMock, Invoke(at))
                .RETURN(sourceFile);

            REQUIRE_THAT(
                stacktrace.source_file(at),
                Catch::Matchers::Equals(sourceFile));
        }

        SECTION("Testing line.")
        {
            std::size_t const line = GENERATE(0u, 1u, 42u);
            REQUIRE_CALL(inner->lineMock, Invoke(at))
                .RETURN(line);

            REQUIRE(line == stacktrace.source_line(at));
        }
    }
}
