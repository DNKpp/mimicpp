//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/CallConvention.hpp"
#include "mimic++/InterfaceMock.hpp"
#include "mimic++/Mock.hpp"

#define CALL_CONVENTION __attribute__((ms_abi))
MIMICPP_REGISTER_CALL_CONVENTION(CALL_CONVENTION, call_convention)

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "call_convention::remove_call_convention removes the call-convention.",
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
    using namespace call_convention;

    STATIC_REQUIRE(std::same_as<Return(Args...), typename remove_call_convention<Return CALL_CONVENTION(Args...)>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...), remove_call_convention_t<Return CALL_CONVENTION(Args...)>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) const, typename remove_call_convention<Return CALL_CONVENTION(Args...) const>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) const, remove_call_convention_t<Return CALL_CONVENTION(Args...) const>>);

    STATIC_REQUIRE(std::same_as<Return(Args...)&, typename remove_call_convention<Return CALL_CONVENTION(Args...)&>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...)&, remove_call_convention_t<Return CALL_CONVENTION(Args...)&>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) const&, typename remove_call_convention<Return CALL_CONVENTION(Args...) const&>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) const&, remove_call_convention_t<Return CALL_CONVENTION(Args...) const&>>);

    STATIC_REQUIRE(std::same_as<Return(Args...)&&, typename remove_call_convention<Return CALL_CONVENTION(Args...) &&>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...)&&, remove_call_convention_t<Return CALL_CONVENTION(Args...) &&>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) const&&, typename remove_call_convention<Return CALL_CONVENTION(Args...) const&&>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) const&&, remove_call_convention_t<Return CALL_CONVENTION(Args...) const&&>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) noexcept, typename remove_call_convention<Return CALL_CONVENTION(Args...) noexcept>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) noexcept, remove_call_convention_t<Return CALL_CONVENTION(Args...) noexcept>>);

    STATIC_REQUIRE(std::same_as<Return(Args...) const noexcept, typename remove_call_convention<Return CALL_CONVENTION(Args...) const noexcept>::type>);
    STATIC_REQUIRE(std::same_as<Return(Args...) const noexcept, remove_call_convention_t<Return CALL_CONVENTION(Args...) const noexcept>>);

    STATIC_REQUIRE(std::same_as < Return(Args...) & noexcept, typename remove_call_convention < Return CALL_CONVENTION(Args...) & noexcept > ::type >);
    STATIC_REQUIRE(std::same_as < Return(Args...) & noexcept, remove_call_convention_t < Return CALL_CONVENTION(Args...) & noexcept >>);

    STATIC_REQUIRE(std::same_as < Return(Args...) const& noexcept, typename remove_call_convention<Return CALL_CONVENTION(Args...) const & noexcept>::type >);
    STATIC_REQUIRE(std::same_as < Return(Args...) const& noexcept, remove_call_convention_t < Return CALL_CONVENTION(Args...) const& noexcept >>);

    STATIC_REQUIRE(std::same_as < Return(Args...) && noexcept, typename remove_call_convention < Return CALL_CONVENTION(Args...) && noexcept > ::type >);
    STATIC_REQUIRE(std::same_as < Return(Args...) && noexcept, remove_call_convention_t < Return CALL_CONVENTION(Args...) && noexcept >>);

    STATIC_REQUIRE(std::same_as < Return(Args...) const&& noexcept, typename remove_call_convention<Return CALL_CONVENTION(Args...) const && noexcept>::type >);
    STATIC_REQUIRE(std::same_as < Return(Args...) const&& noexcept, remove_call_convention_t < Return CALL_CONVENTION(Args...) const&& noexcept >>);
}

TEMPLATE_TEST_CASE_SIG(
    "call_convention::add_call_convention adds the call-convention (if not present).",
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
    using namespace call_convention;

    SECTION("If call-convention is present, signatures stay as they are.")
    {
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...), typename add_call_convention<Return CALL_CONVENTION(Args...)>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...), add_call_convention_t<Return CALL_CONVENTION(Args...)>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const, typename add_call_convention<Return CALL_CONVENTION(Args...) const>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const, add_call_convention_t<Return CALL_CONVENTION(Args...) const>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...)&, typename add_call_convention<Return CALL_CONVENTION(Args...)&>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...)&, add_call_convention_t<Return CALL_CONVENTION(Args...)&>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const&, typename add_call_convention<Return CALL_CONVENTION(Args...) const&>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const&, add_call_convention_t<Return CALL_CONVENTION(Args...) const&>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...)&&, typename add_call_convention<Return CALL_CONVENTION(Args...) &&>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...)&&, add_call_convention_t<Return CALL_CONVENTION(Args...) &&>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const&&, typename add_call_convention<Return CALL_CONVENTION(Args...) const&&>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const&&, add_call_convention_t<Return CALL_CONVENTION(Args...) const&&>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) noexcept, typename add_call_convention<Return CALL_CONVENTION(Args...) noexcept>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) noexcept, add_call_convention_t<Return CALL_CONVENTION(Args...) noexcept>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const noexcept, typename add_call_convention<Return CALL_CONVENTION(Args...) const noexcept>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const noexcept, add_call_convention_t<Return CALL_CONVENTION(Args...) const noexcept>>);

        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) & noexcept, typename add_call_convention < Return CALL_CONVENTION(Args...) & noexcept > ::type >);
        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) & noexcept, add_call_convention_t < Return CALL_CONVENTION(Args...) & noexcept >>);

        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) const& noexcept, typename add_call_convention<Return CALL_CONVENTION(Args...) const & noexcept>::type >);
        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) const& noexcept, add_call_convention_t < Return CALL_CONVENTION(Args...) const& noexcept >>);

        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) && noexcept, typename add_call_convention < Return CALL_CONVENTION(Args...) && noexcept > ::type >);
        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) && noexcept, add_call_convention_t < Return CALL_CONVENTION(Args...) && noexcept >>);

        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) const&& noexcept, typename add_call_convention<Return CALL_CONVENTION(Args...) const && noexcept>::type >);
        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) const&& noexcept, add_call_convention_t < Return CALL_CONVENTION(Args...) const&& noexcept >>);
    }

    SECTION("If call-convention is not present, signatures get them added.")
    {
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...), typename add_call_convention<Return(Args...)>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...), add_call_convention_t<Return(Args...)>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const, typename add_call_convention<Return(Args...) const>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const, add_call_convention_t<Return(Args...) const>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...)&, typename add_call_convention<Return(Args...)&>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...)&, add_call_convention_t<Return(Args...)&>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const&, typename add_call_convention<Return(Args...) const&>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const&, add_call_convention_t<Return(Args...) const&>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...)&&, typename add_call_convention<Return(Args...) &&>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...)&&, add_call_convention_t<Return(Args...) &&>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const&&, typename add_call_convention<Return(Args...) const&&>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const&&, add_call_convention_t<Return(Args...) const&&>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) noexcept, typename add_call_convention<Return(Args...) noexcept>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) noexcept, add_call_convention_t<Return(Args...) noexcept>>);

        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const noexcept, typename add_call_convention<Return(Args...) const noexcept>::type>);
        STATIC_REQUIRE(std::same_as<Return CALL_CONVENTION(Args...) const noexcept, add_call_convention_t<Return(Args...) const noexcept>>);

        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) & noexcept, typename add_call_convention < Return(Args...) & noexcept > ::type >);
        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) & noexcept, add_call_convention_t < Return(Args...) & noexcept >>);

        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) const& noexcept, typename add_call_convention<Return(Args...) const & noexcept>::type >);
        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) const& noexcept, add_call_convention_t < Return(Args...) const& noexcept >>);

        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) && noexcept, typename add_call_convention < Return(Args...) && noexcept > ::type >);
        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) && noexcept, add_call_convention_t < Return(Args...) && noexcept >>);

        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) const&& noexcept, typename add_call_convention<Return(Args...) const && noexcept>::type >);
        STATIC_REQUIRE(std::same_as < Return CALL_CONVENTION(Args...) const&& noexcept, add_call_convention_t < Return(Args...) const&& noexcept >>);
    }
}

TEST_CASE(
    "The desired call-convention specializations have been enabled for the framework.",
    "[type_traits]")
{
    using SignatureT = void CALL_CONVENTION() const& noexcept;

    using tag_t = signature_call_convention_t<SignatureT>;
    STATIC_REQUIRE(std::same_as<call_convention::tag, tag_t>);

    using traits_t = call_convention_traits<tag_t>;
    STATIC_REQUIRE(std::same_as<call_convention::tag, traits_t::tag_t>);
    STATIC_REQUIRE(std::same_as < void() const& noexcept, traits_t::remove_call_convention_t < SignatureT >>);
    STATIC_REQUIRE(std::same_as<SignatureT, traits_t::add_call_convention_t<void() const & noexcept>>);

    struct Derived
    {
    };

    STATIC_REQUIRE(
        std::same_as<
            call_convention::CallInterface<Derived, SignatureT>,
            traits_t::call_interface_t<Derived, SignatureT>>);
}

TEST_CASE(
    "All signature traits are supported by signatures with the desired call-convention.",
    "[type_traits]")
{
    SECTION("When trait actually removes.")
    {
        using SignatureT = void CALL_CONVENTION() const& noexcept;

        STATIC_CHECK(std::same_as < void() const& noexcept, signature_remove_call_convention_t < SignatureT >>);

        STATIC_CHECK(std::same_as<void CALL_CONVENTION() const&, signature_remove_noexcept_t<SignatureT>>);
        STATIC_CHECK(signature_remove_noexcept<SignatureT>::value);
        STATIC_CHECK(std::same_as < void CALL_CONVENTION()& noexcept, signature_remove_const_qualifier_t < SignatureT >>);
        STATIC_CHECK(signature_remove_const_qualifier<SignatureT>::value);
        STATIC_CHECK(std::same_as<void CALL_CONVENTION() const noexcept, signature_remove_ref_qualifier_t<SignatureT>>);
        STATIC_CHECK(std::same_as<void(), signature_decay_t<SignatureT>>);
    }

    SECTION("When trait silently removes nothing.")
    {
        using SignatureT = void CALL_CONVENTION();

        STATIC_CHECK(std::same_as<SignatureT, signature_remove_noexcept_t<SignatureT>>);
        STATIC_CHECK(!signature_remove_noexcept<SignatureT>::value);
        STATIC_CHECK(std::same_as<SignatureT, signature_remove_const_qualifier_t<SignatureT>>);
        STATIC_CHECK(!signature_remove_const_qualifier<SignatureT>::value);
        STATIC_CHECK(std::same_as<SignatureT, signature_remove_ref_qualifier_t<SignatureT>>);
        STATIC_CHECK(std::same_as<void(), signature_decay_t<SignatureT>>);
    }

    SECTION("When trait actually adds.")
    {
        using SignatureT = void CALL_CONVENTION();

        STATIC_CHECK(std::same_as<void CALL_CONVENTION() noexcept, signature_add_noexcept_t<SignatureT>>);
        STATIC_CHECK(signature_add_noexcept<SignatureT>::value);
        STATIC_CHECK(std::same_as<void CALL_CONVENTION() const, signature_add_const_qualifier_t<SignatureT>>);
        STATIC_CHECK(signature_add_const_qualifier<SignatureT>::value);
    }

    SECTION("When trait silently adds nothing.")
    {
        using SignatureT = void CALL_CONVENTION() const& noexcept;

        STATIC_CHECK(std::same_as < void CALL_CONVENTION() const& noexcept, signature_add_noexcept_t < SignatureT >>);
        STATIC_CHECK(!signature_add_noexcept<SignatureT>::value);
        STATIC_CHECK(std::same_as < void CALL_CONVENTION() const& noexcept, signature_add_const_qualifier_t < SignatureT >>);
        STATIC_CHECK(!signature_add_const_qualifier<SignatureT>::value);
    }
}

TEST_CASE(
    "Mocks support explicit call-conventions.",
    "[mock]")
{
    SECTION("Signatures without any other specs.")
    {
        using MockT = Mock<void CALL_CONVENTION()>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (CALL_CONVENTION MockT::*)(util::SourceLocation)>);
    }

    SECTION("Signatures with noexcept.")
    {
        using MockT = Mock<void CALL_CONVENTION() noexcept>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (CALL_CONVENTION MockT::*)(util::SourceLocation) noexcept>);
    }

    SECTION("Signatures with const.")
    {
        using MockT = Mock<void CALL_CONVENTION() const>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (CALL_CONVENTION MockT::*)(util::SourceLocation) const>);
    }

    SECTION("Signatures with const noexcept.")
    {
        using MockT = Mock<void CALL_CONVENTION() const noexcept>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (CALL_CONVENTION MockT::*)(util::SourceLocation) const noexcept>);
    }

    SECTION("Signatures with &.")
    {
        using MockT = Mock<void CALL_CONVENTION()&>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (CALL_CONVENTION MockT::*)(util::SourceLocation)&>);
    }

    SECTION("Signatures with & noexcept.")
    {
        using MockT = Mock<void CALL_CONVENTION() & noexcept>;
        STATIC_REQUIRE(
            std::convertible_to < decltype(&MockT::operator()),
            void (CALL_CONVENTION MockT::*)(util::SourceLocation)& noexcept >);
    }

    SECTION("Signatures with const&.")
    {
        using MockT = Mock<void CALL_CONVENTION() const&>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (CALL_CONVENTION MockT::*)(util::SourceLocation) const&>);
    }

    SECTION("Signatures with const& noexcept.")
    {
        using MockT = Mock<void CALL_CONVENTION() const & noexcept>;
        STATIC_REQUIRE(
            std::convertible_to < decltype(&MockT::operator()),
            void (CALL_CONVENTION MockT::*)(util::SourceLocation) const& noexcept >);
    }

    SECTION("Signatures with &&.")
    {
        using MockT = Mock<void CALL_CONVENTION() &&>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (CALL_CONVENTION MockT::*)(util::SourceLocation) &&>);
    }

    SECTION("Signatures with && noexcept.")
    {
        using MockT = Mock<void CALL_CONVENTION() && noexcept>;
        STATIC_REQUIRE(
            std::convertible_to < decltype(&MockT::operator()),
            void (CALL_CONVENTION MockT::*)(util::SourceLocation)&& noexcept >);
    }

    SECTION("Signatures with const&&.")
    {
        using MockT = Mock<void CALL_CONVENTION() const&&>;
        STATIC_REQUIRE(
            std::convertible_to<
                decltype(&MockT::operator()),
                void (CALL_CONVENTION MockT::*)(util::SourceLocation) const&&>);
    }

    SECTION("Signatures with const&& noexcept.")
    {
        using MockT = Mock<void CALL_CONVENTION() const && noexcept>;
        STATIC_REQUIRE(
            std::convertible_to < decltype(&MockT::operator()),
            void (CALL_CONVENTION MockT::*)(util::SourceLocation) const&& noexcept >);
    }

    SECTION("Mocks still supports overloading.")
    {
        Mock<
            void(),
            void CALL_CONVENTION() const>
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
    "Interface-Mocks support explicit call-conventions.",
    "[mock]")
{
    SECTION("When mocking single method.")
    {
        class Interface
        {
        public:
            virtual ~Interface() = default;

            virtual void CALL_CONVENTION foo() = 0;
        };

        class Derived
            : public Interface
        {
        public:
            MIMICPP_MOCK_METHOD(foo, void, (), , CALL_CONVENTION);
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

            virtual void CALL_CONVENTION foo() = 0;
        };

        class Derived
            : public Interface
        {
        public:
            MIMICPP_MOCK_METHOD(foo, void, (), , (CALL_CONVENTION));
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

            virtual void CALL_CONVENTION foo() = 0;
            virtual void foo() const = 0;
        };

        class Derived
            : public Interface
        {
        public:
            MIMICPP_MOCK_OVERLOADED_METHOD(
                foo,
                MIMICPP_ADD_OVERLOAD(void, (), , CALL_CONVENTION),
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
