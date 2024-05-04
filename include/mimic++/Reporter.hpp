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

	class MatchReport
	{
	public:
		class Finalize
		{
		public:
			std::optional<StringT> description{};

			[[nodiscard]]
			friend bool operator ==(const Finalize&, const Finalize&) = default;
		};

		class Times
		{
		public:
			bool isApplicable{};
			std::optional<StringT> description{};

			[[nodiscard]]
			friend bool operator ==(const Times&, const Times&) = default;
		};

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

	class IReporter
	{
	public:
		virtual ~IReporter() = default;

		[[noreturn]]
		virtual void report_no_matches(
			CallReport call,
			std::vector<MatchReport> matchReports
		) = 0;

		[[noreturn]]
		virtual void report_inapplicable_matches(
			CallReport call,
			std::vector<MatchReport> matchReports
		) = 0;

		virtual void report_full_match(
			CallReport call,
			MatchReport matchReport
		) noexcept = 0;

		virtual void report_unfulfilled_expectation(
			ExpectationReport expectationReport
		) = 0;

		virtual void report_error(StringT message) = 0;
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
			assert(std::ranges::all_of(
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
			assert(std::ranges::all_of(
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
	template <std::derived_from<IReporter> T, typename... Args>
		requires std::constructible_from<T, Args...>
	void install_reporter(Args&&... args)  // NOLINT(cppcoreguidelines-missing-std-forward)
	{
		detail::get_reporter() = std::make_unique<T>(
			std::forward<Args>(args)...);
	}
}

#endif
