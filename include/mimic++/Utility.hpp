// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITY_HPP
#define MIMICPP_UTILITY_HPP

#pragma once

#include <cstddef>

namespace mimicpp
{
	enum class Uuid
		: std::size_t
	{
	};

	enum class ValueCategory
	{
		lvalue,
		rvalue
	};
}

#endif
