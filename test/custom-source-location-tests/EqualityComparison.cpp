//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "CustomSourceLocation.hpp"

using namespace mimicpp;

namespace
{
    struct entry
    {
        std::string functionName{};
        std::string fileName{};
        std::size_t line{};
    };
}

TEST_CASE(
    "Two source-locations are member-wise compared.",
    "[util][util::source_location]")
{
    std::shared_ptr const firstInner = std::make_shared<CustomBackend::Inner>();
    util::SourceLocation const first{CustomBackend{firstInner}};
    std::shared_ptr const secondInner = std::make_shared<CustomBackend::Inner>();
    util::SourceLocation const second{CustomBackend{secondInner}};

    auto const [expected, firstEntry, secondEntry] = GENERATE(
        (table<bool, entry, entry>)({
            { true,  {"Description", "Source.cpp", 1337u},  {"Description", "Source.cpp", 1337u}},
            {false, {"Description1", "Source.cpp", 1337u}, {"Description2", "Source.cpp", 1337u}},
            {false, {"Description", "Source1.cpp", 1337u}, {"Description", "Source2.cpp", 1337u}},
            {false,    {"Description", "Source.cpp", 42u},  {"Description", "Source.cpp", 1337u}},
    }));

    REQUIRE_CALL(firstInner->functionNameMock, Invoke())
        .TIMES(AT_MOST(1u))
        .LR_RETURN(firstEntry.functionName);
    REQUIRE_CALL(firstInner->fileNameMock, Invoke())
        .TIMES(AT_MOST(1u))
        .LR_RETURN(firstEntry.fileName);
    REQUIRE_CALL(firstInner->lineMock, Invoke())
        .TIMES(AT_MOST(1u))
        .LR_RETURN(firstEntry.line);

    REQUIRE_CALL(secondInner->functionNameMock, Invoke())
        .TIMES(AT_MOST(1u))
        .LR_RETURN(secondEntry.functionName);
    REQUIRE_CALL(secondInner->fileNameMock, Invoke())
        .TIMES(AT_MOST(1u))
        .LR_RETURN(secondEntry.fileName);
    REQUIRE_CALL(secondInner->lineMock, Invoke())
        .TIMES(AT_MOST(1u))
        .LR_RETURN(secondEntry.line);

    SECTION("first == second")
    {
        CHECK(expected == (first == second));
    }

    SECTION("second == first")
    {
        CHECK(expected == (second == first));
    }

    SECTION("first != second")
    {
        CHECK_FALSE(expected == (first != second));
    }

    SECTION("second != first")
    {
        CHECK_FALSE(expected == (second != first));
    }
}
