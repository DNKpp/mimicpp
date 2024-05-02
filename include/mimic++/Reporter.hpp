// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTER_HPP
#define MIMICPP_REPORTER_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/Printer.hpp"

#include <exception>
#include <memory>
#include <vector>

namespace mimicpp
{
	template <typename Signature>
	class Expectation;

	class DefaultReporter
	{
	public:
		template <typename Return, typename... Params>
		[[noreturn]]
		static void report_fail(
			const call::Info<Return, Params...>& callInfo,
			const std::vector<call::MatchResult_NoT>& results
		)
		{
			std::terminate();
		}

		template <typename Return, typename... Params>
		[[noreturn]]
		static void report_fail(
			const call::Info<Return, Params...>& callInfo,
			const std::vector<call::MatchResult_NotApplicableT>& results
		)
		{
			std::terminate();
		}

		template <typename Return, typename... Params>
		static constexpr void report_ok(
			const call::Info<Return, Params...>& callInfo,
			const call::MatchResult_OkT& result
		)
		{
		}

		static void report_error(const StringT& message)
		{
			if (0 != std::uncaught_exceptions())
			{
				std::terminate();
			}
		}

		template <typename Return, typename... Params, typename Signature>
		static constexpr void report_unhandled_exception(
			const call::Info<Return, Params...>& callInfo,
			std::shared_ptr<Expectation<Signature>> expectation,
			std::exception_ptr exception
		)
		{
		}

		template <typename Signature>
		static constexpr void report_unsatisfied_expectation(
			std::shared_ptr<Expectation<Signature>> expectation
		)
		{
			if (0 != std::uncaught_exceptions())
			{
				std::terminate();
			}
		}
	};

	[[nodiscard]]
	constexpr auto create_reporter() noexcept
	{
#ifdef MIMICPP_CUSTOM_REPORTER
		return MIMICPP_CUSTOM_REPORTER{};
#else
		return DefaultReporter{};
#endif
	}

	template <typename Return, typename... Params>
	[[noreturn]]
	void report_fail(
		const call::Info<Return, Params...>& callInfo,
		std::vector<call::MatchResult_NoT> results
	)
	{
		auto reporter = create_reporter();
		reporter.report_fail(
			callInfo,
			std::move(results));
	}

	template <typename Return, typename... Params>
	[[noreturn]]
	void report_fail(
		const call::Info<Return, Params...>& callInfo,
		std::vector<call::MatchResult_NotApplicableT> results
	)
	{
		auto reporter = create_reporter();
		reporter.report_fail(
			callInfo,
			std::move(results));
	}

	template <typename Return, typename... Params>
	void report_ok(
		const call::Info<Return, Params...>& callInfo,
		call::MatchResult_OkT result
	)
	{
		auto reporter = create_reporter();
		reporter.report_ok(
			callInfo,
			std::move(result));
	}

	inline void report_error(StringT message)
	{
		auto reporter = create_reporter();
		reporter.report_error(
			std::move(message));
	}

	template <typename Return, typename... Params, typename Signature>
	void report_unhandled_exception(
		const call::Info<Return, Params...>& callInfo,
		std::shared_ptr<Expectation<Signature>> expectation,
		std::exception_ptr exception
	)
	{
		auto reporter = create_reporter();
		reporter.report_unhandled_exception(
			callInfo,
			std::move(expectation),
			std::move(exception));
	}

	template <typename Signature>
	void report_unsatisfied_expectation(
		std::shared_ptr<Expectation<Signature>> expectation
	)
	{
		auto reporter = create_reporter();
		reporter.report_unsatisfied_expectation(
			std::move(expectation));
	}
}

#endif
