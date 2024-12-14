// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CALL_CONVENTION_HPP
#define MIMICPP_CALL_CONVENTION_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/TypeTraits.hpp"

#include <concepts>
#include <source_location>
#include <tuple>

namespace mimicpp
{
    /**
     * \defgroup CALL_CONVENTIONS call-conventions
     * \ingroup MOCK
     * \brief Contains helper macros, which lets users register any desired call-convention.
     * \details Call-conventions are used to communicate to the compiler, how it should lay out certain function-calls
     * (e.g. whether the caller or callee is responsible for cleaning up the stack-frame). As call-conventions are by
     * no means an official c++-feature, it's very hard to find a portable solution. Due to this, ``mimic++`` lets users
     * register the call-conventions they require by themselves (e.g. the for the microsoft ``COM``-framework).
     * \see \ref MIMICPP_REGISTER_CALL_CONVENTION "MIMICPP_REGISTER_CALL_CONVENTION"
     *
     * \note Even if compilers accept a call-convention, it doesn't necessarily have an effect on the function
     * (e.g. the ``msvc`` ignores ``__stdcall`` on ``x86_64`` architectures).
     * In these cases, ``mimic++`` can not properly distinguish between the default call-convention or ignored ones.
     * It's then the best to perform no registration for that particular call-convention at all.
     * As compilers do simply ignore any appearance of these call-conventions, they can be still applied on the mock signatures.
     * \see https://learn.microsoft.com/en-us/cpp/cpp/stdcall
     *
     * \details The following snippet registers the ``__stdcall`` call-convention. This should usually be done once
     * per test-executable.
     * \snippet RegisterCallConvention.cpp register __stdcall
     *
     * \details Every registered call-convention can be easily applied on mocks.
     * \snippet RegisterCallConvention.cpp mock __stdcall
     *
     * \details Interface mocks are also aware of any registered call-convention.
     * \snippet RegisterCallConvention.cpp mock interface __stdcall
     */

    /**
     * \defgroup CALL_CONVENTIONS_DETAIL detail
     * \ingroup CALL_CONVENTIONS
     * \brief Contains several macros, used for call-convention internals.
     * \attention These macros should never be used directly by users.
     */
}

/**
 * \brief Generates the desired ``remove_call_convention`` trait-specialization.
 * \ingroup CALL_CONVENTIONS_DETAIL
 * \param call_convention The convention to be removed.
 * \param specs All other function specifications (e.g. ``const`` and ``noexcept``).
 * \attention Must be used from within the desired namespace.
 */
#define MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, specs) \
    template <typename Return, typename... Params>                           \
    struct remove_call_convention<Return call_convention(Params...) specs>   \
    {                                                                        \
        using type = Return(Params...) specs;                                \
    }

/**
 * \brief Generates the desired ``add_call_convention`` trait-specialization.
 * \ingroup CALL_CONVENTIONS_DETAIL
 * \param call_convention The convention to be added.
 * \param specs All other function specifications (e.g. ``const`` and ``noexcept``).
 * \attention Must be used from within the desired namespace.
 */
#define MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, specs) \
    template <typename Return, typename... Params>                        \
    struct add_call_convention<Return(Params...) specs>                   \
    {                                                                     \
        using type = Return call_convention(Params...) specs;             \
    }

/**
 * \brief Generates the desired ``CallInterface``-specialization.
 * \ingroup CALL_CONVENTIONS_DETAIL
 * \param call_convention The used call-convention.
 * \param specs All other function specifications (e.g. ``const`` and ``noexcept``).
 */
#define MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, specs)      \
    template <typename Derived, typename Return, typename... Params>                      \
    class CallInterface<                                                                  \
        Derived,                                                                          \
        Return call_convention(Params...) specs>                                          \
    {                                                                                     \
    public:                                                                               \
        constexpr Return call_convention operator()(                                      \
            Params... params,                                                             \
            const ::std::source_location& from = ::std::source_location::current()) specs \
        {                                                                                 \
            return static_cast<const Derived&>(*this)                                     \
                .handle_call(::std::tuple{::std::ref(params)...}, from);                  \
        }                                                                                 \
    }

/**
 * \brief Generates the all required specializations.
 * \ingroup CALL_CONVENTIONS_DETAIL
 * \param call_convention The used call-convention.
 * \param specs All other function specifications (e.g. ``const`` and ``noexcept``).
 */
#define MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, specs) \
    MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, specs);             \
    MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, specs);                \
    MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, specs)

/**
 * \brief Registers the desired call-convention.
 * \ingroup CALL_CONVENTIONS
 * \param call_convention The desired call-convention.
 * \param namespace_name The namespace, in which all specializations will be defined.
 * \details For example, this makes ``mimic++`` aware of the ``__stdcall`` call-convention.
 * \snippet RegisterCallConvention.cpp register __stdcall
 */
#define MIMICPP_REGISTER_CALL_CONVENTION(call_convention, namespace_name)                                  \
    namespace namespace_name                                                                               \
    {                                                                                                      \
        struct tag                                                                                         \
        {                                                                                                  \
        };                                                                                                 \
                                                                                                           \
        inline constexpr bool is_default_call_convention = ::std::same_as<void(), void call_convention()>; \
                                                                                                           \
        template <typename Signature>                                                                      \
        struct remove_call_convention;                                                                     \
                                                                                                           \
        template <typename Signature>                                                                      \
        using remove_call_convention_t = typename remove_call_convention<Signature>::type;                 \
                                                                                                           \
        template <typename Signature>                                                                      \
        concept has_call_convention = !std::same_as<Signature, remove_call_convention_t<Signature>>;       \
                                                                                                           \
        template <typename Signature>                                                                      \
        struct add_call_convention;                                                                        \
                                                                                                           \
        template <typename Signature>                                                                      \
        using add_call_convention_t = typename add_call_convention<Signature>::type;                       \
                                                                                                           \
        template <has_call_convention Signature>                                                           \
        struct add_call_convention<Signature>                                                              \
        {                                                                                                  \
            using type = Signature;                                                                        \
        };                                                                                                 \
                                                                                                           \
        template <typename Derived, typename Signature>                                                    \
        class CallInterface;                                                                               \
                                                                                                           \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, );                          \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, noexcept);                  \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, const);                     \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, const noexcept);            \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, &);                         \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, & noexcept);                \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, const&);                    \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, const& noexcept);           \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, &&);                        \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, && noexcept);               \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, const&&);                   \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_SPECIALIZATIONS(call_convention, const&& noexcept);          \
    }                                                                                                      \
                                                                                                           \
    template <::namespace_name::has_call_convention Signature>                                             \
        requires(!::namespace_name::is_default_call_convention)                                            \
    struct ::mimicpp::signature_call_convention<Signature>                                                 \
    {                                                                                                      \
        using type = ::namespace_name::tag;                                                                \
    };                                                                                                     \
                                                                                                           \
    /* In cases, where the call-convention is the default, we still want to get this tag,                  \
    because it's not guaranteed, that it will be applied on member functions (e.g. __vectorcall).          \
    Due to this, we must explicitly mark the call-interface operators.*/                                   \
    template <::mimicpp::has_default_call_convention Signature>                                            \
        requires ::namespace_name::is_default_call_convention                                              \
    struct ::mimicpp::signature_call_convention<Signature>                                                 \
    {                                                                                                      \
        using type = ::namespace_name::tag;                                                                \
    };                                                                                                     \
                                                                                                           \
    template <>                                                                                            \
    struct ::mimicpp::call_convention_traits<::namespace_name::tag>                                        \
    {                                                                                                      \
        using tag_t = ::namespace_name::tag;                                                               \
                                                                                                           \
        template <typename Signature>                                                                      \
        using remove_call_convention_t = ::namespace_name::remove_call_convention_t<Signature>;            \
                                                                                                           \
        template <typename Signature>                                                                      \
        using add_call_convention_t = ::namespace_name::add_call_convention_t<Signature>;                  \
                                                                                                           \
        template <typename Derived, typename Signature>                                                    \
        using call_interface_t = ::namespace_name::CallInterface<Derived, Signature>;                      \
    }

#endif
