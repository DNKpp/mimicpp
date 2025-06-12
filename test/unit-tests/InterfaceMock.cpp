//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/InterfaceMock.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

#define TO_STRING_IMPL(...) #__VA_ARGS__
#define TO_STRING(...)      TO_STRING_IMPL(__VA_ARGS__)

TEST_CASE(
    "MIMICPP_DETAIL_STRIP_PARENS removes outer parens, if present.",
    "[mock][mock::interface]")
{
    namespace Matches = Catch::Matchers;

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_STRIP_PARENS()),
        Matches::Equals(""));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_STRIP_PARENS(())),
        Matches::Equals(""));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_STRIP_PARENS((()))),
        Matches::Equals("()"));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_STRIP_PARENS(Test())),
        Matches::Equals("Test()"));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_STRIP_PARENS((Test()))),
        Matches::Equals("Test()"));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_STRIP_PARENS(((Test())))),
        Matches::Equals("(Test())"));

    // clang-format off
    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_STRIP_PARENS(((,Test(),)))),
        Matches::Equals("(,Test(),)"));
    // clang-format on
}

TEST_CASE(
    "MIMICPP_DETAIL_MAKE_SIGNATURE_LIST creates a list of signatures from the given arguments.",
    "[mock][mock::interface]")
{
    STATIC_REQUIRE(
        std::same_as<
            std::tuple<>,
            std::tuple<MIMICPP_DETAIL_MAKE_SIGNATURE_LIST()>>);

    STATIC_REQUIRE(
        std::same_as<
            std::tuple<void()>,
            std::tuple<MIMICPP_DETAIL_MAKE_SIGNATURE_LIST((void, (), , ))>>);

    STATIC_REQUIRE(
        std::same_as<
            std::tuple<const int&(float&&) const noexcept, void()>,
            std::tuple<MIMICPP_DETAIL_MAKE_SIGNATURE_LIST(
                (const int&, (float&&), const noexcept, ),
                (void, (), , ))>>);
}

TEST_CASE(
    "MIMICPP_DETAIL_MAKE_OVERLOADED_MOCK creates a mock from a list of signatures.",
    "[mock][mock::interface]")
{
    SECTION("Just void()")
    {
        struct helper
        {
            MIMICPP_DETAIL_MAKE_OVERLOADED_MOCK(
                mock,
                test,
                (void()));
        };

        STATIC_REQUIRE(std::is_invocable_r_v<void, decltype(helper::mock)>);
    }

    SECTION("Just float&(int&&)")
    {
        struct helper
        {
            MIMICPP_DETAIL_MAKE_OVERLOADED_MOCK(
                mock,
                test,
                (float&(int&&)));
        };

        STATIC_REQUIRE(std::is_invocable_r_v<float&, decltype(helper::mock), int&&>);
    }
}

TEST_CASE(
    "MIMICPP_DETAIL_MAKE_PARAM_LIST creates the param list for the given types.",
    "[mock][mock::interface]")
{
    namespace Matches = Catch::Matchers;

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST()),
        Matches::Equals(""));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST(int)),
        Matches::Equals("int arg_i"));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST(const int&, int&&)),
        Matches::Matches("const int& arg_i\\s*, int&& arg_ii"));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST(int, int)),
        Matches::Matches("int arg_i\\s*, int arg_ii"));

    REQUIRE_THAT(
        TO_STRING(MIMICPP_DETAIL_MAKE_PARAM_LIST((std::tuple<int, float>))),
        Matches::Equals("std::tuple<int, float> arg_i"));
}

namespace
{
    struct single_pack_helper
    {
        template <typename... Args>
        [[nodiscard]]
        decltype(auto) operator()(Args&&... arg_i)
        {
            return MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE(i, , (Args...));
        }
    };
}

TEST_CASE(
    "MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE constructs a tuple from all kind of args (may be a parameter or a parameter pack).",
    "[mock][mock::interface]")
{
    SECTION("When a single parameter is given.")
    {
        int arg_i{};
        std::tuple t = MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE(i, , int);
        STATIC_REQUIRE(std::same_as<std::tuple<int&&>, decltype(t)>);

        REQUIRE(&arg_i == &std::get<0>(t));
    }

    SECTION("When an empty parameter-pack is given.")
    {
        std::tuple t = single_pack_helper{}();
        STATIC_REQUIRE(std::same_as<std::tuple<>, decltype(t)>);
    }

    SECTION("When a non-empty parameter-pack is given.")
    {
        std::string str{};
        int value{42};
        std::tuple t = single_pack_helper{}(str, std::move(value));
        STATIC_REQUIRE(std::same_as<std::tuple<std::string&, int&&>, decltype(t)>);

        REQUIRE(&str == &std::get<0>(t));
        REQUIRE(&value == &std::get<1>(t));
    }
}

TEST_CASE(
    "MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE returns a comma separated list of tuples.",
    "[mock][mock::interface]")
{
    SECTION("When no arguments are given.")
    {
        std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE());
        STATIC_REQUIRE(std::same_as<std::tuple<>, decltype(result)>);
    }

    SECTION("When single argument is given.")
    {
        // This has to be named exactly like this, because the macro internally expects that.
        int arg_i{};

        SECTION("By value")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<int&&>>, decltype(result)>);
            REQUIRE(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By const value")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(const int));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<const int&&>>, decltype(result)>);
            REQUIRE(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By lvalue-ref.")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int&));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<int&>>, decltype(result)>);
            REQUIRE(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By const lvalue-ref.")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(const int&));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<const int&>>, decltype(result)>);
            REQUIRE(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By rvalue-ref.")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int&&));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<int&&>>, decltype(result)>);
            REQUIRE(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By const rvalue-ref")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(const int&&));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<const int&&>>, decltype(result)>);
            REQUIRE(&arg_i == &std::get<0>(std::get<0>(result)));
        }
    }

    SECTION("When multiple arguments are given.")
    {
        // This has to be named exactly like this, because the macro internally expects that.
        int arg_i{};
        std::string arg_ii{};

        std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int, std::string&));
        STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<int&&>, std::tuple<std::string&>>, decltype(result)>);
        auto& [param0, param1] = result;
        REQUIRE(&arg_i == &std::get<0>(param0));
        REQUIRE(&arg_ii == &std::get<0>(param1));
    }

    SECTION("When parameter pack is given.")
    {
        SECTION("As only argument.")
        {
            const auto packHelper = [&]<typename... Args>(Args&&... arg_i) {
                return MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(Args...);
            };

            std::string str{};
            int value{42};

            std::tuple result = packHelper(str, std::move(value));
            STATIC_REQUIRE(std::same_as<std::tuple<std::string&, int&&>, decltype(result)>);
            REQUIRE(&str == &std::get<0>(result));
            REQUIRE(&value == &std::get<1>(result));
        }

        SECTION("As front argument.")
        {
            int arg_ii{};
            const auto packHelper = [&]<typename... Args>(Args&&... arg_i) {
                return std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(Args..., int&));
            };

            std::string str{};
            int value{42};

            std::tuple result = packHelper(str, std::move(value));
            STATIC_REQUIRE(
                std::same_as<
                    std::tuple<std::tuple<std::string&, int&&>, std::tuple<int&>>,
                    decltype(result)>);
            auto& [param0, param1] = result;
            REQUIRE(&str == &std::get<0>(param0));
            REQUIRE(&value == &std::get<1>(param0));
            REQUIRE(&arg_ii == &std::get<0>(param1));
        }

        SECTION("As back argument.")
        {
            int arg_i{};
            const auto packHelper = [&]<typename... Args>(Args&&... arg_ii) {
                return std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int&, Args...));
            };

            std::string str{};
            int value{42};

            std::tuple result = packHelper(str, std::move(value));
            STATIC_REQUIRE(
                std::same_as<
                    std::tuple<std::tuple<int&>, std::tuple<std::string&, int&&>>,
                    decltype(result)>);
            auto& [param0, param1] = result;
            REQUIRE(&str == &std::get<0>(param1));
            REQUIRE(&value == &std::get<1>(param1));
            REQUIRE(&arg_i == &std::get<0>(param0));
        }
    }
}

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
