//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "Common.hpp"
#include "mimic++/TypeTraits.hpp"

TEMPLATE_TEST_CASE_SIG(
    "signature_add_lvalue_ref_qualifier adds lvalue-ref if not already present.",
    "[type_traits]",
    ((bool dummy, typename Return, typename... Args), dummy, Return, Args...),
    TEST_SIGNATURE_COLLECTION)
{
    static constexpr auto check = []<typename ExpectAdded, typename Expected, typename Input>(
                                      std::type_identity<ExpectAdded> const,
                                      std::type_identity<Expected> const,
                                      std::type_identity<Input> const) {
        STATIC_CHECK(std::same_as<Expected, typename mimicpp::signature_add_lvalue_ref_qualifier<Input>::type>);
        STATIC_CHECK(ExpectAdded{} == mimicpp::signature_add_lvalue_ref_qualifier<Input>::value);
        STATIC_CHECK(std::same_as<Expected, mimicpp::signature_add_lvalue_ref_qualifier_t<Input>>);
    };

    SECTION("Variadic c++ function.")
    {
        check(type_v<std::true_type>, type_v<Return(Args...)&>, type_v<Return(Args...)>);
        check(type_v<std::true_type>, type_v < Return(Args...) & noexcept >, type_v<Return(Args...) noexcept>);
        check(type_v<std::true_type>, type_v<Return(Args...) const&>, type_v<Return(Args...) const>);
        check(type_v<std::true_type>, type_v < Return(Args...) const& noexcept >, type_v<Return(Args...) const noexcept>);

        check(type_v<std::false_type>, type_v<Return(Args...)&>, type_v<Return(Args...)&>);
        check(type_v<std::false_type>, type_v < Return(Args...) & noexcept >, type_v < Return(Args...) & noexcept >);
        check(type_v<std::false_type>, type_v<Return(Args...) const&>, type_v<Return(Args...) const&>);
        check(type_v<std::false_type>, type_v < Return(Args...) const& noexcept >, type_v < Return(Args...) const& noexcept >);
    }

    SECTION("Function with c-ellipsis.")
    {
        check(type_v<std::true_type>, type_v<Return(Args..., ...)&>, type_v<Return(Args..., ...)>);
        check(type_v<std::true_type>, type_v < Return(Args..., ...) & noexcept >, type_v<Return(Args..., ...) noexcept>);
        check(type_v<std::true_type>, type_v<Return(Args..., ...) const&>, type_v<Return(Args..., ...) const>);
        check(type_v<std::true_type>, type_v < Return(Args..., ...) const& noexcept >, type_v<Return(Args..., ...) const noexcept>);

        check(type_v<std::false_type>, type_v<Return(Args..., ...)&>, type_v<Return(Args..., ...)&>);
        check(type_v<std::false_type>, type_v < Return(Args..., ...) & noexcept >, type_v < Return(Args..., ...) & noexcept >);
        check(type_v<std::false_type>, type_v<Return(Args..., ...) const&>, type_v<Return(Args..., ...) const&>);
        check(type_v<std::false_type>, type_v < Return(Args..., ...) const& noexcept >, type_v < Return(Args..., ...) const& noexcept >);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "signature_add_rvalue_ref_qualifier adds rvalue-ref if not already present.",
    "[type_traits]",
    ((bool dummy, typename Return, typename... Args), dummy, Return, Args...),
    TEST_SIGNATURE_COLLECTION)
{
    static constexpr auto check = []<typename ExpectAdded, typename Expected, typename Input>(
                                      std::type_identity<ExpectAdded> const,
                                      std::type_identity<Expected> const,
                                      std::type_identity<Input> const) {
        STATIC_CHECK(std::same_as<Expected, typename mimicpp::signature_add_rvalue_ref_qualifier<Input>::type>);
        STATIC_CHECK(ExpectAdded{} == mimicpp::signature_add_rvalue_ref_qualifier<Input>::value);
        STATIC_CHECK(std::same_as<Expected, mimicpp::signature_add_rvalue_ref_qualifier_t<Input>>);
    };

    SECTION("Variadic c++ function.")
    {
        check(type_v<std::true_type>, type_v<Return(Args...) &&>, type_v<Return(Args...)>);
        check(type_v<std::true_type>, type_v < Return(Args...) && noexcept >, type_v<Return(Args...) noexcept>);
        check(type_v<std::true_type>, type_v<Return(Args...) const&&>, type_v<Return(Args...) const>);
        check(type_v<std::true_type>, type_v < Return(Args...) const&& noexcept >, type_v<Return(Args...) const noexcept>);

        check(type_v<std::false_type>, type_v<Return(Args...) &&>, type_v<Return(Args...) &&>);
        check(type_v<std::false_type>, type_v < Return(Args...) && noexcept >, type_v < Return(Args...) && noexcept >);
        check(type_v<std::false_type>, type_v<Return(Args...) const&&>, type_v<Return(Args...) const&&>);
        check(type_v<std::false_type>, type_v < Return(Args...) const&& noexcept >, type_v < Return(Args...) const&& noexcept >);
    }

    SECTION("Function with c-ellipsis.")
    {
        check(type_v<std::true_type>, type_v<Return(Args..., ...) &&>, type_v<Return(Args..., ...)>);
        check(type_v<std::true_type>, type_v < Return(Args..., ...) && noexcept >, type_v<Return(Args..., ...) noexcept>);
        check(type_v<std::true_type>, type_v<Return(Args..., ...) const&&>, type_v<Return(Args..., ...) const>);
        check(type_v<std::true_type>, type_v < Return(Args..., ...) const&& noexcept >, type_v<Return(Args..., ...) const noexcept>);

        check(type_v<std::false_type>, type_v<Return(Args..., ...) &&>, type_v<Return(Args..., ...) &&>);
        check(type_v<std::false_type>, type_v < Return(Args..., ...) && noexcept >, type_v < Return(Args..., ...) && noexcept >);
        check(type_v<std::false_type>, type_v<Return(Args..., ...) const&&>, type_v<Return(Args..., ...) const&&>);
        check(type_v<std::false_type>, type_v < Return(Args..., ...) const&& noexcept >, type_v < Return(Args..., ...) const&& noexcept >);
    }
}
