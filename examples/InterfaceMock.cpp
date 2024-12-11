// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/InterfaceMock.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE(
    "Mocking interface methods by hand.",
    "[example][example::mock][example::mock::interface]")
{
    //! [interface mock manual]
    class Interface
    {
    public:
        virtual ~Interface() = default;
        virtual void foo() = 0;
    };

    class Derived
        : public Interface
    {
    public:
        ~Derived() override = default;

        // Begin boilerplate

        // we build our mock object by hand
        mimicpp::Mock<void()> foo_{};

        // and forward the incoming call from the interface function to the mock object.
        void foo() override
        {
            return foo_();
        }

        // End boilerplate
    };

    // this may be a function from somewhere else, working with an interface.
    constexpr auto my_function = [](Interface& obj) {
        obj.foo();
    };

    Derived mock{};
    SCOPED_EXP mock.foo_.expect_call();

    my_function(mock);
    //! [interface mock manual]
}

TEST_CASE(
    "Interfaces methods can be mocked.",
    "[example][example::mock][example::mock::interface]")
{
    //! [interface mock simple]
    class Interface
    {
    public:
        virtual ~Interface() = default;
        virtual void foo() = 0;
    };

    class Derived
        : public Interface
    {
    public:
        ~Derived() override = default;

        // this generates the override method and a mock object named foo_
        MOCK_METHOD(foo, void, ());
    };

    // this may be a function from somewhere else, working with an interface.
    constexpr auto my_function = [](Interface& obj) {
        obj.foo();
    };

    Derived mock{};
    SCOPED_EXP mock.foo_.expect_call(); // note the _ suffix. That's the name of the mock object.

    my_function(mock);
    //! [interface mock simple]
}

TEST_CASE(
    "Overloaded interface methods can be mocked.",
    "[example][example::mock][example::mock::interface]")
{
    //! [interface mock overloaded]
    class Interface
    {
    public:
        virtual ~Interface() = default;
        virtual void foo() = 0;
        virtual void foo() const noexcept = 0;
    };

    class Derived
        : public Interface
    {
    public:
        ~Derived() override = default;

        // this generates both overrides and a mock object named foo_
        MOCK_OVERLOADED_METHOD(
            foo,
            ADD_OVERLOAD(void, ()),
            ADD_OVERLOAD(void, (), const noexcept)); // note the ``const noexcept`` specifications as third argument
    };

    // this may be a function from somewhere else, working with an immutable interface.
    constexpr auto my_function = [](const Interface& obj) {
        obj.foo();
    };

    Derived mock{};
    SCOPED_EXP std::as_const(mock).foo_.expect_call(); // We explicitly require the const overload to be called.

    my_function(mock);
    //! [interface mock overloaded]
}

TEST_CASE(
    "Multiple-inheritance is supported.",
    "[example][example::mock][example::mock::interface]")
{
    //! [interface mock multiple inheritance]
    namespace expect = mimicpp::expect;
    namespace finally = mimicpp::finally;

    class InterfaceA
    {
    public:
        virtual ~InterfaceA() = default;
        virtual void foo() = 0;

        virtual void bar() = 0;
    };

    class InterfaceB
    {
    public:
        virtual ~InterfaceB() = default;
        virtual void foo() = 0;

        virtual int bar() const noexcept = 0;
    };

    class Derived
        : public InterfaceA,
          public InterfaceB
    {
    public:
        ~Derived() override = default;

        // Both interfaces have a foo() method with the same signature, so we do not need to overload.
        MOCK_METHOD(foo, void, ()); // mocks both foo() methods from InterfaceA and InterfaceB

        // Both interfaces have a bar() method, but with different signature. So, let's work on that overload-set.
        MOCK_OVERLOADED_METHOD(
            bar,
            ADD_OVERLOAD(void, ()),                 // from InterfaceA
            ADD_OVERLOAD(int, (), const noexcept)); // from InterfaceB
    };

    // let's build a function, which requires the InterfaceA
    constexpr auto use_interfaceA = [](InterfaceA& obj) {
        obj.foo();
        obj.bar();
    };

    // and another, which requires the InterfaceB
    constexpr auto use_interfaceB = [](InterfaceB& obj) {
        obj.foo();
        std::ignore = obj.bar(); // InterfaceB::bar returns an int, just explicitly ignore it here
    };

    // setting up the expectations isn't any different from the previous examples
    Derived mock{};
    SCOPED_EXP mock.foo_.expect_call()
        and expect::twice();
    SCOPED_EXP mock.bar_.expect_call();               // selects the non-const overload of bar()
    SCOPED_EXP std::as_const(mock).bar_.expect_call() // selects the const overload of bar()
        and finally::returns(42);

    use_interfaceA(mock); // calls foo() and the non-const bar()
    use_interfaceB(mock); // calls foo() and the const bar()
                          //! [interface mock multiple inheritance]
}
