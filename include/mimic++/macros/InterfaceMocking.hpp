//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MACROS_INTERFACE_MOCKING_HPP
#define MIMICPP_MACROS_INTERFACE_MOCKING_HPP

#include "mimic++/macros/Common.hpp"

namespace mimicpp
{
    /**
     * \defgroup MOCK_INTERFACES_DETAIL detail
     * \ingroup MOCK_INTERFACES
     * \brief Contains several macros, used for interface mock implementation.
     * \attention These macros should never be used directly by users.
     */
}

/**
 * \brief Adds an overload to an interface mock. Used only in combination with \ref MIMICPP_MOCK_OVERLOADED_METHOD.
 * \ingroup MOCK_INTERFACES
 * \param ret The return type.
 * \param param_type_list The parameter types.
 * \param ... An optional parameter for categories (e.g. ``const``, ``noexcept``, etc.).
 */
#define MIMICPP_ADD_OVERLOAD(ret, param_type_list, ...) \
    MIMICPP_DETAIL_SELECT_MAKE_OVERLOAD_INFOS(          \
        __VA_ARGS__,                                    \
        MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_ALL,         \
        MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_SPECS,       \
        MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_BASIC)(ret, param_type_list, __VA_ARGS__, ) // clangCl doesn't compile without that extra ,

namespace mimicpp
{
    /**
     * \defgroup MOCK_INTERFACES_DETAIL_MAKE_FACADES make facades
     * \ingroup MOCK_INTERFACES_DETAIL
     * \brief Creates all required facades.
     */
}

/**
 * \brief Creates the facade function.
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_FACADES
 * \param ignore Ignored
 * \param traits The interface traits.
 * \param target_name The callable target name.
 * \param fn_name The function name.
 * \param linkage The function linkage.
 * \param ret The return type.
 * \param call_convention The call-convention.
 * \param param_type_list The parameter types.
 * \param specs Additional specifiers (e.g. ``const``, ``noexcept``, etc.).
 * \param param_list Enclosed parameter list.
 * \param forward_list Enclosed forward statements.
 */
#define MIMICPP_DETAIL_MAKE_FACADE_FUNCTION(ignore, traits, target_name, fn_name, linkage, ret, call_convention, param_type_list, specs, param_list, forward_list, ...) \
    linkage MIMICPP_DETAIL_STRIP_PARENS(ret)                                                                                                                            \
    call_convention fn_name param_list specs                                                                                                                            \
    {                                                                                                                                                                   \
        using Signature = ::mimicpp::facade::detail::apply_normalized_specs_t<                                                                                          \
            MIMICPP_DETAIL_STRIP_PARENS(ret) param_type_list,                                                                                                           \
            ::mimicpp::util::StaticString{#specs}>;                                                                                                                     \
        auto args = ::std::tuple_cat(MIMICPP_DETAIL_STRIP_PARENS(forward_list));                                                                                        \
                                                                                                                                                                        \
        return [&]<typename T = traits>() -> decltype(auto) {                                                                                                           \
            if constexpr (::mimicpp::facade::detail::is_member_v<T>)                                                                                                    \
            {                                                                                                                                                           \
                return T::template invoke<Signature>(target_name, this, ::std::move(args));                                                                             \
            }                                                                                                                                                           \
            else                                                                                                                                                        \
            {                                                                                                                                                           \
                return T::template invoke<Signature>(target_name, ::std::move(args));                                                                                   \
            }                                                                                                                                                           \
        }();                                                                                                                                                            \
    }

/**
 * \brief Creates a single overload for the given information and extends the `specs` with override.
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_FACADES
 * \param ignore Ignored
 * \param traits The interface traits.
 * \param target_name The callable target name.
 * \param fn_name The function name.
 * \param linkage The function linkage.
 * \param ret The return type.
 * \param call_convention The call-convention.
 * \param param_type_list The parameter types.
 * \param specs Additional specifiers (e.g. ``const``, ``noexcept``, etc.).
 * \param param_list Enclosed parameter list.
 * \param forward_list Enclosed forward statements.
 */
#define MIMICPP_DETAIL_MAKE_METHOD_OVERRIDE(ignore, traits, target_name, fn_name, linkage, ret, call_convention, param_type_list, specs, param_list, forward_list, ...) \
    MIMICPP_DETAIL_MAKE_FACADE_FUNCTION(                                                                                                                                \
        ignore,                                                                                                                                                         \
        traits,                                                                                                                                                         \
        target_name,                                                                                                                                                    \
        fn_name,                                                                                                                                                        \
        linkage,                                                                                                                                                        \
        ret,                                                                                                                                                            \
        call_convention,                                                                                                                                                \
        param_type_list,                                                                                                                                                \
        specs override,                                                                                                                                                 \
        param_list,                                                                                                                                                     \
        forward_list,                                                                                                                                                   \
        VA_ARGS)

/**
 * \brief Creates all overloads for a specific function facade.
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_FACADES
 * \param op The operation for each element (see `MIMICPP_DETAIL_MAKE_FACADE_FUNCTION` as an example for the list of required arguments).
 * \param traits The interface traits.
 * \param target_name The callable target name.
 * \param fn_name The function name to be overloaded.
 * \param linkage The linkage for both, the facade functions and the target.
 */
#define MIMICPP_DETAIL_MAKE_INTERFACE_FACADE_OVERLOADS(op, traits, target_name, fn_name, linkage, ...) \
    MIMICPP_DETAIL_FOR_EACH_EXT(                                                                       \
        op,                                                                                            \
        ,                                                                                              \
        MIMICPP_DETAIL_NO_DELIMITER,                                                                   \
        MIMICPP_DETAIL_STRIP_PARENS,                                                                   \
        (traits, target_name, fn_name, linkage),                                                       \
        __VA_ARGS__)

/**
 * \brief Creates all overloads for a specific function as overrides and the mock member.
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_FACADES
 * \param traits The interface traits.
 * \param target_name The callable target name.
 * \param fn_name The function name to be overloaded.
 * \param linkage The linkage for both, the facade functions and the target.
 */
#define MIMICPP_DETAIL_MAKE_INTERFACE_FACADE(fn_op, traits, target_name, fn_name, linkage, ...)               \
    MIMICPP_DETAIL_MAKE_INTERFACE_FACADE_OVERLOADS(fn_op, traits, target_name, fn_name, linkage, __VA_ARGS__) \
    MIMICPP_DETAIL_MAKE_FACADE_TARGET(                                                                        \
        traits,                                                                                               \
        target_name,                                                                                          \
        fn_name,                                                                                              \
        linkage,                                                                                              \
        (MIMICPP_DETAIL_MAKE_SIGNATURE_LIST(__VA_ARGS__)))

/**
 * \brief Entry point for mocking an overloaded interface method.
 * \ingroup MOCK_INTERFACES
 * \param fn_name The name of the overload-set.
 * \param ... Overloads must be declared using the \ref MIMICPP_ADD_OVERLOAD macro.
 *
 * \details
 * This macro defines a single mock object that supports an arbitrary number of overloads.
 * Each overload is implemented as an override method, forwarding calls to the mock object.
 * \snippet InterfaceMock.cpp interface mock overloaded
 */
#define MIMICPP_MOCK_OVERLOADED_METHOD(fn_name, ...) \
    MIMICPP_DETAIL_MAKE_INTERFACE_FACADE(            \
        MIMICPP_DETAIL_MAKE_METHOD_OVERRIDE,         \
        ::mimicpp::facade::mock_as_member,           \
        fn_name##_,                                  \
        fn_name,                                     \
        ,                                            \
        __VA_ARGS__)

/**
 * \brief Entry point for mocking a single interface method.
 * \ingroup MOCK_INTERFACES
 * \param fn_name The method name.
 * \param param_type_list The list of parameter types.
 * \param ... Optional qualifiers (e.g., ``const``, ``noexcept``).
 * \details
 * This macro defines a single mock object for one method signature and generates a corresponding
 * override method. The override method forwards calls to the mock object.
 * \snippet InterfaceMock.cpp interface mock simple
 */
#define MIMICPP_MOCK_METHOD(fn_name, ret, param_type_list, ...) \
    MIMICPP_MOCK_OVERLOADED_METHOD(                             \
        fn_name,                                                \
        MIMICPP_ADD_OVERLOAD(ret, param_type_list __VA_OPT__(, ) __VA_ARGS__))

/**
 * \brief Entry point for mocking an overloaded interface method with an explicit *this* pointer.
 * \ingroup MOCK_INTERFACES
 * \param fn_name The name of the overload-set.
 * \param ... Overloads must be declared using the \ref MIMICPP_ADD_OVERLOAD macro.
 *
 * \details
 * This macro defines a single mock object that supports an arbitrary number of overloads.
 * Each overload is implemented as an override method, forwarding calls to the mock object
 * while prepending an explicit *this* pointer to the argument list.
 *
 * \attention
 * This macro requires a `self_type` alias to be defined in the class where the mock is declared.
 * If `self_type` does not match the actual containing type, the behavior is undefined.
 * \snippet InterfaceMock.cpp interface mock with this
 */
#define MIMICPP_MOCK_OVERLOADED_METHOD_WITH_THIS(fn_name, ...)  \
    MIMICPP_DETAIL_MAKE_INTERFACE_FACADE(                       \
        MIMICPP_DETAIL_MAKE_METHOD_OVERRIDE,                    \
        ::mimicpp::facade::mock_as_member_with_this<self_type>, \
        fn_name##_,                                             \
        fn_name,                                                \
        ,                                                       \
        __VA_ARGS__)

/**
 * \brief Entry point for mocking a single interface method with an explicit *this* pointer.
 * \ingroup MOCK_INTERFACES
 * \param fn_name The method name.
 * \param param_type_list The list of parameter types.
 * \param ... Optional qualifiers (e.g., ``const``, ``noexcept``).
 *
 * \details
 * This macro defines a single mock object for one method signature and generates a corresponding
 * override method. The override method forwards calls to the mock object and prepends an explicit
 * *this* pointer to the argument list.
 *
 * \attention
 * This macro requires a `self_type` alias to be defined in the class where the mock is declared.
 * If `self_type` does not match the actual containing type, the behavior is undefined.
 * \snippet InterfaceMock.cpp interface mock with this
 */
#define MIMICPP_MOCK_METHOD_WITH_THIS(fn_name, ret, param_type_list, ...) \
    MIMICPP_MOCK_OVERLOADED_METHOD_WITH_THIS(                             \
        fn_name,                                                          \
        MIMICPP_ADD_OVERLOAD(ret, param_type_list __VA_OPT__(, ) __VA_ARGS__))

/**
 * \brief The most powerful entry point for creating a facade overload-set.
 * \ingroup MOCK_INTERFACES
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
    MIMICPP_DETAIL_MAKE_INTERFACE_FACADE(                                              \
        MIMICPP_DETAIL_MAKE_FACADE_FUNCTION,                                           \
        traits,                                                                        \
        target_name,                                                                   \
        fn_name,                                                                       \
        linkage,                                                                       \
        __VA_ARGS__)

/**
 * \brief The most powerful entry point for creating a single facade function.
 * \ingroup MOCK_INTERFACES
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

#ifndef MIMICPP_CONFIG_ONLY_PREFIXED_MACROS

    /**
     * \brief Shorthand variant of \ref MIMICPP_MOCK_METHOD.
     * \ingroup MOCK_INTERFACES
     */
    #define MOCK_METHOD MIMICPP_MOCK_METHOD

    /**
     * \brief Shorthand variant of \ref MIMICPP_MOCK_METHOD_WITH_THIS.
     * \ingroup MOCK_INTERFACES
     * \snippet InterfaceMock.cpp interface mock with this
     */
    #define MOCK_METHOD_WITH_THIS MIMICPP_MOCK_METHOD_WITH_THIS

    /**
     * \brief Shorthand variant of \ref MIMICPP_MOCK_OVERLOADED_METHOD.
     * \ingroup MOCK_INTERFACES
     */
    #define MOCK_OVERLOADED_METHOD MIMICPP_MOCK_OVERLOADED_METHOD

    /**
     * \brief Shorthand variant of \ref MIMICPP_MOCK_OVERLOADED_METHOD_WITH_THIS.
     * \ingroup MOCK_INTERFACES
     * \snippet InterfaceMock.cpp interface mock with this
     */
    #define MOCK_OVERLOADED_METHOD_WITH_THIS MIMICPP_MOCK_OVERLOADED_METHOD_WITH_THIS

    /**
     * \brief Shorthand variant of \ref MIMICPP_ADD_OVERLOAD.
     * \ingroup MOCK_INTERFACES
     */
    #define ADD_OVERLOAD MIMICPP_ADD_OVERLOAD

#endif

#endif
