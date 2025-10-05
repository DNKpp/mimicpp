//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Facade.hpp"
#include "mimic++/ScopedSequence.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

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
    "MIMICPP_DETAIL_MAKE_PARAM_LIST creates the param list for the given types.",
    "[mock][mock::interface]")
{
    namespace Matches = Catch::Matchers;

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST()),
        Matches::Equals(""));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST(int)),
        Matches::Equals("int arg_i"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST(const int&, int&&)),
        Matches::Matches("const int& arg_i\\s*, int&& arg_ii"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST(int, int)),
        Matches::Matches("int arg_i\\s*, int arg_ii"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST((std::tuple<int, float>))),
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
    "MIMICPP_MOCK_OVERLOADED_METHOD can handle at least up to 64 signatures.",
    "[mock][mock::interface]")
{
    struct interface
    {
        virtual ~interface() = default;

        virtual void foo() = 0;
        virtual void foo() const = 0;
        virtual void foo(int) = 0;
        virtual void foo(int) const = 0;
        virtual void foo(float) = 0;
        virtual void foo(float) const = 0;
        virtual void foo(double) = 0;
        virtual void foo(double) const = 0; // 8

        virtual void foo(short) = 0;
        virtual void foo(short) const = 0;
        virtual void foo(bool) = 0;
        virtual void foo(bool) const = 0;
        virtual void foo(unsigned) = 0;
        virtual void foo(unsigned) const = 0;
        virtual void foo(char) = 0;
        virtual void foo(char) const = 0; // 16

        virtual void foo(wchar_t) = 0;
        virtual void foo(wchar_t) const = 0;
        virtual void foo(wchar_t, int) = 0;
        virtual void foo(wchar_t, int) const = 0;
        virtual void foo(wchar_t, float) = 0;
        virtual void foo(wchar_t, float) const = 0;
        virtual void foo(wchar_t, double) = 0;
        virtual void foo(wchar_t, double) const = 0; // 24

        virtual void foo(wchar_t, short) = 0;
        virtual void foo(wchar_t, short) const = 0;
        virtual void foo(wchar_t, bool) = 0;
        virtual void foo(wchar_t, bool) const = 0;
        virtual void foo(wchar_t, unsigned) = 0;
        virtual void foo(wchar_t, unsigned) const = 0;
        virtual void foo(wchar_t, char) = 0;
        virtual void foo(wchar_t, char) const = 0; // 32

        virtual void foo(char16_t) = 0;
        virtual void foo(char16_t) const = 0;
        virtual void foo(char16_t, int) = 0;
        virtual void foo(char16_t, int) const = 0;
        virtual void foo(char16_t, float) = 0;
        virtual void foo(char16_t, float) const = 0;
        virtual void foo(char16_t, double) = 0;
        virtual void foo(char16_t, double) const = 0; // 40

        virtual void foo(char16_t, short) = 0;
        virtual void foo(char16_t, short) const = 0;
        virtual void foo(char16_t, bool) = 0;
        virtual void foo(char16_t, bool) const = 0;
        virtual void foo(char16_t, unsigned) = 0;
        virtual void foo(char16_t, unsigned) const = 0;
        virtual void foo(char16_t, char) = 0;
        virtual void foo(char16_t, char) const = 0; // 48

        virtual void foo(char32_t) = 0;
        virtual void foo(char32_t) const = 0;
        virtual void foo(char32_t, int) = 0;
        virtual void foo(char32_t, int) const = 0;
        virtual void foo(char32_t, float) = 0;
        virtual void foo(char32_t, float) const = 0;
        virtual void foo(char32_t, double) = 0;
        virtual void foo(char32_t, double) const = 0; // 56

        virtual void foo(char32_t, short) = 0;
        virtual void foo(char32_t, short) const = 0;
        virtual void foo(char32_t, bool) = 0;
        virtual void foo(char32_t, bool) const = 0;
        virtual void foo(char32_t, unsigned) = 0;
        virtual void foo(char32_t, unsigned) const = 0;
        virtual void foo(char32_t, char) = 0;
        virtual void foo(char32_t, char) const = 0; // 64
    };

    struct derived
        : public interface
    {
        MIMICPP_MOCK_OVERLOADED_METHOD(
            foo,
            MIMICPP_ADD_OVERLOAD(void, ()),
            MIMICPP_ADD_OVERLOAD(void, (), const),
            MIMICPP_ADD_OVERLOAD(void, (int)),
            MIMICPP_ADD_OVERLOAD(void, (int), const),
            MIMICPP_ADD_OVERLOAD(void, (float)),
            MIMICPP_ADD_OVERLOAD(void, (float), const),
            MIMICPP_ADD_OVERLOAD(void, (double)),
            MIMICPP_ADD_OVERLOAD(void, (double), const),
            MIMICPP_ADD_OVERLOAD(void, (short)),
            MIMICPP_ADD_OVERLOAD(void, (short), const),
            MIMICPP_ADD_OVERLOAD(void, (bool)),
            MIMICPP_ADD_OVERLOAD(void, (bool), const),
            MIMICPP_ADD_OVERLOAD(void, (unsigned)),
            MIMICPP_ADD_OVERLOAD(void, (unsigned), const),
            MIMICPP_ADD_OVERLOAD(void, (char)),
            MIMICPP_ADD_OVERLOAD(void, (char), const),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t)),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t), const),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, int)),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, int), const),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, float)),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, float), const),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, double)),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, double), const),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, short)),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, short), const),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, bool)),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, bool), const),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, unsigned)),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, unsigned), const),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, char)),
            MIMICPP_ADD_OVERLOAD(void, (wchar_t, char), const),
            MIMICPP_ADD_OVERLOAD(void, (char16_t)),
            MIMICPP_ADD_OVERLOAD(void, (char16_t), const),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, int)),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, int), const),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, float)),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, float), const),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, double)),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, double), const),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, short)),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, short), const),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, bool)),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, bool), const),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, unsigned)),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, unsigned), const),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, char)),
            MIMICPP_ADD_OVERLOAD(void, (char16_t, char), const),
            MIMICPP_ADD_OVERLOAD(void, (char32_t)),
            MIMICPP_ADD_OVERLOAD(void, (char32_t), const),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, int)),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, int), const),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, float)),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, float), const),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, double)),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, double), const),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, short)),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, short), const),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, bool)),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, bool), const),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, unsigned)),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, unsigned), const),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, char)),
            MIMICPP_ADD_OVERLOAD(void, (char32_t, char), const));
    };

    derived mock{};

    ScopedSequence sequence{};

    // Completed expectations for all 64 overloads
    sequence += mock.foo_.expect_call();
    sequence += std::as_const(mock).foo_.expect_call();
    sequence += mock.foo_.expect_call(matches::type<int>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<int>);
    sequence += mock.foo_.expect_call(matches::type<float>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<float>);
    sequence += mock.foo_.expect_call(matches::type<double>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<double>);
    sequence += mock.foo_.expect_call(matches::type<short>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<short>);
    sequence += mock.foo_.expect_call(matches::type<bool>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<bool>);
    sequence += mock.foo_.expect_call(matches::type<unsigned>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<unsigned>);
    sequence += mock.foo_.expect_call(matches::type<char>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char>);

    sequence += mock.foo_.expect_call(matches::type<wchar_t>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<wchar_t>);
    sequence += mock.foo_.expect_call(matches::type<wchar_t>, matches::type<int>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<wchar_t>, matches::type<int>);
    sequence += mock.foo_.expect_call(matches::type<wchar_t>, matches::type<float>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<wchar_t>, matches::type<float>);
    sequence += mock.foo_.expect_call(matches::type<wchar_t>, matches::type<double>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<wchar_t>, matches::type<double>);
    sequence += mock.foo_.expect_call(matches::type<wchar_t>, matches::type<short>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<wchar_t>, matches::type<short>);
    sequence += mock.foo_.expect_call(matches::type<wchar_t>, matches::type<bool>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<wchar_t>, matches::type<bool>);
    sequence += mock.foo_.expect_call(matches::type<wchar_t>, matches::type<unsigned>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<wchar_t>, matches::type<unsigned>);
    sequence += mock.foo_.expect_call(matches::type<wchar_t>, matches::type<char>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<wchar_t>, matches::type<char>);

    sequence += mock.foo_.expect_call(matches::type<char16_t>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char16_t>);
    sequence += mock.foo_.expect_call(matches::type<char16_t>, matches::type<int>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char16_t>, matches::type<int>);
    sequence += mock.foo_.expect_call(matches::type<char16_t>, matches::type<float>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char16_t>, matches::type<float>);
    sequence += mock.foo_.expect_call(matches::type<char16_t>, matches::type<double>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char16_t>, matches::type<double>);
    sequence += mock.foo_.expect_call(matches::type<char16_t>, matches::type<short>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char16_t>, matches::type<short>);
    sequence += mock.foo_.expect_call(matches::type<char16_t>, matches::type<bool>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char16_t>, matches::type<bool>);
    sequence += mock.foo_.expect_call(matches::type<char16_t>, matches::type<unsigned>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char16_t>, matches::type<unsigned>);
    sequence += mock.foo_.expect_call(matches::type<char16_t>, matches::type<char>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char16_t>, matches::type<char>);

    sequence += mock.foo_.expect_call(matches::type<char32_t>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char32_t>);
    sequence += mock.foo_.expect_call(matches::type<char32_t>, matches::type<int>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char32_t>, matches::type<int>);
    sequence += mock.foo_.expect_call(matches::type<char32_t>, matches::type<float>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char32_t>, matches::type<float>);
    sequence += mock.foo_.expect_call(matches::type<char32_t>, matches::type<double>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char32_t>, matches::type<double>);
    sequence += mock.foo_.expect_call(matches::type<char32_t>, matches::type<short>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char32_t>, matches::type<short>);
    sequence += mock.foo_.expect_call(matches::type<char32_t>, matches::type<bool>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char32_t>, matches::type<bool>);
    sequence += mock.foo_.expect_call(matches::type<char32_t>, matches::type<unsigned>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char32_t>, matches::type<unsigned>);
    sequence += mock.foo_.expect_call(matches::type<char32_t>, matches::type<char>);
    sequence += std::as_const(mock).foo_.expect_call(matches::type<char32_t>, matches::type<char>);

    mock.foo();
    std::as_const(mock).foo();
    mock.foo(42);
    std::as_const(mock).foo(42);
    mock.foo(4.2f);
    std::as_const(mock).foo(4.2f);
    mock.foo(4.2);
    std::as_const(mock).foo(4.2);
    mock.foo(static_cast<short>(7));
    std::as_const(mock).foo(static_cast<short>(7));
    mock.foo(true);
    std::as_const(mock).foo(true);
    mock.foo(7u);
    std::as_const(mock).foo(7u);
    mock.foo('a');
    std::as_const(mock).foo('a');

    mock.foo(L'x');
    std::as_const(mock).foo(L'x');
    mock.foo(L'x', 1);
    std::as_const(mock).foo(L'x', 1);
    mock.foo(L'x', 4.2f);
    std::as_const(mock).foo(L'x', 4.2f);
    mock.foo(L'x', 4.2);
    std::as_const(mock).foo(L'x', 4.2);
    mock.foo(L'x', static_cast<short>(2));
    std::as_const(mock).foo(L'x', static_cast<short>(2));
    mock.foo(L'x', true);
    std::as_const(mock).foo(L'x', true);
    mock.foo(L'x', 3u);
    std::as_const(mock).foo(L'x', 3u);
    mock.foo(L'x', 'c');
    std::as_const(mock).foo(L'x', 'c');

    mock.foo(u'x');
    std::as_const(mock).foo(u'x');
    mock.foo(u'x', 1);
    std::as_const(mock).foo(u'x', 1);
    mock.foo(u'x', 4.2f);
    std::as_const(mock).foo(u'x', 4.2f);
    mock.foo(u'x', 4.2);
    std::as_const(mock).foo(u'x', 4.2);
    mock.foo(u'x', static_cast<short>(2));
    std::as_const(mock).foo(u'x', static_cast<short>(2));
    mock.foo(u'x', true);
    std::as_const(mock).foo(u'x', true);
    mock.foo(u'x', 3u);
    std::as_const(mock).foo(u'x', 3u);
    mock.foo(u'x', 'c');
    std::as_const(mock).foo(u'x', 'c');

    mock.foo(U'x');
    std::as_const(mock).foo(U'x');
    mock.foo(U'x', 1);
    std::as_const(mock).foo(U'x', 1);
    mock.foo(U'x', 4.2f);
    std::as_const(mock).foo(U'x', 4.2f);
    mock.foo(U'x', 4.2);
    std::as_const(mock).foo(U'x', 4.2);
    mock.foo(U'x', static_cast<short>(2));
    std::as_const(mock).foo(U'x', static_cast<short>(2));
    mock.foo(U'x', true);
    std::as_const(mock).foo(U'x', true);
    mock.foo(U'x', 3u);
    std::as_const(mock).foo(U'x', 3u);
    mock.foo(U'x', 'c');
    std::as_const(mock).foo(U'x', 'c');
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

TEST_CASE(
    "MIMICPP_MOCK_OVERLOADED_METHOD_WITH_THIS generates a mock explicit *this* param.",
    "[mock][mock::interface]")
{
    struct interface
    {
        virtual ~interface() = default;
        virtual void foo() = 0;
        virtual void foo() const = 0;
    };

    struct derived
        : public interface
    {
        using self_type = derived;

        MIMICPP_MOCK_OVERLOADED_METHOD_WITH_THIS(
            foo,
            MIMICPP_ADD_OVERLOAD(void, ()),
            MIMICPP_ADD_OVERLOAD(void, (), const));
    };

    using Expected = Mock<void(derived*), void(derived const*) const>;
    STATIC_REQUIRE(std::same_as<Expected, decltype(derived::foo_)>);

    derived object{};

    {
        SCOPED_EXP object.foo_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo());
    }

    {
        SCOPED_EXP std::as_const(object).foo_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo());
    }
}

TEST_CASE(
    "MIMICPP_MOCK_METHOD_WITH_THIS generates a mock with explicit *this* param.",
    "[mock][mock::interface]")
{
    struct interface
    {
        virtual ~interface() = default;
        virtual void foo() = 0;
        virtual void foo_const() const = 0;

        virtual void foo_lvalue() & = 0;
        virtual void foo_lvalue_const() const& = 0;

        virtual void foo_rvalue() && = 0;
        virtual void foo_rvalue_const() const&& = 0;

        virtual void foo_noexcept() noexcept = 0;
        virtual void foo_const_noexcept() const noexcept = 0;

        virtual void foo_lvalue_noexcept() & noexcept = 0;
        virtual void foo_lvalue_const_noexcept() const& noexcept = 0;

        virtual void foo_rvalue_noexcept() && noexcept = 0;
        virtual void foo_rvalue_const_noexcept() const&& noexcept = 0;
    };

    struct derived
        : public interface
    {
        using self_type = derived;

        MIMICPP_MOCK_METHOD_WITH_THIS(foo, void, ());
        MIMICPP_MOCK_METHOD_WITH_THIS(foo_const, void, (), const);

        MIMICPP_MOCK_METHOD_WITH_THIS(foo_lvalue, void, (), &);
        MIMICPP_MOCK_METHOD_WITH_THIS(foo_lvalue_const, void, (), const&);

        MIMICPP_MOCK_METHOD_WITH_THIS(foo_rvalue, void, (), &&);
        MIMICPP_MOCK_METHOD_WITH_THIS(foo_rvalue_const, void, (), const&&);

        MIMICPP_MOCK_METHOD_WITH_THIS(foo_noexcept, void, (), noexcept);
        MIMICPP_MOCK_METHOD_WITH_THIS(foo_const_noexcept, void, (), const noexcept);

        MIMICPP_MOCK_METHOD_WITH_THIS(foo_lvalue_noexcept, void, (), & noexcept);
        MIMICPP_MOCK_METHOD_WITH_THIS(foo_lvalue_const_noexcept, void, (), const& noexcept);

        MIMICPP_MOCK_METHOD_WITH_THIS(foo_rvalue_noexcept, void, (), && noexcept);
        MIMICPP_MOCK_METHOD_WITH_THIS(foo_rvalue_const_noexcept, void, (), const&& noexcept);
    };

    derived object{};

    SECTION("Mocking an unqualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived*)>, decltype(derived::foo_)>);
        SCOPED_EXP object.foo_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo());
    }

    SECTION("Mocking const qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived const*) const>, decltype(derived::foo_const_)>);
        SCOPED_EXP std::as_const(object).foo_const_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo_const());
    }

    SECTION("Mocking lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived*)&>, decltype(derived::foo_lvalue_)>);
        SCOPED_EXP object.foo_lvalue_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo_lvalue());
    }

    SECTION("Mocking const lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived const*) const&>, decltype(derived::foo_lvalue_const_)>);
        SCOPED_EXP std::as_const(object).foo_lvalue_const_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo_lvalue_const());
    }

    SECTION("Mocking rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived*) &&>, decltype(derived::foo_rvalue_)>);
        SCOPED_EXP std::move(object).foo_rvalue_.expect_call(&object);
        REQUIRE_NOTHROW(std::move(object).foo_rvalue());
    }

    SECTION("Mocking const rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived const*) const&&>, decltype(derived::foo_rvalue_const_)>);
        SCOPED_EXP std::move(std::as_const(object)).foo_rvalue_const_.expect_call(&object);
        REQUIRE_NOTHROW(std::move(std::as_const(object)).foo_rvalue_const());
    }

    SECTION("Mocking a noexcept member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived*) noexcept>, decltype(derived::foo_noexcept_)>);
        SCOPED_EXP object.foo_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo_noexcept());
    }

    SECTION("Mocking noexcept const qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived const*) const noexcept>, decltype(derived::foo_const_noexcept_)>);
        SCOPED_EXP std::as_const(object).foo_const_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo_const_noexcept());
    }

    SECTION("Mocking noexcept lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived*) & noexcept>, decltype(derived::foo_lvalue_noexcept_)>);
        SCOPED_EXP object.foo_lvalue_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo_lvalue_noexcept());
    }

    SECTION("Mocking noexcept const lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived const*) const & noexcept>, decltype(derived::foo_lvalue_const_noexcept_)>);
        SCOPED_EXP std::as_const(object).foo_lvalue_const_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo_lvalue_const_noexcept());
    }

    SECTION("Mocking noexcept rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived*) && noexcept>, decltype(derived::foo_rvalue_noexcept_)>);
        SCOPED_EXP std::move(object).foo_rvalue_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(std::move(object).foo_rvalue_noexcept());
    }

    SECTION("Mocking noexcept const rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(derived const*) const && noexcept>, decltype(derived::foo_rvalue_const_noexcept_)>);
        SCOPED_EXP std::move(std::as_const(object)).foo_rvalue_const_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(std::move(std::as_const(object)).foo_rvalue_const_noexcept());
    }
}
