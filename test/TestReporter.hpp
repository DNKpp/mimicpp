// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mimic++/Reporter.hpp"

#include <any>
#include <ranges>

#define MIMICPP_REPORTER_DEFINED

inline std::vector<mimicpp::call::MatchResult_NoT> g_NoMatchResults{};
inline std::vector<mimicpp::call::MatchResult_ExhaustedT> g_ExhaustedMatchResults{};
inline std::vector<mimicpp::call::MatchResult_OkT> g_OkMatchResults{};
inline std::vector<std::any> g_UnsatisfiedExpectations{};

class TestExpectationError
{
};

namespace mimicpp
{
	template <typename Return, typename... Params>
	void report_fail(
		call::Info<Return, Params...> callInfo,
		std::vector<call::MatchResult_NoT> results
	)
	{
		g_NoMatchResults.insert(
			std::ranges::end(g_NoMatchResults),
			std::ranges::begin(results),
			std::ranges::end(results));

		throw TestExpectationError{};
	}

	template <typename Return, typename... Params>
	void report_fail(
		call::Info<Return, Params...> callInfo,
		std::vector<call::MatchResult_ExhaustedT> results
	)
	{
		g_ExhaustedMatchResults.insert(
			std::ranges::end(g_ExhaustedMatchResults),
			std::ranges::begin(results),
			std::ranges::end(results));

		throw TestExpectationError{};
	}

	template <typename Signature>
	void report_unsatisfied_expectation(
		std::shared_ptr<Expectation<Signature>> expectation
	)
	{
		g_UnsatisfiedExpectations.emplace_back(std::move(expectation));
	}

	template <typename Return, typename... Params>
	void report_ok(
		call::Info<Return, Params...> callInfo,
		call::MatchResult_OkT result
	)
	{
		g_OkMatchResults.emplace_back(std::move(result));
	}

	class ScopedReporter
	{
	public:
		~ScopedReporter() noexcept
		{
			g_OkMatchResults.clear();
			g_ExhaustedMatchResults.clear();
			g_NoMatchResults.clear();
			g_UnsatisfiedExpectations.clear();
		}

		ScopedReporter() noexcept
		{
			g_OkMatchResults.clear();
			g_ExhaustedMatchResults.clear();
			g_NoMatchResults.clear();
			g_UnsatisfiedExpectations.clear();
		}

		ScopedReporter(const ScopedReporter&) = delete;
		ScopedReporter& operator =(const ScopedReporter&) = delete;
		ScopedReporter(ScopedReporter&&) = delete;
		ScopedReporter& operator =(ScopedReporter&&) = delete;

		// ReSharper disable once CppMemberFunctionMayBeStatic
		auto& no_match_reports() noexcept
		{
			return g_NoMatchResults;
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		auto& exhausted_match_reports() noexcept
		{
			return g_ExhaustedMatchResults;
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		auto& ok_match_reports() noexcept
		{
			return g_OkMatchResults;
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		auto& unsatisfied_expectations() noexcept
		{
			return g_UnsatisfiedExpectations;
		}
	};
}
