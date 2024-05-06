#ifndef MIMICPP_ADAPTERS_CATCH2_HPP
#define MIMICPP_ADAPTERS_CATCH2_HPP

#pragma once

#include "mimic++/Reporter.hpp"

#include <iterator>

#if __has_include(<catch2/catch_test_macros.hpp>)
	#include <catch2/catch_test_macros.hpp>
#else
	#error "Unable to find catch2 includes."
#endif

namespace mimicpp::detail
{
	[[noreturn]]
	inline void send_catch_fail(const StringViewT msg)
	{
#ifdef CATCH_CONFIG_PREFIX_ALL
		CATCH_FAIL(msg);
#else
		FAIL(msg);
#endif

		unreachable();
	}

	inline void send_catch_succeed(const StringViewT msg)
	{
#ifdef CATCH_CONFIG_PREFIX_ALL
		CATCH_SUCCEED(msg);
#else
		SUCCEED(msg);
#endif
	}
	
	inline void send_catch_warn(const StringViewT msg)
	{
#ifdef CATCH_CONFIG_PREFIX_ALL
		CATCH_WARN(msg);
#else
		WARN(msg);
#endif
	}
}

namespace mimicpp
{
	class Catch2Reporter final
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

			detail::send_catch_fail(ss.view());
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

			detail::send_catch_fail(ss.view());
		}

		void report_full_match(const CallReport call, const MatchReport matchReport) noexcept override
		{
			StringStreamT ss{};
			format_to(
				std::ostreambuf_iterator{ss},
				"Found match for {}\n",
				stringify_call_report(call));

			ss << stringify_match_report(matchReport) << "\n";

			detail::send_catch_succeed(ss.view());
		}

		void report_unfulfilled_expectation(const ExpectationReport expectationReport) override
		{
			if (0 == std::uncaught_exceptions())
			{
				StringStreamT ss{};
				ss << "Unfulfilled expectation:\n"
					<< stringify_expectation_report(expectationReport) << "\n";

				detail::send_catch_fail(ss.view());
			}
		}

		void report_error(const StringT message) override
		{
			if (0 == std::uncaught_exceptions())
			{
				detail::send_catch_fail(message);
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

			detail::send_catch_warn(ss.view());
		}
	};
}

namespace mimicpp::detail
{
	inline const ReporterInstaller<Catch2Reporter> installer{};
}

#endif
