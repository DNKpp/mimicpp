// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CALL_CONVENTIONS_VECTORCALL_HPP
#define MIMICPP_CALL_CONVENTIONS_VECTORCALL_HPP

#pragma once

#include "mimic++/TypeTraits.hpp"

#include <source_location>

static_assert(
    0u != sizeof(void(__vectorcall*)()),
    "Your compiler doesn't support __vectorcall. Do not include this file!");

namespace mimicpp::vectorcall_call_convention
{
    /**
     * \defgroup TYPE_TRAITS_VECTORCALL_CALL_CONVENTION vectorcall call-convention
     * \ingroup TYPE_TRAITS
     * \brief Contains traits specializations related to the ``__vectorcall`` call-convention.
     * \see https://learn.microsoft.com/en-us/cpp/cpp/vectorcall
     * \note ``__vectorcall`` is not supported on variadic functions, thus all specializations for c-ellipsis are omitted.
     */

    struct tag
    {
    };

    inline constexpr bool is_default_call_convention = std::same_as<void(), void __vectorcall()>;

    /**
     * \defgroup TYPE_TRAITS_VECTORCALL_CALL_CONVENTION_REMOVE_CALL_CONVENTION remove_call_convention
     * \ingroup TYPE_TRAITS_VECTORCALL_CALL_CONVENTION
     * \brief Removes the ``__stdcall`` call-convention from the given signature.
     *
     *\{
     */

    template <typename Signature>
    struct remove_call_convention;

    template <typename Signature>
    using remove_call_convention_t = typename remove_call_convention<Signature>::type;

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...)>
    {
        using type = Return(Params...);
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) noexcept>
    {
        using type = Return(Params...) noexcept;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) const>
    {
        using type = Return(Params...) const;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) const noexcept>
    {
        using type = Return(Params...) const noexcept;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...)&>
    {
        using type = Return(Params...) &;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) & noexcept>
    {
        using type = Return(Params...) & noexcept;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) const&>
    {
        using type = Return(Params...) const&;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) const & noexcept>
    {
        using type = Return(Params...) const& noexcept;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) &&>
    {
        using type = Return(Params...) &&;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) && noexcept>
    {
        using type = Return(Params...) && noexcept;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) const&&>
    {
        using type = Return(Params...) const&&;
    };

    template <typename Return, typename... Params>
    struct remove_call_convention<Return __vectorcall(Params...) const && noexcept>
    {
        using type = Return(Params...) const&& noexcept;
    };

    /**
     * \}
     */

    template <typename Signature>
    concept has_call_convention = !std::same_as<
        Signature,
        remove_call_convention_t<Signature>>;

    /**
     * \defgroup TYPE_TRAITS_VECTORCALL_CALL_CONVENTION_ADD_CALL_CONVENTION add_call_convention
     * \ingroup TYPE_TRAITS_VECTORCALL_CALL_CONVENTION
     * \brief Adds the ``__stdcall`` call-convention to the given signature if no other is specified.
     *
     *\{
     */

    template <typename Signature>
    struct add_call_convention;

    template <typename Signature>
    using add_call_convention_t = typename add_call_convention<Signature>::type;

    template <has_call_convention Signature>
    struct add_call_convention<Signature>
    {
        using type = Signature;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...)>
    {
        using type = Return __vectorcall(Params...);
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) noexcept>
    {
        using type = Return __vectorcall(Params...) noexcept;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) const>
    {
        using type = Return __vectorcall(Params...) const;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) const noexcept>
    {
        using type = Return __vectorcall(Params...) const noexcept;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...)&>
    {
        using type = Return __vectorcall(Params...) &;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) & noexcept>
    {
        using type = Return __vectorcall(Params...) & noexcept;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) const&>
    {
        using type = Return __vectorcall(Params...) const&;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) const & noexcept>
    {
        using type = Return __vectorcall(Params...) const& noexcept;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) &&>
    {
        using type = Return __vectorcall(Params...) &&;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) && noexcept>
    {
        using type = Return __vectorcall(Params...) && noexcept;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) const&&>
    {
        using type = Return __vectorcall(Params...) const&&;
    };

    template <typename Return, typename... Params>
    struct add_call_convention<Return(Params...) const && noexcept>
    {
        using type = Return __vectorcall(Params...) const&& noexcept;
    };

    /**
     * \}
     */

    template <
        typename Derived,
        typename Signature,
        Constness constQualifier = signature_const_qualification_v<Signature>,
        ValueCategory refQualifier = signature_ref_qualification_v<Signature>,
        typename ParamList = signature_param_list_t<Signature>>
    class VectorcallCallInterface;

    template <typename Derived, typename Signature, typename... Params>
    class VectorcallCallInterface<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::any,
        std::tuple<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> __vectorcall operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class VectorcallCallInterface<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::any,
        std::tuple<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> __vectorcall operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class VectorcallCallInterface<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::lvalue,
        std::tuple<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> __vectorcall operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) & noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class VectorcallCallInterface<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::lvalue,
        std::tuple<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> __vectorcall operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const& noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class VectorcallCallInterface<
        Derived,
        Signature,
        Constness::non_const,
        ValueCategory::rvalue,
        std::tuple<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> __vectorcall operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) && noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    template <typename Derived, typename Signature, typename... Params>
    class VectorcallCallInterface<
        Derived,
        Signature,
        Constness::as_const,
        ValueCategory::rvalue,
        std::tuple<Params...>>
    {
    public:
        constexpr signature_return_type_t<Signature> __vectorcall operator()(
            Params... params,
            const std::source_location& from = std::source_location::current()) const&& noexcept(signature_is_noexcept_v<Signature>)
        {
            return static_cast<const Derived&>(*this)
                .handle_call(std::tuple{std::ref(params)...}, from);
        }
    };

    /**
     * \}
     */
}

// In cases, where __vectorcall is not the default, the detection is straight-forward.
template <mimicpp::vectorcall_call_convention::has_call_convention Signature>
    requires(!mimicpp::vectorcall_call_convention::is_default_call_convention)
struct mimicpp::signature_call_convention<Signature>
{
    using type = vectorcall_call_convention::tag;
};

// In cases, where __vectorcall is the default, we still want to get this tag,
// because (at least on msvc) __vectorcall is only applied to non-member functions by default.
// Due to this, we must explicitly mark the mock invoke operators as __vectorcall.
template <mimicpp::has_default_call_convention Signature>
    requires mimicpp::vectorcall_call_convention::is_default_call_convention
struct mimicpp::signature_call_convention<Signature>
{
    using type = vectorcall_call_convention::tag;
};

template <>
struct mimicpp::call_convention_traits<mimicpp::vectorcall_call_convention::tag>
{
    using tag_t = vectorcall_call_convention::tag;

    template <typename Signature>
    using remove_call_convention_t = vectorcall_call_convention::remove_call_convention_t<Signature>;

    template <typename Signature>
    using add_call_convention_t = vectorcall_call_convention::add_call_convention_t<Signature>;

    template <typename Derived, typename Signature>
    using call_interface_t = vectorcall_call_convention::VectorcallCallInterface<Derived, Signature>;
};

#endif
