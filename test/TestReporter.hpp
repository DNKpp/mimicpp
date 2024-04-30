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

struct unhandled_exception_info
{
	std::any call{};
	std::any expectation{};
	std::exception_ptr exception{};
};
inline std::vector<unhandled_exception_info> g_UnhandledExceptions{};
inline std::vector<std::any> g_UnsatisfiedExpectations{};
inline std::vector<std::string> g_Errors{};

class NoMatchError
{
};

class ExhaustedMatchError
{
};

namespace mimicpp
{
	template <typename Return, typename... Params>
	void report_fail(
		const call::Info<Return, Params...>& callInfo,
		std::vector<call::MatchResult_NoT> results
	)
	{
		g_NoMatchResults.insert(
			std::ranges::end(g_NoMatchResults),
			std::ranges::begin(results),
			std::ranges::end(results));

		throw NoMatchError{};
	}

	template <typename Return, typename... Params>
	void report_fail(
		const call::Info<Return, Params...>& callInfo,
		std::vector<call::MatchResult_ExhaustedT> results
	)
	{
		g_ExhaustedMatchResults.insert(
			std::ranges::end(g_ExhaustedMatchResults),
			std::ranges::begin(results),
			std::ranges::end(results));

		throw ExhaustedMatchError{};
	}

	template <typename Return, typename... Params>
	void report_ok(
		const call::Info<Return, Params...>& callInfo,
		call::MatchResult_OkT result
	)
	{
		g_OkMatchResults.emplace_back(std::move(result));
	}

	void report_error(StringT message)
	{
		g_Errors.emplace_back(std::move(message));
	}

	template <typename Return, typename... Params, typename Signature>
	void report_unhandled_exception(
		const call::Info<Return, Params...>& callInfo,
		std::shared_ptr<Expectation<Signature>> expectation,
		std::exception_ptr exception
	)
	{
		g_UnhandledExceptions.emplace_back(
			unhandled_exception_info{
				.call = callInfo,
				.expectation = std::move(expectation),
				.exception = exception
			});
	}

	template <typename Signature>
	void report_unsatisfied_expectation(
		std::shared_ptr<Expectation<Signature>> expectation
	)
	{
		g_UnsatisfiedExpectations.emplace_back(std::move(expectation));
	}

	class ScopedReporter
	{
	public:
		// ReSharper disable CppMemberFunctionMayBeStatic

		~ScopedReporter() noexcept
		{
			clear();
		}

		ScopedReporter() noexcept
		{
			clear();
		}

		void clear()
		{
			g_OkMatchResults.clear();
			g_ExhaustedMatchResults.clear();
			g_NoMatchResults.clear();
			g_Errors.clear();
			g_UnhandledExceptions.clear();
			g_UnsatisfiedExpectations.clear();
		}

		ScopedReporter(const ScopedReporter&) = delete;
		ScopedReporter& operator =(const ScopedReporter&) = delete;
		ScopedReporter(ScopedReporter&&) = delete;
		ScopedReporter& operator =(ScopedReporter&&) = delete;

		auto& no_match_reports() noexcept
		{
			return g_NoMatchResults;
		}

		auto& exhausted_match_reports() noexcept
		{
			return g_ExhaustedMatchResults;
		}

		auto& ok_match_reports() noexcept
		{
			return g_OkMatchResults;
		}

		auto& errors() noexcept
		{
			return g_Errors;
		}

		auto& unhandled_exceptions() noexcept
		{
			return g_UnhandledExceptions;
		}

		auto& unsatisfied_expectations() noexcept
		{
			return g_UnsatisfiedExpectations;
		}

		// ReSharper restore CppMemberFunctionMayBeStatic
	};
}
