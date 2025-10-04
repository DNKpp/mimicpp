//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

//
// Created by dnkpp on 16.06.25.
//

#ifndef MIMICPP_MACROS_INTERFACE_MOCKING_HPP
#define MIMICPP_MACROS_INTERFACE_MOCKING_HPP

namespace mimicpp
{
    /**
     * \defgroup MOCK_INTERFACES_DETAIL detail
     * \ingroup MOCK_INTERFACES
     * \brief Contains several macros, used for interface mock implementation.
     * \attention These macros should never be used directly by users.
     */

    /**
     * \defgroup MOCK_INTERFACES_DETAIL_STRIP_PARENS strip_parens
     * \ingroup MOCK_INTERFACES_DETAIL
     * \brief Removes an enclosing pair of (), if present.
     */
}

/**
 * \brief Removes an enclosing pair of (), if present.
 * \ingroup MOCK_INTERFACES_DETAIL_STRIP_PARENS
 * \param x token
 * \see Inspired by https://stackoverflow.com/a/62984543
 */
#define MIMICPP_DETAIL_STRIP_PARENS(x) MIMICPP_DETAIL_STRIP_PARENS_OUTER(MIMICPP_DETAIL_STRIP_PARENS_INNER x)

/**
 * \brief Black-magic.
 * \ingroup MOCK_INTERFACES_DETAIL_STRIP_PARENS
 */
#define MIMICPP_DETAIL_STRIP_PARENS_INNER(...) MIMICPP_DETAIL_STRIP_PARENS_INNER __VA_ARGS__

/**
 * \brief Black-magic.
 * \ingroup MOCK_INTERFACES_DETAIL_STRIP_PARENS
 */
#define MIMICPP_DETAIL_STRIP_PARENS_OUTER(...) MIMICPP_DETAIL_STRIP_PARENS_OUTER_(__VA_ARGS__)

/**
 * \brief Black-magic.
 * \ingroup MOCK_INTERFACES_DETAIL_STRIP_PARENS
 */
#define MIMICPP_DETAIL_STRIP_PARENS_OUTER_(...) MIMICPP_DETAIL_STRIP_PARENS_STRIPPED_##__VA_ARGS__

/**
 * \brief Swallows the leftover token.
 * \ingroup MOCK_INTERFACES_DETAIL_STRIP_PARENS
 */
#define MIMICPP_DETAIL_STRIP_PARENS_STRIPPED_MIMICPP_DETAIL_STRIP_PARENS_INNER

namespace mimicpp
{
    /**
     * \defgroup MOCK_INTERFACES_DETAIL_FOR_EACH for_each
     * \ingroup MOCK_INTERFACES_DETAIL
     * \brief This is an implementation of a for-loop for the preprocessor.
     * \details This solution is highly inspired by the blog-article of David Mazieres.
     * He does a very good job in explaining the dark corners of the macro language, but even now, I do not
     * fully understand how this works. Either way, thank you very much!
     * \see https://www.scs.stanford.edu/~dm/blog/va-opt.html
     * \details All macros in this group are required to make that work.
     */
}

/**
 * \brief Pastes a pair of parentheses.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_PARENS ()

/**
 * \brief Pastes a comma.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_COMMA_DELIMITER() ,

/**
 * \brief Pastes nothing.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_NO_DELIMITER()

/**
 * \brief Pastes all arguments as provided.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_IDENTITY(...) __VA_ARGS__

/**
 * \brief Part of the fake recursion.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_EXPAND(...) MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(__VA_ARGS__))))

/**
 * \brief Part of the fake recursion.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_EXPAND3(...) MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(__VA_ARGS__))))

/**
 * \brief Part of the fake recursion.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_EXPAND2(...) MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(__VA_ARGS__))))

/**
 * \brief Part of the fake recursion.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_EXPAND1(...) __VA_ARGS__

/**
 * \brief Calls the given macro with all other arguments.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 * \param macro Macro to be called.
 * \param sequence First argument.
 * \param ... Accepts arbitrary arguments and forwards them.
 */
#define MIMICPP_DETAIL_FOR_EACH_EXT_INDIRECT(macro, sequence, ...) macro(sequence, __VA_ARGS__)

/**
 * \brief The starting point of the for-each implementation.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 * \param macro The strategy to be executed.
 * \param token A token, which will be expanded for each element.
 * \param delimiter The delimiter, which will added between element.
 * \param projection_macro The projection for the current element.
 * \param bound Addition data, which will be added to the call arguments.
 *
 * \details This is a very versatile implementation for the for-loop.
 *
 * During the development, it was necessary to generate unique names for function parameters, which I could also directly refer to.
 * That was the reason, why I've added the ``token`` argument. The first element will simply be called with the ``token`` content, but the second
 * with twice the token content and so on. It's ok, to provide an empty argument.
 */
#define MIMICPP_DETAIL_FOR_EACH_EXT(macro, token, delimiter, projection_macro, bound, ...) \
    __VA_OPT__(MIMICPP_DETAIL_EXPAND(MIMICPP_DETAIL_FOR_EACH_EXT_HELPER(macro, token, token, delimiter, projection_macro, bound, __VA_ARGS__)))

/**
 * \brief Black-magic.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_FOR_EACH_EXT_HELPER(macro, token, sequence, delimiter, projection_macro, bound, a1, ...)     \
    MIMICPP_DETAIL_FOR_EACH_EXT_INDIRECT(macro, sequence, MIMICPP_DETAIL_STRIP_PARENS(bound), projection_macro(a1)) \
    __VA_OPT__(delimiter() MIMICPP_FOR_EACH_EXT_AGAIN MIMICPP_DETAIL_PARENS(macro, token, sequence##token, delimiter, projection_macro, bound, __VA_ARGS__))

/**
 * \brief Black-magic.
 * \ingroup MOCK_INTERFACES_DETAIL_FOR_EACH
 */
#define MIMICPP_FOR_EACH_EXT_AGAIN() MIMICPP_DETAIL_FOR_EACH_EXT_HELPER

namespace mimicpp
{
    /**
     * \defgroup MOCK_INTERFACES_DETAIL_MAKE_SIGNATURE_LIST make_signature_list
     * \ingroup MOCK_INTERFACES_DETAIL
     * \brief Converts all given arguments to a signature.
     */
}

/**
 * \brief Converts the given information to a single signature.
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_SIGNATURE_LIST
 * \param sequence Unused.
 * \param bound_data Unused.
 * \param call_convention The call-convention.
 * \param ret The return type.
 * \param param_type_list The parameter types.
 * \param specs Additional specs (e.g. ``const``, ``noexcept``).
 */
#define MIMICPP_DETAIL_MAKE_SIGNATURE(sequence, bound_data, ret, call_convention, param_type_list, specs, ...) \
    ::mimicpp::detail::apply_normalized_specs_t<                                                               \
        MIMICPP_DETAIL_STRIP_PARENS(ret) call_convention param_type_list,                                      \
        ::mimicpp::util::StaticString{#specs}>

/**
 * \brief Converts all given arguments to a signature list (not enclosed by parentheses).
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_SIGNATURE_LIST
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
 * \brief Creates a mimicpp::Mock object for the given signatures.
 * \ingroup MOCK_INTERFACES_DETAIL
 * \param traits The interface traits.
 * \param mock_name The mock name.
 * \param fn_name The function name.
 * \param linkage The linkage specifier(s).
 * \param signatures The given signatures. Enclosing parentheses will be stripped.
 */
#define MIMICPP_DETAIL_MAKE_OVERLOADED_MOCK(traits, mock_name, fn_name, linkage, signatures) \
    linkage typename traits::mock_type<MIMICPP_DETAIL_STRIP_PARENS(signatures)> mock_name    \
    {                                                                                        \
        [&]<typename T = traits>() {                                                         \
            if constexpr (::mimicpp::detail::is_member_facade_v<T>)                          \
            {                                                                                \
                return T::make_settings(this, #fn_name);                                     \
            }                                                                                \
            else                                                                             \
            {                                                                                \
                return T::make_settings(#fn_name);                                           \
            }                                                                                \
        }()                                                                                  \
    }

namespace mimicpp
{
    /**
     * \defgroup MOCK_INTERFACES_DETAIL_MAKE_PARAM_LIST make_param_list
     * \ingroup MOCK_INTERFACES_DETAIL
     * \brief Converts all given arguments to a parameter-list.
     */
}

/**
 * \brief Converts the given information to a single parameter.
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_PARAM_LIST
 * \param sequence A unique sequence, which will be appended to the parameter name (as suffix).
 * \param bound_data Unused.
 * \param type The type of the parameter. Enclosing parentheses will be stripped.
 */
#define MIMICPP_DETAIL_MAKE_PARAM(sequence, bound_data, type) MIMICPP_DETAIL_STRIP_PARENS(type) arg_##sequence

/**
 * \brief Converts all given arguments to a parameter-list (not enclosed by parentheses).
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_PARAM_LIST
 */
#define MIMICPP_DETAIL_MAKE_PARAM_LIST(...) \
    MIMICPP_DETAIL_FOR_EACH_EXT(            \
        MIMICPP_DETAIL_MAKE_PARAM,          \
        i,                                  \
        MIMICPP_DETAIL_COMMA_DELIMITER,     \
        MIMICPP_DETAIL_IDENTITY,            \
        ,                                   \
        __VA_ARGS__)

namespace mimicpp
{
    /**
     * \defgroup MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE forward_args
     * \ingroup MOCK_INTERFACES_DETAIL
     * \brief Creates comma-separated forwarding ``std::tuple``s for each given argument (not enclosed by parentheses).
     * \details The whole purpose of these macros are to somehow generate forwarding references from the given context.
     * The first version just applied a ``std::forward`` call onto each argument, but this wasn't sufficient to
     * support parameter-packs.
     *
     * As parameter-packs are applied in the form ``T...``, the ``MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE`` macro must
     * somehow handle these kind of arguments. There is no way to distinguish via trait, whether the ``param_type``
     * is a type or pack, so we can not simply apply an expanding ``...`` when required.
     * So the question is, how to unify the handling?
     *
     * The solution here is to pass both, either types or packs, into a tuple and then make use of the (possibly)
     * simultaneous expansion feature.
     * When the param is a type, there is only one pack (the tuple with exactly one argument) in the scope.
     * When the param is a pack, both will be expanded simultaneously, because they appear in the same pattern.
     * \see https://en.cppreference.com/w/cpp/language/pack
     */
}

/**
 * \brief Creates a forwarding ``std::tuple`` for the given argument.
 * \ingroup MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE
 * \param sequence A unique sequence, which will be appended to the parameter name (as suffix).
 * \param bound_data Unused.
 * \param param_type The type of the parameter. Enclosing parentheses will be stripped.
 */
#define MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE(sequence, bound_data, param_type)                    \
    [&]<typename... Type>([[maybe_unused]] ::mimicpp::util::type_list<Type...> const) noexcept { \
        return ::std::forward_as_tuple(::std::forward<Type>(arg_##sequence)...);                 \
    }(::mimicpp::util::type_list<MIMICPP_DETAIL_STRIP_PARENS(param_type)>{})

/**
 * \brief Creates forwarding ``std::tuple``s for each given argument (not enclosed by parentheses).
 * \ingroup MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE
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
     * \defgroup MOCK_INTERFACES_DETAIL_MAKE_OVERLOAD_INFOS make_overload_infos
     * \ingroup MOCK_INTERFACES_DETAIL
     * \brief Related functions for MIMICPP_ADD_OVERLOAD.
     */
}

/**
 * \brief Base overload, extending the overload info (enclosed by parentheses).
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_OVERLOAD_INFOS
 * \param ret The return type.
 * \param param_type_list The parameter types.
 * \param specs An optional parameter for categories (e.g. ``const``, ``noexcept``, etc.).
 * \param call_convention An optional parameter for the utilized call-convention.
 * \details Strips all optional parenthesize from the arguments.
 * \note No parens will be stripped from ``ret``, because a return type may contain commas (e.g. ``std::tuple<int, int>``).
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
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_OVERLOAD_INFOS
 * \param ret The return type.
 * \param param_type_list The parameter types.
 * \param specs An optional parameter for categories (e.g. ``const``, ``noexcept``, etc.).
 */
#define MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_SPECS(ret, param_type_list, specs, ...) \
    MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_ALL(ret, param_type_list, specs, )

/**
 * \brief Simple overload, extending the overload info (enclosed by parentheses).
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_OVERLOAD_INFOS
 * \param ret The return type.
 * \param param_type_list The parameter types.
 */
#define MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_BASIC(ret, param_type_list, ...) \
    MIMICPP_DETAIL_MAKE_OVERLOAD_INFOS_ALL(ret, param_type_list, , )

/**
 * \brief Selects the correct overload, depending on the number of arguments.
 * \ingroup MOCK_INTERFACES_DETAIL_MAKE_OVERLOAD_INFOS
 * \see For an explanation of that pattern: https://stackoverflow.com/a/16683147
 */
#define MIMICPP_DETAIL_SELECT_MAKE_OVERLOAD_INFOS(_1, _2, N, ...) N

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
 * \param traits The (enclosed) interface traits.
 * \param fn_header The (enclosed)  facade function header.
 * \param sig The (enclosed) signature.
 * \param target_name The invocation target name.
 * \param invoke_args The (enclosed) invoke-argument list.
 */
#define MIMICPP_DETAIL_MAKE_FACADE_FUNCTION(ignore, traits, target_name, fn_name, linkage, ret, call_convention, param_type_list, specs, param_list, forward_list, ...) \
    linkage MIMICPP_DETAIL_STRIP_PARENS(ret)                                                                                                                            \
    call_convention fn_name param_list specs                                                                                                                            \
    {                                                                                                                                                                   \
        using Signature = ::mimicpp::detail::apply_normalized_specs_t<                                                                                                  \
            MIMICPP_DETAIL_STRIP_PARENS(ret) param_type_list,                                                                                                           \
            ::mimicpp::util::StaticString{#specs}>;                                                                                                                     \
        auto args = ::std::tuple_cat(MIMICPP_DETAIL_STRIP_PARENS(forward_list));                                                                                        \
                                                                                                                                                                        \
        return [&]<typename T = MIMICPP_DETAIL_STRIP_PARENS(traits)>() -> decltype(auto) {                                                                              \
            if constexpr (::mimicpp::detail::is_member_facade_v<T>)                                                                                                     \
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
    MIMICPP_DETAIL_MAKE_OVERLOADED_MOCK(                                                                      \
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
        ::mimicpp::detail::interface_mock_traits,    \
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
#define MIMICPP_MOCK_OVERLOADED_METHOD_WITH_THIS(fn_name, ...)         \
    MIMICPP_DETAIL_MAKE_INTERFACE_FACADE(                              \
        MIMICPP_DETAIL_MAKE_METHOD_OVERRIDE,                           \
        ::mimicpp::detail::interface_mock_with_this_traits<self_type>, \
        fn_name##_,                                                    \
        fn_name,                                                       \
        ,                                                              \
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
