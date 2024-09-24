// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_INTERFACE_MOCK_HPP
#define MIMICPP_INTERFACE_MOCK_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Mock.hpp"

#endif

namespace mimicpp
{
	/**
	 * \defgroup MOCK_INTERFACES interfaces
	 * \ingroup MOCK
	 * \brief Contains utility to simplify interface mocking.
	 */

	/**
	 * \defgroup MOCK_INTERFACES_DETAIL detail
	 * \ingroup MOCK_INTERFACES
	 */
}

/**
 * \brief Removes an enclosing pair of (), if present.
 * \ingroup MOCK_INTERFACES_DETAIL
 * \param x token
 * \see Inspired by https://stackoverflow.com/a/62984543
 */
#define MIMICPP_DETAIL_STRIP_PARENS(x) MIMICPP_DETAIL_STRIP_PARENS_OUTER(MIMICPP_DETAIL_STRIP_PARENS_INNER x)
#define MIMICPP_DETAIL_STRIP_PARENS_INNER(...) MIMICPP_DETAIL_STRIP_PARENS_INNER __VA_ARGS__
#define MIMICPP_DETAIL_STRIP_PARENS_OUTER(...) MIMICPP_DETAIL_STRIP_PARENS_OUTER_(__VA_ARGS__)
#define MIMICPP_DETAIL_STRIP_PARENS_OUTER_(...) MIMICPP_DETAIL_STRIP_PARENS_STRIPPED_ ## __VA_ARGS__
#define MIMICPP_DETAIL_STRIP_PARENS_STRIPPED_MIMICPP_DETAIL_STRIP_PARENS_INNER

namespace mimicpp
{
	/**
	 * \defgroup MOCK_INTERFACES_DETAIL_FOR_EACH for_each
	 * \ingroup MOCK_INTERFACES_DETAIL
	 * \brief This is an implementation of a for-loop for the preprocessor.
	 * \detail This solution is highly inspired by the blog-article of David Mazieres.
	 * He does a very good job in explaining the dark corners of the macro language, but even now, I do not
	 * fully understand how this works. Either way, thank you very much!
	 * \see https://www.scs.stanford.edu/~dm/blog/va-opt.html
	 * \detail All macros in this group are required to make that work.
	 * \{
	 */
}

#define MIMICPP_DETAIL_PARENS ()

#define MIMICPP_DETAIL_EXPAND(...) MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(MIMICPP_DETAIL_EXPAND3(__VA_ARGS__))))
#define MIMICPP_DETAIL_EXPAND3(...) MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(MIMICPP_DETAIL_EXPAND2(__VA_ARGS__))))
#define MIMICPP_DETAIL_EXPAND2(...) MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(MIMICPP_DETAIL_EXPAND1(__VA_ARGS__))))
#define MIMICPP_DETAIL_EXPAND1(...) __VA_ARGS__

#define MIMICPP_DETAIL_FOR_EACH_EXT_INDIRECT(macro, sequence, ...) macro(sequence, __VA_ARGS__)

/**
 * \brief The starting point of the for-each implementation.
 * \param macro The strategy to be executed.
 * \param token A token, which will be expanded for each element.
 * \param delimiter The delimiter, which will added between element.
 * \param projection_macro The projection for the current element.
 * \param bound Addition data, which will be added to the call arguments.
 *
 * \detail This is a very versatile implementation for the for-loop.
 *
 * During the development, it was necessary to generate unique names for function parameters, which I could also directly refer to.
 * That was the reason, why I've added the ``token`` argument. The first element will simply be called with the ``token`` content, but the second
 * with twice the token content and so on. It's ok, to provide an empty argument.
 */
#define MIMICPP_DETAIL_FOR_EACH_EXT(macro, token, delimiter, projection_macro, bound, ...) \
  __VA_OPT__(MIMICPP_DETAIL_EXPAND(MIMICPP_DETAIL_FOR_EACH_EXT_HELPER(macro, token, token, delimiter, projection_macro, bound, __VA_ARGS__)))
#define MIMICPP_DETAIL_FOR_EACH_EXT_HELPER(macro, token, sequence, delimiter, projection_macro, bound, a1, ...)												\
  MIMICPP_DETAIL_FOR_EACH_EXT_INDIRECT(macro, sequence, MIMICPP_DETAIL_STRIP_PARENS(bound), projection_macro(a1))											\
  __VA_OPT__(delimiter() MIMICPP_FOR_EACH_EXT_AGAIN MIMICPP_DETAIL_PARENS (macro, token, sequence##token, delimiter, projection_macro, bound, __VA_ARGS__))
#define MIMICPP_FOR_EACH_EXT_AGAIN() MIMICPP_DETAIL_FOR_EACH_EXT_HELPER

#define MIMICPP_DETAIL_COMMA_DELIMITER() ,
#define MIMICPP_DETAIL_NO_DELIMITER()

namespace mimicpp
{
	/**
	 * \}
	 */

	/**
	 * \defgroup MOCK_INTERFACES_DETAIL_MAKE_SIGNATURE make_signature
	 * \ingroup MOCK_INTERFACES_DETAIL
	 * \brief Converts all given arguments to a signature.
	 * \{
	 */
}

#define MIMICPP_DETAIL_MAKE_SIGNATURE(sequence, bound_data, ret, param_type_list, specs, ...) ret param_type_list specs

/**
 * \brief Converts all given arguments to a signature list (not enclosed by parentheses).
 */
#define MIMICPP_DETAIL_MAKE_SIGNATURE_LIST(...) \
	MIMICPP_DETAIL_FOR_EACH_EXT(				\
		MIMICPP_DETAIL_MAKE_SIGNATURE,			\
		,										\
		MIMICPP_DETAIL_COMMA_DELIMITER,			\
		MIMICPP_DETAIL_STRIP_PARENS,			\
		,										\
		__VA_ARGS__)

namespace mimicpp
{
	/**
	 * \}
	 */
}
