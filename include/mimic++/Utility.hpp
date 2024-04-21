// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITY_HPP
#define MIMICPP_UTILITY_HPP

#pragma once

#include <cstddef>
#include <format>

namespace mimicpp
{
	enum class Uuid
		: std::size_t
	{
	};
}

template <>
struct std::formatter<mimicpp::Uuid, char>
{
	using UuidT = mimicpp::Uuid;

	static auto format(
		const UuidT uuid,
		std::format_context& ctx
	)
	{
		return std::format_to(
			ctx.out(),
			"UUID{{}}",
			static_cast<std::underlying_type_t<UuidT>>(uuid));
	}
};

namespace mimicpp
{
	enum class Constness
	{
		non_const = 0b01,
		as_const = 0b10,
		any = non_const | as_const
	};

	[[nodiscard]]
	constexpr bool matches(const Constness lhs, const Constness rhs) noexcept
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
	constexpr bool matches(const ValueCategory lhs, const ValueCategory rhs) noexcept
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
		std::format_context& ctx
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
		std::format_context& ctx
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

#endif
