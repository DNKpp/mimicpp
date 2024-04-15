// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CALL_HPP
#define MIMICPP_CALL_HPP

#pragma once

#include <tuple>

namespace mimicpp
{
	enum class ValueCategory
	{
		lvalue,
		rvalue
	};

	template <typename Signature>
	class Call;

	template <typename Return, typename... Args>
	class Call<Return(Args...)>
	{
	public:
		using ParamListT = std::tuple<std::reference_wrapper<std::remove_reference_t<Args>>...>;
		using UuidT = std::ptrdiff_t;

		ParamListT params;
		UuidT fromUuid{};
		ValueCategory fromCategory{};
		bool fromConst{};
	};
}

#endif
