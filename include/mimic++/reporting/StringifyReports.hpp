//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_STRINGIFY_REPORTS_HPP
#define MIMICPP_REPORTING_STRINGIFY_REPORTS_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Stacktrace.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/reporting/CallReport.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/MatchReport.hpp"
#include "mimic++/reporting/TypeReport.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace mimicpp::reporting::detail
{
    template <print_iterator OutIter>
    OutIter stringify_stacktrace(OutIter out, Stacktrace const& stacktrace)
    {
        if (!stacktrace.empty())
        {
            out = format::format_to(std::move(out), "Stacktrace:\n");
            out = mimicpp::print(std::move(out), stacktrace);
        }

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_call_report_arguments(OutIter out, CallReport const& call, StringViewT const prefix)
    {
        for (int i{};
             auto const& [type, state] : call.argDetails)
        {
            out = std::ranges::copy(prefix, std::move(out)).out;
            out = format::format_to(std::move(out), "arg[{}] => ", i++);
            out = std::ranges::copy(type.name(), std::move(out)).out;
            out = format::format_to(std::move(out), ": ");
            out = std::ranges::copy(state, std::move(out)).out;
            out = format::format_to(std::move(out), "\n");
        }

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_call_report_from(OutIter out, CallReport const& call)
    {
        out = format::format_to(std::move(out), "Call from ");

        if (!call.stacktrace.empty())
        {
            out = stacktrace::detail::print_entry(std::move(out), call.stacktrace, 0u);
        }
        else
        {
            out = mimicpp::print(std::move(out), call.fromLoc);
        }
        out = format::format_to(std::move(out), "\n");

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_expectation_report_from(OutIter out, ExpectationReport const& expectation)
    {
        out = format::format_to(std::move(out), "Expectation from ");
        out = mimicpp::print(std::move(out), expectation.info.sourceLocation);
        out = format::format_to(std::move(out), "\n");

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_expectation_report_requirement_descriptions(
        OutIter out,
        std::span<std::optional<StringT> const> const descriptions,
        StringViewT const linePrefix)
    {
        for (auto const& description : descriptions)
        {
            if (description)
            {
                out = std::ranges::copy(linePrefix, std::move(out)).out;
                out = std::ranges::copy(*description, std::move(out)).out;
                out = format::format_to(std::move(out), "\n");
            }
        }

        return out;
    }

    [[nodiscard]]
    inline StringT stringify_full_match(CallReport const& call, ExpectationReport expectation)
    {
        assert(std::holds_alternative<state_applicable>(expectation.controlReport) && "Report denotes inapplicable expectation.");

        StringStreamT ss{};
        ss << "Matched ";
        stringify_call_report_from(std::ostreambuf_iterator{ss}, call);

        // Todo: target
        ss << "\t" << "Where\n";
        stringify_call_report_arguments(std::ostreambuf_iterator{ss}, call, "\t\t");

        ss << "\t" << "Chose ";
        stringify_expectation_report_from(std::ostreambuf_iterator{ss}, expectation);

        if (!expectation.requirementDescriptions.empty())
        {
            ss << "\t" << "With Adherence(s):\n";
            std::ranges::sort(expectation.requirementDescriptions);
            stringify_expectation_report_requirement_descriptions(
                std::ostreambuf_iterator{ss},
                expectation.requirementDescriptions,
                "\t  + ");
        }
        else
        {
            ss << "\t" << "With any Requirements.\n";
        }

        ss << "\n";
        stringify_stacktrace(
            std::ostreambuf_iterator{ss},
            call.stacktrace);

        return std::move(ss).str();
    }

    [[nodiscard]]
    inline StringT stringify_inapplicable_matches(CallReport const& call, std::vector<ExpectationReport> expectations)
    {
        assert(!expectations.empty() && "No expectations given.");

        StringStreamT ss{};

        ss << "Unmatched ";
        stringify_call_report_from(std::ostreambuf_iterator{ss}, call);

        // Todo: target
        ss << "\t" << "Where\n";
        stringify_call_report_arguments(std::ostreambuf_iterator{ss}, call, "\t\t");

        ss << expectations.size() << " inapplicable but otherwise matching Expectation(s):\n";

        for (int i{};
             auto& expReport : expectations)
        {
            ss << "\t#" << ++i << " ";
            stringify_expectation_report_from(std::ostreambuf_iterator{ss}, expReport);

            if (!expReport.requirementDescriptions.empty())
            {
                ss << "\t" << "With Adherence(s):\n";
                std::ranges::sort(expReport.requirementDescriptions);
                stringify_expectation_report_requirement_descriptions(
                    std::ostreambuf_iterator{ss},
                    expReport.requirementDescriptions,
                    "\t  + ");
            }
            else
            {
                ss << "\t" << "With any Requirements.\n";
            }
        }

        ss << "\n";
        stringify_stacktrace(
            std::ostreambuf_iterator{ss},
            call.stacktrace);

        return std::move(ss).str();
    }

    [[nodiscard]]
    inline std::span<std::optional<StringT>> partition_requirement_descriptions(
        std::span<std::optional<StringT>> descriptions,
        std::vector<bool>& outcomes) noexcept
    {
        assert(descriptions.size() == outcomes.size() && "Size mismatch.");
        assert(!descriptions.empty() && "No requirements can not be a no-match.");

        auto midpoint = descriptions.end();
        auto end = outcomes.end();
        for (auto iter = std::ranges::find(outcomes, false);
             iter != end;
             iter = std::ranges::find(iter, end, false))
        {
            auto const index = std::ranges::distance(outcomes.cbegin(), iter);
            std::ranges::iter_swap(iter, --end);
            std::ranges::iter_swap(descriptions.begin() + index, --midpoint);
        }

        assert(midpoint != descriptions.end() && "No violated descriptions can not be a no-match.");
        return {midpoint, descriptions.end()};
    }

    [[nodiscard]]
    inline StringT stringify_no_matches(CallReport const& call, std::vector<NoMatchReport> noMatchReports)
    {
        StringStreamT ss{};

        ss << "Unmatched ";
        stringify_call_report_from(std::ostreambuf_iterator{ss}, call);

        // Todo: target
        ss << "\t" << "Where\n";
        stringify_call_report_arguments(std::ostreambuf_iterator{ss}, call, "\t\t");

        if (noMatchReports.empty())
        {
            ss << "No Expectations available!\n";
        }
        else
        {
            ss << noMatchReports.size() << " non-matching Expectation(s):\n";

            for (int i{};
                 auto& [expReport, outcomes] : noMatchReports)
            {
                ss << "\t#" << ++i << " ";
                stringify_expectation_report_from(std::ostreambuf_iterator{ss}, expReport);

                ss << "\t" << "Due to Violation(s):\n";
                std::span const violations = partition_requirement_descriptions(expReport.requirementDescriptions, outcomes.outcomes);
                std::ranges::sort(violations);
                stringify_expectation_report_requirement_descriptions(
                    std::ostreambuf_iterator{ss},
                    violations,
                    "\t  - ");

                if (std::span const adherences{expReport.requirementDescriptions.data(), violations.data()};
                    !adherences.empty())
                {
                    ss << "\t" << "With Adherence(s):\n";
                    std::ranges::sort(adherences);
                    stringify_expectation_report_requirement_descriptions(
                        std::ostreambuf_iterator{ss},
                        adherences,
                        "\t  + ");
                }
            }
        }

        ss << "\n";
        stringify_stacktrace(
            std::ostreambuf_iterator{ss},
            call.stacktrace);

        return std::move(ss).str();
    }
}

#endif
