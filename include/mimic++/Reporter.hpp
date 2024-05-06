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

#include <algorithm>
#include <cassert>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <typeindex>
#include <vector>

namespace mimicpp
{
	/**
	 * \defgroup REPORTING reporting
	 * \brief Contains reporting related symbols
	 * \details Reports are simplified object representations of ``mimicpp`` types. In fact, reports are used to communicate with
	 * independent domains (e.g. unit-test frameworks) over the ``IReporter`` interface and are thus designed to provide as much
	 * transparent information as possible, without requiring them to be a generic type.
	 *
	 * At any time there exists exactly one global reporter, which may be directly or indirectly exchanged by users.
	 * Reports are sent to the currently installed reporter via the ``report_xyz`` free-functions. Most of those functions require, that
	 * reports are handled in a specific manner (e.g. ``report_no_matches`` is expected to never return) and custom reporters **must**
	 * follow that specification, otherwise this will lead to undefined behavior. For further details, have a look at the specific
	 * function documentation.
	 *
	 * \note In general users shall not directly interact with the installed reporter, except when they want to replace it. 
	 *
	 *\{
	 */

	/**
	 * \brief Contains the extracted info from a typed ``call::Info``.
	 * \details This type is meant to be used to communicate with independent domains via the reporter interface and thus contains
	 * the generic information as plain ``std`` types (e.g. the return type is provided as ``std::type_index`` instead of an actual
	 * type).
	 */
	class CallReport
	{
	public:
		class Arg
		{
		public:
			std::type_index typeIndex;
			StringT stateString;

			[[nodiscard]]
			friend bool operator ==(const Arg&, const Arg&) = default;
		};

		std::type_index returnTypeIndex;
		std::vector<Arg> argDetails{};
		std::source_location fromLoc{};
		ValueCategory fromCategory{};
		Constness fromConstness{};

		[[nodiscard]]
		friend bool operator ==(const CallReport& lhs, const CallReport& rhs)
		{
			return lhs.returnTypeIndex == rhs.returnTypeIndex
					&& lhs.argDetails == rhs.argDetails
					&& is_same_source_location(lhs.fromLoc, rhs.fromLoc)
					&& lhs.fromCategory == rhs.fromCategory
					&& lhs.fromConstness == rhs.fromConstness;
		}
	};

	/**
	 * \brief Generates the call report for a given call info.
	 * \tparam Return The function return type.
	 * \tparam Params The function parameter types.
	 * \param callInfo The call info.
	 * \return The call report.
	 * \relatesalso CallReport
	 * \relatesalso call::Info
	 */
	template <typename Return, typename... Params>
	[[nodiscard]]
	CallReport make_call_report(const call::Info<Return, Params...>& callInfo)
	{
		return CallReport{
			.returnTypeIndex = typeid(Return),
			.argDetails = std::apply(
				[](auto&... args)
				{
					return std::vector<CallReport::Arg>{
						CallReport::Arg{
							.typeIndex = typeid(Params),
							.stateString = mimicpp::print(args.get())
						}...
					};
				},
				callInfo.args),
			.fromLoc = callInfo.fromSourceLocation,
			.fromCategory = callInfo.fromCategory,
			.fromConstness = callInfo.fromConstness
		};
	}

	/**
	 * \brief Contains the extracted info from a typed expectation.
	 * \details This type is meant to be used to communicate with independent domains via the reporter interface and thus contains
	 * the generic information as plain ``std`` types.
	 */
	class ExpectationReport
	{
	public:
		StringT description{};

		[[nodiscard]]
		friend bool operator==(const ExpectationReport&, const ExpectationReport&) = default;
	};

	template <typename Signature>
	[[nodiscard]]
	ExpectationReport make_expectation_report(const Expectation<Signature>& expectation)
	{
		return ExpectationReport{};
	}

	/**
	 * \brief Contains the detailed information for match outcomes.
	 * \details This type is meant to be used to communicate with independent domains via the reporter interface and thus contains
	 * the generic information as plain ``std`` types.
	 */
	class MatchReport
	{
	public:
		/**
		 * \brief Information about the used finalizer.
		 */
		class Finalize
		{
		public:
			std::optional<StringT> description{};

			[[nodiscard]]
			friend bool operator ==(const Finalize&, const Finalize&) = default;
		};

		/**
		 * \brief Information about the current times state.
		 * \details This type contains a description about the current state of the ``times`` policy. This description is gather
		 * in parallel to the ``matches`` (before the ``consume`` step) and thus contains more detailed information about the
		 * outcome.
		 */
		class Times
		{
		public:
			bool isApplicable{};
			std::optional<StringT> description{};

			[[nodiscard]]
			friend bool operator ==(const Times&, const Times&) = default;
		};

		/**
		 * \brief Information a used expectation policy.
		 * \details This type contains a description about a given expectation policy.
		 */
		class Expectation
		{
		public:
			bool isMatching{};
			std::optional<StringT> description{};

			[[nodiscard]]
			friend bool operator ==(const Expectation&, const Expectation&) = default;
		};

		Finalize finalizeReport{};
		Times timesReport{};
		std::vector<Expectation> expectationReports{};

		[[nodiscard]]
		friend bool operator ==(const MatchReport&, const MatchReport&) = default;
	};

	/**
	 * \brief Determines, whether a match report actually denotes a ``full``, ``inapplicable`` or ``no`` match.
	 * \param report The report to evaluate.
	 * \return The actual result.
	 */
	[[nodiscard]]
	inline MatchResult evaluate_match_report(const MatchReport& report)
	{
		if (!std::ranges::all_of(report.expectationReports, &MatchReport::Expectation::isMatching))
		{
			return MatchResult::none;
		}

		if (!report.timesReport.isApplicable)
		{
			return MatchResult::inapplicable;
		}

		return MatchResult::full;
	}

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
			CallReport call,
			MatchReport matchReport
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
			CallReport call,
			ExpectationReport expectationReport,
			std::exception_ptr exception
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
			->report_no_matches(
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
			->report_inapplicable_matches(
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
}

#endif
