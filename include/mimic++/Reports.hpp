// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTS_HPP
#define MIMICPP_REPORTS_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/Printer.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <typeindex>
#include <variant>
#include <vector>

namespace mimicpp
{
	/**
	 * \defgroup REPORTING_REPORTS reports
	 * \ingroup REPORTING
	 * \brief Contains reports of ``mimic++`` types.
	 * \details Reports are simplified object representations of ``mimic++`` types. In fact, reports are used to communicate with
	 * independent domains (e.g. unit-test frameworks) over the ``IReporter`` interface and are thus designed to provide as much
	 * transparent information as possible, without requiring them to be a generic type.
	 *
	 * \{
	 */

	struct state_inapplicable
	{
		int min{};
		int max{};
		int count{};
		std::vector<sequence::rating> sequenceRatings{};
		std::vector<sequence::Tag> inapplicableSequences{};

		[[nodiscard]]
		friend bool operator ==(const state_inapplicable&, const state_inapplicable&) = default;
	};

	struct state_applicable
	{
		int min{};
		int max{};
		int count{};
		std::vector<sequence::rating> sequenceRatings{};

		[[nodiscard]]
		friend bool operator ==(const state_applicable&, const state_applicable&) = default;
	};

	struct state_saturated
	{
		int min{};
		int max{};
		int count{};
		std::vector<sequence::Tag> sequences{};

		[[nodiscard]]
		friend bool operator ==(const state_saturated&, const state_saturated&) = default;
	};

	using control_state_t = std::variant<
		state_inapplicable,
		state_applicable,
		state_saturated>;

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
	 * \brief Converts the given report to text.
	 * \param report The report.
	 * \return The report text.
	 * \relatesalso CallReport
	 */
	[[nodiscard]]
	inline StringT stringify_call_report(const CallReport& report)
	{
		StringStreamT out{};
		format_to(
			std::ostreambuf_iterator{out},
			"call from {}\n",
			mimicpp::print(report.fromLoc));

		format_to(
			std::ostreambuf_iterator{out},
			"constness: {}\n"
			"value category: {}\n"
			"return type: {}\n",
			report.fromConstness,
			report.fromCategory,
			report.returnTypeIndex.name());

		if (!std::ranges::empty(report.argDetails))
		{
			out << "args:\n";
			for (const std::size_t i : std::views::iota(0u, std::ranges::size(report.argDetails)))
			{
				format_to(
					std::ostreambuf_iterator{out},
					"\targ[{}]: {{\n"
					"\t\ttype: {},\n"
					"\t\tvalue: {}\n"
					"\t}},\n",
					i,
					report.argDetails[i].typeIndex.name(),
					report.argDetails[i].stateString);
			}
		}

		return std::move(out).str();
	}

	/**
	 * \brief Contains the extracted info from a typed expectation.
	 * \details This type is meant to be used to communicate with independent domains via the reporter interface and thus contains
	 * the generic information as plain ``std`` types.
	 */
	class ExpectationReport
	{
	public:
		std::optional<std::source_location> sourceLocation{};
		std::optional<StringT> finalizerDescription{};
		std::optional<StringT> timesDescription{};
		std::vector<std::optional<StringT>> expectationDescriptions{};

		[[nodiscard]]
		friend bool operator ==(const ExpectationReport& lhs, const ExpectationReport& rhs)
		{
			return lhs.finalizerDescription == rhs.finalizerDescription
					&& lhs.timesDescription == rhs.timesDescription
					&& lhs.expectationDescriptions == rhs.expectationDescriptions
					&& lhs.sourceLocation.has_value() == rhs.sourceLocation.has_value()
					&& (!lhs.sourceLocation.has_value()
						|| is_same_source_location(*lhs.sourceLocation, *rhs.sourceLocation));
		}
	};

	/**
	 * \brief Converts the given report to text.
	 * \param report The report.
	 * \return The report text.
	 * \relatesalso ExpectationReport
	 */
	[[nodiscard]]
	inline StringT stringify_expectation_report(const ExpectationReport& report)
	{
		StringStreamT out{};

		out << "Expectation report:\n";

		if (report.sourceLocation)
		{
			out << "from: ";
			mimicpp::print(
				std::ostreambuf_iterator{out},
				*report.sourceLocation);
			out << "\n";
		}

		if (report.timesDescription)
		{
			format_to(
				std::ostreambuf_iterator{out},
				"times: {}\n",
				*report.timesDescription);
		}

		if (std::ranges::any_of(
			report.expectationDescriptions,
			[](const auto& desc) { return desc.has_value(); }))
		{
			out << "expects:\n";
			for (const auto& desc
				: report.expectationDescriptions
				| std::views::filter([](const auto& desc) { return desc.has_value(); }))
			{
				format_to(
					std::ostreambuf_iterator{out},
					"\t{},\n",
					*desc);
			}
		}

		if (report.finalizerDescription)
		{
			format_to(
				std::ostreambuf_iterator{out},
				"finally: {}\n",
				*report.finalizerDescription);
		}

		return std::move(out).str();
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
			std::optional<std::vector<sequence::rating>> ratings{};
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

		std::optional<std::source_location> sourceLocation{};
		Finalize finalizeReport{};
		Times timesReport{};
		control_state_t controlReport{};
		std::vector<Expectation> expectationReports{};

		[[nodiscard]]
		friend bool operator ==(const MatchReport& lhs, const MatchReport& rhs)
		{
			return lhs.finalizeReport == rhs.finalizeReport
					&& lhs.timesReport == rhs.timesReport
					&& lhs.controlReport == rhs.controlReport
					&& lhs.expectationReports == rhs.expectationReports
					&& lhs.sourceLocation.has_value() == rhs.sourceLocation.has_value()
					&& (!lhs.sourceLocation.has_value()
						|| is_same_source_location(*lhs.sourceLocation, *rhs.sourceLocation));
		}
	};

	/**
	 * \brief Determines, whether a match report actually denotes a ``full``, ``inapplicable`` or ``no`` match.
	 * \param report The report to evaluate.
	 * \return The actual result.
	 */
	[[nodiscard]]
	inline MatchResult evaluate_match_report(const MatchReport& report) noexcept
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
	 * \brief Converts the given report to text.
	 * \param report The report.
	 * \return The report text.
	 * \relatesalso MatchReport
	 */
	[[nodiscard]]
	inline StringT stringify_match_report(const MatchReport& report)
	{
		std::vector<StringT> matchedExpectationDescriptions{};
		std::vector<StringT> unmatchedExpectationDescriptions{};

		for (const auto& [isMatching, description] : report.expectationReports)
		{
			if (description)
			{
				if (isMatching)
				{
					matchedExpectationDescriptions.emplace_back(*description);
				}
				else
				{
					unmatchedExpectationDescriptions.emplace_back(*description);
				}
			}
		}

		StringStreamT out{};

		switch (evaluate_match_report(report))
		{
		case MatchResult::full:
			out << "Matched expectation: {\n";
			break;

		case MatchResult::inapplicable:
			format_to(
				std::ostreambuf_iterator{out},
				"Inapplicable, but otherwise matched expectation: {{\n"
				"reason: {}\n",
				report.timesReport.description.value_or("No reason provided."));
			break;

		case MatchResult::none:
			out << "Unmatched expectation: {\n";
			break;

		// GCOVR_EXCL_START
		default:  // NOLINT(clang-diagnostic-covered-switch-default)
			unreachable();
		// GCOVR_EXCL_STOP
		}

		if (report.sourceLocation)
		{
			out << "from: ";
			mimicpp::print(
				std::ostreambuf_iterator{out},
				*report.sourceLocation);
			out << "\n";
		}

		if (!std::ranges::empty(unmatchedExpectationDescriptions))
		{
			out << "failed:\n";
			for (const auto& desc : unmatchedExpectationDescriptions)
			{
				format_to(
					std::ostreambuf_iterator{out},
					"\t{},\n",
					desc);
			}
		}

		if (!std::ranges::empty(matchedExpectationDescriptions))
		{
			out << "passed:\n";
			for (const auto& desc : matchedExpectationDescriptions)
			{
				format_to(
					std::ostreambuf_iterator{out},
					"\t{},\n",
					desc);
			}
		}

		out << "}\n";

		return std::move(out).str();
	}

	/**
	 * \}
	 */
}

#endif
