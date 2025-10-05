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
 * \param specs An optional parameter for the specifiers (e.g. `const`, `noexcept`, `override`, etc.).
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
 * \param specs An optional parameter for the specifiers (e.g. `const`, `noexcept`, `override`, etc.).
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

/**
 * \brief Adds an overload to the currently built facade.
 * \ingroup FACADE
 * \param ret The return type.
 * \param param_type_list The parameter types.
 * \param ... An optional parameter for the specifiers (e.g. `const`, `noexcept`, `override`, etc.).
 */
#define MIMICPP_ADD_OVERLOAD(ret, param_type_list, ...) \
    MIMICPP_DETAIL_SELECT_MAKE_OVERLOAD_INFOS(          \
        __VA_ARGS__,                                    \
        MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_ALL,         \
        MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_SPECS,       \
        MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_BASIC)(ret, param_type_list, __VA_ARGS__, ) // clangCl doesn't compile without that extra `,`

namespace mimicpp
{
    /**
     * \defgroup FACADE_DETAIL_GENERATE_FACADE generate facade
     * \ingroup FACADE_DETAIL
     * \brief Creates all necessary symbols for a facade specification.
     */
}

/**
 * \brief Creates the target object with the specified signatures.
 * \ingroup FACADE_DETAIL_GENERATE_FACADE
 * \param traits The interface traits.
 * \param target_name The target name.
 * \param fn_name The function name.
 * \param linkage The linkage specifier(s).
 * \param signatures The given signatures. Enclosing parentheses will be stripped.
 */
#define MIMICPP_DETAIL_GENERATE_FACADE_TARGET(traits, target_name, fn_name, linkage, signatures) \
    linkage typename traits::target_type<MIMICPP_DETAIL_STRIP_PARENS(signatures)> target_name    \
    {                                                                                            \
        [&]<typename T = traits>() {                                                             \
            if constexpr (::mimicpp::facade::detail::is_member_v<T>)                             \
            {                                                                                    \
                return T::make_settings(this, #fn_name);                                         \
            }                                                                                    \
            else                                                                                 \
            {                                                                                    \
                return T::make_settings(#fn_name);                                               \
            }                                                                                    \
        }()                                                                                      \
    }

/**
 * \brief Creates a single facade function.
 * \ingroup FACADE_DETAIL_GENERATE_FACADE
 * \param ignore Ignored
 * \param traits The interface traits.
 * \param target_name The callable target name.
 * \param fn_name The function name.
 * \param linkage The function linkage.
 * \param ret The return type.
 * \param call_convention The call-convention.
 * \param param_type_list The parameter types.
 * \param specs The specifiers (e.g. `const`, `noexcept`, `override`, etc.).
 * \param param_list Enclosed parameter list.
 * \param forward_list Enclosed forward statements.
 */
#define MIMICPP_DETAIL_GENERATE_FACADE_FUNCTION(ignore, traits, target_name, fn_name, linkage, ret, call_convention, param_type_list, specs, param_list, forward_list, ...) \
    linkage MIMICPP_DETAIL_STRIP_PARENS(ret)                                                                                                                                \
    call_convention fn_name param_list specs                                                                                                                                \
    {                                                                                                                                                                       \
        using Signature = ::mimicpp::facade::detail::apply_normalized_specs_t<                                                                                              \
            MIMICPP_DETAIL_STRIP_PARENS(ret) param_type_list,                                                                                                               \
            ::mimicpp::util::StaticString{#specs}>;                                                                                                                         \
        auto args = ::std::tuple_cat(MIMICPP_DETAIL_STRIP_PARENS(forward_list));                                                                                            \
                                                                                                                                                                            \
        return [&]<typename T = traits>() -> decltype(auto) {                                                                                                               \
            if constexpr (::mimicpp::facade::detail::is_member_v<T>)                                                                                                        \
            {                                                                                                                                                               \
                return T::template invoke<Signature>(target_name, this, ::std::move(args));                                                                                 \
            }                                                                                                                                                               \
            else                                                                                                                                                            \
            {                                                                                                                                                               \
                return T::template invoke<Signature>(target_name, ::std::move(args));                                                                                       \
            }                                                                                                                                                               \
        }();                                                                                                                                                                \
    }

/**
 * \brief Creates all overloads for a specific function facade.
 * \ingroup FACADE_DETAIL_GENERATE_FACADE
 * \param op The operation for each element (see `MIMICPP_DETAIL_GENERATE_FACADE_FUNCTION` as an example for the list of required arguments).
 * \param traits The interface traits.
 * \param target_name The callable target name.
 * \param fn_name The function name to be overloaded.
 * \param linkage The linkage for both, the facade functions and the target.
 */
#define MIMICPP_DETAIL_GENERATE_FACADE_OVERLOADS(op, traits, target_name, fn_name, linkage, ...) \
    MIMICPP_DETAIL_FOR_EACH_EXT(                                                                 \
        op,                                                                                      \
        ,                                                                                        \
        MIMICPP_DETAIL_NO_DELIMITER,                                                             \
        MIMICPP_DETAIL_STRIP_PARENS,                                                             \
        (traits, target_name, fn_name, linkage),                                                 \
        __VA_ARGS__)

/**
 * \brief Creates all overloads for a specific function facade and the target object.
 * \ingroup FACADE_DETAIL_GENERATE_FACADE
 * \param traits The interface traits.
 * \param target_name The callable target name.
 * \param fn_name The function name to be overloaded.
 * \param linkage The linkage for both, the facade functions and the target.
 */
#define MIMICPP_DETAIL_GENERATE_FACADE(fn_op, traits, target_name, fn_name, linkage, ...)               \
    MIMICPP_DETAIL_GENERATE_FACADE_OVERLOADS(fn_op, traits, target_name, fn_name, linkage, __VA_ARGS__) \
    MIMICPP_DETAIL_GENERATE_FACADE_TARGET(                                                              \
        traits,                                                                                         \
        target_name,                                                                                    \
        fn_name,                                                                                        \
        linkage,                                                                                        \
        (MIMICPP_DETAIL_MAKE_SIGNATURE_LIST(__VA_ARGS__)))

/**
 * \brief The most powerful entry point for creating a facade overload-set.
 * \ingroup FACADE
 * \param traits The facade-traits.
 * \param target_name The name of the underlying target object.
 * \param fn_name The name of the facade overload-set.
 * \param linkage The linkage for the facade functions and the target object.
 * \param ... Overloads must be declared using the \ref MIMICPP_ADD_OVERLOAD macro.
 *
 * \details
 * This macro defines a single target object that supports an arbitrary number of overloads.
 * Each overload is implemented as its own facade function, forwarding calls to the underlying target object.
 */
#define MIMICPP_MAKE_OVERLOADED_FACADE_EXT(traits, target_name, fn_name, linkage, ...) \
    MIMICPP_DETAIL_GENERATE_FACADE(                                                    \
        MIMICPP_DETAIL_GENERATE_FACADE_FUNCTION,                                       \
        traits,                                                                        \
        target_name,                                                                   \
        fn_name,                                                                       \
        linkage,                                                                       \
        __VA_ARGS__)

/**
 * \brief The most powerful entry point for creating a single facade function.
 * \ingroup FACADE
 * \param traits The facade-traits.
 * \param target_name The name of the underlying target object.
 * \param fn_name The name of the facade function.
 * \param linkage The linkage for the facade function and the target object.
 * \param ... Optional qualifiers (e.g., ``const``, ``noexcept``).
 *
 * \details
 * This macro defines a single target object for one method signature and generates a corresponding facade method.
 * The facade method forwards its calls to the underlying target object.
 */
#define MIMICPP_MAKE_FACADE_EXT(traits, target_name, fn_name, linkage, ret, param_type_list, ...) \
    MIMICPP_MAKE_OVERLOADED_FACADE_EXT(                                                           \
        traits,                                                                                   \
        target_name,                                                                              \
        fn_name,                                                                                  \
        linkage,                                                                                  \
        MIMICPP_ADD_OVERLOAD(ret, param_type_list __VA_OPT__(, ) __VA_ARGS__))

#endif
