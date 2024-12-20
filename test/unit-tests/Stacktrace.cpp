// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Stacktrace.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

#ifdef MIMICPP_CONFIG_USE_CPPTRACE

TEST_CASE(
    "stacktrace_traits<BackendT>::current() generates a new cpptrace::stacktrace.",
    "[cpptrace][stacktrace]")
{
    using BackendT = mimicpp::cpptrace::Backend;
    using traits_t = stacktrace_traits<BackendT>;

    const BackendT first = traits_t::current(0);
    const BackendT second = traits_t::current(0);

    REQUIRE_THAT(
        first.data().frames | std::views::drop(1),
        Catch::Matchers::RangeEquals(second.data().frames | std::views::drop(1)));
    REQUIRE(first.data().frames.front() != second.data().frames.front());
}

#endif

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
struct custom::find_stacktrace_backend<register_tag>
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

            REQUIRE(description == stacktrace.description(at));
        }

        SECTION("Testing source_file.")
        {
            const std::string sourceFile = GENERATE("", "Test", " Hello, World! ");
            REQUIRE_CALL(traits_t::sourceMock, Invoke(_, at))
                .RETURN(sourceFile);

            REQUIRE(sourceFile == stacktrace.source_file(at));
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
