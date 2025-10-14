//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Facade.hpp"
#include "mimic++/ScopedSequence.hpp"
#include "mimic++/macros/InterfaceMocking.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "SuppressionMacros.hpp"
#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

START_WARNING_SUPPRESSION
SUPPRESS_DEPRECATION

TEST_CASE(
    "MIMICPP_MOCK_OVERLOADED_METHOD creates mock and overloaded functions.",
    "[mock][mock::interface]")
{
    SECTION("Just void()")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual void foo() = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, ()));
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("Just void() const")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual void foo() const = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, (), const));
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("Just void() &")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual void foo() & = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, (), &));
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("Just void() const &")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual void foo() const& = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, (), const&));
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("Just void() &&")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual void foo() && = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, (), &&));
        };

        derived mock{};
        ScopedExpectation expectation = std::move(mock).foo_.expect_call();
        std::move(mock).foo();
    }

    SECTION("Just void() const &&")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual void foo() const&& = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, (), const&&));
        };

        derived mock{};
        ScopedExpectation expectation = std::move(mock).foo_.expect_call();
        std::move(mock).foo();
    }

    SECTION("Just void() const noexcept")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual void foo() const noexcept = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, (), (const noexcept)));
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("Just std::tuple<int, float>()")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual std::tuple<int, float> foo() = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD((std::tuple<int, float>), ()));
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call()
                                    and finally::returns(std::tuple<int, float>{});
        mock.foo();
    }

    SECTION("Just int(float&&) const noexcept")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual int foo(float&&) const noexcept = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(int, (float&&), const noexcept));
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call(4.2f)
                                    and finally::returns(42);
        REQUIRE(42 == mock.foo(4.2f));
    }

    SECTION("int(float&&) const noexcept and void()")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual int foo(float&&) const noexcept = 0;
            virtual void foo() = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, ()),
                MIMICPP_ADD_OVERLOAD(int, (float&&), const noexcept));
        };

        derived mock{};
        {
            ScopedExpectation expectation = mock.foo_.expect_call();
            mock.foo();
        }

        {
            ScopedExpectation expectation = mock.foo_.expect_call(4.2f)
                                        and finally::returns(42);
            REQUIRE(42 == mock.foo(4.2f));
        }
    }
}

TEST_CASE(
    "MIMICPP_MOCK_METHOD creates mock and overrides function.",
    "[mock][mock::interface]")
{
    SECTION("Just void()")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual void foo() = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_METHOD(foo, void, ());
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("Just int(float&&) const noexcept")
    {
        struct interface
        {
            virtual ~interface() = default;
            virtual int foo(float&&) const noexcept = 0;
        };

        struct derived
            : public interface
        {
            MIMICPP_MOCK_METHOD(foo, int, (float&&), const noexcept);
        };

        derived mock{};
        ScopedExpectation expectation = mock.foo_.expect_call(4.2f)
                                    and finally::returns(42);
        REQUIRE(42 == mock.foo(4.2f));
    }
}

namespace
{
    template <typename... Args>
    class VariadicInterface
    {
    public:
        virtual ~VariadicInterface() = default;
        virtual void foo(Args...) = 0;
    };

    template <typename... Args>
    class VariadicDerived final
        : public VariadicInterface<Args...>
    {
    public:
        MIMICPP_MOCK_METHOD(foo, void, (Args...));
    };
}

TEST_CASE(
    "MIMICPP_MOCK_METHOD supports variadic class-template arguments.",
    "[mock][mock::interface]")
{
    SECTION("Without template arguments.")
    {
        VariadicDerived mock{};

        ScopedExpectation expectation = mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("With single template argument.")
    {
        VariadicDerived<int> mock{};

        ScopedExpectation expectation = mock.foo_.expect_call(42);
        mock.foo(42);
    }

    SECTION("With multiple template arguments.")
    {
        VariadicDerived<int, std::string> mock{};

        ScopedExpectation expectation = mock.foo_.expect_call(42, "Hello, World!");
        mock.foo(42, "Hello, World!");
    }
}

TEST_CASE(
    "Interface mock omits the forwarding functions stacktrace entry.",
    "[mock][mock::interface]")
{
    struct interface
    {
        virtual ~interface() = default;
        virtual void foo() = 0;
    };

    struct derived
        : public interface
    {
        MIMICPP_MOCK_METHOD(foo, void, ());
    };

    ScopedReporter reporter{};

    derived mock{};
    ScopedExpectation const exp = mock.foo_.expect_call();
    [[maybe_unused]] util::SourceLocation constexpr before{};
    mock.foo();
    [[maybe_unused]] util::SourceLocation constexpr after{};

    REQUIRE_THAT(
        reporter.full_match_reports(),
        Catch::Matchers::SizeIs(1u));

    reporting::CallReport const& report = std::get<0>(reporter.full_match_reports().front());

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND
    CHECK_THAT(
        report.stacktrace.source_file(0u),
        Catch::Matchers::Equals(std::string{before.file_name()}));
    // there is no straight-forward way to check the description
    CHECK(before.line() < report.stacktrace.source_line(0u));
    // strict < fails on some compilers
    CHECK(report.stacktrace.source_line(0u) <= after.line());
#else
    REQUIRE(report.stacktrace.empty());
#endif
}

TEST_CASE(
    "Interface mock generates appropriate mock names.",
    "[mock][mock::interface]")
{
    struct interface
    {
        virtual ~interface() = default;
        virtual void foo() = 0;
    };

    struct derived
        : public interface
    {
        MIMICPP_MOCK_METHOD(foo, void, ());
    };

    derived mock{};
    const ScopedExpectation expectation = mock.foo_.expect_call()
                                      and expect::never();
    REQUIRE_THAT(
        expectation.mock_name(),
        Catch::Matchers::ContainsSubstring("derived")
            && Catch::Matchers::EndsWith("::foo")
            && Catch::Matchers::Matches(R"(.+derived::foo)"));
}

STOP_WARNING_SUPPRESSION
