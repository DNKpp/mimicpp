// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITY_HPP
#define MIMICPP_UTILITY_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/TypeTraits.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <functional>
#include <ranges>
#include <source_location>
#include <utility>

namespace mimicpp
{
	template <typename...>
	struct always_false
		: public std::bool_constant<false>
	{
	};

	template <std::size_t priority>
	struct priority_tag
		/** \cond Help doxygen with recursion.*/
		: public priority_tag<priority - 1>
		/** \endcond */
	{
	};

	template <>
	struct priority_tag<0>
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

	template <typename T, template <typename> typename Trait>
	concept satisfies = Trait<T>::value;

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

	[[nodiscard]]
	constexpr bool is_matching(const Constness lhs, const Constness rhs) noexcept
	{
		return std::cmp_not_equal(0, to_underlying(lhs) & to_underlying(rhs));
	}

	[[nodiscard]]
	constexpr bool is_matching(const ValueCategory lhs, const ValueCategory rhs) noexcept
	{
		return std::cmp_not_equal(0, to_underlying(lhs) & to_underlying(rhs));
	}
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

namespace mimicpp
{
	/**
	 * \brief Determines, whether the given type can be used as a string-type.
	 */
	template <typename T>
	concept string = requires
					{
						typename string_traits<std::remove_cvref_t<T>>::char_t;
						typename string_traits<std::remove_cvref_t<T>>::string_t;
					}
					&& std::convertible_to<T, typename string_traits<std::remove_cvref_t<T>>::string_t>
					&& std::equality_comparable<typename string_traits<std::remove_cvref_t<T>>::string_t>
					&& std::ranges::forward_range<typename string_traits<std::remove_cvref_t<T>>::string_t>;
}

namespace mimicpp::custom
{
	/**
	 * \brief User may add specializations, which will then be used for normalize_string conversions.
	 * \ingroup UTILITY
	 */
	template <string String>
	struct normalize_string_converter;
}

namespace mimicpp::detail
{
	template <typename Converted, typename String>
	concept string_converter_for = string<String>
									&& requires
									{
										{ std::invoke(Converted{}, std::declval<String&&>()) } -> string;
									};
}

namespace mimicpp::detail::normalize_string_hook
{
	template <string String, typename Converter = custom::normalize_string_converter<std::remove_cvref_t<String>>>
		requires string_converter_for<
			Converter,
			String>
	[[nodiscard]]
	constexpr decltype(auto) normalize_string_impl(
		String&& str,
		[[maybe_unused]] const priority_tag<1>
	)
	{
		return std::invoke(
			Converter{},
			std::forward<String>(str));
	}

	template <string String>
	struct normalize_string_converter;

	template <string String, typename Converter = normalize_string_converter<std::remove_cvref_t<String>>>
		requires string_converter_for<
			Converter,
			String>
	[[nodiscard]]
	constexpr decltype(auto) normalize_string_impl(
		String&& str,
		[[maybe_unused]] const priority_tag<0>
	)
	{
		return std::invoke(
			Converter{},
			std::forward<String>(str));
	}

	constexpr priority_tag<1> maxTag;

	class NormalizeStringFn
	{
	public:
		template <string String>
		[[nodiscard]]
		constexpr decltype(auto) operator ()(String&& str) const
			requires requires { { normalize_string_hook::normalize_string_impl(str, maxTag) } -> string; }
		{
			return normalize_string_hook::normalize_string_impl(
				std::forward<String>(str),
				maxTag);
		}
	};

	template <string String>
		requires std::same_as<char, typename string_traits<String>::char_t>
	struct normalize_string_converter<String>
	{
		[[nodiscard]]
		std::string operator ()(const typename string_traits<String>::string_t& str) const
		{
			std::string result(std::ranges::size(str), '\0');
			std::ranges::transform(
				str,
				std::ranges::begin(result),
				[](const char c) noexcept
				{
					// see notes of https://en.cppreference.com/w/cpp/string/byte/toupper
					return static_cast<char>(
						static_cast<unsigned char>(std::toupper(c)));
				});

			return result;
		}
	};
}

namespace mimicpp
{
	constexpr detail::normalize_string_hook::NormalizeStringFn normalize_string{};

	template <typename String>
	concept normalizable_string = string<String>
							&& requires
							{
								{
									mimicpp::normalize_string(std::declval<String>())
								} -> string;
							};
}

#endif
