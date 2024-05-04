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
	struct call_report
	{
		struct arg
		{
			std::type_index typeIndex;
			StringT stateString;

			[[nodiscard]]
			friend bool operator ==(const arg&, const arg&) = default;
		};

		std::type_index returnTypeIndex;
		std::vector<arg> argDetails{};
		std::source_location fromLoc{};
		ValueCategory fromCategory{};
		Constness fromConstness{};

		[[nodiscard]]
		friend bool operator ==(const call_report& lhs, const call_report& rhs)
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
	call_report make_call_report(const call::Info<Return, Params...>& callInfo)
	{
		return call_report{
			.returnTypeIndex = typeid(Return),
			.argDetails = std::apply(
				[](auto&... args)
				{
					return std::vector<call_report::arg>{
						call_report::arg{
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

	struct expectation_report
	{
		StringT description{};

		[[nodiscard]]
		friend bool operator==(const expectation_report&, const expectation_report&) = default;
	};

	template <typename Signature>
	[[nodiscard]]
	expectation_report make_expectation_report(const Expectation<Signature>& expectation)
	{
		return expectation_report{};
	}

	struct match_report
	{
		struct finalize
		{
			std::optional<StringT> description{};

			[[nodiscard]]
			friend bool operator ==(const finalize&, const finalize&) = default;
		};

		struct times
		{
			bool isApplicable{};
			std::optional<StringT> description{};

			[[nodiscard]]
			friend bool operator ==(const times&, const times&) = default;
		};

		struct expectation
		{
			bool matched{};
			std::optional<StringT> description{};

			[[nodiscard]]
			friend bool operator ==(const expectation&, const expectation&) = default;
		};

		finalize finalizeReport{};
		times timesReport{};
		std::vector<expectation> expectationReports{};

		[[nodiscard]]
		friend bool operator ==(const match_report&, const match_report&) = default;
	};

	[[nodiscard]]
	inline MatchResult evaluate_match_report(const match_report& report)
	{
		if (!std::ranges::all_of(report.expectationReports, &match_report::expectation::matched))
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
			call_report call,
			std::vector<match_report> matchReports
		) = 0;

		[[noreturn]]
		virtual void report_inapplicable_matches(
			call_report call,
			std::vector<match_report> matchReports
		) = 0;

		virtual void report_full_match(
			call_report call,
			match_report matchReport
		) noexcept = 0;

		virtual void report_unfulfilled_expectation(
			expectation_report expectationReport
		) = 0;

		virtual void report_error(StringT message) = 0;
		virtual void report_unhandled_exception(
			call_report call,
			expectation_report expectationReport,
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
			call_report call,
			std::vector<match_report> matchReports
		) override
		{
			std::terminate();
		}

		[[noreturn]]
		void report_inapplicable_matches(
			call_report call,
			std::vector<match_report> matchReports
		) override
		{
			std::terminate();
		}

		void report_full_match(
			call_report call,
			match_report matchReport
		) noexcept override
		{
		}

		void report_unfulfilled_expectation(
			expectation_report expectationReport
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
			call_report call,
			expectation_report expectationReport,
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
		std::vector<match_report> matchReports
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
		std::vector<match_report> matchReports
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
		match_report matchReport
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
