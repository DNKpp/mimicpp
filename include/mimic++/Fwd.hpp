// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_FWD_HPP
#define MIMICPP_FWD_HPP

#pragma once

#include <string>

namespace mimicpp::call
{
	template <typename Return, typename... Args>
	class Info;
}

namespace mimicpp
{
	template <typename Signature>
	struct signature_add_noexcept;

	template <typename Signature>
	using signature_add_noexcept_t = typename signature_add_noexcept<Signature>::type;

	template <typename Signature>
	struct signature_remove_noexcept;

	template <typename Signature>
	using signature_remove_noexcept_t = typename signature_remove_noexcept<Signature>::type;

	template <typename Signature>
	struct signature_decay;

	template <typename Signature>
	using signature_decay_t = typename signature_decay<Signature>::type;

	template <typename Signature>
	struct signature_return_type;

	template <typename Signature>
	using signature_return_type_t = typename signature_return_type<Signature>::type;

	template <std::size_t index, typename Signature>
	struct signature_param_type;

	template <std::size_t index, typename Signature>
	using signature_param_type_t = typename signature_param_type<index, Signature>::type;

	template <typename Signature>
	struct signature_param_list;

	template <typename Signature>
	using signature_param_list_t = typename signature_param_list<Signature>::type;

	template <typename First, typename Second>
	struct is_overloadable_with;

	template <typename First, typename Second>
	inline constexpr bool is_overloadable_with_v = is_overloadable_with<First, Second>::value;

	template <typename First, typename... Others>
	struct is_overload_set;

	template <typename First, typename... Others>
	inline constexpr bool is_overload_set_v = is_overload_set<First, Others...>::value;
}

namespace mimicpp
{
	template <typename Signature>
	class Expectation;

	class ScopedExpectation;

	enum class MatchResult
	{
		none,
		inapplicable,
		full
	};

	class CallReport;
	class MatchReport;
	class ExpectationReport;

	using CharT = char;
	using CharTraitsT = std::char_traits<CharT>;
	using StringT = std::basic_string<CharT, CharTraitsT>;
}

namespace mimicpp::sequence
{
	enum Tag
		: std::ptrdiff_t
	{
	};

	enum class Id
		: int
	{
	};

	struct rating
	{
		int priority{};
		Tag tag{};

		[[nodiscard]]
		friend bool operator==(const rating&, const rating&) = default;
	};
}

#endif
