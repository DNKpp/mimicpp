// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TYPE_TRAITS_HPP
#define MIMICPP_TYPE_TRAITS_HPP

#pragma once

#include <type_traits>

namespace mimicpp
{
	template <typename Signature>
	struct signature_add_noexcept;

	template <typename Signature>
	using signature_add_noexcept_t = typename signature_add_noexcept<Signature>::type;

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...)>
	{
		using type = Return(Params...) noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...)>
	{
		using type = Return(Params..., ...) noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) noexcept>
	{
		using type = Return(Params...) noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) noexcept>
	{
		using type = Return(Params..., ...) noexcept;
	};

	template <typename Signature>
	struct signature_remove_noexcept;

	template <typename Signature>
	using signature_remove_noexcept_t = typename signature_remove_noexcept<Signature>::type;

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...)>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...)>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) noexcept>
	{
		using type = Return(Params..., ...);
	};
}

#endif
