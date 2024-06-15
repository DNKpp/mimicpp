// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_ADAPTERS_GTEST_HPP
#define MIMICPP_ADAPTERS_GTEST_HPP

#pragma once

#include "mimic++/Reporter.hpp"

#include <iterator>

#if __has_include(<gtest/gtest.h>)
#include <gtest/gtest.h>
#else
	#error "Unable to find gtest includes."
#endif

namespace mimicpp::detail::gtest
{
	struct failure
	{
	};

	[[noreturn]]
	inline void send_fail(const StringT& msg)
	{
		// GTEST_FAIL has an immediate return
		std::invoke(
			[&]
			{
				GTEST_FAIL() << msg.data();
			});

		throw failure{};
	}

	// This indicates, whether success shall be reported.
	// It's main use-case is to (temporarily) disable success reporting in the reporter test,
	// because it seems, that these are somehow treated as fatal failures?!
	inline std::atomic_bool g_EnableSuccessReporting{true};

	inline void send_succeed(const StringT& msg)
	{
		if (g_EnableSuccessReporting)
		{
			GTEST_SUCCEED() << msg.data();
		}
	}

	inline void send_warn([[maybe_unused]] const StringT& msg)
	{
		// seems unsupported
	}
}

namespace mimicpp
{
	/**
	 * \brief Reporter for the integration into gtest.
	 * \ingroup REPORTING_ADAPTERS
	 * \details This reporter enables the integration of ``mimic++`` into ``gtest``.
	 *
	 * This reporter installs itself by simply including this header file into any source file of the test executable.
	 */
	class GTestReporter final
		: public IReporter
	{
	public:
		[[noreturn]]
		void report_no_matches(const CallReport call, const std::vector<MatchReport> matchReports) override
		{
			StringStreamT ss{};
			format_to(
				std::ostreambuf_iterator{ss},
				"No match for {}\n",
				stringify_call_report(call));

			if (std::ranges::empty(matchReports))
			{
				ss << "No expectations available.\n";
			}
			else
			{
				format_to(
					std::ostreambuf_iterator{ss},
					"{} available expectations:\n",
					std::ranges::size(matchReports));

				for (const auto& report : matchReports)
				{
					ss << stringify_match_report(report) << "\n";
				}
			}

			detail::gtest::send_fail(ss.str());
		}

		[[noreturn]]
		void report_inapplicable_matches(const CallReport call, const std::vector<MatchReport> matchReports) override
		{
			StringStreamT ss{};
			format_to(
				std::ostreambuf_iterator{ss},
				"No applicable match for {}\n",
				stringify_call_report(call));

			ss << "Tested expectations:\n";
			for (const auto& report : matchReports)
			{
				ss << stringify_match_report(report) << "\n";
			}

			detail::gtest::send_fail(ss.str());
		}

		void report_full_match(const CallReport call, const MatchReport matchReport) noexcept override
		{
			StringStreamT ss{};
			format_to(
				std::ostreambuf_iterator{ss},
				"Found match for {}\n",
				stringify_call_report(call));

			ss << stringify_match_report(matchReport) << "\n";

			detail::gtest::send_succeed(ss.str());
		}

		void report_unfulfilled_expectation(const ExpectationReport expectationReport) override
		{
			if (0 == std::uncaught_exceptions())
			{
				StringStreamT ss{};
				ss << "Unfulfilled expectation:\n"
					<< stringify_expectation_report(expectationReport) << "\n";

				detail::gtest::send_fail(ss.str());
			}
		}

		void report_error(const StringT message) override
		{
			if (0 == std::uncaught_exceptions())
			{
				detail::gtest::send_fail(message);
			}
		}

		void report_unhandled_exception(
			const CallReport call,
			const ExpectationReport expectationReport,
			const std::exception_ptr exception
		) override
		{
			StringStreamT ss{};
			ss << "Unhandled exception: ";

			try
			{
				std::rethrow_exception(exception);
			}
			catch (const std::exception& e)
			{
				format_to(
					std::ostreambuf_iterator{ss},
					"what: {}\n",
					e.what());
			}
			catch (...)
			{
				ss << "Unknown exception type.\n";
			}

			format_to(
				std::ostreambuf_iterator{ss},
				"while checking expectation:\n"
				"{}\n",
				stringify_expectation_report(expectationReport));

			format_to(
				std::ostreambuf_iterator{ss},
				"For {}\n",
				stringify_call_report(call));

			detail::gtest::send_warn(ss.str());
		}
	};
}

namespace mimicpp::detail::gtest
{
	inline const ReporterInstaller<GTestReporter> installer{};
}

#endif
