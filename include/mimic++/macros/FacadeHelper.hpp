//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MACROS_FACADE_HELPER_HPP
#define MIMICPP_MACROS_FACADE_HELPER_HPP

#pragma once

#include "mimic++/macros/Common.hpp"

namespace mimicpp::facade
{
    /**
     * \defgroup FACADE_DETAIL detail
     * \ingroup FACADE
     * \brief Contains several macros, used for facade implementation.
     * \attention Users should never use these macros directly.
     */

    /**
     * \defgroup FACADE_DETAIL_MAKE_SIGNATURE_LIST make_signature_list
     * \ingroup FACADE_DETAIL
     * \brief Converts all given arguments to a signature.
     */
}

/**
 * \brief Converts the given information to a single signature.
 * \ingroup FACADE_DETAIL_MAKE_SIGNATURE_LIST
 * \param sequence Unused.
 * \param bound_data Unused.
 * \param call_convention The call-convention.
 * \param ret The return type.
 * \param param_type_list The parameter types.
 * \param specs Additional specs (e.g. ``const``, ``noexcept``).
 */
#define MIMICPP_DETAIL_MAKE_SIGNATURE(sequence, bound_data, ret, call_convention, param_type_list, specs, ...) \
    ::mimicpp::facade::detail::apply_normalized_specs_t<                                                       \
        MIMICPP_DETAIL_STRIP_PARENS(ret) call_convention param_type_list,                                      \
        ::mimicpp::util::StaticString{#specs}>

/**
 * \brief Converts all given arguments to a signature list (not enclosed by parentheses).
 * \ingroup FACADE_DETAIL_MAKE_SIGNATURE_LIST
 */
#define MIMICPP_DETAIL_MAKE_SIGNATURE_LIST(...) \
    MIMICPP_DETAIL_FOR_EACH_EXT(                \
        MIMICPP_DETAIL_MAKE_SIGNATURE,          \
        ,                                       \
        MIMICPP_DETAIL_COMMA_DELIMITER,         \
        MIMICPP_DETAIL_STRIP_PARENS,            \
        ,                                       \
        __VA_ARGS__)

/**
 * \brief Creates the target object with the specified signatures.
 * \ingroup FACADE_DETAIL
 * \param traits The interface traits.
 * \param target_name The target name.
 * \param fn_name The function name.
 * \param linkage The linkage specifier(s).
 * \param signatures The given signatures. Enclosing parentheses will be stripped.
 */
#define MIMICPP_DETAIL_MAKE_FACADE_TARGET(traits, target_name, fn_name, linkage, signatures)  \
    linkage typename traits::target_type<MIMICPP_DETAIL_STRIP_PARENS(signatures)> target_name \
    {                                                                                         \
        [&]<typename T = traits>() {                                                          \
            if constexpr (::mimicpp::facade::detail::is_member_v<T>)                          \
            {                                                                                 \
                return T::make_settings(this, #fn_name);                                      \
            }                                                                                 \
            else                                                                              \
            {                                                                                 \
                return T::make_settings(#fn_name);                                            \
            }                                                                                 \
        }()                                                                                   \
    }

namespace mimicpp::facade
{
    /**
     * \defgroup FACADE_DETAIL_MAKE_PARAM_LIST make_param_list
     * \ingroup FACADE_DETAIL
     * \brief Converts all given arguments to a parameter-list.
     */
}

/**
 * \brief Converts the given information to a single parameter.
 * \ingroup FACADE_DETAIL_MAKE_PARAM_LIST
 * \param sequence A unique sequence, which will be appended to the parameter name (as suffix).
 * \param bound_data Unused.
 * \param type The type of the parameter. Enclosing parentheses will be stripped.
 */
#define MIMICPP_DETAIL_MAKE_PARAM(sequence, bound_data, type) MIMICPP_DETAIL_STRIP_PARENS(type) arg_##sequence

/**
 * \brief Converts all given parameters to a parameter-list (not enclosed by parentheses).
 * \ingroup FACADE_DETAIL_MAKE_PARAM_LIST
 */
#define MIMICPP_DETAIL_MAKE_PARAM_LIST(...) \
    MIMICPP_DETAIL_FOR_EACH_EXT(            \
        MIMICPP_DETAIL_MAKE_PARAM,          \
        i,                                  \
        MIMICPP_DETAIL_COMMA_DELIMITER,     \
        MIMICPP_DETAIL_IDENTITY,            \
        ,                                   \
        __VA_ARGS__)

namespace mimicpp::facade
{
    /**
     * \defgroup FACADE_DETAIL_FORWARD_ARGS_AS_TUPLE forward_args
     * \ingroup FACADE_DETAIL
     * \brief Creates comma-separated forwarding `std::tuple`s for each given argument (not enclosed by parentheses).
     * \details
     * The whole purpose of these macros is to somehow generate forwarding references from the given context.
     * The first version just applied a `std::forward` call onto each argument, but this wasn't enough to support parameter-packs.
     *
     * As parameter-packs are applied in the form `T...`,
     * the `MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE` macro must somehow handle this kind of argument.
     * There is no way to distinguish via trait, whether the `param_type` is a type or pack,
     * so we cannot simply apply an expanding `...` when required.
     * So the question is, how to unify the handling?
     *
     * The solution here is to pass both, either types or packs, into a tuple and then make use of the (possibly) simultaneous expansion feature.
     * When the param is a type, there is only one pack (the tuple with exactly one argument) in the scope.
     * When the param is a pack, both will be expanded simultaneously, because they appear in the same pattern.
     *
     * \see https://en.cppreference.com/w/cpp/language/pack
     * \see https://dnkpp.github.io/2024-12-15-simultaneous-pack-expansion-inside-macros/
     */
}

/**
 * \brief Creates a forwarding `std::tuple` for the given argument.
 * \ingroup FACADE_DETAIL_FORWARD_ARGS_AS_TUPLE
 * \param sequence A unique sequence, which will be appended to the parameter name (as suffix).
 * \param bound_data Unused.
 * \param param_type The type of the parameter. Enclosing parentheses will be stripped.
 */
#define MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE(sequence, bound_data, param_type)                    \
    [&]<typename... Type>([[maybe_unused]] ::mimicpp::util::type_list<Type...> const) noexcept { \
        return ::std::forward_as_tuple(::std::forward<Type>(arg_##sequence)...);                 \
    }(::mimicpp::util::type_list<MIMICPP_DETAIL_STRIP_PARENS(param_type)>{})

/**
 * \brief Creates forwarding `std::tuple`s for each given argument (not enclosed by parentheses).
 * \ingroup FACADE_DETAIL_FORWARD_ARGS_AS_TUPLE
 */
#define MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(...) \
    MIMICPP_DETAIL_FOR_EACH_EXT(                  \
        MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE,      \
        i,                                        \
        MIMICPP_DETAIL_COMMA_DELIMITER,           \
        MIMICPP_DETAIL_IDENTITY,                  \
        ,                                         \
        __VA_ARGS__)

namespace mimicpp
{
    /**
     * \defgroup FACADE_DETAIL_MAKE_OVERLOAD_INFOS make_overload_infos
     * \ingroup FACADE_DETAIL
     * \brief Related functions for MIMICPP_ADD_OVERLOAD.
     */
}

/**
 * \brief Base overload, extending the overload info (enclosed by parentheses).
 * \ingroup FACADE_DETAIL_MAKE_OVERLOAD_INFOS
 * \param ret The return type.
 * \param param_type_list The parameter types.
 * \param specs An optional parameter for categories (e.g. `const`, `noexcept`, etc.).
 * \param call_convention An optional parameter for the used call-convention.
 * \details Strips all optional parenthesizes from the arguments.
 * \note No parens will be stripped from `ret`, because a return type may contain commas (e.g. `std::tuple<int, int`).
 * This must be done in a later step.
 */
#define MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_ALL(ret, param_type_list, specs, call_convention, ...) \
    (                                                                                             \
        ret,                                                                                      \
        MIMICPP_DETAIL_STRIP_PARENS(call_convention),                                             \
        param_type_list,                                                                          \
        MIMICPP_DETAIL_STRIP_PARENS(specs),                                                       \
        (MIMICPP_DETAIL_MAKE_PARAM_LIST(MIMICPP_DETAIL_STRIP_PARENS(param_type_list))),           \
        (MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(MIMICPP_DETAIL_STRIP_PARENS(param_type_list))))

/**
 * \brief Simple overload, extending the overload info (enclosed by parentheses).
 * \ingroup FACADE_DETAIL_MAKE_OVERLOAD_INFOS
 * \param ret The return type.
 * \param param_type_list The parameter types.
 * \param specs An optional parameter for categories (e.g. `const`, `noexcept`, etc.).
 */
#define MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_SPECS(ret, param_type_list, specs, ...) \
    MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_ALL(ret, param_type_list, specs, )

/**
 * \brief Simple overload, extending the overload info (enclosed by parentheses).
 * \ingroup FACADE_DETAIL_MAKE_OVERLOAD_INFOS
 * \param ret The return type.
 * \param param_type_list The parameter types.
 */
#define MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_BASIC(ret, param_type_list, ...) \
    MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_ALL(ret, param_type_list, , )

/**
 * \brief Selects the correct overload, depending on the number of arguments.
 * \ingroup FACADE_DETAIL_MAKE_OVERLOAD_INFOS
 * \see For an explanation of that pattern: https://stackoverflow.com/a/16683147
 */
#define MIMICPP_DETAIL_SELECT_MAKE_OVERLOAD_INFOS(_1, _2, N, ...) N

#endif
