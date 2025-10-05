//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

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

namespace
{
    struct TestTraits
    {
        static constexpr bool is_member{true};

        template <typename... Signatures>
        using target_type = Mock<Signatures...>;

        template <typename Signature, typename... Args>
        static constexpr decltype(auto) invoke(
            auto& mock,
            [[maybe_unused]] auto* self,
            std::tuple<Args...>&& args)
        {
            return facade::detail::apply<Signature>(mock, std::move(args));
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
    "MIMICPP_MAKE_OVERLOADED_FACADE_EXT can handle at least up to 64 signatures.",
    "[facade]")
{
    struct type
    {
        MIMICPP_MAKE_OVERLOADED_FACADE_EXT(
            TestTraits,
            foo_,
            foo,
            /*linkage*/,
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

    type mock{};

    ScopedSequence sequence{};

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
