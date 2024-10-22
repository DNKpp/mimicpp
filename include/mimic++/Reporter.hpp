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
#include "mimic++/Reports.hpp"

#include <algorithm>
#include <cassert>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <vector>

namespace mimicpp
{
	/**
	 * \defgroup REPORTING reporting
	 * \brief Contains reporting related symbols
	 * \details Reporting is executed, when something notably has been detected by ``mimic++``; often it is expected, that the reporter
	 * reacts to such a report in a specific manner (e.g. aborting the test case). For example the ``DefaultReporter`` simply throws
	 * exceptions on error reports, while other more specialized reporters handle such cases slightly different (but still abort the
	 * current test).
	 * These specialized Reporters are used to send reports to a specific destination (e.g. the utilized unit-test framework),
	 * which often provide more advanced mechanics for printing failed tests to the users.
	 *
	 * Users may provide their own reporter implementation; e.g. if there is no reporter for the desired unit-test framework.
	 *
	 * At any time there exists exactly one global reporter, which may be directly or indirectly exchanged by users.
	 * Reports are sent to the currently installed reporter via the ``report_xyz`` free-functions. Most of those functions require, that
	 * reports are handled in a specific manner (e.g. ``report_no_matches`` is expected to never return) and custom reporters **must**
	 * follow that specification, otherwise this will lead to undefined behavior. For further details, have a look at the specific
	 * function documentation.
	 *
	 * \note In general users shall not directly interact with the installed reporter, except when they want to replace it.
	 *
	 * \{
	 */

	/**
	 * \brief The reporter interface.
	 * \details This is the central interface to be used, when creating reporters for external domains.
	 */
	class IReporter
	{
	public:
		/**
		 * \brief Defaulted virtual destructor.
		 */
		virtual ~IReporter() = default;

		/**
		 * \brief Expects reports about all ``none`` matching expectations. This is only called, if there are no better options available.
		 * \param call The call report. 
		 * \param matchReports Reports of all ``none`` matching expectations.
		 * \details This function is called, when no match has been found and there are no other expectations, which are matching but
		 * inapplicable. In fact, this is the fallback reporting mechanism, for unmatched calls.
		 * \note ``matchReports`` may be empty.
		 *
		 * \attention Derived reporter implementations must never return and shall instead leave the function via a thrown exception or
		 * a terminating mechanism (e.g. ``std::terminate``). Otherwise, this will result in undefined behavior.
		 */
		[[noreturn]]
		virtual void report_no_matches(
			CallReport call,
			std::vector<MatchReport> matchReports
		) = 0;

		/**
		 * \brief Expects reports about all ``inapplicable`` matching expectations. This is only called, if there are no better options available.
		 * \param call The call report. 
		 * \param matchReports Reports of all ``inapplicable`` matching expectations.
		 * \details This function is called, when no applicable match has been found, but actually the call expectations are fulfilled. This in fact
		 * happens, when the ``times`` policy is already saturated (e.g. it was once expected and already matched once) or otherwise not applicable
		 * (e.g. a sequence element is not the current element).
		 *
		 * \attention Derived reporter implementations must never return and shall instead leave the function via a thrown exception or
		 * a terminating mechanism (e.g. ``std::terminate``). Otherwise, this will result in undefined behavior.
		 */
		[[noreturn]]
		virtual void report_inapplicable_matches(
			CallReport call,
			std::vector<MatchReport> matchReports
		) = 0;

		/**
		 * \brief Expects the report about a ``full`` matching expectation.
		 * \param call The call report. 
		 * \param matchReport Report of the ``full`` matching expectation.
		 * \details This function is called, when a match has been found. There are no other expectations on the behavior of this function;
		 * except the ``noexcept`` guarantee. Implementations shall simply return to the caller.
		 */
		virtual void report_full_match(
			CallReport call,
			MatchReport matchReport
		) noexcept = 0;

		/**
		 * \brief Expects the report of an unfulfilled expectation.
		 * \param expectationReport The expectation report.
		 * \details This function is called, when an unfulfilled expectation goes out of scope. In fact this happens, when the ``times`` policy is not
		 * satisfied.
		 *
		 * \note In general, it is expected that this function does not return, but throws an exception instead. But, as this function is always called
		 * when an unfulfilled expectation goes out of scope, implementations shall check whether an uncaught exception already exists (e.g. via
		 * ``std::uncaught_exceptions``) before throwing by themselves.
		 * \see ``DefaultReporter::report_unfulfilled_expectation`` for an example.
		 */
		virtual void report_unfulfilled_expectation(
			ExpectationReport expectationReport
		) = 0;

		/**
		 * \brief Expects rather unspecific errors.
		 * \param message The error message.
		 * \details This function is called, when an unspecific error occurs.
		 *
		 * \note In general, it is expected that this function does not return, but throws an exception instead. But, as this function may be called
		 * due to any reason, implementations shall check whether an uncaught exception already exists (e.g. via ``std::uncaught_exceptions``) before
		 * throwing by themselves.
		 * \see ``DefaultReporter::report_error`` for an example.
		 */
		virtual void report_error(StringT message) = 0;

		/**
		 * \brief Expects reports about unhandled exceptions, during ``handle_call``.
		 * \param call The call report.
		 * \param expectationReport The expectation report.
		 * \param exception The exception.
		 * \details This function is called, when an expectation throws during a ``matches`` call. There are no expectations on the behavior of this
		 * function. As this function is called inside a ``catch`` block, throwing exceptions will result in a terminate call.
		 */
		virtual void report_unhandled_exception(
			CallReport call,
			ExpectationReport expectationReport,
			std::exception_ptr exception
		) = 0;

	protected:
		[[nodiscard]]
		IReporter() = default;

		IReporter(const IReporter&) = default;
		IReporter& operator =(const IReporter&) = default;
		IReporter(IReporter&&) = default;
		IReporter& operator =(IReporter&&) = default;
	};

	template <typename Data = std::nullptr_t>
	class Error final
		: public std::runtime_error
	{
	public:
		[[nodiscard]]
		explicit Error(
			const std::string& what,
			Data&& data = Data{},
			const std::source_location& loc = std::source_location::current()
		)
			: std::runtime_error{what},
			m_Data{std::move(data)},
			m_Loc{loc}
		{
		}

		[[nodiscard]]
		const Data& data() const noexcept
		{
			return m_Data;
		}

		[[nodiscard]]
		const std::source_location& where() const noexcept
		{
			return m_Loc;
		}

	private:
		Data m_Data;
		std::source_location m_Loc;
	};

	namespace detail
	{
		[[nodiscard]]
		inline StringT stringify_no_match_report(const CallReport& call, const std::span<const MatchReport> matchReports)
		{
			StringStreamT ss{};
			ss << "No match for ";
			mimicpp::print(
				std::ostreambuf_iterator{ss},
				call);
			ss << "\n";

			if (std::ranges::empty(matchReports))
			{
				ss << "No expectations available.\n";
			}
			else
			{
				format::format_to(
					std::ostreambuf_iterator{ss},
					"{} available expectation(s):\n",
					std::ranges::size(matchReports));

				for (const auto& report : matchReports)
				{
					mimicpp::print(
						std::ostreambuf_iterator{ss},
						report);
					ss << "\n";
				}
			}

			return std::move(ss).str();
		}

		[[nodiscard]]
		inline StringT stringify_inapplicable_match_report(const CallReport& call, const std::span<const MatchReport> matchReports)
		{
			StringStreamT ss{};
			ss << "No applicable match for ";
			mimicpp::print(
				std::ostreambuf_iterator{ss},
				call);
			ss << "\n";

			ss << "Tested expectations:\n";
			for (const auto& report : matchReports)
			{
				mimicpp::print(
					std::ostreambuf_iterator{ss},
					report);
				ss << "\n";
			}

			return std::move(ss).str();
		}

		[[nodiscard]]
		inline StringT stringify_report(const CallReport& call, const MatchReport& matchReport)
		{
			StringStreamT ss{};
			ss << "Found match for ";
			mimicpp::print(
				std::ostreambuf_iterator{ss},
				call);
			ss << "\n";

			mimicpp::print(
				std::ostreambuf_iterator{ss},
				matchReport);
			ss << "\n";

			return std::move(ss).str();
		}

		[[nodiscard]]
		inline StringT stringify_unfulfilled_expectation(const ExpectationReport& expectationReport)
		{
			StringStreamT ss{};
			ss << "Unfulfilled expectation:\n";
			mimicpp::print(
				std::ostreambuf_iterator{ss},
				expectationReport);
			ss << "\n";

			return std::move(ss).str();
		}

		[[nodiscard]]
		inline StringT stringify_unhandled_exception(
			const CallReport& call,
			const ExpectationReport& expectationReport,
			const std::exception_ptr& exception
		)
		{
			StringStreamT ss{};
			ss << "Unhandled exception: ";

			try
			{
				std::rethrow_exception(exception);
			}
			catch (const std::exception& e)
			{
				ss << "what: "
					<< e.what()
					<< "\n";
			}
			catch (...)
			{
				ss << "Unknown exception type.\n";
			}

			ss << "while checking expectation:\n";
			mimicpp::print(
				std::ostreambuf_iterator{ss},
				expectationReport);
			ss << "\n";

			ss << "For ";
			mimicpp::print(
				std::ostreambuf_iterator{ss},
				call);
			ss << "\n";

			return std::move(ss).str();
		}
	}

	/**
	 * \brief A reporter, which creates text messages and reports them via the provided callbacks.
	 * \tparam successReporter The success reporter callback.
	 * \tparam warningReporter The warning reporter callback.
	 * \tparam failReporter The fail reporter callback. This reporter must never return!
	 */
	template <
		std::invocable<const StringT&> auto successReporter,
		std::invocable<const StringT&> auto warningReporter,
		std::invocable<const StringT&> auto failReporter
	>
	class BasicReporter
		: public IReporter
	{
	public:
		[[noreturn]]
		void report_no_matches(const CallReport call, const std::vector<MatchReport> matchReports) override
		{
			send_fail(
				detail::stringify_no_match_report(call, matchReports));
		}

		[[noreturn]]
		void report_inapplicable_matches(const CallReport call, const std::vector<MatchReport> matchReports) override
		{
			send_fail(
				detail::stringify_inapplicable_match_report(call, matchReports));
		}

		void report_full_match(const CallReport call, const MatchReport matchReport) noexcept override
		{
			send_success(
				detail::stringify_report(call, matchReport));
		}

		void report_unfulfilled_expectation(const ExpectationReport expectationReport) override
		{
			if (0 == std::uncaught_exceptions())
			{
				send_fail(
					detail::stringify_unfulfilled_expectation(expectationReport));
			}
		}

		void report_error(const StringT message) override
		{
			if (0 == std::uncaught_exceptions())
			{
				send_fail(message);
			}
		}

		void report_unhandled_exception(
			const CallReport call,
			const ExpectationReport expectationReport,
			const std::exception_ptr exception
		) override
		{
			send_warning(
				detail::stringify_unhandled_exception(call, expectationReport, exception));
		}

	private:
		void send_success(const StringT& msg)
		{
			std::invoke(successReporter, msg);
		}

		void send_warning(const StringT& msg)
		{
			std::invoke(warningReporter, msg);
		}

		[[noreturn]]
		void send_fail(const StringT& msg)
		{
			// GCOVR_EXCL_START
			std::invoke(failReporter, msg);
			unreachable();
			// GCOVR_EXCL_STOP
		}
	};

	using UnmatchedCallT = Error<std::tuple<CallReport, std::vector<MatchReport>>>;
	using UnfulfilledExpectationT = Error<ExpectationReport>;

	/**
	 * \brief The default reporter.
	 */
	class DefaultReporter final
		: public IReporter
	{
	public:
		[[noreturn]]
		void report_no_matches(
			CallReport call,
			std::vector<MatchReport> matchReports
		) override
		{
			assert(
				std::ranges::all_of(
					matchReports,
					std::bind_front(std::equal_to{}, MatchResult::none),
					&evaluate_match_report));

			const std::source_location loc{call.fromLoc};
			throw UnmatchedCallT{
				"No match found.",
				{std::move(call), std::move(matchReports)},
				loc
			};
		}

		[[noreturn]]
		void report_inapplicable_matches(
			CallReport call,
			std::vector<MatchReport> matchReports
		) override
		{
			assert(
				std::ranges::all_of(
					matchReports,
					std::bind_front(std::equal_to{}, MatchResult::inapplicable),
					&evaluate_match_report));

			const std::source_location loc{call.fromLoc};
			throw UnmatchedCallT{
				"No applicable match found.",
				{std::move(call), std::move(matchReports)},
				loc
			};
		}

		void report_full_match(
			[[maybe_unused]] CallReport call,
			[[maybe_unused]] MatchReport matchReport
		) noexcept override
		{
			assert(MatchResult::full == evaluate_match_report(matchReport));
		}

		void report_unfulfilled_expectation(
			ExpectationReport expectationReport
		) override
		{
			if (0 == std::uncaught_exceptions())
			{
				throw UnfulfilledExpectationT{
					"Expectation is unfulfilled.",
					std::move(expectationReport)
				};
			}
		}

		void report_error(StringT message) override
		{
			if (0 == std::uncaught_exceptions())
			{
				throw Error{message};
			}
		}

		void report_unhandled_exception(
			[[maybe_unused]] CallReport call,
			[[maybe_unused]] ExpectationReport expectationReport,
			[[maybe_unused]] std::exception_ptr exception
		) override
		{
		}
	};

	/**
	 * \}
	 */
}

namespace mimicpp::detail
{
	[[nodiscard]]
	inline std::unique_ptr<IReporter>& get_reporter() noexcept
	{
		static std::unique_ptr<IReporter> reporter{
			std::make_unique<DefaultReporter>()
		};
		return reporter;
	}

	[[noreturn]]
	inline void report_no_matches(
		CallReport callReport,
		std::vector<MatchReport> matchReports
	)
	{
		get_reporter()
			// GCOVR_EXCL_START
			->report_no_matches(
				// GCOVR_EXCL_STOP
				std::move(callReport),
				std::move(matchReports));

		// GCOVR_EXCL_START
		// ReSharper disable once CppUnreachableCode
		unreachable();
		// GCOVR_EXCL_STOP
	}

	[[noreturn]]
	inline void report_inapplicable_matches(
		CallReport callReport,
		std::vector<MatchReport> matchReports
	)
	{
		get_reporter()
			// GCOVR_EXCL_START
			->report_inapplicable_matches(
				// GCOVR_EXCL_STOP
				std::move(callReport),
				std::move(matchReports));

		// GCOVR_EXCL_START
		// ReSharper disable once CppUnreachableCode
		unreachable();
		// GCOVR_EXCL_STOP
	}

	inline void report_full_match(
		CallReport callReport,
		MatchReport matchReport
	) noexcept
	{
		get_reporter()
			->report_full_match(
				std::move(callReport),
				std::move(matchReport));
	}

	inline void report_unfulfilled_expectation(
		ExpectationReport expectationReport
	)
	{
		get_reporter()
			->report_unfulfilled_expectation(std::move(expectationReport));
	}

	inline void report_error(StringT message)
	{
		get_reporter()
			->report_error(std::move(message));
	}

	inline void report_unhandled_exception(
		CallReport callReport,
		ExpectationReport expectationReport,
		const std::exception_ptr& exception
	)
	{
		get_reporter()
			->report_unhandled_exception(
				std::move(callReport),
				std::move(expectationReport),
				exception);
	}
}

namespace mimicpp
{
	/**
	 * \brief Replaces the previous reporter with a newly constructed one.
	 * \tparam T The desired reporter type.
	 * \tparam Args The constructor argument types for ``T``.
	 * \param args The constructor arguments.
	 * \ingroup REPORTING
	 * \details This function accesses the globally available reporter and replaces it with a new instance.
	 */
	template <std::derived_from<IReporter> T, typename... Args>
		requires std::constructible_from<T, Args...>
	void install_reporter(Args&&... args)  // NOLINT(cppcoreguidelines-missing-std-forward)
	{
		detail::get_reporter() = std::make_unique<T>(
			std::forward<Args>(args)...);
	}

	namespace detail
	{
		template <typename T>
		class ReporterInstaller
		{
		public:
			template <typename... Args>
			explicit ReporterInstaller(Args&&... args)
			{
				install_reporter<T>(
					std::forward<Args>(args)...);
			}
		};
	}

	/**
	 * \defgroup REPORTING_ADAPTERS test framework adapters
	 * \ingroup REPORTING
	 * \brief Reporter integrations for various third-party frameworks.
	 * \details These reporters are specialized implementations, which provide seamless integrations of ``mimic++`` into the desired
	 * unit-test framework. Integrations are enabled by simply including the specific header into any source file. The include order
	 * doesn't matter.
	 *
	 * \note Including multiple headers of the ``adapters`` subdirectory into one executable is possible, but with caveats. It's unspecified
	 * which reporter will be active at the program start. So, if you need multiple reporters in one executable, you should explicitly
	 * install the desired reporter on a per test case basis. 
	 *
	 *\{
	 */
}

#endif
