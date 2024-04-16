// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTER_HPP
#define MIMICPP_REPORTER_HPP

#pragma once

#include "mimic++/Call.hpp"

#include <vector>

namespace mimicpp
{
	template <typename Signature>
	void report_fail(
		call::Info<Signature> callInfo,
		std::vector<call::MatchResult_NoT> results
	);

	template <typename Signature>
	void report_fail(
		call::Info<Signature> callInfo,
		std::vector<call::MatchResult_PartialT> results
	);

	template <typename Signature>
	void report_ok(
		call::Info<Signature> callInfo,
		call::MatchResult_OkT result
	);
}

#endif
