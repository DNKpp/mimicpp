//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/BasicReporter.hpp"
#include "mimic++/Mock.hpp"
#include "mimic++/reporting/GlobalReporter.hpp"

using namespace mimicpp;

namespace
{
    struct saturated
    {
    };

    class TestException
    {
    };

    inline std::optional<StringT> g_SuccessMessage{};

    void send_success(const StringViewT msg)
    {
        if (g_SuccessMessage.has_value())
        {
            throw saturated{};
        }

        g_SuccessMessage.emplace(msg);
    }

    inline std::optional<StringT> g_WarningMessage{};

    void send_warning(const StringViewT msg)
    {
        if (g_WarningMessage.has_value())
        {
            throw saturated{};
        }

        g_WarningMessage.emplace(msg);
    }

    inline std::optional<StringT> g_FailMessage{};

    [[noreturn]]
    void send_fail(const StringViewT msg)
    {
        if (g_FailMessage.has_value())
        {
            throw saturated{};
        }

        g_FailMessage.emplace(msg);
        throw TestException{};
    }

    class ScopedReporter
    {
    public:
        ~ScopedReporter() noexcept
        {
            reporting::install_reporter<reporting::DefaultReporter>();
        }

        ScopedReporter() noexcept
        {
            reporting::install_reporter<
                reporting::BasicReporter<
                    &send_success,
                    &send_warning,
                    &send_fail>>();
        }
    };

    class StdException
        : public std::runtime_error
    {
    public:
        StdException()
            : std::runtime_error{"An std::exception type."}
        {
        }
    };

    template <typename Exception>
    class ThrowOnMatches
    {
    public:
        [[maybe_unused]]
        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        [[maybe_unused]]
        static bool matches([[maybe_unused]] const auto& info)
        {
            throw Exception{};
        }

        [[maybe_unused]]
        static constexpr std::nullopt_t describe() noexcept
        {
            return std::nullopt;
        }

        [[maybe_unused]]
        static constexpr void consume([[maybe_unused]] const auto& info) noexcept
        {
        }
    };
}

TEST_CASE(
    "BasicReporter forwards messages to the installed callbacks.",
    "[report]")
{
    namespace matches = Catch::Matchers;

    const ScopedReporter reporter{};
    g_SuccessMessage.reset();
    g_WarningMessage.reset();
    g_FailMessage.reset();

    SECTION("When success is reported.")
    {
        Mock<void(int)> mock{};

        SCOPED_EXP mock.expect_call(42);

        mock(42);
        REQUIRE_THAT(
            g_SuccessMessage.value(),
            matches::StartsWith("Found match for"));
    }

    SECTION("Sends fail, when no match can be found.")
    {
        Mock<void(int)> mock{};

        SECTION("When no expectation exists.")
        {
            REQUIRE_THROWS_AS(
                mock(1337),
                TestException);

            REQUIRE_THAT(
                g_FailMessage.value(),
                matches::StartsWith("No match for")
                    && matches::ContainsSubstring("No expectations available."));
        }

        SECTION("When non matching expectation exists.")
        {
            SCOPED_EXP mock.expect_call(42)
                && expect::at_most(1); // prevent unfulfilled reporting

            REQUIRE_THROWS_AS(
                mock(1337),
                TestException);

            REQUIRE_THAT(
                g_FailMessage.value(),
                matches::StartsWith("No match for")
                    && matches::ContainsSubstring("1 available expectation(s):"));
        }
    }

    SECTION("Sends fail, when no applicable match can be found.")
    {
        Mock<void(int)> mock{};

        SCOPED_EXP mock.expect_call(42);

        mock(42);
        REQUIRE_THROWS_AS(
            mock(42),
            TestException);

        REQUIRE_THAT(
            g_FailMessage.value(),
            matches::StartsWith("No applicable match for")
                && matches::ContainsSubstring("Tested expectations:"));
    }

    SECTION("Sends fail, when unfulfilled expectation is reported.")
    {
        constexpr auto action = [] {
            Mock<void()> mock{};
            SCOPED_EXP mock.expect_call();
        };

        REQUIRE_THROWS_AS(
            action(),
            TestException);

        REQUIRE_THAT(
            g_FailMessage.value(),
            matches::StartsWith("Unfulfilled expectation:"));
    }

    SECTION("Does not send fail, when unfulfilled expectation is reported, but an other exception already exists.")
    {
        constexpr auto action = [] {
            Mock<void()> mock{};
            SCOPED_EXP mock.expect_call();
            throw 42;
        };

        REQUIRE_THROWS_AS(
            action(),
            int);

        REQUIRE(!g_FailMessage);
    }

    SECTION("Sends fail, when generic error is reported.")
    {
        REQUIRE_THROWS_AS(
            reporting::detail::report_error("Hello, World!"),
            TestException);

        REQUIRE_THAT(
            g_FailMessage.value(),
            matches::Equals("Hello, World!"));
    }

    SECTION("Does not send fail, when generic error is reported, but an other exception already exists.")
    {
        struct helper
        {
            ~helper()
            {
                reporting::detail::report_error("Hello, World!");
            }
        };

        const auto action = [] {
            helper h{};
            throw 42;
        };

        REQUIRE_THROWS_AS(
            action(),
            int);

        REQUIRE(!g_FailMessage);
    }

    SECTION("Sends warning, when unhandled exception is reported.")
    {
        Mock<void()> mock{};
        SCOPED_EXP mock.expect_call(); // this will actually consume the call

        SECTION("When a std::exception is thrown.")
        {
            SCOPED_EXP mock.expect_call()
                && expect::at_most(1)
                && ThrowOnMatches<StdException>{};

            REQUIRE_NOTHROW(mock());
            REQUIRE_THAT(
                g_WarningMessage.value(),
                matches::StartsWith("Unhandled exception: what: An std::exception type."));
        }

        SECTION("When an unknown exception is thrown.")
        {
            SCOPED_EXP mock.expect_call()
                && expect::at_most(1)
                && ThrowOnMatches<TestException>{};

            REQUIRE_NOTHROW(mock());
            REQUIRE_THAT(
                g_WarningMessage.value(),
                matches::StartsWith("Unhandled exception: Unknown exception type."));
        }
    }
}
