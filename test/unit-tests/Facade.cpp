//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/InterfaceMock.hpp"

using namespace mimicpp;

namespace
{
    struct TestTraits
    {
        static constexpr bool is_member{true};

        template <typename... Signatures>
        using mock_type = Mock<Signatures...>;

        template <typename Signature, typename... Args>
        static constexpr decltype(auto) invoke(
            auto& mock,
            [[maybe_unused]] auto* self,
            std::tuple<Args...>&& args)
        {
            return detail::indirectly_apply_mock<Signature>(mock, std::move(args));
        }

        template <typename Self>
        [[nodiscard]]
        static MockSettings make_settings([[maybe_unused]] Self const* const self, StringT functionName)
        {
            return MockSettings{.name = std::move(functionName), .stacktraceSkip = 1u};
        }
    };
}

TEST_CASE(
    "MIMICPP_MAKE_OVERLOADED_FACADE_EXT creates a target object and overloaded facade functions.",
    "[facade]")
{
    SECTION("Non-ref qualified functions.")
    {
        struct type
        {
            MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
                TestTraits,
                foo_,
                foo,
                /*linkage*/,
                MIMICPP_ADD_OVERLOAD(void, ()),
                MIMICPP_ADD_OVERLOAD(void, (), const));
        };

        using Expected = Mock<void(), void() const>;
        STATIC_REQUIRE(std::same_as<Expected, decltype(type::foo_)>);

        type object{};

        SECTION("When unqualified overload is used.")
        {
            ScopedExpectation expectation = object.foo_.expect_call();
            object.foo();
        }

        SECTION("When const qualified overload is used.")
        {
            ScopedExpectation expectation = std::as_const(object).foo_.expect_call();
            std::as_const(object).foo();
        }
    }

    SECTION("Non-ref qualified noexcept functions.")
    {
        struct type
        {
            MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
                TestTraits,
                foo_,
                foo,
                /*linkage*/,
                MIMICPP_ADD_OVERLOAD(void, (), noexcept),
                MIMICPP_ADD_OVERLOAD(void, (), const noexcept));
        };

        using Expected = Mock<void() noexcept, void() const noexcept>;
        STATIC_REQUIRE(std::same_as<Expected, decltype(type::foo_)>);

        type object{};

        SECTION("When unqualified overload is used.")
        {
            ScopedExpectation expectation = object.foo_.expect_call();
            object.foo();
        }

        SECTION("When const qualified overload is used.")
        {
            ScopedExpectation expectation = std::as_const(object).foo_.expect_call();
            std::as_const(object).foo();
        }
    }

    SECTION("Ref qualified functions.")
    {
        struct type
        {
            MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
                TestTraits,
                foo_,
                foo,
                /*linkage*/,
                MIMICPP_ADD_OVERLOAD(void, (), &),
                MIMICPP_ADD_OVERLOAD(void, (), const&),
                MIMICPP_ADD_OVERLOAD(void, (), &&),
                MIMICPP_ADD_OVERLOAD(void, (), const&&));
        };

        using Expected = Mock<
            void()&,
            void() const&,
            void()&&,
            void() const&&>;
        STATIC_REQUIRE(std::same_as<Expected, decltype(type::foo_)>);

        type object{};

        SECTION("When lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = object.foo_.expect_call();
            object.foo();
        }

        SECTION("When const lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::as_const(object).foo_.expect_call();
            std::as_const(object).foo();
        }

        SECTION("When rvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::move(object).foo_.expect_call();
            std::move(object).foo();
        }

        SECTION("When const lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::move(std::as_const(object)).foo_.expect_call();
            std::move(std::as_const(object)).foo();
        }
    }

    SECTION("Ref qualified noexcept functions.")
    {
        struct type
        {
            MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
                TestTraits,
                foo_,
                foo,
                /*linkage*/,
                MIMICPP_ADD_OVERLOAD(void, (), & noexcept),
                MIMICPP_ADD_OVERLOAD(void, (), const& noexcept),
                MIMICPP_ADD_OVERLOAD(void, (), && noexcept),
                MIMICPP_ADD_OVERLOAD(void, (), const&& noexcept));
        };

        using Expected = Mock<
            void() & noexcept,
            void() const & noexcept,
            void() && noexcept,
            void() const && noexcept>;
        STATIC_REQUIRE(std::same_as<Expected, decltype(type::foo_)>);

        type object{};

        SECTION("When lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = object.foo_.expect_call();
            object.foo();
        }

        SECTION("When const lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::as_const(object).foo_.expect_call();
            std::as_const(object).foo();
        }

        SECTION("When rvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::move(object).foo_.expect_call();
            std::move(object).foo();
        }

        SECTION("When const lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::move(std::as_const(object)).foo_.expect_call();
            std::move(std::as_const(object)).foo();
        }
    }
}

TEST_CASE(
    "MIMICPP_MAKE_OVERLOADED_FACADE_EXT supports override and final.",
    "[facade]")
{
    struct non_ref_interface
    {
        virtual ~non_ref_interface() = default;
        virtual void foo() = 0;
        virtual void foo() const = 0;
    };

    struct ref_interface
    {
        virtual ~ref_interface() = default;
        virtual void foo() & = 0;
        virtual void foo() const& = 0;
        virtual void foo() && = 0;
        virtual void foo() const&& = 0;
    };

    SECTION("Non-ref qualified functions.")
    {
        struct derived
            : public non_ref_interface
        {
            MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
                TestTraits,
                foo_,
                foo,
                /*linkage*/,
                MIMICPP_ADD_OVERLOAD(void, (), override),
                MIMICPP_ADD_OVERLOAD(void, (), const override));
        };

        using Expected = Mock<void(), void() const>;
        STATIC_REQUIRE(std::same_as<Expected, decltype(derived::foo_)>);

        derived object{};
        non_ref_interface& base = object;

        SECTION("When unqualified overload is used.")
        {
            ScopedExpectation expectation = object.foo_.expect_call();
            base.foo();
        }

        SECTION("When const qualified overload is used.")
        {
            ScopedExpectation expectation = std::as_const(object).foo_.expect_call();
            std::as_const(base).foo();
        }
    }

    SECTION("Non-ref qualified noexcept functions.")
    {
        struct derived
            : public non_ref_interface
        {
            MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
                TestTraits,
                foo_,
                foo,
                /*linkage*/,
                MIMICPP_ADD_OVERLOAD(void, (), noexcept override),
                MIMICPP_ADD_OVERLOAD(void, (), const noexcept override));
        };

        using Expected = Mock<void() noexcept, void() const noexcept>;
        STATIC_REQUIRE(std::same_as<Expected, decltype(derived::foo_)>);

        derived object{};
        non_ref_interface& base = object;

        SECTION("When unqualified overload is used.")
        {
            ScopedExpectation expectation = object.foo_.expect_call();
            base.foo();
        }

        SECTION("When const qualified overload is used.")
        {
            ScopedExpectation expectation = std::as_const(object).foo_.expect_call();
            std::as_const(base).foo();
        }
    }

    SECTION("Ref qualified functions.")
    {
        struct derived
            : public ref_interface
        {
            MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
                TestTraits,
                foo_,
                foo,
                /*linkage*/,
                MIMICPP_ADD_OVERLOAD(void, (), &override),
                MIMICPP_ADD_OVERLOAD(void, (), const& override),
                MIMICPP_ADD_OVERLOAD(void, (), &&override),
                MIMICPP_ADD_OVERLOAD(void, (), const&& override));
        };

        using Expected = Mock<
            void()&,
            void() const&,
            void()&&,
            void() const&&>;
        STATIC_REQUIRE(std::same_as<Expected, decltype(derived::foo_)>);

        derived object{};
        ref_interface& base = object;

        SECTION("When lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = object.foo_.expect_call();
            base.foo();
        }

        SECTION("When const lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::as_const(object).foo_.expect_call();
            std::as_const(base).foo();
        }

        SECTION("When rvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::move(object).foo_.expect_call();
            std::move(base).foo();
        }

        SECTION("When const lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::move(std::as_const(object)).foo_.expect_call();
            std::move(std::as_const(base)).foo();
        }
    }

    SECTION("Ref qualified noexcept functions.")
    {
        struct derived
            : public ref_interface
        {
            MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
                TestTraits,
                foo_,
                foo,
                /*linkage*/,
                MIMICPP_ADD_OVERLOAD(void, (), & noexcept override),
                MIMICPP_ADD_OVERLOAD(void, (), const& noexcept override),
                MIMICPP_ADD_OVERLOAD(void, (), && noexcept override),
                MIMICPP_ADD_OVERLOAD(void, (), const&& noexcept override));
        };

        using Expected = Mock<
            void() & noexcept,
            void() const & noexcept,
            void() && noexcept,
            void() const && noexcept>;
        STATIC_REQUIRE(std::same_as<Expected, decltype(derived::foo_)>);

        derived object{};
        ref_interface& base = object;

        SECTION("When lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = object.foo_.expect_call();
            base.foo();
        }

        SECTION("When const lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::as_const(object).foo_.expect_call();
            std::as_const(base).foo();
        }

        SECTION("When rvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::move(object).foo_.expect_call();
            std::move(base).foo();
        }

        SECTION("When const lvalue-ref qualified overload is used.")
        {
            ScopedExpectation expectation = std::move(std::as_const(object)).foo_.expect_call();
            std::move(std::as_const(base)).foo();
        }
    }
}
