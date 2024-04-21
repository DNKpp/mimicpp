// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TYPE_TRAITS_HPP
#define MIMICPP_TYPE_TRAITS_HPP

#pragma once

#include <tuple>

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

	template <typename Signature>
	struct signature_decay;

	template <typename Signature>
	using signature_decay_t = typename signature_decay<Signature>::type;

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...)>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) &>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const &>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) &&>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const &&>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) & noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const & noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) && noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const && noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...)>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) &>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const &>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) &&>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const &&>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) & noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const & noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) && noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const && noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Signature>
	struct signature_return_type;

	template <typename Signature>
	using signature_return_type_t = typename signature_return_type<Signature>::type;

	template <typename Return, typename... Params>
	struct signature_return_type<Return(Params...)>
	{
		using type = Return;
	};

	template <typename Return, typename... Params>
	struct signature_return_type<Return(Params..., ...)>
	{
		using type = Return;
	};

	template <typename Return, typename... Params>
	struct signature_return_type<Return(Params...) noexcept>
	{
		using type = Return;
	};

	template <typename Return, typename... Params>
	struct signature_return_type<Return(Params..., ...) noexcept>
	{
		using type = Return;
	};

	template <std::size_t index, typename Signature>
	struct signature_param_type;

	template <std::size_t index, typename Signature>
	using signature_param_type_t = typename signature_param_type<index, Signature>::type;

	template <std::size_t index, typename Return, typename... Params>
	struct signature_param_type<index, Return(Params...)>
		: public std::tuple_element<index, std::tuple<Params...>>
	{
	};

	template <std::size_t index, typename Return, typename... Params>
	struct signature_param_type<index, Return(Params...) noexcept>
		: public std::tuple_element<index, std::tuple<Params...>>
	{
	};
}

#endif
