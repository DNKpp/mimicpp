// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTER_HPP
#define MIMICPP_REPORTER_HPP

#pragma once

#include "mimic++/Call.hpp"

#include <memory>
#include <vector>

namespace mimicpp
{
	template <typename Return, typename... Params>
	[[noreturn]]
	void report_fail(
		const call::Info<Return, Params...>& callInfo,
		std::vector<call::MatchResult_NoT> results
	);

	template <typename Return, typename... Params>
	[[noreturn]]
	void report_fail(
		const call::Info<Return, Params...>& callInfo,
		std::vector<call::MatchResult_ExhaustedT> results
	);

	template <typename Return, typename... Params>
	void report_ok(
		const call::Info<Return, Params...>& callInfo,
		call::MatchResult_OkT result
	);

	template <typename Signature>
	class Expectation;

	template <typename Return, typename... Params, typename Signature>
	void report_unhandled_exception(
		const call::Info<Return, Params...>& callInfo,
		std::shared_ptr<Expectation<Signature>> expectation,
		std::exception_ptr exception
	);

	template <typename Signature>
	void report_unsatisfied_expectation(
		std::shared_ptr<Expectation<Signature>> expectation
	);
}

#endif
