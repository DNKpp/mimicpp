//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Common.hpp"
#include "mimic++/Facade.hpp"
#include "mimic++/ScopedSequence.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "facade::detail::apply_normalized_specs omits override and final specifiers.",
    "[facade][detail]",
    ((typename Return, typename... Params), Return, Params...),
    (void),
    (void, int),
    (void, int, double, (std::tuple<int, float>)),
    (std::tuple<int, float>))
{
    using RawSig = Return(Params...);

    SECTION("Without noexcept.")
    {
        SECTION("Without other specs.")
        {
            STATIC_REQUIRE(std::same_as<Return(Params...), facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"override"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...), facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"final"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...), facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{""}>>);
        }

        SECTION("With const.")
        {
            STATIC_REQUIRE(std::same_as<Return(Params...) const, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const override"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) const, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const final"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) const, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const"}>>);
        }

        SECTION("With lvalue-ref.")
        {
            STATIC_REQUIRE(std::same_as<Return(Params...)&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"& override"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...)&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"& final"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...)&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"&"}>>);
        }

        SECTION("With const lvalue-ref.")
        {
            STATIC_REQUIRE(std::same_as<Return(Params...) const&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const& override"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) const&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const& final"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) const&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const&"}>>);
        }

        SECTION("With rvalue-ref.")
        {
            STATIC_REQUIRE(std::same_as<Return(Params...)&&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"&& override"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...)&&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"&& final"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...)&&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"&&"}>>);
        }

        SECTION("With const rvalue-ref.")
        {
            STATIC_REQUIRE(std::same_as<Return(Params...) const&&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const&& override"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) const&&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const&& final"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) const&&, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const&&"}>>);
        }
    }

    SECTION("With noexcept.")
    {
        SECTION("Without other specs.")
        {
            STATIC_REQUIRE(std::same_as<Return(Params...) noexcept, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"noexcept override"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) noexcept, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"noexcept final"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) noexcept, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"noexcept"}>>);
        }

        SECTION("With const.")
        {
            STATIC_REQUIRE(std::same_as<Return(Params...) const noexcept, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const noexcept override"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) const noexcept, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const noexcept final"}>>);
            STATIC_REQUIRE(std::same_as<Return(Params...) const noexcept, facade::detail::apply_normalized_specs_t<RawSig, util::StaticString{"const noexcept"}>>);
        }

        SECTION("With lvalue-ref.")
        {
            STATIC_REQUIRE(std::same_as < Return(Params...) & noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"& noexcept override"} >>);
            STATIC_REQUIRE(std::same_as < Return(Params...) & noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"& noexcept final"} >>);
            STATIC_REQUIRE(std::same_as < Return(Params...) & noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"& noexcept"} >>);
        }

        SECTION("With const lvalue-ref.")
        {
            STATIC_REQUIRE(std::same_as < Return(Params...) const& noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"const& noexcept override"} >>);
            STATIC_REQUIRE(std::same_as < Return(Params...) const& noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"const& noexcept final"} >>);
            STATIC_REQUIRE(std::same_as < Return(Params...) const& noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"const& noexcept"} >>);
        }

        SECTION("With rvalue-ref.")
        {
            STATIC_REQUIRE(std::same_as < Return(Params...) && noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"&& noexcept override"} >>);
            STATIC_REQUIRE(std::same_as < Return(Params...) && noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"&& noexcept final"} >>);
            STATIC_REQUIRE(std::same_as < Return(Params...) && noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"&& noexcept"} >>);
        }

        SECTION("With const rvalue-ref.")
        {
            STATIC_REQUIRE(std::same_as < Return(Params...) const&& noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"const&& noexcept override"} >>);
            STATIC_REQUIRE(std::same_as < Return(Params...) const&& noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"const&& noexcept final"} >>);
            STATIC_REQUIRE(std::same_as < Return(Params...) const&& noexcept, facade::detail::apply_normalized_specs_t < RawSig, util::StaticString{"const&& noexcept"} >>);
        }
    }
}

TEST_CASE(
    "MIMICPP_DETAIL_GENERATE_FACADE_TARGET creates a mock from a list of signatures.",
    "[facade][detail]")
{
    SECTION("Just void()")
    {
        struct helper
        {
            MIMICPP_DETAIL_GENERATE_FACADE_TARGET(
                TestTraits,
                mock,
                test,
                /*linkage*/,
                (void()));
        };

        STATIC_REQUIRE(std::is_invocable_r_v<void, decltype(helper::mock)>);
    }

    SECTION("Just float&(int&&)")
    {
        struct helper
        {
            MIMICPP_DETAIL_GENERATE_FACADE_TARGET(
                TestTraits,
                mock,
                test,
                /*linkage*/,
                (float&(int&&)));
        };

        STATIC_REQUIRE(std::is_invocable_r_v<float&, decltype(helper::mock), int&&>);
    }
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

TEST_CASE(
    "MIMICPP_MAKE_MEMBER_MOCK (and MIMICPP_MAKE_OVERLOADED_MEMBER_MOCK) generates a member-object mock.",
    "[mock][mock::facade]")
{
    struct Type
    {
        MIMICPP_MAKE_MEMBER_MOCK(foo, void, ());
        MIMICPP_MAKE_MEMBER_MOCK(foo_const, void, (), const);

        MIMICPP_MAKE_MEMBER_MOCK(foo_lvalue, void, (), &);
        MIMICPP_MAKE_MEMBER_MOCK(foo_lvalue_const, void, (), const&);

        MIMICPP_MAKE_MEMBER_MOCK(foo_rvalue, void, (), &&);
        MIMICPP_MAKE_MEMBER_MOCK(foo_rvalue_const, void, (), const&&);

        MIMICPP_MAKE_MEMBER_MOCK(foo_noexcept, void, (), noexcept);
        MIMICPP_MAKE_MEMBER_MOCK(foo_const_noexcept, void, (), const noexcept);

        MIMICPP_MAKE_MEMBER_MOCK(foo_lvalue_noexcept, void, (), & noexcept);
        MIMICPP_MAKE_MEMBER_MOCK(foo_lvalue_const_noexcept, void, (), const& noexcept);

        MIMICPP_MAKE_MEMBER_MOCK(foo_rvalue_noexcept, void, (), && noexcept);
        MIMICPP_MAKE_MEMBER_MOCK(foo_rvalue_const_noexcept, void, (), const&& noexcept);
    };

    Type object{};

    SECTION("Mocking an unqualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void()>, decltype(Type::foo_)>);
        ScopedExpectation const expectation = object.foo_.expect_call();
        REQUIRE_NOTHROW(object.foo());
    }

    SECTION("Mocking const qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() const>, decltype(Type::foo_const_)>);
        ScopedExpectation const expectation = std::as_const(object).foo_const_.expect_call();
        REQUIRE_NOTHROW(std::as_const(object).foo_const());
    }

    SECTION("Mocking lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void()&>, decltype(Type::foo_lvalue_)>);
        ScopedExpectation const expectation = object.foo_lvalue_.expect_call();
        REQUIRE_NOTHROW(object.foo_lvalue());
    }

    SECTION("Mocking const lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() const&>, decltype(Type::foo_lvalue_const_)>);
        ScopedExpectation const expectation = std::as_const(object).foo_lvalue_const_.expect_call();
        REQUIRE_NOTHROW(std::as_const(object).foo_lvalue_const());
    }

    SECTION("Mocking rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() &&>, decltype(Type::foo_rvalue_)>);
        ScopedExpectation const expectation = std::move(object).foo_rvalue_.expect_call();
        REQUIRE_NOTHROW(std::move(object).foo_rvalue());
    }

    SECTION("Mocking const rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() const&&>, decltype(Type::foo_rvalue_const_)>);
        ScopedExpectation const expectation = std::move(std::as_const(object)).foo_rvalue_const_.expect_call();
        REQUIRE_NOTHROW(std::move(std::as_const(object)).foo_rvalue_const());
    }

    SECTION("Mocking a noexcept member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() noexcept>, decltype(Type::foo_noexcept_)>);
        ScopedExpectation const expectation = object.foo_noexcept_.expect_call();
        REQUIRE_NOTHROW(object.foo_noexcept());
    }

    SECTION("Mocking noexcept const qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() const noexcept>, decltype(Type::foo_const_noexcept_)>);
        ScopedExpectation const expectation = std::as_const(object).foo_const_noexcept_.expect_call();
        REQUIRE_NOTHROW(std::as_const(object).foo_const_noexcept());
    }

    SECTION("Mocking noexcept lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() & noexcept>, decltype(Type::foo_lvalue_noexcept_)>);
        ScopedExpectation const expectation = object.foo_lvalue_noexcept_.expect_call();
        REQUIRE_NOTHROW(object.foo_lvalue_noexcept());
    }

    SECTION("Mocking noexcept const lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() const & noexcept>, decltype(Type::foo_lvalue_const_noexcept_)>);
        ScopedExpectation const expectation = std::as_const(object).foo_lvalue_const_noexcept_.expect_call();
        REQUIRE_NOTHROW(std::as_const(object).foo_lvalue_const_noexcept());
    }

    SECTION("Mocking noexcept rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() && noexcept>, decltype(Type::foo_rvalue_noexcept_)>);
        ScopedExpectation const expectation = std::move(object).foo_rvalue_noexcept_.expect_call();
        REQUIRE_NOTHROW(std::move(object).foo_rvalue_noexcept());
    }

    SECTION("Mocking noexcept const rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void() const && noexcept>, decltype(Type::foo_rvalue_const_noexcept_)>);
        ScopedExpectation const expectation = std::move(std::as_const(object)).foo_rvalue_const_noexcept_.expect_call();
        REQUIRE_NOTHROW(std::move(std::as_const(object)).foo_rvalue_const_noexcept());
    }
}

TEST_CASE(
    "MIMICPP_MOCK_METHOD_WITH_THIS generates a mock with explicit *this* param.",
    "[mock][mock::interface]")
{
    struct Type
    {
        using self_type = Type;

        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo, void, ());
        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_const, void, (), const);

        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_lvalue, void, (), &);
        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_lvalue_const, void, (), const&);

        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_rvalue, void, (), &&);
        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_rvalue_const, void, (), const&&);

        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_noexcept, void, (), noexcept);
        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_const_noexcept, void, (), const noexcept);

        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_lvalue_noexcept, void, (), & noexcept);
        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_lvalue_const_noexcept, void, (), const& noexcept);

        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_rvalue_noexcept, void, (), && noexcept);
        MIMICPP_MAKE_MEMBER_MOCK_WITH_THIS(foo_rvalue_const_noexcept, void, (), const&& noexcept);
    };

    Type object{};

    SECTION("Mocking an unqualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type*)>, decltype(Type::foo_)>);
        SCOPED_EXP object.foo_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo());
    }

    SECTION("Mocking const qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type const*) const>, decltype(Type::foo_const_)>);
        SCOPED_EXP std::as_const(object).foo_const_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo_const());
    }

    SECTION("Mocking lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type*)&>, decltype(Type::foo_lvalue_)>);
        SCOPED_EXP object.foo_lvalue_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo_lvalue());
    }

    SECTION("Mocking const lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type const*) const&>, decltype(Type::foo_lvalue_const_)>);
        SCOPED_EXP std::as_const(object).foo_lvalue_const_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo_lvalue_const());
    }

    SECTION("Mocking rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type*) &&>, decltype(Type::foo_rvalue_)>);
        SCOPED_EXP std::move(object).foo_rvalue_.expect_call(&object);
        REQUIRE_NOTHROW(std::move(object).foo_rvalue());
    }

    SECTION("Mocking const rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type const*) const&&>, decltype(Type::foo_rvalue_const_)>);
        SCOPED_EXP std::move(std::as_const(object)).foo_rvalue_const_.expect_call(&object);
        REQUIRE_NOTHROW(std::move(std::as_const(object)).foo_rvalue_const());
    }

    SECTION("Mocking a noexcept member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type*) noexcept>, decltype(Type::foo_noexcept_)>);
        SCOPED_EXP object.foo_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo_noexcept());
    }

    SECTION("Mocking noexcept const qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type const*) const noexcept>, decltype(Type::foo_const_noexcept_)>);
        SCOPED_EXP std::as_const(object).foo_const_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo_const_noexcept());
    }

    SECTION("Mocking noexcept lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type*) & noexcept>, decltype(Type::foo_lvalue_noexcept_)>);
        SCOPED_EXP object.foo_lvalue_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(object.foo_lvalue_noexcept());
    }

    SECTION("Mocking noexcept const lvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type const*) const & noexcept>, decltype(Type::foo_lvalue_const_noexcept_)>);
        SCOPED_EXP std::as_const(object).foo_lvalue_const_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(std::as_const(object).foo_lvalue_const_noexcept());
    }

    SECTION("Mocking noexcept rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type*) && noexcept>, decltype(Type::foo_rvalue_noexcept_)>);
        SCOPED_EXP std::move(object).foo_rvalue_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(std::move(object).foo_rvalue_noexcept());
    }

    SECTION("Mocking noexcept const rvalue qualified member function.")
    {
        STATIC_REQUIRE(std::same_as<Mock<void(Type const*) const && noexcept>, decltype(Type::foo_rvalue_const_noexcept_)>);
        SCOPED_EXP std::move(std::as_const(object)).foo_rvalue_const_noexcept_.expect_call(&object);
        REQUIRE_NOTHROW(std::move(std::as_const(object)).foo_rvalue_const_noexcept());
    }
}
