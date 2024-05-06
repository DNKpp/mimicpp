// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mimic++/Reporter.hpp"

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

class TestReporter final
	: public mimicpp::IReporter
{
public:
	using call_report_t = mimicpp::CallReport;
	using expectation_report_t = mimicpp::ExpectationReport;
	using match_report_t = mimicpp::MatchReport;

	std::vector<std::tuple<call_report_t, match_report_t>> noMatchResults{};

	[[noreturn]]
	void report_no_matches(
		call_report_t call,
		std::vector<mimicpp::MatchReport> matchReports
	) override
	{
		for (auto& exp : matchReports)
		{
			noMatchResults.emplace_back(
				call,
				std::move(exp));
		}

		throw NoMatchError{};
	}

	std::vector<std::tuple<call_report_t, match_report_t>> inapplicableMatchResults{};

	[[noreturn]]
	void report_inapplicable_matches(
		call_report_t call,
		std::vector<mimicpp::MatchReport> matchReports
	) override
	{
		for (auto& exp : matchReports)
		{
			inapplicableMatchResults.emplace_back(
				call,
				std::move(exp));
		}

		throw NonApplicableMatchError{};
	}

	std::vector<std::tuple<call_report_t, match_report_t>> fullMatchResults{};

	void report_full_match(
		call_report_t call,
		mimicpp::MatchReport matchReport
	) noexcept override
	{
		fullMatchResults.emplace_back(
			std::move(call),
			std::move(matchReport));
	}

	std::vector<expectation_report_t> unfulfilledExpectations{};

	void report_unfulfilled_expectation(
		expectation_report_t expectationReport
	) override
	{
		unfulfilledExpectations.emplace_back(std::move(expectationReport));
	}

	std::vector<mimicpp::StringT> errors{};

	void report_error(mimicpp::StringT message) override
	{
		errors.emplace_back(std::move(message));
	}

	struct unhandled_exception_info
	{
		call_report_t call;
		expectation_report_t expectation{};
		std::exception_ptr exception;
	};

	std::vector<unhandled_exception_info> unhandledExceptions{};

	void report_unhandled_exception(
		call_report_t call,
		expectation_report_t expectationReport,
		std::exception_ptr exception
	) override
	{
		unhandledExceptions.emplace_back(
			std::move(call),
			std::move(expectationReport),
			std::move(exception));
	}
};

class ScopedReporter
{
public:
	~ScopedReporter() noexcept
	{
		mimicpp::install_reporter<mimicpp::DefaultReporter>();
	}

	ScopedReporter() noexcept
	{
		mimicpp::install_reporter<TestReporter>();
	}

	ScopedReporter(const ScopedReporter&) = delete;
	ScopedReporter& operator =(const ScopedReporter&) = delete;
	ScopedReporter(ScopedReporter&&) = delete;
	ScopedReporter& operator =(ScopedReporter&&) = delete;

	auto& no_match_reports() noexcept
	{
		return reporter()
			.noMatchResults;
	}

	auto& inapplicable_match_reports() noexcept
	{
		return reporter()
			.inapplicableMatchResults;
	}

	auto& full_match_reports() noexcept
	{
		return reporter()
			.fullMatchResults;
	}

	auto& errors() noexcept
	{
		return reporter()
			.errors;
	}

	auto& unhandled_exceptions() noexcept
	{
		return reporter()
			.unhandledExceptions;
	}

	auto& unfulfilled_expectations() noexcept
	{
		return reporter()
			.unfulfilledExpectations;
	}

private:
	[[nodiscard]]
	static const TestReporter& reporter()
	{
		return dynamic_cast<const TestReporter&>(
			*mimicpp::detail::get_reporter());
	}
};
