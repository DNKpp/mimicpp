//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/CallConvention.hpp"
#include "mimic++/InterfaceMock.hpp"
#include "mimic++/Mock.hpp"

MIMICPP_REGISTER_CALL_CONVENTION(__vectorcall, vectorcall_call_convention);

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "vectorcall_call_convention::remove_call_convention removes the call-convention.",
    "[type_traits]",
    ((bool dummy, typename Return, typename... Args), dummy, Return, Args...),
    (true, void),
    (true, void, int),
    (true, void, float, int),
    (true, void, float&),
    (true, void, const float&),
    (true, void, float&&),
    (true, void, const float&&),
    (true, void, float*),
    (true, void, const float*),

    (true, double),
    (true, double, int),
    (true, double, float, int),
    (true, double, float&),
    (true, double, const float&),
    (true, double, float&&),
    (true, double, const float&&),
    (true, double, float*),
    (true, double, const float*),

    (true, double&),
    (true, double&, int),
    (true, double&, float, int),
    (true, double&, float&),
    (true, double&, const float&),
    (true, double&, float&&),
    (true, double&, const float&&),
    (true, double&, float*),
    (true, double&, const float*),

    (true, const double&),
    (true, const double&, int),
    (true, const double&, float, int),
    (true, const double&, float&),
    (true, const double&, const float&),
    (true, const double&, float&&),
    (true, const double&, const float&&),
    (true, const double&, float*),
    (true, const double&, const float*),

    (true, double&&),
    (true, double&&, int),
    (true, double&&, float, int),
    (true, double&&, float&),
    (true, double&&, const float&),
    (true, double&&, float&&),
    (true, double&&, const float&&),
    (true, double&&, float*),
    (true, double&&, const float*),

    (true, const double&&),
    (true, const double&&, int),
    (true, const double&&, float, int),
    (true, const double&&, float&),
    (true, const double&&, const float&),
    (true, const double&&, float&&),
    (true, const double&&, const float&&),
    (true, const double&&, float*),
    (true, const double&&, const float*),

    (true, void*),
    (true, void*, int),
    (true, void*, float, int),
    (true, void*, float&),
    (true, void*, const float&),
    (true, void*, float&&),
    (true, void*, const float&&),
    (true, void*, float*),
    (true, void*, const float*),

    (true, const void*),
    (true, const void*, int),
    (true, const void*, float, int),
    (true, const void*, float&),
    (true, const void*, const float&),
    (true, const void*, float&&),
    (true, const void*, const float&&),
    (true, const void*, float*),
    (true, const void*, const float*))
{
    using namespace vectorcall_call_convention;

    STATIC_REQUIRE(std::same_as<Return(Args...), typename remove_call_convention<Return __vectorcall(Args...)>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...), remove_call_convention_t<Return __vectorcall(Args...)>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) const, typename remove_call_convention<Return __vectorcall(Args...) const>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) const, remove_call_convention_t<Return __vectorcall(Args...) const>>);

    STATIC_REQUIRE(std::same_as<Return(Args...)&, typename remove_call_convention<Return __vectorcall(Args...)&>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...)&, remove_call_convention_t<Return __vectorcall(Args...)&>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) const&, typename remove_call_convention<Return __vectorcall(Args...) const&>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) const&, remove_call_convention_t<Return __vectorcall(Args...) const&>>);

    STATIC_REQUIRE(std::same_as<Return(Args...)&&, typename remove_call_convention<Return __vectorcall(Args...) &&>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...)&&, remove_call_convention_t<Return __vectorcall(Args...) &&>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) const&&, typename remove_call_convention<Return __vectorcall(Args...) const&&>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) const&&, remove_call_convention_t<Return __vectorcall(Args...) const&&>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) noexcept, typename remove_call_convention<Return __vectorcall(Args...) noexcept>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) noexcept, remove_call_convention_t<Return __vectorcall(Args...) noexcept>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) const noexcept, typename remove_call_convention<Return __vectorcall(Args...) const noexcept>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) const noexcept, remove_call_convention_t<Return __vectorcall(Args...) const noexcept>>);

    STATIC_REQUIRE(std::same_as < Return(Args...) & noexcept, typename remove_call_convention < Return __vectorcall(Args...) & noexcept > ::type >);
    STATIC_REQUIRE(std::same_as < Return(Args...) & noexcept, remove_call_convention_t < Return __vectorcall(Args...) & noexcept >>);

    STATIC_REQUIRE(std::same_as < Return(Args...) const& noexcept, typename remove_call_convention<Return __vectorcall(Args...) const & noexcept>::type >);
    STATIC_REQUIRE(std::same_as < Return(Args...) const& noexcept, remove_call_convention_t < Return __vectorcall(Args...) const& noexcept >>);

    STATIC_REQUIRE(std::same_as < Return(Args...) && noexcept, typename remove_call_convention < Return __vectorcall(Args...) && noexcept > ::type >);
    STATIC_REQUIRE(std::same_as < Return(Args...) && noexcept, remove_call_convention_t < Return __vectorcall(Args...) && noexcept >>);

    STATIC_REQUIRE(std::same_as < Return(Args...) const&& noexcept, typename remove_call_convention<Return __vectorcall(Args...) const && noexcept>::type >);
    STATIC_REQUIRE(std::same_as < Return(Args...) const&& noexcept, remove_call_convention_t < Return __vectorcall(Args...) const&& noexcept >>);
}

TEMPLATE_TEST_CASE_SIG(
    "vectorcall_call_convention::add_call_convention adds the call-convention (if not present).",
    "[type_traits]",
    ((bool dummy, typename Return, typename... Args), dummy, Return, Args...),
    (true, void),
    (true, void, int),
    (true, void, float, int),
    (true, void, float&),
    (true, void, const float&),
    (true, void, float&&),
    (true, void, const float&&),
    (true, void, float*),
    (true, void, const float*),

    (true, double),
    (true, double, int),
    (true, double, float, int),
    (true, double, float&),
    (true, double, const float&),
    (true, double, float&&),
    (true, double, const float&&),
    (true, double, float*),
    (true, double, const float*),

    (true, double&),
    (true, double&, int),
    (true, double&, float, int),
    (true, double&, float&),
    (true, double&, const float&),
    (true, double&, float&&),
    (true, double&, const float&&),
    (true, double&, float*),
    (true, double&, const float*),

    (true, const double&),
    (true, const double&, int),
    (true, const double&, float, int),
    (true, const double&, float&),
    (true, const double&, const float&),
    (true, const double&, float&&),
    (true, const double&, const float&&),
    (true, const double&, float*),
    (true, const double&, const float*),

    (true, double&&),
    (true, double&&, int),
    (true, double&&, float, int),
    (true, double&&, float&),
    (true, double&&, const float&),
    (true, double&&, float&&),
    (true, double&&, const float&&),
    (true, double&&, float*),
    (true, double&&, const float*),

    (true, const double&&),
    (true, const double&&, int),
    (true, const double&&, float, int),
    (true, const double&&, float&),
    (true, const double&&, const float&),
    (true, const double&&, float&&),
    (true, const double&&, const float&&),
    (true, const double&&, float*),
    (true, const double&&, const float*),

    (true, void*),
    (true, void*, int),
    (true, void*, float, int),
    (true, void*, float&),
    (true, void*, const float&),
    (true, void*, float&&),
    (true, void*, const float&&),
    (true, void*, float*),
    (true, void*, const float*),

    (true, const void*),
    (true, const void*, int),
    (true, const void*, float, int),
    (true, const void*, float&),
    (true, const void*, const float&),
    (true, const void*, float&&),
    (true, const void*, const float&&),
    (true, const void*, float*),
    (true, const void*, const float*))
{
    using namespace vectorcall_call_convention;

    SECTION("If call-convention is present, signatures stay as they are.")
    {
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...), typename add_call_convention<Return __vectorcall(Args...)>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...), add_call_convention_t<Return __vectorcall(Args...)>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const, typename add_call_convention<Return __vectorcall(Args...) const>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const, add_call_convention_t<Return __vectorcall(Args...) const>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...)&, typename add_call_convention<Return __vectorcall(Args...)&>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...)&, add_call_convention_t<Return __vectorcall(Args...)&>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const&, typename add_call_convention<Return __vectorcall(Args...) const&>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const&, add_call_convention_t<Return __vectorcall(Args...) const&>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...)&&, typename add_call_convention<Return __vectorcall(Args...) &&>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...)&&, add_call_convention_t<Return __vectorcall(Args...) &&>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const&&, typename add_call_convention<Return __vectorcall(Args...) const&&>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const&&, add_call_convention_t<Return __vectorcall(Args...) const&&>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) noexcept, typename add_call_convention<Return __vectorcall(Args...) noexcept>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) noexcept, add_call_convention_t<Return __vectorcall(Args...) noexcept>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const noexcept, typename add_call_convention<Return __vectorcall(Args...) const noexcept>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const noexcept, add_call_convention_t<Return __vectorcall(Args...) const noexcept>>);

        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) & noexcept, typename add_call_convention < Return __vectorcall(Args...) & noexcept > ::type >);
        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) & noexcept, add_call_convention_t < Return __vectorcall(Args...) & noexcept >>);

        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) const& noexcept, typename add_call_convention<Return __vectorcall(Args...) const & noexcept>::type >);
        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) const& noexcept, add_call_convention_t < Return __vectorcall(Args...) const& noexcept >>);

        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) && noexcept, typename add_call_convention < Return __vectorcall(Args...) && noexcept > ::type >);
        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) && noexcept, add_call_convention_t < Return __vectorcall(Args...) && noexcept >>);

        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) const&& noexcept, typename add_call_convention<Return __vectorcall(Args...) const && noexcept>::type >);
        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) const&& noexcept, add_call_convention_t < Return __vectorcall(Args...) const&& noexcept >>);
    }

    SECTION("If call-convention is not present, signatures get them added.")
    {
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...), typename add_call_convention<Return(Args...)>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...), add_call_convention_t<Return(Args...)>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const, typename add_call_convention<Return(Args...) const>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const, add_call_convention_t<Return(Args...) const>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...)&, typename add_call_convention<Return(Args...)&>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...)&, add_call_convention_t<Return(Args...)&>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const&, typename add_call_convention<Return(Args...) const&>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const&, add_call_convention_t<Return(Args...) const&>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...)&&, typename add_call_convention<Return(Args...) &&>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...)&&, add_call_convention_t<Return(Args...) &&>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const&&, typename add_call_convention<Return(Args...) const&&>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const&&, add_call_convention_t<Return(Args...) const&&>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) noexcept, typename add_call_convention<Return(Args...) noexcept>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) noexcept, add_call_convention_t<Return(Args...) noexcept>>);

        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const noexcept, typename add_call_convention<Return(Args...) const noexcept>::type>);
        STATIC_REQUIRE(std::same_as<Return __vectorcall(Args...) const noexcept, add_call_convention_t<Return(Args...) const noexcept>>);

        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) & noexcept, typename add_call_convention < Return(Args...) & noexcept > ::type >);
        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) & noexcept, add_call_convention_t < Return(Args...) & noexcept >>);

        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) const& noexcept, typename add_call_convention<Return(Args...) const & noexcept>::type >);
        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) const& noexcept, add_call_convention_t < Return(Args...) const& noexcept >>);

        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) && noexcept, typename add_call_convention < Return(Args...) && noexcept > ::type >);
        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) && noexcept, add_call_convention_t < Return(Args...) && noexcept >>);

        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) const&& noexcept, typename add_call_convention<Return(Args...) const && noexcept>::type >);
        STATIC_REQUIRE(std::same_as < Return __vectorcall(Args...) const&& noexcept, add_call_convention_t < Return(Args...) const&& noexcept >>);
    }
}

TEST_CASE(
    "__vectorcall specializations have been enabled for the framework.",
    "[type_traits]")
{
    using SignatureT = void __vectorcall() const& noexcept;

    using tag_t = signature_call_convention_t<SignatureT>;
    STATIC_REQUIRE(std::same_as<vectorcall_call_convention::tag, tag_t>);

    using traits_t = call_convention_traits<tag_t>;
    STATIC_REQUIRE(std::same_as<vectorcall_call_convention::tag, traits_t::tag_t>);
    STATIC_REQUIRE(std::same_as < void() const& noexcept, traits_t::remove_call_convention_t < SignatureT >>);
    STATIC_REQUIRE(std::same_as<SignatureT, traits_t::add_call_convention_t<void() const & noexcept>>);

    struct Derived
    {
    };

    STATIC_REQUIRE(
        std::same_as<
            vectorcall_call_convention::CallInterface<Derived, SignatureT>,
            traits_t::call_interface_t<Derived, SignatureT>>);
}

TEST_CASE(
    "All signature traits are supported by signatures with __vectorcall.",
    "[type_traits]")
{
    SECTION("When trait actually removes.")
    {
        using SignatureT = void __vectorcall() const& noexcept;

        STATIC_CHECK(std::same_as < void() const& noexcept, signature_remove_call_convention_t < SignatureT >>);

        STATIC_CHECK(std::same_as<void CALL_CONVENTION() const noexcept, signature_remove_ref_qualifier_t<SignatureT>>);
        STATIC_CHECK(std::same_as < void CALL_CONVENTION()& noexcept, signature_remove_const_qualifier_t < SignatureT >>);
        STATIC_CHECK(std::same_as<void CALL_CONVENTION() const&, signature_remove_noexcept_t<SignatureT>>);
        STATIC_CHECK(signature_remove_noexcept<SignatureT>::value);
        STATIC_CHECK(std::same_as<void(), signature_decay_t<SignatureT>>);
    }

    SECTION("When trait silently removes nothing.")
    {
        using SignatureT = void __vectorcall();

        STATIC_CHECK(std::same_as<SignatureT, signature_remove_ref_qualifier_t<SignatureT>>);
        STATIC_CHECK(std::same_as<SignatureT, signature_remove_const_qualifier_t<SignatureT>>);
        STATIC_CHECK(std::same_as<SignatureT, signature_remove_noexcept_t<SignatureT>>);
        STATIC_CHECK(!signature_remove_noexcept<SignatureT>::value);
        STATIC_CHECK(std::same_as<void(), signature_decay_t<SignatureT>>);
    }
}

TEST_CASE(
    "Mocks support __vectorcall.",
    "[mock]")
{
    SECTION("Signatures without any other specs.")
    {
        using MockT = Mock<void __vectorcall()>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (__vectorcall MockT::*)(util::SourceLocation)>);
    }

    SECTION("Signatures with noexcept.")
    {
        using MockT = Mock<void __vectorcall() noexcept>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (__vectorcall MockT::*)(util::SourceLocation) noexcept>);
    }

    SECTION("Signatures with const.")
    {
        using MockT = Mock<void __vectorcall() const>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (__vectorcall MockT::*)(util::SourceLocation) const>);
    }

    SECTION("Signatures with const noexcept.")
    {
        using MockT = Mock<void __vectorcall() const noexcept>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (__vectorcall MockT::*)(util::SourceLocation) const noexcept>);
    }

    SECTION("Signatures with &.")
    {
        using MockT = Mock<void __vectorcall()&>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (__vectorcall MockT::*)(util::SourceLocation)&>);
    }

    SECTION("Signatures with & noexcept.")
    {
        using MockT = Mock<void __vectorcall() & noexcept>;
        STATIC_REQUIRE(
            std::convertible_to < decltype(&MockT::operator()),
            void (__vectorcall MockT::*)(util::SourceLocation)& noexcept >);
    }

    SECTION("Signatures with const&.")
    {
        using MockT = Mock<void __vectorcall() const&>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (__vectorcall MockT::*)(util::SourceLocation) const&>);
    }

    SECTION("Signatures with const& noexcept.")
    {
        using MockT = Mock<void __vectorcall() const & noexcept>;
        STATIC_REQUIRE(
            std::convertible_to < decltype(&MockT::operator()),
            void (__vectorcall MockT::*)(util::SourceLocation) const& noexcept >);
    }

    SECTION("Signatures with &&.")
    {
        using MockT = Mock<void __vectorcall() &&>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (__vectorcall MockT::*)(util::SourceLocation) &&>);
    }

    SECTION("Signatures with && noexcept.")
    {
        using MockT = Mock<void __vectorcall() && noexcept>;
        STATIC_REQUIRE(
            std::convertible_to < decltype(&MockT::operator()),
            void (__vectorcall MockT::*)(util::SourceLocation)&& noexcept >);
    }

    SECTION("Signatures with const&&.")
    {
        using MockT = Mock<void __vectorcall() const&&>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (__vectorcall MockT::*)(util::SourceLocation) const&&>);
    }

    SECTION("Signatures with const&& noexcept.")
    {
        using MockT = Mock<void __vectorcall() const && noexcept>;
        STATIC_REQUIRE(
            std::convertible_to < decltype(&MockT::operator()),
            void (__vectorcall MockT::*)(util::SourceLocation) const&& noexcept >);
    }

    SECTION("Mocks still supports overloading.")
    {
        Mock<
            void(),
            void __vectorcall() const>
            mock{};

        {
            MIMICPP_SCOPED_EXPECTATION std::as_const(mock).expect_call();
            std::as_const(mock)();
        }

        {
            MIMICPP_SCOPED_EXPECTATION mock.expect_call();
            mock();
        }
    }
}

TEST_CASE(
    "Interface-Mocks support __vectorcall.",
    "[mock]")
{
    SECTION("When mocking single method.")
    {
        class Interface
        {
        public:
            virtual ~Interface() = default;

            virtual void __vectorcall foo() = 0;
        };

        class Derived
            : public Interface
        {
        public:
            MIMICPP_MOCK_METHOD(foo, void, (), , __vectorcall);
        };

        Derived mock{};
        MIMICPP_SCOPED_EXPECTATION mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("When mocking single method, and call-convention is parenthesized.")
    {
        class Interface
        {
        public:
            virtual ~Interface() = default;

            virtual void __vectorcall foo() = 0;
        };

        class Derived
            : public Interface
        {
        public:
            MIMICPP_MOCK_METHOD(foo, void, (), , (__vectorcall));
        };

        Derived mock{};
        MIMICPP_SCOPED_EXPECTATION mock.foo_.expect_call();
        mock.foo();
    }

    SECTION("When mocking method overload-set.")
    {
        class Interface
        {
        public:
            virtual ~Interface() = default;

            virtual void __vectorcall foo() = 0;
            virtual void foo() const = 0;
        };

        class Derived
            : public Interface
        {
        public:
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, (), , __vectorcall),
                MIMICPP_ADD_OVERLOAD(void, (), const));
        };

        Derived mock{};

        {
            MIMICPP_SCOPED_EXPECTATION mock.foo_.expect_call();
            mock.foo();
        }

        {
            MIMICPP_SCOPED_EXPECTATION std::as_const(mock).foo_.expect_call();
            std::as_const(mock).foo();
        }
    }
}
