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
			bool matched{};
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
		if (!std::ranges::all_of(report.expectationReports, &MatchReport::Expectation::matched))
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
			std::terminate();
		}

		[[noreturn]]
		void report_inapplicable_matches(
			CallReport call,
			std::vector<MatchReport> matchReports
		) override
		{
			std::terminate();
		}

		void report_full_match(
			CallReport call,
			MatchReport matchReport
		) noexcept override
		{
		}

		void report_unfulfilled_expectation(
			ExpectationReport expectationReport
		) override
		{
			if (0 != std::uncaught_exceptions())
			{
				std::terminate();
			}
		}

		void report_error(StringT message) override
		{
			if (0 != std::uncaught_exceptions())
			{
				std::terminate();
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

	template <typename Return, typename... Params>
	[[noreturn]]
	void report_no_matches(
		const call::Info<Return, Params...>& callInfo,
		std::vector<MatchReport> matchReports
	)
	{
		get_reporter()
			->report_no_matches(
				make_call_report(callInfo),
				std::move(matchReports));
	}

	template <typename Return, typename... Params>
	[[noreturn]]
	void report_inapplicable_matches(
		const call::Info<Return, Params...>& callInfo,
		std::vector<MatchReport> matchReports
	)
	{
		get_reporter()
			->report_inapplicable_matches(
				make_call_report(callInfo),
				std::move(matchReports));
	}

	template <typename Return, typename... Params>
	void report_full_match(
		const call::Info<Return, Params...>& callInfo,
		MatchReport matchReport
	) noexcept
	{
		get_reporter()
			->report_full_match(
				make_call_report(callInfo),
				std::move(matchReport));
	}

	template <typename Signature>
	void report_unfulfilled_expectation(
		std::shared_ptr<Expectation<Signature>> expectation
	)
	{
		get_reporter()
			->report_unfulfilled_expectation(
				make_expectation_report(*expectation));
	}

	inline void report_error(StringT message)
	{
		get_reporter()
			->report_error(std::move(message));
	}

	template <typename Return, typename... Params, typename Signature>
	void report_unhandled_exception(
		const call::Info<Return, Params...>& callInfo,
		std::shared_ptr<Expectation<Signature>> expectation,
		std::exception_ptr exception
	)
	{
		get_reporter()
			->report_unhandled_exception(
				make_call_report(callInfo),
				make_expectation_report(*expectation),
				std::move(exception));
	}
}

namespace mimicpp
{
	template <std::derived_from<IReporter> T, typename... Args>
		requires std::constructible_from<T, Args...>
	void install_reporter(Args&&... args)
	{
		detail::get_reporter() = std::make_unique<T>(
			std::forward<Args>(args)...);
	}
}

#endif
