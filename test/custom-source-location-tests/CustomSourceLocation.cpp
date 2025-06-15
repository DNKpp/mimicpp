//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "CustomSourceLocation.hpp"

using namespace mimicpp;

TEST_CASE(
    "mimic++ prefers custom registrations, when present.",
    "[util][util::source_location]")
{
    STATIC_REQUIRE(std::same_as<CustomBackend, util::source_location::InstalledBackend>);

    using traits = util::source_location::backend_traits<CustomBackend>;

    std::shared_ptr const inner = std::make_shared<CustomBackend::Inner>();
    REQUIRE_CALL(traits::currentMock, Invoke())
        .LR_RETURN(CustomBackend{inner});
    util::SourceLocation const loc{};

    SECTION("Testing file-name.")
    {
        std::string const fileName = GENERATE("", "Test", " Hello, World! ");
        REQUIRE_CALL(inner->fileNameMock, Invoke())
            .LR_RETURN(fileName);

        CHECK_THAT(
            std::string{loc.file_name()},
            Catch::Matchers::Equals(fileName));
    }

    SECTION("Testing function-name.")
    {
        std::string const functionName = GENERATE("", "Test", " Hello, World! ");
        REQUIRE_CALL(inner->functionNameMock, Invoke())
            .LR_RETURN(functionName);

        CHECK_THAT(
            std::string{loc.function_name()},
            Catch::Matchers::Equals(functionName));
    }

    SECTION("Testing line.")
    {
        std::size_t const line = GENERATE(0u, 1u, 42u);
        REQUIRE_CALL(inner->lineMock, Invoke())
            .LR_RETURN(line);

        REQUIRE(line == loc.line());
    }
}
