// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITY_HPP
#define MIMICPP_UTILITY_HPP

#pragma once

#include <format>
#include <source_location>

namespace mimicpp
{
	enum class Constness
	{
		non_const = 0b01,
		as_const = 0b10,
		any = non_const | as_const
	};

	[[nodiscard]]
	constexpr bool is_matching(const Constness lhs, const Constness rhs) noexcept
	{
		using UnderlyingT = std::underlying_type_t<Constness>;
		return UnderlyingT{0} != (static_cast<UnderlyingT>(lhs) & static_cast<UnderlyingT>(rhs));
	}

	enum class ValueCategory
	{
		lvalue = 0b01,
		rvalue = 0b10,
		any    = lvalue | rvalue
	};

	[[nodiscard]]
	constexpr bool is_matching(const ValueCategory lhs, const ValueCategory rhs) noexcept
	{
		using UnderlyingT = std::underlying_type_t<ValueCategory>;
		return UnderlyingT{0} != (static_cast<UnderlyingT>(lhs) & static_cast<UnderlyingT>(rhs));
	}
}

template <>
struct std::formatter<mimicpp::ValueCategory, char>
	: public std::formatter<std::string_view, char>
{
	using ValueCategoryT = mimicpp::ValueCategory;

	auto format(
		const ValueCategoryT category,
		auto& ctx
	) const
	{
		constexpr auto toString = [](const ValueCategoryT cat)
		{
			switch (cat)
			{
			case ValueCategoryT::lvalue: return "lvalue";
			case ValueCategoryT::rvalue: return "rvalue";
			case ValueCategoryT::any: return "any";
			}

			throw std::runtime_error{"Unknown category value."};
		};

		return std::formatter<std::string_view, char>::format(
			toString(category),
			ctx);
	}
};

template <>
struct std::formatter<mimicpp::Constness, char>
	: public std::formatter<std::string_view, char>
{
	using ConstnessT = mimicpp::Constness;

	auto format(
		const ConstnessT category,
		auto& ctx
	) const
	{
		constexpr auto toString = [](const ConstnessT value)
		{
			switch (value)
			{
			case ConstnessT::non_const: return "mutable";
			case ConstnessT::as_const: return "const";
			case ConstnessT::any: return "any";
			}

			throw std::runtime_error{"Unknown constness value."};
		};

		return std::formatter<std::string_view, char>::format(
			toString(category),
			ctx);
	}
};

namespace mimicpp
{
	template <typename ...>
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

	template <typename T>
		requires std::is_enum_v<T>
	[[nodiscard]]
	constexpr std::underlying_type_t<T> to_underlying(const T value) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(value);
	}
}

#endif
