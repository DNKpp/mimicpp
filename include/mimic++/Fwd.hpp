// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_FWD_HPP
#define MIMICPP_FWD_HPP

#pragma once

#include <string>
#include <variant>

namespace mimicpp::call
{
	enum class MatchCategory
	{
		no,
		non_applicable,
		ok
	};

	template <MatchCategory category>
	class GenericMatchResult;

	using MatchResult_NoT = GenericMatchResult<MatchCategory::no>;
	using MatchResult_NotApplicableT = GenericMatchResult<MatchCategory::non_applicable>;
	using MatchResult_OkT = GenericMatchResult<MatchCategory::ok>;

	using MatchResultT = std::variant<
		MatchResult_NoT,
		MatchResult_NotApplicableT,
		MatchResult_OkT
	>;

	template <typename Return, typename... Args>
	class Info;
}

namespace mimicpp
{
	template <typename Signature>
	class Expectation;

	using CharT = char;
	using CharTraitsT = std::char_traits<CharT>;
	using StringT = std::basic_string<CharT, CharTraitsT>;
}

#endif
