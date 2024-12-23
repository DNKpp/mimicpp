// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Stacktrace.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include <catch2/trompeloeil.hpp>

#include "../unit-tests/TestTypes.hpp"

using namespace mimicpp;

namespace
{
    class CustomBackend
    {
    };
}

struct custom::find_stacktrace_backend
{
    using type = CustomBackend;
};

template <>
struct mimicpp::stacktrace_traits<CustomBackend>
{
    using BackendT = CustomBackend;

    inline static InvocableMock<CustomBackend, std::size_t> currentMock{};

    [[nodiscard]]
    static BackendT current(const std::size_t skip)
    {
        return currentMock.Invoke(skip);
    }

    inline static InvocableMock<std::size_t, const BackendT&> sizeMock{};

    [[nodiscard]]
    static std::size_t size(const std::any& storage)
    {
        return sizeMock.Invoke(get(storage));
    }

    inline static InvocableMock<bool, const BackendT&> emptyMock{};

    [[nodiscard]]
    static bool empty(const std::any& storage)
    {
        return emptyMock.Invoke(get(storage));
    }

    inline static InvocableMock<std::string, const BackendT&, std::size_t> descriptionMock{};

    [[nodiscard]]
    static std::string description(const std::any& storage, const std::size_t at)
    {
        return descriptionMock.Invoke(get(storage), at);
    }

    inline static InvocableMock<std::string, const BackendT&, std::size_t> sourceMock{};

    [[nodiscard]]
    static std::string source_file(const std::any& storage, const std::size_t at)
    {
        return sourceMock.Invoke(get(storage), at);
    }

    inline static InvocableMock<std::size_t, const BackendT&, std::size_t> lineMock{};

    [[nodiscard]]
    static std::size_t source_line(const std::any& storage, const std::size_t at)
    {
        return lineMock.Invoke(get(storage), at);
    }

    [[nodiscard]]
    static const BackendT& get(const std::any& storage)
    {
        return std::any_cast<const BackendT&>(storage);
    }
};

TEST_CASE(
    "current_stacktrace prefers custom registrations, when present.",
    "[stacktrace]")
{
    using trompeloeil::_;
    using trompeloeil::gt;

    using traits_t = stacktrace_traits<CustomBackend>;

    REQUIRE_CALL(traits_t::currentMock, Invoke(gt(42)))
        .RETURN(CustomBackend{});
    const Stacktrace stacktrace = current_stacktrace(42);

    SECTION("Testing empty.")
    {
        const bool empty = GENERATE(true, false);
        REQUIRE_CALL(traits_t::emptyMock, Invoke(_))
            .RETURN(empty);

        REQUIRE(empty == stacktrace.empty());
    }

    SECTION("Testing size.")
    {
        const std::size_t size = GENERATE(0u, 1u, 42u);
        REQUIRE_CALL(traits_t::sizeMock, Invoke(_))
            .RETURN(size);

        REQUIRE(size == stacktrace.size());
    }

    SECTION("Testing backend entries.")
    {
        const std::size_t at = GENERATE(0, 1, 42);

        SECTION("Testing description.")
        {
            const std::string description = GENERATE("", "Test", " Hello, World! ");
            REQUIRE_CALL(traits_t::descriptionMock, Invoke(_, at))
                .RETURN(description);

            REQUIRE_THAT(
                stacktrace.description(at),
                Catch::Matchers::Equals(description));
        }

        SECTION("Testing source_file.")
        {
            const std::string sourceFile = GENERATE("", "Test", " Hello, World! ");
            REQUIRE_CALL(traits_t::sourceMock, Invoke(_, at))
                .RETURN(sourceFile);

            REQUIRE_THAT(
                stacktrace.source_file(at),
                Catch::Matchers::Equals(sourceFile));
        }

        SECTION("Testing line.")
        {
            const std::size_t line = GENERATE(0u, 1u, 42u);
            REQUIRE_CALL(traits_t::lineMock, Invoke(_, at))
                .RETURN(line);

            REQUIRE(line == stacktrace.source_line(at));
        }
    }
}

TEST_CASE(
    "Stacktrace is printable.",
    "[stacktrace]")
{
    using trompeloeil::_;
    using traits_t = stacktrace_traits<CustomBackend>;
    const Stacktrace stacktrace{CustomBackend{}};

    SECTION("When stacktrace is empty")
    {
        REQUIRE_CALL(traits_t::emptyMock, Invoke(_))
            .RETURN(true);

        const auto text = mimicpp::print(stacktrace);
        REQUIRE_THAT(
            text,
            Catch::Matchers::Equals("empty"));
    }

    SECTION("When stacktrace contains one entry.")
    {
        REQUIRE_CALL(traits_t::emptyMock, Invoke(_))
            .RETURN(false);
        REQUIRE_CALL(traits_t::sizeMock, Invoke(_))
            .RETURN(1u);
        REQUIRE_CALL(traits_t::sourceMock, Invoke(_, 0u))
            .RETURN("test.cpp");
        REQUIRE_CALL(traits_t::lineMock, Invoke(_, 0u))
            .RETURN(1337u);
        REQUIRE_CALL(traits_t::descriptionMock, Invoke(_, 0u))
            .RETURN("Hello, World!");

        const auto text = mimicpp::print(stacktrace);
        REQUIRE_THAT(
            text,
            Catch::Matchers::Equals("test.cpp [1337], Hello, World!\n"));
    }

    SECTION("When stacktrace contains multiple entries.")
    {
        REQUIRE_CALL(traits_t::emptyMock, Invoke(_))
            .RETURN(false);
        REQUIRE_CALL(traits_t::sizeMock, Invoke(_))
            .RETURN(2u);
        REQUIRE_CALL(traits_t::sourceMock, Invoke(_, 0u))
            .RETURN("other-test.cpp");
        REQUIRE_CALL(traits_t::lineMock, Invoke(_, 0u))
            .RETURN(42u);
        REQUIRE_CALL(traits_t::descriptionMock, Invoke(_, 0u))
            .RETURN("Hello, mimic++!");
        REQUIRE_CALL(traits_t::sourceMock, Invoke(_, 1u))
            .RETURN("test.cpp");
        REQUIRE_CALL(traits_t::lineMock, Invoke(_, 1u))
            .RETURN(1337u);
        REQUIRE_CALL(traits_t::descriptionMock, Invoke(_, 1u))
            .RETURN("Hello, World!");

        const auto text = mimicpp::print(stacktrace);
        REQUIRE_THAT(
            text,
            Catch::Matchers::Equals(
                "other-test.cpp [42], Hello, mimic++!\n"
                "test.cpp [1337], Hello, World!\n"));
    }
}
