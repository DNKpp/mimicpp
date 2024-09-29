// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITY_HPP
#define MIMICPP_UTILITY_HPP

#pragma once

#include "mimic++/Fwd.hpp"

#include <cassert>
#include <format>
#include <source_location>
#include <utility>

namespace mimicpp
{

	[[nodiscard]]
	constexpr bool is_matching(const Constness lhs, const Constness rhs) noexcept
	{
		using UnderlyingT = std::underlying_type_t<Constness>;
		return UnderlyingT{0} != (static_cast<UnderlyingT>(lhs) & static_cast<UnderlyingT>(rhs));
	}


	[[nodiscard]]
	constexpr bool is_matching(const ValueCategory lhs, const ValueCategory rhs) noexcept
	{
		using UnderlyingT = std::underlying_type_t<ValueCategory>;
		return UnderlyingT{0} != (static_cast<UnderlyingT>(lhs) & static_cast<UnderlyingT>(rhs));
	}
}

namespace mimicpp
{
	template <typename...>
	struct always_false
		: public std::bool_constant<false>
	{
	};

	[[nodiscard]]
	constexpr bool is_same_source_location(
		const std::source_location& lhs,
		const std::source_location& rhs
	) noexcept
	{
		return std::string_view{lhs.file_name()} == std::string_view{rhs.file_name()}
				&& std::string_view{lhs.function_name()} == std::string_view{rhs.function_name()}
				&& lhs.line() == rhs.line()
				&& lhs.column() == rhs.column();
	}

	template <typename From, typename To>
	concept explicitly_convertible_to =
		requires
		{
			static_cast<To>(std::declval<From>());
		};

	template <typename From, typename To>
	concept nothrow_explicitly_convertible_to =
		explicitly_convertible_to<From, To>
		&& requires
		{
			{ static_cast<To>(std::declval<From>()) } noexcept;
		};

	template <typename T, typename... Others>
	concept same_as_any = (... || std::same_as<T, Others>);

	template <typename T>
		requires std::is_enum_v<T>
	[[nodiscard]]
	constexpr std::underlying_type_t<T> to_underlying(const T value) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(value);
	}

	// GCOVR_EXCL_START

#ifdef __cpp_lib_unreachable
	using std::unreachable;
#else

	/**
	 * \brief Invokes undefined behavior
	 * \see https://en.cppreference.com/w/cpp/utility/unreachable
	 * \note Implementation directly taken from https://en.cppreference.com/w/cpp/utility/unreachable
	 */
	[[noreturn]]
	inline void unreachable()
	{
		// Uses compiler specific extensions if possible.
		// Even if no extension is used, undefined behavior is still raised by
		// an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
		__assume(false);
#else // GCC, Clang
	    __builtin_unreachable();
#endif

		// ReSharper disable once CppUnreachableCode
		assert(false);
	}
#endif

	// GCOVR_EXCL_STOP
}

namespace mimicpp::detail
{
	template <typename Parsed, typename... Rest>
	struct unique;

	template <typename... Uniques, typename First, typename... Others>
	struct unique<
			std::tuple<Uniques...>,
			First,
			Others...>
	{
		using current_t = std::conditional_t<
			same_as_any<First, Uniques...>,
			std::tuple<Uniques...>,
			std::tuple<Uniques..., First>>;

		using type_t = typename unique<
			current_t,
			Others...>::type_t;
	};

	template <typename... Uniques>
	struct unique<std::tuple<Uniques...>>
	{
		using type_t = std::tuple<Uniques...>;
	};

	template <typename... Types>
	using unique_list_t = typename unique<std::tuple<>, Types...>::type_t;
}

#endif
