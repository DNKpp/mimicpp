// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CALL_HPP
#define MIMICPP_CALL_HPP

#pragma once

#include <format>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace mimicpp::call
{
	enum class ValueCategory
	{
		lvalue,
		rvalue
	};

	template <typename Signature>
	class Info;

	template <typename Return, typename... Args>
	class Info<Return(Args...)>
	{
	public:
		using ParamListT = std::tuple<std::reference_wrapper<std::remove_reference_t<Args>>...>;
		using UuidT = std::ptrdiff_t;

		ParamListT params;
		UuidT fromUuid{};
		ValueCategory fromCategory{};
		bool fromConst{};
	};
}

namespace mimicpp::call
{
	enum class MatchCategory
	{
		no,
		partial,
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
			case MatchCategoryT::partial: return "MatchCategory::partial";
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
	template <MatchCategory category>
	struct GenericSubMatchResult
		: public std::integral_constant<MatchCategory, category>
	{
		std::optional<std::string> msg{};
	};

	using SubMatchResult_NoT = GenericSubMatchResult<MatchCategory::no>;
	using SubMatchResult_PartialT = GenericSubMatchResult<MatchCategory::partial>;
	using SubMatchResult_OkT = GenericSubMatchResult<MatchCategory::ok>;

	using SubMatchResultT = std::variant<
		SubMatchResult_NoT,
		SubMatchResult_PartialT,
		SubMatchResult_OkT
	>;

	template <MatchCategory category>
	struct GenericMatchResult
		: public std::integral_constant<MatchCategory, category>
	{
		std::vector<GenericSubMatchResult<category>> subMatchResults{};
	};

	using MatchResult_NoT = GenericMatchResult<MatchCategory::no>;
	using MatchResult_PartialT = GenericMatchResult<MatchCategory::partial>;
	using MatchResult_OkT = GenericMatchResult<MatchCategory::ok>;

	using MatchResultT = std::variant<
		MatchResult_NoT,
		MatchResult_PartialT,
		MatchResult_OkT
	>;
}

namespace mimicpp::call::detail
{
	[[nodiscard]]
	inline MatchResultT evaluate_sub_match_results(std::vector<SubMatchResultT> subResults)
	{
		std::vector<SubMatchResult_NoT> noResults{};
		std::vector<SubMatchResult_PartialT> partialResults{};
		std::vector<SubMatchResult_OkT> okResults{};

		static_assert(3 == std::variant_size_v<SubMatchResultT>, "Unexpected SubMatchResult alternative count.");
		static_assert(3 == std::variant_size_v<MatchResultT>, "Unexpected MatchResult alternative count.");

		for (auto& result : subResults)
		{
			if (std::holds_alternative<SubMatchResult_NoT>(result))
			{
				noResults.emplace_back(std::get<SubMatchResult_NoT>(std::move(result)));
			}
			else if (std::holds_alternative<SubMatchResult_PartialT>(result))
			{
				partialResults.emplace_back(std::get<SubMatchResult_PartialT>(std::move(result)));
			}
			else
			{
				okResults.emplace_back(std::get<SubMatchResult_OkT>(std::move(result)));
			}
		}

		if (!std::ranges::empty(noResults))
		{
			return MatchResult_NoT{
				.subMatchResults = std::move(noResults)
			};
		}

		if (!std::ranges::empty(partialResults))
		{
			return MatchResult_PartialT{
				.subMatchResults = std::move(partialResults)
			};
		}

		return MatchResult_OkT{
			.subMatchResults = std::move(okResults)
		};
	}
}

#endif
