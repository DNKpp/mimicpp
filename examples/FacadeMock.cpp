//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Facade.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

TEST_CASE(
    "Mocking a polymorphic type by hand.",
    "[example][example::mock][example::mock::facade]")
{
    //! [facade mock manual]
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
    //! [facade mock manual]
}

TEST_CASE(
    "Virtual functions can be mocked.",
    "[example][example::mock][example::mock::facade]")
{
    //! [facade interface mock simple]
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

        // this generates the override method and a mock object named `foo_`
        MAKE_MEMBER_MOCK(foo, void, (), override);
    };

    // this may be a function from somewhere else, working with an interface.
    constexpr auto my_function = [](Interface& obj) {
        obj.foo();
    };

    Derived mock{};
    SCOPED_EXP mock.foo_.expect_call(); // note the `_` suffix. That's the name of the mock object.

    my_function(mock);
    //! [facade interface mock]
}

TEST_CASE(
    "Overloaded virtual functions can be mocked.",
    "[example][example::mock][example::mock::facade]")
{
    //! [facade interface mock overloaded]
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

        // this generates both overrides and a mock object named `foo_`
        MAKE_OVERLOADED_MEMBER_MOCK(
            foo,
            ADD_OVERLOAD(void, (), override),
            ADD_OVERLOAD(void, (), const noexcept override)); // note the `const noexcept` specifications as third argument
    };

    // this may be a function from somewhere else, working with an immutable interface.
    constexpr auto my_function = [](Interface const& obj) {
        obj.foo();
    };

    Derived mock{};
    SCOPED_EXP std::as_const(mock).foo_.expect_call(); // We explicitly require the const overload to be called.

    my_function(mock);
    //! [facade interface mock overloaded]
}

TEST_CASE(
    "Facade-Mocks can have an explicit *this* param.",
    "[example][example::mock][example::mock::facade]")
{
    //! [facade mock with this]
    namespace finally = mimicpp::finally;

    class Base
    {
    public:
        virtual ~Base() = default;

        virtual int foo() const
        {
            return 42;
        }
    };

    class Derived
        : public Base
    {
    public:
        ~Derived() override = default;

        // This alias is required because the `..._WITH_THIS` macros are not able to determine the current type by themselves.
        using self_type = Derived;

        // This generates the override method and a mock object named foo_, which expects an explicit *this* param.
        // => `Mock<int(Derived const*) const>`
        MAKE_MEMBER_MOCK_WITH_THIS(foo, int, (), const override);
    };

    Derived object{};

    // The first param behaves like an actual *this* pointer.
    SCOPED_EXP object.foo_.expect_call(&object)
        // Let's redirect the call to the actual `Base::foo` implementation.
        and finally::returns_apply_all_result_of([](auto* self) { return self->Base::foo(); });

    CHECK(42 == object.foo());
    //! [facade mock with this]
}

TEST_CASE(
    "Multiple-inheritance is supported.",
    "[example][example::mock][example::mock::facade]")
{
    //! [facade mock multiple inheritance]
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
        MAKE_MEMBER_MOCK(foo, void, (), override); // mocks both foo() methods from InterfaceA and InterfaceB

        // Both interfaces have a bar() method, but with different signature. So, let's work on that overload-set.
        MAKE_OVERLOADED_MEMBER_MOCK(
            bar,
            ADD_OVERLOAD(void, (), override),                // from InterfaceA
            ADD_OVERLOAD(int, (), const noexcept override)); // from InterfaceB
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

    //! [facade mock multiple inheritance]
}
