//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MACROS_FACADE_HELPER_HPP
#define MIMICPP_MACROS_FACADE_HELPER_HPP

#pragma once

#include "mimic++/macros/Common.hpp"

namespace mimicpp
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

#endif
