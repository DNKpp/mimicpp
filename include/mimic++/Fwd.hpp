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
	class Expectation;

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
}

namespace mimicpp::sequence::detail
{
	struct sequence_rating
	{
		int priority{};
		Tag tag{};

		[[nodiscard]]
		friend bool operator==(const sequence_rating&, const sequence_rating&) = default;
	};
}

#endif
