//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_MATCH_REPORT_HPP
#define MIMICPP_REPORTING_MATCH_REPORT_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Utility.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/StatePrinter.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"

#include <algorithm>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace mimicpp::reporting
{
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
            friend bool operator==(const Finalize&, const Finalize&) = default;
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
            friend bool operator==(const Expectation&, const Expectation&) = default;
        };

        detail::expectation_info expectationInfo{};
        Finalize finalizeReport{};
        control_state_t controlReport{};
        std::vector<Expectation> expectationReports{};

        [[nodiscard]]
        friend bool operator==(const MatchReport& lhs, const MatchReport& rhs) = default;
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

        if (!std::holds_alternative<state_applicable>(report.controlReport))
        {
            return MatchResult::inapplicable;
        }

        return MatchResult::full;
    }
}

template <>
struct mimicpp::printing::detail::state::common_type_printer<mimicpp::reporting::MatchReport>
{
    using MatchReport = reporting::MatchReport;

    template <print_iterator OutIter>
    static OutIter print(OutIter out, const MatchReport& report)
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

        bool printReason = false;
        switch (evaluate_match_report(report))
        {
        case MatchResult::full:
            out = format::format_to(
                std::move(out),
                "Matched expectation: {{\n");
            break;

        case MatchResult::inapplicable:
            out = format::format_to(
                std::move(out),
                "Inapplicable, but otherwise matched expectation: {{\n");
            printReason = true;
            break;

        case MatchResult::none:
            out = format::format_to(
                std::move(out),
                "Unmatched expectation: {{\n");
            break;

        // GCOVR_EXCL_START
        default: // NOLINT(clang-diagnostic-covered-switch-default)
            unreachable();
            // GCOVR_EXCL_STOP
        }

        out = format::format_to(
            std::move(out),
            "mock: {}\n",
            report.expectationInfo.mockName);

        if (printReason)
        {
            out = format::format_to(std::move(out), "reason: ");
            out = std::visit(
                std::bind_front(reporting::detail::control_state_printer{}, std::move(out)),
                report.controlReport);
            out = format::format_to(std::move(out), "\n");
        }

        out = format::format_to(
            std::move(out),
            "from: ");
        out = mimicpp::print(
            std::move(out),
            report.expectationInfo.sourceLocation);
        out = format::format_to(
            std::move(out),
            "\n");

        if (!std::ranges::empty(unmatchedExpectationDescriptions))
        {
            out = format::format_to(
                std::move(out),
                "failed:\n");
            for (const auto& desc : unmatchedExpectationDescriptions)
            {
                out = format::format_to(
                    std::move(out),
                    "\t{},\n",
                    desc);
            }
        }

        if (!std::ranges::empty(matchedExpectationDescriptions))
        {
            out = format::format_to(
                std::move(out),
                "passed:\n");
            for (const auto& desc : matchedExpectationDescriptions)
            {
                out = format::format_to(
                    std::move(out),
                    "\t{},\n",
                    desc);
            }
        }

        return format::format_to(
            std::move(out),
            "}}\n");
    }
};

#endif
