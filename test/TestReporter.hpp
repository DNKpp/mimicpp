// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mimic++/Fwd.hpp"

#include <any>
#include <exception>
#include <memory>
#include <ranges>
#include <vector>

class NoMatchError
{
};

class NonApplicableMatchError
{
};

class TestReporter
{
public:
	template <typename Return, typename... Params>
	[[noreturn]]
	static void report_fail(
		const mimicpp::call::Info<Return, Params...>& callInfo,
		const std::vector<mimicpp::call::MatchResult_NoT>& results
	);

	template <typename Return, typename... Params>
	[[noreturn]]
	static void report_fail(
		const mimicpp::call::Info<Return, Params...>& callInfo,
		const std::vector<mimicpp::call::MatchResult_NotApplicableT>& results
	);

	template <typename Return, typename... Params>
	static constexpr void report_ok(
		const mimicpp::call::Info<Return, Params...>& callInfo,
		const mimicpp::call::MatchResult_OkT& result
	);

	static void report_error(const mimicpp::StringT& message);

	template <typename Return, typename... Params, typename Signature>
	static constexpr void report_unhandled_exception(
		const mimicpp::call::Info<Return, Params...>& callInfo,
		std::shared_ptr<mimicpp::Expectation<Signature>> expectation,
		std::exception_ptr exception
	);

	template <typename Signature>
	static constexpr void report_unsatisfied_expectation(
		std::shared_ptr<mimicpp::Expectation<Signature>> expectation
	);
};

#define MIMICPP_CUSTOM_REPORTER TestReporter

#include "mimic++/Reporter.hpp"

inline std::vector<mimicpp::call::MatchResult_NoT> g_NoMatchResults{};
inline std::vector<mimicpp::call::MatchResult_NotApplicableT> g_NonApplicableMatchResults{};
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

template <typename Return, typename... Params>
void TestReporter::report_fail(
	const mimicpp::call::Info<Return, Params...>& callInfo,
	const std::vector<mimicpp::call::MatchResult_NoT>& results
)
{
	g_NoMatchResults.insert(
		std::ranges::end(g_NoMatchResults),
		std::ranges::begin(results),
		std::ranges::end(results));

	throw NoMatchError{};
}

template <typename Return, typename... Params>
void TestReporter::report_fail(
	const mimicpp::call::Info<Return, Params...>& callInfo,
	const std::vector<mimicpp::call::MatchResult_NotApplicableT>& results
)
{
	g_NonApplicableMatchResults.insert(
		std::ranges::end(g_NonApplicableMatchResults),
		std::ranges::begin(results),
		std::ranges::end(results));

	throw NonApplicableMatchError{};
}

template <typename Return, typename... Params>
constexpr void TestReporter::report_ok(
	const mimicpp::call::Info<Return, Params...>& callInfo,
	const mimicpp::call::MatchResult_OkT& result
)
{
	g_OkMatchResults.emplace_back(std::move(result));
}

inline void TestReporter::report_error(const mimicpp::StringT& message)
{
	g_Errors.emplace_back(std::move(message));
}

template <typename Return, typename... Params, typename Signature>
constexpr void TestReporter::report_unhandled_exception(
	const mimicpp::call::Info<Return, Params...>& callInfo,
	std::shared_ptr<mimicpp::Expectation<Signature>> expectation,
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
constexpr void TestReporter::report_unsatisfied_expectation(std::shared_ptr<mimicpp::Expectation<Signature>> expectation)
{
	g_UnsatisfiedExpectations.emplace_back(std::move(expectation));
}

namespace mimicpp
{
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
			g_NonApplicableMatchResults.clear();
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

		auto& non_applicable_match_reports() noexcept
		{
			return g_NonApplicableMatchResults;
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
