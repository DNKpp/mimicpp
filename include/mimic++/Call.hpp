// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CALL_HPP
#define MIMICPP_CALL_HPP

#pragma once

#include "mimic++/TypeTraits.hpp"
#include "mimic++/Utility.hpp"

#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <source_location>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

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

namespace mimicpp::call
{
	enum class MatchCategory
	{
		no,
		non_applicable,
		ok
	};
}

template <typename Char>
struct std::formatter<mimicpp::call::MatchCategory, Char>
	: public std::formatter<std::basic_string_view<Char>, Char>
{
	using MatchCategoryT = mimicpp::call::MatchCategory;

	auto format(
		const MatchCategoryT category,
		auto& ctx
	) const
	{
		constexpr auto toString = [](const MatchCategoryT cat)
		{
			switch (cat)
			{
			case MatchCategoryT::no: return "no match";
			case MatchCategoryT::non_applicable: return "non applicable match";
			case MatchCategoryT::ok: return "full match";
			}

			throw std::runtime_error{"Unknown category value."};
		};

		return std::formatter<std::basic_string_view<Char>, Char>::format(
			toString(category),
			ctx);
	}
};

namespace mimicpp::call
{
	class SubMatchResult
	{
	public:
		bool matched{};
		std::optional<std::string> msg{};

		[[nodiscard]]
		friend bool operator ==(const SubMatchResult&, const SubMatchResult&) = default;
	};

	template <MatchCategory category>
	class GenericMatchResult
		: public std::integral_constant<MatchCategory, category>
	{
	public:
		std::vector<SubMatchResult> subMatchResults{};

		[[nodiscard]]
		friend bool operator ==(const GenericMatchResult&, const GenericMatchResult&) = default;
	};

	using MatchResult_NoT = GenericMatchResult<MatchCategory::no>;
	using MatchResult_NotApplicableT = GenericMatchResult<MatchCategory::non_applicable>;
	using MatchResult_OkT = GenericMatchResult<MatchCategory::ok>;

	using MatchResultT = std::variant<
		MatchResult_NoT,
		MatchResult_NotApplicableT,
		MatchResult_OkT
	>;
}

namespace mimicpp::call::detail
{
	[[nodiscard]]
	inline MatchResultT evaluate_sub_match_results(const bool isApplicable, std::vector<SubMatchResult> subResults) noexcept
	{
		static_assert(3 == std::variant_size_v<MatchResultT>, "Unexpected MatchResult alternative count.");

		if (!std::ranges::all_of(subResults, &SubMatchResult::matched))
		{
			return MatchResult_NoT{
				.subMatchResults = std::move(subResults)
			};
		}

		if (!isApplicable)
		{
			return MatchResult_NotApplicableT{
				.subMatchResults = std::move(subResults)
			};
		}

		return MatchResult_OkT{
			.subMatchResults = std::move(subResults)
		};
	}
}

namespace mimicpp::detail
{
	template <typename Derived, typename Base>
	[[nodiscard]]
	constexpr const Derived& derived_cast(const Base& self) noexcept
	{
		static_assert(
			std::derived_from<Derived, Derived>,
			"Derived must inherit from Base.");
		return static_cast<const Derived&>(self);
	}
}

#endif
