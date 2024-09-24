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
