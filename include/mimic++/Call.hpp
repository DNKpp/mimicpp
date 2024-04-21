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

namespace mimicpp::call
{
	template <typename Return, typename... Args>
	class Info
	{
	public:
		using ParamListT = std::tuple<std::reference_wrapper<std::remove_reference_t<Args>>...>;

		ParamListT params;
		Uuid fromUuid{};
		ValueCategory fromCategory{};
		Constness fromConstness{};
		source_location_data fromSourceLocation{std::source_location::current()};
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
		exhausted,
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
		std::format_context& ctx
	) const
	{
		constexpr auto toString = [](const MatchCategoryT cat)
		{
			switch (cat)
			{
			case MatchCategoryT::no: return "MatchCategory::no";
			case MatchCategoryT::exhausted: return "MatchCategory::exhausted";
			case MatchCategoryT::ok: return "MatchCategory::ok";
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
	using MatchResult_ExhaustedT = GenericMatchResult<MatchCategory::exhausted>;
	using MatchResult_OkT = GenericMatchResult<MatchCategory::ok>;

	using MatchResultT = std::variant<
		MatchResult_NoT,
		MatchResult_ExhaustedT,
		MatchResult_OkT
	>;
}

namespace mimicpp::call::detail
{
	[[nodiscard]]
	inline MatchResultT evaluate_sub_match_results(const bool isSaturated, std::vector<SubMatchResult> subResults) noexcept
	{
		static_assert(3 == std::variant_size_v<MatchResultT>, "Unexpected MatchResult alternative count.");

		if (!std::ranges::all_of(subResults, &SubMatchResult::matched))
		{
			return MatchResult_NoT{
				.subMatchResults = std::move(subResults)
			};
		}

		if (isSaturated)
		{
			return MatchResult_ExhaustedT{
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

namespace mimicpp
{
	//template <typename Signature, typename Derived>
	//class EnableCallOperator;

	//template <typename Return, typename... Params, typename Derived>
	//class EnableCallOperator<Return(Params...), Derived>
	//{
	//	using SourceLocT = source_location_data;
	//	using CallInfoT = call::Info<Return(Params...)>;

	//public:
	//	template <SourceLocT from = SourceLocT{std::source_location::current()}>
	//	constexpr Return operator ()(Params... params)
	//	{
	//		return detail::derived_cast<Derived>()
	//			.handle_call(
	//				CallInfoT{
	//					.params = {std::ref(params)...},
	//					.fromCategory = ValueCategory::any,
	//					.fromConstness = Constness::non_const,
	//					.fromSourceLocation = from
	//				});
	//	}
	//};

	//template <typename Return, typename... Params, typename Derived>
	//class EnableCallOperator<Return(Params...) const, Derived>
	//{
	//	using SourceLocT = source_location_data;
	//	using CallInfoT = call::Info<Return(Params...)>;

	//public:
	//	template <SourceLocT from = SourceLocT{std::source_location::current()}>
	//	constexpr Return operator ()(Params... params) const
	//	{
	//		return detail::derived_cast<Derived>()
	//			.handle_call(
	//				CallInfoT{
	//					.params = {std::ref(params)...},
	//					.fromCategory = ValueCategory::any,
	//					.fromConstness = Constness::as_const,
	//					.fromSourceLocation = from
	//				});
	//	}
	//};
}

#endif
