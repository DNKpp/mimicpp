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
	enum class ValueCategory
	{
		lvalue,
		rvalue
	};
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
			}

			throw std::runtime_error{"Unknown category value."};
		};

		return std::formatter<std::string_view, char>::format(
			toString(category),
			ctx);
	}
};

#endif
