// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/InterfaceMock.hpp"
#include "mimic++/Mock.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include <catch2/catch_test_macros.hpp>

namespace
{
    //! [variadic mock def]
    template <typename... Args>
    class VariadicClass
    {
    public:
        mimicpp::Mock<void(Args...)> myMock{};
    };

    //! [variadic mock def]
}

TEST_CASE(
    "Mock supports variadic template args.",
    "[example][example::mock]")
{
    //! [variadic mock]
    VariadicClass myClass{};
    SCOPED_EXP myClass.myMock.expect_call();
    myClass.myMock();

    VariadicClass<const std::string&, int> myOtherClass{};
    SCOPED_EXP myOtherClass.myMock.expect_call("Hello, World!", 1337);
    myOtherClass.myMock("Hello, World!", 1337);
    //! [variadic mock]
}

namespace
{
    //! [variadic interface def]
    template <typename... Args>
    class VariadicInterface
    {
    public:
        virtual ~VariadicInterface() = default;
        virtual void foo(Args...) = 0;
        virtual int bar(int, Args...) = 0;
        virtual std::string bar(Args..., int, Args...) const = 0; // you can go absolutely crazy!
    };

    template <typename... Args>
    class VariadicDerived final
        : public VariadicInterface<Args...>
    {
    public:
        MOCK_METHOD(foo, void, (Args...));
        MOCK_OVERLOADED_METHOD(
            bar,
            ADD_OVERLOAD(int, (int, Args...)),
            ADD_OVERLOAD(std::string, (Args..., int, Args...), const));
    };

    //! [variadic interface def]
}

TEST_CASE(
    "MOCK_METHOD and MOCK_OVERLOADED_METHOD do support variadic templates, too.",
    "[example][example::mock][example::mock::interface]")
{
    namespace finally = mimicpp::finally;
    namespace matches = mimicpp::matches;

    SECTION("Without template arguments.")
    {
        //! [variadic interface zero]
        VariadicDerived mock{};

        SCOPED_EXP mock.foo_.expect_call(); // foo has no arguments
        mock.foo();

        SCOPED_EXP mock.bar_.expect_call(42)                    // both bars have just an int parameter. This will select the non-const
            and finally::returns(1337);                         // version, which returns an int.
        SCOPED_EXP std::as_const(mock).bar_.expect_call(42)     // to refer to the const version, just use a const ref
            and finally::returns(std::string{"Hello, World!"}); // this returns a std::string

        REQUIRE("Hello, World!" == std::as_const(mock).bar(42));
        REQUIRE(1337 == mock.bar(42));
        //! [variadic interface zero]
    }

    SECTION("With multiple template arguments.")
    {
        //! [variadic interface 2]
        VariadicDerived<int, std::string> mock{};

        SCOPED_EXP mock.foo_.expect_call(1337, matches::range::has_size(3));
        mock.foo(1337, "Hey"); // second param size of 3 (without null-terminator).
        //! [variadic interface 2]
    }
}
