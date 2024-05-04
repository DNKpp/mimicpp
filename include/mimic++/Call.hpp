// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CALL_HPP
#define MIMICPP_CALL_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/Utility.hpp"

#include <source_location>
#include <tuple>
#include <utility>

namespace mimicpp::call::detail
{
	template <typename... Args, std::size_t... indices>
	[[nodiscard]]
	constexpr bool is_equal_param_list(
		const std::tuple<std::reference_wrapper<Args>...>& lhs,
		const std::tuple<std::reference_wrapper<Args>...>& rhs,
		const std::index_sequence<indices...>
	) noexcept
	{
		return (
			...
			&& (std::addressof(
					std::get<indices>(lhs).get())
				== std::addressof(
					std::get<indices>(rhs).get())));
	}
}

namespace mimicpp::call
{
	template <typename Return, typename... Args>
	class Info
	{
	public:
		using ArgListT = std::tuple<std::reference_wrapper<std::remove_reference_t<Args>>...>;

		ArgListT args;
		ValueCategory fromCategory{};
		Constness fromConstness{};
		std::source_location fromSourceLocation{};

		[[nodiscard]]
		friend bool operator ==(const Info& lhs, const Info& rhs)
		{
			return lhs.fromCategory == rhs.fromCategory
					&& lhs.fromConstness == rhs.fromConstness
					&& detail::is_equal_param_list(lhs.args, rhs.args, std::index_sequence_for<Args...>{})
					&& is_same_source_location(lhs.fromSourceLocation, rhs.fromSourceLocation);
		}
	};

	template <typename Signature>
	struct info_for_signature
		: public info_for_signature<signature_decay_t<Signature>>
	{
	};

	template <typename Signature>
	using info_for_signature_t = typename info_for_signature<Signature>::type;

	template <typename Return, typename... Args>
	struct info_for_signature<Return(Args...)>
	{
		using type = Info<Return, Args...>;
	};
}

#endif
