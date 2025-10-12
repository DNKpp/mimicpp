//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MACROS_INTERFACE_MOCKING_HPP
#define MIMICPP_MACROS_INTERFACE_MOCKING_HPP

#include "mimic++/macros/Common.hpp"
#include "mimic++/macros/Facade.hpp"

namespace mimicpp
{
    /**
     * \defgroup MOCK_INTERFACES interfaces
     * \ingroup MOCK
     * \ingroup FACADE
     * \brief [Deprecated] Utilities to simplify interface mocking.
     * \details
     * Please note that all symbols in this section are deprecated and will be removed in a future release.
     * I kindly recommend replacing their usage with the new, more feature-rich facade generator macros,
     * which offer improved flexibility and maintainability.
     * For more information, please refer to the \ref FACADE "facade section" in the documentation.
     */

    /**
     * \defgroup MOCK_INTERFACES_DETAIL detail
     * \ingroup MOCK_INTERFACES
     * \brief Contains several macros, used for interface mock implementation.
     * \attention These macros should never be used directly by users.
     */
}

/**
 * \brief Creates a single overload for the given information and extends the `specs` with override.
 * \ingroup MOCK_INTERFACES_DETAIL
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
    [[deprecated("(MIMICPP_)MOCK_(OVERLOADED_)METHOD is deprecated, use (MIMICPP_)MAKE_(OVERLOADED_)MEMBER_MOCK instead.")]]                                            \
    MIMICPP_DETAIL_GENERATE_FACADE_FUNCTION(                                                                                                                            \
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
    MIMICPP_DETAIL_GENERATE_FACADE(                  \
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

#ifndef MIMICPP_CONFIG_ONLY_PREFIXED_MACROS
    /**
     * \brief Shorthand variant of \ref MIMICPP_MOCK_METHOD.
     * \ingroup MOCK_INTERFACES
     * \copydoc MIMICPP_MOCK_METHOD
     */
    #define MOCK_METHOD MIMICPP_MOCK_METHOD

    /**
     * \brief Shorthand variant of \ref MIMICPP_MOCK_OVERLOADED_METHOD.
     * \ingroup MOCK_INTERFACES
     * \copydoc MIMICPP_MOCK_OVERLOADED_METHOD
     */
    #define MOCK_OVERLOADED_METHOD MIMICPP_MOCK_OVERLOADED_METHOD

#endif

#endif
