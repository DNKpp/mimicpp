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

#define MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, specs) \
    template <typename Return, typename... Params>                           \
    struct remove_call_convention<Return call_convention(Params...) specs>   \
    {                                                                        \
        using type = Return(Params...) specs;                                \
    }

#define MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, specs) \
    template <typename Return, typename... Params>                        \
    struct add_call_convention<Return(Params...) specs>                   \
    {                                                                     \
        using type = Return call_convention(Params...) specs;             \
    }

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
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, );                                   \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, noexcept);                           \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, const);                              \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, const noexcept);                     \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, &);                                  \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, & noexcept);                         \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, const&);                             \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, const& noexcept);                    \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, &&);                                 \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, && noexcept);                        \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, const&&);                            \
        MIMICPP_DETAIL_DEFINE_REMOVE_CALL_CONVENTION(call_convention, const&& noexcept);                   \
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
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, );                                      \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, noexcept);                              \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, const);                                 \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, const noexcept);                        \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, &);                                     \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, & noexcept);                            \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, const&);                                \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, const& noexcept);                       \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, &&);                                    \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, && noexcept);                           \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, const&&);                               \
        MIMICPP_DETAIL_DEFINE_ADD_CALL_CONVENTION(call_convention, const&& noexcept);                      \
                                                                                                           \
        template <typename Derived, typename Signature>                                                    \
        class CallInterface;                                                                               \
                                                                                                           \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, );                           \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, noexcept);                   \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, const);                      \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, const noexcept);             \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, &);                          \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, & noexcept);                 \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, const&);                     \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, const& noexcept);            \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, &&);                         \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, && noexcept);                \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, const&&);                    \
        MIMICPP_DETAIL_DEFINE_CALL_CONVENTION_CALL_INTERFACE(call_convention, const&& noexcept);           \
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

MIMICPP_REGISTER_CALL_CONVENTION(__cdecl, cdecl_call_convention);

#endif
