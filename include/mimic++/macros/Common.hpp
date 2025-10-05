//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MACROS_COMMON_HPP
#define MIMICPP_MACROS_COMMON_HPP

#pragma once

namespace mimicpp
{
    /**
     * \defgroup MACRO_DETAIL detail macros
     * \brief Contains several internally used macros.
     * \attention Users should never use these macros directly.
     */

    /**
     * \defgroup MACRO_DETAIL_STRIP_PARENS strip_parens
     * \ingroup MACRO_DETAIL
     * \brief Removes an enclosing pair of (), if present.
     */
}

/**
 * \brief Removes an enclosing pair of (), if present.
 * \ingroup MACRO_DETAIL_STRIP_PARENS
 * \param x token
 * \see Inspired by https://stackoverflow.com/a/62984543
 */
#define MIMICPP_DETAIL_STRIP_PARENS(x) MIMICPP_DETAIL_STRIP_PARENS_OUTER(MIMICPP_DETAIL_STRIP_PARENS_INNER x)

/**
 * \brief Black-magic.
 * \ingroup MACRO_DETAIL_STRIP_PARENS
 */
#define MIMICPP_DETAIL_STRIP_PARENS_INNER(...) MIMICPP_DETAIL_STRIP_PARENS_INNER __VA_ARGS__

/**
 * \brief Black-magic.
 * \ingroup MACRO_DETAIL_STRIP_PARENS
 */
#define MIMICPP_DETAIL_STRIP_PARENS_OUTER(...) MIMICPP_DETAIL_STRIP_PARENS_OUTER_(__VA_ARGS__)

/**
 * \brief Black-magic.
 * \ingroup MACRO_DETAIL_STRIP_PARENS
 */
#define MIMICPP_DETAIL_STRIP_PARENS_OUTER_(...) MIMICPP_DETAIL_STRIP_PARENS_STRIPPED_##__VA_ARGS__

/**
 * \brief Swallows the leftover token.
 * \ingroup MACRO_DETAIL_STRIP_PARENS
 */
#define MIMICPP_DETAIL_STRIP_PARENS_STRIPPED_MIMICPP_DETAIL_STRIP_PARENS_INNER

namespace mimicpp
{
    /**
     * \defgroup MACRO_DETAIL_FOR_EACH for_each
     * \ingroup MACRO_DETAIL
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
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_PARENS ()

/**
 * \brief Pastes a comma.
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_COMMA_DELIMITER() ,

/**
 * \brief Pastes nothing.
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_NO_DELIMITER()

/**
 * \brief Pastes all arguments as provided.
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_IDENTITY(...) __VA_ARGS__

/**
 * \brief Part of the fake recursion.
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_EXPAND(...) MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(__VA_ARGS__))))

/**
 * \brief Part of the fake recursion.
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_EXPAND3(...) MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(__VA_ARGS__))))

/**
 * \brief Part of the fake recursion.
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_EXPAND2(...) MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(__VA_ARGS__))))

/**
 * \brief Part of the fake recursion.
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_EXPAND1(...) __VA_ARGS__

/**
 * \brief Calls the given macro with all other arguments.
 * \ingroup MACRO_DETAIL_FOR_EACH
 * \param macro Macro to be called.
 * \param sequence First argument.
 * \param ... Accepts arbitrary arguments and forwards them.
 */
#define MIMICPP_DETAIL_FOR_EACH_EXT_INDIRECT(macro, sequence, ...) macro(sequence, __VA_ARGS__)

/**
 * \brief The starting point of the for-each implementation.
 * \ingroup MACRO_DETAIL_FOR_EACH
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
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_DETAIL_FOR_EACH_EXT_HELPER(macro, token, sequence, delimiter, projection_macro, bound, a1, ...)     \
    MIMICPP_DETAIL_FOR_EACH_EXT_INDIRECT(macro, sequence, MIMICPP_DETAIL_STRIP_PARENS(bound), projection_macro(a1)) \
    __VA_OPT__(delimiter() MIMICPP_FOR_EACH_EXT_AGAIN MIMICPP_DETAIL_PARENS(macro, token, sequence##token, delimiter, projection_macro, bound, __VA_ARGS__))

/**
 * \brief Black-magic.
 * \ingroup MACRO_DETAIL_FOR_EACH
 */
#define MIMICPP_FOR_EACH_EXT_AGAIN() MIMICPP_DETAIL_FOR_EACH_EXT_HELPER

namespace mimicpp
{
    /**
     * \defgroup MACRO_DETAIL_STRINGIFY stringify
     * \ingroup MACRO_DETAIL
     * \brief Stringifies the provided arguments.
     */
}

/**
 * \brief Black-magic.
 * \ingroup MACRO_DETAIL_STRINGIFY
 */
#define MIMICPP_DETAIL_STRINGIFY_IMPL(...) #__VA_ARGS__

/**
 * \brief Stringifies the provided arguments
 * \ingroup MACRO_DETAIL_STRINGIFY
 */
#define MIMICPP_DETAIL_STRINGIFY(...) MIMICPP_DETAIL_STRINGIFY_IMPL(__VA_ARGS__)

#endif
