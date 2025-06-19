//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/TypeTraits.hpp"

#include <optional>

namespace
{
    template <typename>
    struct signature_helper;

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args...)>
    {
        using sig = Return(Args...);
        using sig_noexcept = Return(Args...) noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args..., ...)>
    {
        using sig = Return(Args..., ...);
        using sig_noexcept = Return(Args..., ...) noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args...) const>
    {
        using sig = Return(Args...) const;
        using sig_noexcept = Return(Args...) const noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args..., ...) const>
    {
        using sig = Return(Args..., ...) const;
        using sig_noexcept = Return(Args..., ...) const noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args...)&>
    {
        using sig = Return(Args...) &;
        using sig_noexcept = Return(Args...) & noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args..., ...)&>
    {
        using sig = Return(Args..., ...) &;
        using sig_noexcept = Return(Args..., ...) & noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args...) const&>
    {
        using sig = Return(Args...) const&;
        using sig_noexcept = Return(Args...) const& noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args..., ...) const&>
    {
        using sig = Return(Args..., ...) const&;
        using sig_noexcept = Return(Args..., ...) const& noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args...) &&>
    {
        using sig = Return(Args...) &&;
        using sig_noexcept = Return(Args...) && noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args..., ...) &&>
    {
        using sig = Return(Args..., ...) &&;
        using sig_noexcept = Return(Args..., ...) && noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args...) const&&>
    {
        using sig = Return(Args...) const&&;
        using sig_noexcept = Return(Args...) const&& noexcept;
    };

    template <typename Return, typename... Args>
    struct signature_helper<Return(Args..., ...) const&&>
    {
        using sig = Return(Args..., ...) const&&;
        using sig_noexcept = Return(Args..., ...) const&& noexcept;
    };

    template <typename Signature>
    constexpr std::type_identity<Signature> type_v{};
}

TEMPLATE_TEST_CASE_SIG(
    "signature_call_convention infers default call-convention tag for general function types.",
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
    static constexpr auto check = []<typename T>(std::type_identity<T> const) {
        using Signature = T;
        using Expected = mimicpp::detail::default_call_convention;

        STATIC_CHECK(std::same_as<Expected, typename mimicpp::signature_call_convention<Signature>::type>);
        STATIC_CHECK(std::same_as<Expected, mimicpp::signature_call_convention_t<Signature>>);
    };

    SECTION("Variadic c++ function.")
    {
        check(type_v<Return(Args...)>);
        check(type_v<Return(Args...) const>);
        check(type_v<Return(Args...)&>);
        check(type_v<Return(Args...) const&>);
        check(type_v<Return(Args...) &&>);
        check(type_v<Return(Args...) const&&>);

        check(type_v<Return(Args...) noexcept>);
        check(type_v<Return(Args...) const noexcept>);
        check(type_v < Return(Args...) & noexcept >);
        check(type_v < Return(Args...) const& noexcept >);
        check(type_v < Return(Args...) && noexcept >);
        check(type_v < Return(Args...) const&& noexcept >);
    }

    SECTION("Function with c-ellipsis.")
    {
        check(type_v<Return(Args..., ...)>);
        check(type_v<Return(Args..., ...) const>);
        check(type_v<Return(Args..., ...)&>);
        check(type_v<Return(Args..., ...) const&>);
        check(type_v<Return(Args..., ...) &&>);
        check(type_v<Return(Args..., ...) const&&>);

        check(type_v<Return(Args..., ...) noexcept>);
        check(type_v<Return(Args..., ...) const noexcept>);
        check(type_v < Return(Args..., ...) & noexcept >);
        check(type_v < Return(Args..., ...) const& noexcept >);
        check(type_v < Return(Args..., ...) && noexcept >);
        check(type_v < Return(Args..., ...) const&& noexcept >);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_remove_call_convention does nothing for signatures with default call-conv..",
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
    static constexpr auto check = []<typename T>(std::type_identity<T> const) {
        using Signature = T;

        STATIC_CHECK(std::same_as<Signature, typename mimicpp::signature_remove_call_convention<Signature>::type>);
        STATIC_CHECK(std::same_as<Signature, mimicpp::signature_remove_call_convention_t<Signature>>);
    };

    SECTION("Variadic c++ function.")
    {
        check(type_v<Return(Args...)>);
        check(type_v<Return(Args...) const>);
        check(type_v<Return(Args...)&>);
        check(type_v<Return(Args...) const&>);
        check(type_v<Return(Args...) &&>);
        check(type_v<Return(Args...) const&&>);

        check(type_v<Return(Args...) noexcept>);
        check(type_v<Return(Args...) const noexcept>);
        check(type_v < Return(Args...) & noexcept >);
        check(type_v < Return(Args...) const& noexcept >);
        check(type_v < Return(Args...) && noexcept >);
        check(type_v < Return(Args...) const&& noexcept >);
    }

    SECTION("Function with c-ellipsis.")
    {
        check(type_v<Return(Args..., ...)>);
        check(type_v<Return(Args..., ...) const>);
        check(type_v<Return(Args..., ...)&>);
        check(type_v<Return(Args..., ...) const&>);
        check(type_v<Return(Args..., ...) &&>);
        check(type_v<Return(Args..., ...) const&&>);

        check(type_v<Return(Args..., ...) noexcept>);
        check(type_v<Return(Args..., ...) const noexcept>);
        check(type_v < Return(Args..., ...) & noexcept >);
        check(type_v < Return(Args..., ...) const& noexcept >);
        check(type_v < Return(Args..., ...) && noexcept >);
        check(type_v < Return(Args..., ...) const&& noexcept >);
    }
}

TEMPLATE_TEST_CASE(
    "signature_add_noexcept adds noexcept qualifier if not already present.",
    "[type_traits]",
    void(),
    void(int),
    void(...),
    void(float, int),
    void(float, ...),
    double(),
    double(int),
    double(...),
    double(float, int),
    double(float, ...),

    void() const,
    void(int) const,
    void(...) const,
    void(float, int) const,
    void(float, ...) const,
    double() const,
    double(int) const,
    double(...) const,
    double(float, int) const,
    double(float, ...) const,

    void() &,
    void(int) &,
    void(...) &,
    void(float, int) &,
    void(float, ...) &,
    double() &,
    double(int) &,
    double(...) &,
    double(float, int) &,
    double(float, ...) &,

    void() const&,
    void(int) const&,
    void(...) const&,
    void(float, int) const&,
    void(float, ...) const&,
    double() const&,
    double(int) const&,
    double(...) const&,
    double(float, int) const&,
    double(float, ...) const&,

    void() &&,
    void(int) &&,
    void(...) &&,
    void(float, int) &&,
    void(float, ...) &&,
    double() &&,
    double(int) &&,
    double(...) &&,
    double(float, int) &&,
    double(float, ...) &&,

    void() const&&,
    void(int) const&&,
    void(...) const&&,
    void(float, int) const&&,
    void(float, ...) const&&,
    double() const&&,
    double(int) const&&,
    double(...) const&&,
    double(float, int) const&&,
    double(float, ...) const&&)
{
    using ExpectedT = typename signature_helper<TestType>::sig_noexcept;

    STATIC_CHECK(std::same_as<ExpectedT, typename mimicpp::signature_add_noexcept<TestType>::type>);
    STATIC_CHECK(mimicpp::signature_add_noexcept<TestType>::value);
    STATIC_CHECK(std::same_as<ExpectedT, mimicpp::signature_add_noexcept_t<TestType>>);

    STATIC_CHECK(std::same_as<ExpectedT, typename mimicpp::signature_add_noexcept<ExpectedT>::type>);
    STATIC_CHECK(!mimicpp::signature_add_noexcept<ExpectedT>::value);
    STATIC_CHECK(std::same_as<ExpectedT, mimicpp::signature_add_noexcept_t<ExpectedT>>);
}

TEMPLATE_TEST_CASE(
    "signature_remove_noexcept removes noexcept qualifier if present.",
    "[type_traits]",
    void(),
    void(int),
    void(...),
    void(float, int),
    void(float, ...),
    double(),
    double(int),
    double(...),
    double(float, int),
    double(float, ...),

    void() const,
    void(int) const,
    void(...) const,
    void(float, int) const,
    void(float, ...) const,
    double() const,
    double(int) const,
    double(...) const,
    double(float, int) const,
    double(float, ...) const,

    void() &,
    void(int) &,
    void(...) &,
    void(float, int) &,
    void(float, ...) &,
    double() &,
    double(int) &,
    double(...) &,
    double(float, int) &,
    double(float, ...) &,

    void() const&,
    void(int) const&,
    void(...) const&,
    void(float, int) const&,
    void(float, ...) const&,
    double() const&,
    double(int) const&,
    double(...) const&,
    double(float, int) const&,
    double(float, ...) const&,

    void() &&,
    void(int) &&,
    void(...) &&,
    void(float, int) &&,
    void(float, ...) &&,
    double() &&,
    double(int) &&,
    double(...) &&,
    double(float, int) &&,
    double(float, ...) &&,

    void() const&&,
    void(int) const&&,
    void(...) const&&,
    void(float, int) const&&,
    void(float, ...) const&&,
    double() const&&,
    double(int) const&&,
    double(...) const&&,
    double(float, int) const&&,
    double(float, ...) const&&)
{
    using SignatureT = TestType;
    STATIC_CHECK(std::same_as<SignatureT, typename mimicpp::signature_remove_noexcept<SignatureT>::type>);
    STATIC_CHECK(!mimicpp::signature_remove_noexcept<TestType>::value);
    STATIC_CHECK(std::same_as<SignatureT, mimicpp::signature_remove_noexcept_t<SignatureT>>);

    using SignatureNoexceptT = typename signature_helper<SignatureT>::sig_noexcept;
    STATIC_CHECK(std::same_as<SignatureT, typename mimicpp::signature_remove_noexcept<SignatureNoexceptT>::type>);
    STATIC_CHECK(mimicpp::signature_remove_noexcept<SignatureNoexceptT>::value);
    STATIC_CHECK(std::same_as<SignatureT, mimicpp::signature_remove_noexcept_t<SignatureNoexceptT>>);
}

TEMPLATE_TEST_CASE_SIG(
    "signature_remove_ref_qualifier removes the ref-qualification.",
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
    SECTION("Variadic c++ function.")
    {
        using SignatureT = Return(Args...);
        using SignatureNoexceptT = Return(Args...) noexcept;
        using SignatureConstT = Return(Args...) const;
        using SignatureConstNoexceptT = Return(Args...) const noexcept;

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...)>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...)>>);

        STATIC_REQUIRE(std::same_as<SignatureConstT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) const>::type>);
        STATIC_REQUIRE(std::same_as<SignatureConstT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) const>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...)&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...)&>>);

        STATIC_REQUIRE(std::same_as<SignatureConstT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) const&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureConstT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) const&>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) &&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) &&>>);

        STATIC_REQUIRE(std::same_as<SignatureConstT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) const&&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureConstT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) const&&>>);

        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) const noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) const noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) & noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) & noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) const & noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) const & noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) && noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) && noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, typename mimicpp::signature_remove_ref_qualifier<Return(Args...) const && noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args...) const && noexcept>>);
    }

    SECTION("Function with c-ellipsis.")
    {
        using SignatureT = Return(Args..., ...);
        using SignatureNoexceptT = Return(Args..., ...) noexcept;
        using SignatureConstT = Return(Args..., ...) const;
        using SignatureConstNoexceptT = Return(Args..., ...) const noexcept;

        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...)>>);
        STATIC_REQUIRE(std::same_as<SignatureConstT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) const>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...)&>>);
        STATIC_REQUIRE(std::same_as<SignatureConstT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) const&>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) &&>>);
        STATIC_REQUIRE(std::same_as<SignatureConstT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) const&&>>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) const noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) & noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) const & noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) && noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureConstNoexceptT, mimicpp::signature_remove_ref_qualifier_t<Return(Args..., ...) const && noexcept>>);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_add_const_qualifier adds const if not already present.",
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
    static constexpr auto check = []<typename ExpectAdded, typename Expected, typename Input>(
                                      std::type_identity<ExpectAdded> const,
                                      std::type_identity<Expected> const,
                                      std::type_identity<Input> const) {
        STATIC_CHECK(std::same_as<Expected, typename mimicpp::signature_add_const_qualifier<Input>::type>);
        STATIC_CHECK(ExpectAdded{} == mimicpp::signature_add_const_qualifier<Input>::value);
        STATIC_CHECK(std::same_as<Expected, mimicpp::signature_add_const_qualifier_t<Input>>);
    };

    SECTION("Variadic c++ function.")
    {
        check(type_v<std::true_type>, type_v<Return(Args...) const>, type_v<Return(Args...)>);
        check(type_v<std::true_type>, type_v<Return(Args...) const noexcept>, type_v<Return(Args...) noexcept>);
        check(type_v<std::true_type>, type_v<Return(Args...) const&>, type_v<Return(Args...)&>);
        check(type_v<std::true_type>, type_v < Return(Args...) const& noexcept >, type_v < Return(Args...) & noexcept >);
        check(type_v<std::true_type>, type_v<Return(Args...) const&&>, type_v<Return(Args...) &&>);
        check(type_v<std::true_type>, type_v < Return(Args...) const&& noexcept >, type_v < Return(Args...) && noexcept >);

        check(type_v<std::false_type>, type_v<Return(Args...) const>, type_v<Return(Args...) const>);
        check(type_v<std::false_type>, type_v<Return(Args...) const noexcept>, type_v<Return(Args...) const noexcept>);
        check(type_v<std::false_type>, type_v<Return(Args...) const&>, type_v<Return(Args...) const&>);
        check(type_v<std::false_type>, type_v < Return(Args...) const& noexcept >, type_v < Return(Args...) const& noexcept >);
        check(type_v<std::false_type>, type_v<Return(Args...) const&&>, type_v<Return(Args...) const&&>);
        check(type_v<std::false_type>, type_v < Return(Args...) const&& noexcept >, type_v < Return(Args...) const&& noexcept >);
    }

    SECTION("Function with c-ellipsis.")
    {
        check(type_v<std::true_type>, type_v<Return(Args..., ...) const>, type_v<Return(Args..., ...)>);
        check(type_v<std::true_type>, type_v<Return(Args..., ...) const noexcept>, type_v<Return(Args..., ...) noexcept>);
        check(type_v<std::true_type>, type_v<Return(Args..., ...) const&>, type_v<Return(Args..., ...)&>);
        check(type_v<std::true_type>, type_v < Return(Args..., ...) const& noexcept >, type_v < Return(Args..., ...) & noexcept >);
        check(type_v<std::true_type>, type_v<Return(Args..., ...) const&&>, type_v<Return(Args..., ...) &&>);
        check(type_v<std::true_type>, type_v < Return(Args..., ...) const&& noexcept >, type_v < Return(Args..., ...) && noexcept >);

        check(type_v<std::false_type>, type_v<Return(Args..., ...) const>, type_v<Return(Args..., ...) const>);
        check(type_v<std::false_type>, type_v<Return(Args..., ...) const noexcept>, type_v<Return(Args..., ...) const noexcept>);
        check(type_v<std::false_type>, type_v<Return(Args..., ...) const&>, type_v<Return(Args..., ...) const&>);
        check(type_v<std::false_type>, type_v < Return(Args..., ...) const& noexcept >, type_v < Return(Args..., ...) const& noexcept >);
        check(type_v<std::false_type>, type_v<Return(Args..., ...) const&&>, type_v<Return(Args..., ...) const&&>);
        check(type_v<std::false_type>, type_v < Return(Args..., ...) const&& noexcept >, type_v < Return(Args..., ...) const&& noexcept >);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_remove_const_qualifier removes the const-qualification.",
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
    SECTION("Variadic c++ function.")
    {
        using SignatureT = Return(Args...);
        using SignatureNoexceptT = Return(Args...) noexcept;
        using SignatureLValueRefT = Return(Args...)&;
        using SignatureLValueRefNoexceptT = Return(Args...)& noexcept;
        using SignatureRValueRefT = Return(Args...)&&;
        using SignatureRValueRefNoexceptT = Return(Args...)&& noexcept;

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_remove_const_qualifier<Return(Args...)>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_const_qualifier_t<Return(Args...)>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) const>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) const>>);

        STATIC_REQUIRE(std::same_as<SignatureLValueRefT, typename mimicpp::signature_remove_const_qualifier<Return(Args...)&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureLValueRefT, mimicpp::signature_remove_const_qualifier_t<Return(Args...)&>>);

        STATIC_REQUIRE(std::same_as<SignatureLValueRefT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) const&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureLValueRefT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) const&>>);

        STATIC_REQUIRE(std::same_as<SignatureRValueRefT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) &&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureRValueRefT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) &&>>);

        STATIC_REQUIRE(std::same_as<SignatureRValueRefT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) const&&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureRValueRefT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) const&&>>);

        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) const noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) const noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureLValueRefNoexceptT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) & noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureLValueRefNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) & noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureLValueRefNoexceptT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) const & noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureLValueRefNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) const & noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureRValueRefNoexceptT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) && noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureRValueRefNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) && noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureRValueRefNoexceptT, typename mimicpp::signature_remove_const_qualifier<Return(Args...) const && noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureRValueRefNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args...) const && noexcept>>);
    }

    SECTION("Function with c-ellipsis.")
    {
        using SignatureT = Return(Args..., ...);
        using SignatureNoexceptT = Return(Args..., ...) noexcept;
        using SignatureLValueRefT = Return(Args..., ...)&;
        using SignatureLValueRefNoexceptT = Return(Args..., ...)& noexcept;
        using SignatureRValueRefT = Return(Args..., ...)&&;
        using SignatureRValueRefNoexceptT = Return(Args..., ...)&& noexcept;

        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...)>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) const>>);
        STATIC_REQUIRE(std::same_as<SignatureLValueRefT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...)&>>);
        STATIC_REQUIRE(std::same_as<SignatureLValueRefT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) const&>>);
        STATIC_REQUIRE(std::same_as<SignatureRValueRefT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) &&>>);
        STATIC_REQUIRE(std::same_as<SignatureRValueRefT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) const&&>>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) const noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureLValueRefNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) & noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureLValueRefNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) const & noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureRValueRefNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) && noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureRValueRefNoexceptT, mimicpp::signature_remove_const_qualifier_t<Return(Args..., ...) const && noexcept>>);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_decay removes all modifiers.",
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
    SECTION("Variadic c++ function.")
    {
        using SignatureT = Return(Args...);
        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...)>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...)>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...)&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...)&>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const&>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) &&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) &&>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const&&>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const&&>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) & noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) & noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const & noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const & noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) && noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) && noexcept>>);

        STATIC_REQUIRE(std::same_as<SignatureT, typename mimicpp::signature_decay<Return(Args...) const && noexcept>::type>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args...) const && noexcept>>);
    }

    SECTION("Function with c-ellipsis.")
    {
        using SignatureT = Return(Args..., ...);

        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...)>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...)&>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const&>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) &&>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const&&>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) & noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const & noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) && noexcept>>);
        STATIC_REQUIRE(std::same_as<SignatureT, mimicpp::signature_decay_t<Return(Args..., ...) const && noexcept>>);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_return_type extracts the return type from the given signature.",
    "[type_traits]",
    ((bool dummy, typename Expected, typename Signature), dummy, Expected, Signature),
    (true, void, void()),
    (true, void, void(int)),
    (true, void, void(...)),
    (true, void, void(float, int)),
    (true, void, void(float, ...)),

    (true, double, double()),
    (true, double, double(int)),
    (true, double, double(...)),
    (true, double, double(float, int)),
    (true, double, double(float, ...)),

    (true, double&, double&()),
    (true, double&, double&(int)),
    (true, double&, double&(...)),
    (true, double&, double&(float, int)),
    (true, double&, double&(float, ...)),

    (true, const double&, const double&()),
    (true, const double&, const double&(int)),
    (true, const double&, const double&(...)),
    (true, const double&, const double&(float, int)),
    (true, const double&, const double&(float, ...)),

    (true, double&&, double && ()),
    (true, double&&, double && (int)),
    (true, double&&, double && (...)),
    (true, double&&, double && (float, int)),
    (true, double&&, double && (float, ...)),

    (true, const double&&, const double && ()),
    (true, const double&&, const double && (int)),
    (true, const double&&, const double && (...)),
    (true, const double&&, const double && (float, int)),
    (true, const double&&, const double && (float, ...)),

    (true, void*, void*()),
    (true, void*, void*(int)),
    (true, void*, void*(...)),
    (true, void*, void*(float, int)),
    (true, void*, void*(float, ...)),

    (true, const void*, const void*()),
    (true, const void*, const void*(int)),
    (true, const void*, const void*(...)),
    (true, const void*, const void*(float, int)),
    (true, const void*, const void*(float, ...)))
{
    STATIC_REQUIRE(
        std::same_as<
            Expected,
            typename mimicpp::signature_return_type<Signature>::type>);
    STATIC_REQUIRE(
        std::same_as<
            Expected,
            mimicpp::signature_return_type_t<Signature>>);

    STATIC_REQUIRE(
        std::same_as<
            Expected,
            typename mimicpp::signature_return_type<mimicpp::signature_add_noexcept_t<Signature>>::type>);
    STATIC_REQUIRE(
        std::same_as<
            Expected,
            mimicpp::signature_return_type_t<mimicpp::signature_add_noexcept_t<Signature>>>);
}

TEMPLATE_TEST_CASE_SIG(
    "signature_const_qualification extracts the const-qualifier from the given signature.",
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
    using mimicpp::Constness;
    using mimicpp::signature_const_qualification;

    SECTION("Variadic c++ function.")
    {
        STATIC_REQUIRE(Constness::non_const == signature_const_qualification<Return(Args...)>::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v<Return(Args...)>);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification<Return(Args...)&>::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v<Return(Args...)&>);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification<Return(Args...) &&>::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v<Return(Args...) &&>);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification<Return(Args...) noexcept>::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v<Return(Args...) noexcept>);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification < Return(Args...) & noexcept > ::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v < Return(Args...) & noexcept >);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification < Return(Args...) && noexcept > ::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v < Return(Args...) && noexcept >);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification<Return(Args...) const>::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v<Return(Args...) const>);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification<Return(Args...) const&>::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v<Return(Args...) const&>);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification<Return(Args...) const&&>::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v<Return(Args...) const&&>);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification<Return(Args...) const noexcept>::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v<Return(Args...) const noexcept>);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification < Return(Args...) const& noexcept > ::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v < Return(Args...) const& noexcept >);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification < Return(Args...) const&& noexcept > ::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v < Return(Args...) const&& noexcept >);
    }

    SECTION("Function with c-ellipsis.")
    {
        STATIC_REQUIRE(Constness::non_const == signature_const_qualification<Return(Args..., ...)>::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v<Return(Args..., ...)>);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification<Return(Args..., ...)&>::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v<Return(Args..., ...)&>);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification<Return(Args..., ...) &&>::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v<Return(Args..., ...) &&>);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification<Return(Args..., ...) noexcept>::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v<Return(Args..., ...) noexcept>);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification < Return(Args..., ...) & noexcept > ::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v < Return(Args..., ...) & noexcept >);

        STATIC_REQUIRE(Constness::non_const == signature_const_qualification < Return(Args..., ...) && noexcept > ::value);
        STATIC_REQUIRE(Constness::non_const == mimicpp::signature_const_qualification_v < Return(Args..., ...) && noexcept >);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification<Return(Args..., ...) const>::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v<Return(Args..., ...) const>);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification<Return(Args..., ...) const&>::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v<Return(Args..., ...) const&>);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification<Return(Args..., ...) const&&>::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v<Return(Args..., ...) const&&>);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification<Return(Args..., ...) const noexcept>::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v<Return(Args..., ...) const noexcept>);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification < Return(Args..., ...) const& noexcept > ::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v < Return(Args..., ...) const& noexcept >);

        STATIC_REQUIRE(Constness::as_const == signature_const_qualification < Return(Args..., ...) const&& noexcept > ::value);
        STATIC_REQUIRE(Constness::as_const == mimicpp::signature_const_qualification_v < Return(Args..., ...) const&& noexcept >);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_ref_qualification extracts the ref-qualifier from the given signature.",
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
    using mimicpp::signature_ref_qualification;
    using mimicpp::ValueCategory;

    SECTION("Variadic c++ function.")
    {
        STATIC_REQUIRE(ValueCategory::any == signature_ref_qualification<Return(Args...)>::value);
        STATIC_REQUIRE(ValueCategory::any == mimicpp::signature_ref_qualification_v<Return(Args...)>);

        STATIC_REQUIRE(ValueCategory::any == signature_ref_qualification<Return(Args...) const>::value);
        STATIC_REQUIRE(ValueCategory::any == mimicpp::signature_ref_qualification_v<Return(Args...) const>);

        STATIC_REQUIRE(ValueCategory::any == signature_ref_qualification<Return(Args...) noexcept>::value);
        STATIC_REQUIRE(ValueCategory::any == mimicpp::signature_ref_qualification_v<Return(Args...) noexcept>);

        STATIC_REQUIRE(ValueCategory::any == signature_ref_qualification<Return(Args...) const noexcept>::value);
        STATIC_REQUIRE(ValueCategory::any == mimicpp::signature_ref_qualification_v<Return(Args...) const noexcept>);

        STATIC_REQUIRE(ValueCategory::lvalue == signature_ref_qualification<Return(Args...)&>::value);
        STATIC_REQUIRE(ValueCategory::lvalue == mimicpp::signature_ref_qualification_v<Return(Args...)&>);

        STATIC_REQUIRE(ValueCategory::lvalue == signature_ref_qualification<Return(Args...) const&>::value);
        STATIC_REQUIRE(ValueCategory::lvalue == mimicpp::signature_ref_qualification_v<Return(Args...) const&>);

        STATIC_REQUIRE(ValueCategory::lvalue == signature_ref_qualification < Return(Args...) & noexcept > ::value);
        STATIC_REQUIRE(ValueCategory::lvalue == mimicpp::signature_ref_qualification_v < Return(Args...) & noexcept >);

        STATIC_REQUIRE(ValueCategory::lvalue == signature_ref_qualification < Return(Args...) const& noexcept > ::value);
        STATIC_REQUIRE(ValueCategory::lvalue == mimicpp::signature_ref_qualification_v < Return(Args...) const& noexcept >);

        STATIC_REQUIRE(ValueCategory::rvalue == signature_ref_qualification<Return(Args...) &&>::value);
        STATIC_REQUIRE(ValueCategory::rvalue == mimicpp::signature_ref_qualification_v<Return(Args...) &&>);

        STATIC_REQUIRE(ValueCategory::rvalue == signature_ref_qualification < Return(Args...) && noexcept > ::value);
        STATIC_REQUIRE(ValueCategory::rvalue == mimicpp::signature_ref_qualification_v < Return(Args...) && noexcept >);

        STATIC_REQUIRE(ValueCategory::rvalue == signature_ref_qualification<Return(Args...) const&&>::value);
        STATIC_REQUIRE(ValueCategory::rvalue == mimicpp::signature_ref_qualification_v<Return(Args...) const&&>);

        STATIC_REQUIRE(ValueCategory::rvalue == signature_ref_qualification < Return(Args...) const&& noexcept > ::value);
        STATIC_REQUIRE(ValueCategory::rvalue == mimicpp::signature_ref_qualification_v < Return(Args...) const&& noexcept >);
    }

    SECTION("Function with c-ellipsis.")
    {
        STATIC_REQUIRE(ValueCategory::any == signature_ref_qualification<Return(Args..., ...)>::value);
        STATIC_REQUIRE(ValueCategory::any == mimicpp::signature_ref_qualification_v<Return(Args..., ...)>);

        STATIC_REQUIRE(ValueCategory::any == signature_ref_qualification<Return(Args..., ...) const>::value);
        STATIC_REQUIRE(ValueCategory::any == mimicpp::signature_ref_qualification_v<Return(Args..., ...) const>);

        STATIC_REQUIRE(ValueCategory::any == signature_ref_qualification<Return(Args..., ...) noexcept>::value);
        STATIC_REQUIRE(ValueCategory::any == mimicpp::signature_ref_qualification_v<Return(Args..., ...) noexcept>);

        STATIC_REQUIRE(ValueCategory::any == signature_ref_qualification<Return(Args..., ...) const noexcept>::value);
        STATIC_REQUIRE(ValueCategory::any == mimicpp::signature_ref_qualification_v<Return(Args..., ...) const noexcept>);

        STATIC_REQUIRE(ValueCategory::lvalue == signature_ref_qualification<Return(Args..., ...)&>::value);
        STATIC_REQUIRE(ValueCategory::lvalue == mimicpp::signature_ref_qualification_v<Return(Args..., ...)&>);

        STATIC_REQUIRE(ValueCategory::lvalue == signature_ref_qualification<Return(Args..., ...) const&>::value);
        STATIC_REQUIRE(ValueCategory::lvalue == mimicpp::signature_ref_qualification_v<Return(Args..., ...) const&>);

        STATIC_REQUIRE(ValueCategory::lvalue == signature_ref_qualification < Return(Args..., ...) const& noexcept > ::value);
        STATIC_REQUIRE(ValueCategory::lvalue == mimicpp::signature_ref_qualification_v < Return(Args..., ...) const& noexcept >);

        STATIC_REQUIRE(ValueCategory::lvalue == signature_ref_qualification < Return(Args..., ...) & noexcept > ::value);
        STATIC_REQUIRE(ValueCategory::lvalue == mimicpp::signature_ref_qualification_v < Return(Args..., ...) & noexcept >);

        STATIC_REQUIRE(ValueCategory::rvalue == signature_ref_qualification < Return(Args..., ...) && noexcept > ::value);
        STATIC_REQUIRE(ValueCategory::rvalue == mimicpp::signature_ref_qualification_v < Return(Args..., ...) && noexcept >);

        STATIC_REQUIRE(ValueCategory::rvalue == signature_ref_qualification<Return(Args..., ...) &&>::value);
        STATIC_REQUIRE(ValueCategory::rvalue == mimicpp::signature_ref_qualification_v<Return(Args..., ...) &&>);

        STATIC_REQUIRE(ValueCategory::rvalue == signature_ref_qualification<Return(Args..., ...) const&&>::value);
        STATIC_REQUIRE(ValueCategory::rvalue == mimicpp::signature_ref_qualification_v<Return(Args..., ...) const&&>);

        STATIC_REQUIRE(ValueCategory::rvalue == signature_ref_qualification < Return(Args..., ...) const&& noexcept > ::value);
        STATIC_REQUIRE(ValueCategory::rvalue == mimicpp::signature_ref_qualification_v < Return(Args..., ...) const&& noexcept >);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_is_noexcept determines, whether the given signature is noexcept.",
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
    using mimicpp::signature_is_noexcept;

    SECTION("Variadic c++ function.")
    {
        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args...)>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args...)>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args...) const>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args...) const>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args...)&>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args...)&>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args...) const&>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args...) const&>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args...) &&>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args...) &&>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args...) const&&>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args...) const&&>);

        STATIC_REQUIRE(signature_is_noexcept<Return(Args...) noexcept>::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v<Return(Args...) noexcept>);

        STATIC_REQUIRE(signature_is_noexcept<Return(Args...) const noexcept>::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v<Return(Args...) const noexcept>);

        STATIC_REQUIRE(signature_is_noexcept < Return(Args...) & noexcept > ::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v < Return(Args...) & noexcept >);

        STATIC_REQUIRE(signature_is_noexcept < Return(Args...) const& noexcept > ::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v < Return(Args...) const& noexcept >);

        STATIC_REQUIRE(signature_is_noexcept < Return(Args...) && noexcept > ::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v < Return(Args...) && noexcept >);

        STATIC_REQUIRE(signature_is_noexcept < Return(Args...) const&& noexcept > ::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v < Return(Args...) const&& noexcept >);
    }

    SECTION("Function with c-ellipsis.")
    {
        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args..., ...)>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args..., ...)>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args..., ...) const>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args..., ...) const>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args..., ...)&>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args..., ...)&>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args..., ...) const&>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args..., ...) const&>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args..., ...) &&>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args..., ...) &&>);

        STATIC_REQUIRE_FALSE(signature_is_noexcept<Return(Args..., ...) const&&>::value);
        STATIC_REQUIRE_FALSE(mimicpp::signature_is_noexcept_v<Return(Args..., ...) const&&>);

        STATIC_REQUIRE(signature_is_noexcept<Return(Args..., ...) noexcept>::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v<Return(Args..., ...) noexcept>);

        STATIC_REQUIRE(signature_is_noexcept<Return(Args..., ...) const noexcept>::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v<Return(Args..., ...) const noexcept>);

        STATIC_REQUIRE(signature_is_noexcept < Return(Args..., ...) & noexcept > ::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v < Return(Args..., ...) & noexcept >);

        STATIC_REQUIRE(signature_is_noexcept < Return(Args..., ...) const& noexcept > ::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v < Return(Args..., ...) const& noexcept >);

        STATIC_REQUIRE(signature_is_noexcept < Return(Args..., ...) && noexcept > ::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v < Return(Args..., ...) && noexcept >);

        STATIC_REQUIRE(signature_is_noexcept < Return(Args..., ...) const&& noexcept > ::value);
        STATIC_REQUIRE(mimicpp::signature_is_noexcept_v < Return(Args..., ...) const&& noexcept >);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_param_type extracts the n-th param type of the given signature.",
    "[type_traits]",
    ((typename Expected, std::size_t index, typename Signature), Expected, index, Signature),
    (int, 0, void(int)),
    (int, 0, void(int, double)),
    (double, 1, void(int, double)))
{
    STATIC_REQUIRE(
        (
            std::same_as<
                Expected,
                typename mimicpp::signature_param_type<index, Signature>::type>));
    STATIC_REQUIRE(
        (
            std::same_as<
                Expected,
                mimicpp::signature_param_type_t<index, Signature>>));

    STATIC_REQUIRE(
        (
            std::same_as<
                Expected,
                typename mimicpp::signature_param_type<index, mimicpp::signature_add_noexcept_t<Signature>>::type>));
    STATIC_REQUIRE(
        (
            std::same_as<
                Expected,
                mimicpp::signature_param_type_t<index, mimicpp::signature_add_noexcept_t<Signature>>>));
}

TEMPLATE_TEST_CASE_SIG(
    "is_overloadable_with determines, whether the first signature is overloadable with the second one.",
    "[type_traits]",
    ((bool expected, typename First, typename Second), expected, First, Second),
    (false, void(), void()),
    (false, void() const, void() const),
    (false, void() &, void() &),
    (false, void() const&, void() const&),
    (false, void() &&, void() &&),
    (false, void() const&&, void() const&&),

    (false, int(), void()),
    (false, int() const, void() const),
    (false, int() &, void() &),
    (false, int() const&, void() const&),
    (false, int() &&, void() &&),
    (false, int() const&&, void() const&&),

    (false, void(), void() &),
    (false, void(), void() const&),
    (false, void(), void() &&),
    (false, void(), void() const&&),

    (false, void() const, void() &),
    (false, void() const, void() const&),
    (false, void() const, void() &&),
    (false, void() const, void() const&&),

    (true, void(), void() const),
    (true, void() &, void() const&),
    (true, void() &, void() &&),
    (true, void() &, void() const&&),
    (true, void() const&, void() &),
    (true, void() const&, void() &&),
    (true, void() const&, void() const&&),
    (true, void() &&, void() &),
    (true, void() &&, void() const&),
    (true, void() &&, void() const&&),
    (true, void() const&&, void() &),
    (true, void() const&&, void() const&),
    (true, void() const&&, void() &&),

    (true, void(), void(int)),
    (true, int(), void(int)),
    (true, void(int), void(short)))
{
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<First, Second>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<First, Second>);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<Second, First>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<Second, First>);

    using mimicpp::signature_add_noexcept_t;

    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<signature_add_noexcept_t<First>, Second>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<signature_add_noexcept_t<First>, Second>);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<Second, signature_add_noexcept_t<First>>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<Second, signature_add_noexcept_t<First>>);

    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<First, signature_add_noexcept_t<Second>>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<First, signature_add_noexcept_t<Second>>);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<signature_add_noexcept_t<Second>, First>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<signature_add_noexcept_t<Second>, First>);

    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<signature_add_noexcept_t<First>, signature_add_noexcept_t<Second>>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<signature_add_noexcept_t<First>, signature_add_noexcept_t<Second>>);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with<signature_add_noexcept_t<Second>, signature_add_noexcept_t<First>>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overloadable_with_v<signature_add_noexcept_t<Second>, signature_add_noexcept_t<First>>);
}

TEMPLATE_TEST_CASE_SIG(
    "is_overload_set determines, whether given signatures are valid overloads of each other.",
    "[type_traits]",
    ((bool expected, typename First, typename... Others), expected, First, Others...),
    (false, void(), int()),
    (false, void(), void() const&),
    (false, void() &, void() const&, int() &, void() &&, void() const&&),

    (true, void()),
    (true, void(), void() const),
    (true, void() &, void() const&, void() &&, void() const&&),
    (true, short(int), void() &, void() const&, void() &&, void() const&&))
{
    STATIC_REQUIRE(expected == mimicpp::is_overload_set<First, Others...>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overload_set_v<First, Others...>);

    STATIC_REQUIRE(expected == mimicpp::is_overload_set<Others..., First>::value);
    STATIC_REQUIRE(expected == mimicpp::is_overload_set_v<Others..., First>);
}

TEMPLATE_TEST_CASE_SIG(
    "uint_with_size yields a uint type with the expected size.",
    "[type_traits]",
    ((typename Expected, std::size_t size), Expected, size),
    (std::uint8_t, 1),
    (std::uint16_t, 2),
    (std::uint32_t, 4),
    (std::uint64_t, 8))
{
    STATIC_CHECK(size == sizeof(Expected));
    STATIC_REQUIRE(std::same_as<Expected, typename mimicpp::uint_with_size<size>::type>);
    STATIC_REQUIRE(std::same_as<Expected, mimicpp::uint_with_size_t<size>>);
}
