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
