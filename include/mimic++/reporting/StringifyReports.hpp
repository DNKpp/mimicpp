//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_STRINGIFY_REPORTS_HPP
#define MIMICPP_REPORTING_STRINGIFY_REPORTS_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/reporting/CallReport.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/NoMatchReport.hpp"
#include "mimic++/reporting/TypeReport.hpp"
#include "mimic++/utilities/Algorithm.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/Stacktrace.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <exception>
    #include <iterator>
    #include <optional>
    #include <span>
    #include <utility>
#endif

namespace mimicpp::reporting::detail
{
    template <print_iterator OutIter>
    OutIter stringify_stacktrace(OutIter out, util::Stacktrace const& stacktrace)
    {
        if (!stacktrace.empty())
        {
            out = format::format_to(std::move(out), "\nStacktrace:\n");
            out = mimicpp::print(std::move(out), stacktrace);
        }

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_call_report_arguments(OutIter out, CallReport const& call, StringViewT const linePrefix)
    {
        if (call.argDetails.empty())
        {
            return out;
        }

        out = std::ranges::copy(linePrefix, std::move(out)).out;
        out = format::format_to(std::move(out), "Where:\n");

        for (int i{};
             auto const& [type, state] : call.argDetails)
        {
            out = std::ranges::copy(linePrefix, std::move(out)).out;
            out = format::format_to(std::move(out), "\targ[{}] => ", i++);
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
        out = format::format_to(std::move(out), "Call originated from ");

        if (!call.stacktrace.empty())
        {
            out = util::stacktrace::detail::print_entry(std::move(out), call.stacktrace, 0u);
        }
        else
        {
            out = mimicpp::print(std::move(out), call.fromLoc);
        }
        out = format::format_to(std::move(out), "\n");

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_call_report_target(OutIter out, CallReport const& call)
    {
        out = format::format_to(
            std::move(out),
            "\tOn Target `{}` used Overload `{}`\n",
            call.target.name,
            call.target.overloadReport.name());

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_expectation_report_from(OutIter out, ExpectationReport const& expectation)
    {
        out = format::format_to(std::move(out), "Expectation defined at ");
        out = mimicpp::print(std::move(out), expectation.from);
        out = format::format_to(std::move(out), "\n");

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_expectation_report_target(OutIter out, ExpectationReport const& expectation)
    {
        out = format::format_to(
            std::move(out),
            "\tOf Target `{}` related to Overload `{}`\n",
            expectation.target.name,
            expectation.target.overloadReport.name());

        return out;
    }

    template <print_iterator OutIter>
    OutIter stringify_expectation_report_requirement_adherences(
        OutIter out,
        std::span<std::optional<StringT> const> const descriptions,
        StringViewT const linePrefix)
    {
        if (descriptions.empty())
        {
            return out;
        }

        out = std::ranges::copy(linePrefix, std::move(out)).out;
        out = format::format_to(std::move(out), "With Adherence(s):\n");

        for (auto const& description : descriptions)
        {
            if (description)
            {
                out = std::ranges::copy(linePrefix, std::move(out)).out;
                out = format::format_to(std::move(out), "  + ");
                out = std::ranges::copy(*description, std::move(out)).out;
                out = format::format_to(std::move(out), "\n");
            }
        }

        return out;
    }

    struct inapplicable_reason_printer
    {
        template <print_iterator OutIter>
        OutIter operator()([[maybe_unused]] OutIter out, [[maybe_unused]] state_applicable const& state) const
        {
            util::unreachable();
        }

        template <print_iterator OutIter>
        OutIter operator()(OutIter out, state_inapplicable const& state) const
        {
            auto const totalSequences = std::ranges::ssize(state.sequences)
                                      + std::ranges::ssize(state.inapplicableSequences);
            return format::format_to(
                std::move(out),
                "it's not head of {} Sequence(s) ({} total).",
                std::ranges::ssize(state.inapplicableSequences),
                totalSequences);
        }

        template <print_iterator OutIter>
        OutIter operator()(OutIter out, const state_saturated& state) const
        {
            out = format::format_to(
                std::move(out),
                "it's already saturated (matched {} out of {} times).",
                state.count,
                state.max);

            return out;
        }
    };

    template <print_iterator OutIter>
    OutIter stringify_expectation_report_requirement_violations(
        OutIter out,
        std::span<std::optional<StringT> const> const descriptions,
        StringViewT const linePrefix)
    {
        MIMICPP_ASSERT(!descriptions.empty(), "Zero requirements can never be violated.");

        out = std::ranges::copy(linePrefix, std::move(out)).out;
        out = format::format_to(std::move(out), "Due to Violation(s):\n");

        int withoutDescription{0};
        for (auto const& description : descriptions)
        {
            if (description)
            {
                out = std::ranges::copy(linePrefix, std::move(out)).out;
                out = format::format_to(std::move(out), "  - ");
                out = std::ranges::copy(*description, std::move(out)).out;
                out = format::format_to(std::move(out), "\n");
            }
            else
            {
                ++withoutDescription;
            }
        }

        if (0 < withoutDescription)
        {
            out = std::ranges::copy(linePrefix, std::move(out)).out;
            out = format::format_to(std::move(out), "  - ");
            out = format::format_to(
                std::move(out),
                "{} Requirement(s) failed without further description.\n",
                withoutDescription);
        }

        return out;
    }

    struct unfulfilled_reason_printer
    {
        template <print_iterator OutIter>
        OutIter operator()(OutIter out, state_applicable const& state) const
        {
            return stringify_times_state(
                std::move(out),
                state.count,
                state.min,
                state.max);
        }

        template <print_iterator OutIter>
        OutIter operator()(OutIter out, state_inapplicable const& state) const
        {
            return stringify_times_state(
                std::move(out),
                state.count,
                state.min,
                state.max);
        }

        template <print_iterator OutIter>
        OutIter operator()([[maybe_unused]] OutIter out, [[maybe_unused]] state_saturated const& state) const
        {
            util::unreachable();
        }

    private:
        template <print_iterator OutIter>
        static OutIter stringify_times_state(OutIter out, int const current, int const min, int const max)
        {
            const auto verbalizeValue = [](OutIter o, int const value) {
                MIMICPP_ASSERT(0 < value, "Invalid value.");

                switch (value)
                {
                case 1:  return format::format_to(std::move(o), "once");
                case 2:  return format::format_to(std::move(o), "twice");
                default: return format::format_to(std::move(o), "{} times", value);
                }
            };

            MIMICPP_ASSERT(current < min, "State doesn't denote an unsatisfied state.");

            out = format::format_to(std::move(out), "matching ");

            if (min == max)
            {
                out = format::format_to(std::move(out), "exactly ");
                out = verbalizeValue(std::move(out), min);
                out = format::format_to(std::move(out), " ");
            }
            else if (max == std::numeric_limits<int>::max())
            {
                out = format::format_to(std::move(out), "at least ");
                out = verbalizeValue(std::move(out), min);
                out = format::format_to(std::move(out), " ");
            }
            else
            {
                out = format::format_to(
                    std::move(out),
                    "between {} and {} times ",
                    min,
                    max);
            }

            return format::format_to(
                std::move(out),
                "was expected => requires {} further match(es).\n",
                min - current);
        }
    };
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::reporting
{
    [[nodiscard]]
    inline StringT stringify_full_match(CallReport const& call, ExpectationReport expectation)
    {
        MIMICPP_ASSERT(std::holds_alternative<state_applicable>(expectation.controlReport), "Report denotes inapplicable expectation.");

        StringStreamT ss{};
        ss << "Matched ";
        detail::stringify_call_report_from(std::ostreambuf_iterator{ss}, call);
        detail::stringify_call_report_target(std::ostreambuf_iterator{ss}, call);
        detail::stringify_call_report_arguments(std::ostreambuf_iterator{ss}, call, "\t");

        ss << "\t" << "Chose ";
        detail::stringify_expectation_report_from(std::ostreambuf_iterator{ss}, expectation);

        if (!expectation.requirementDescriptions.empty())
        {
            std::ranges::sort(expectation.requirementDescriptions);
            detail::stringify_expectation_report_requirement_adherences(
                std::ostreambuf_iterator{ss},
                expectation.requirementDescriptions,
                "\t");
        }
        else
        {
            ss << "\t" << "Without any Requirements.\n";
        }

        detail::stringify_stacktrace(
            std::ostreambuf_iterator{ss},
            call.stacktrace);

        return std::move(ss).str();
    }

    [[nodiscard]]
    inline StringT stringify_inapplicable_matches(CallReport const& call, std::span<ExpectationReport> expectations)
    {
        MIMICPP_ASSERT(!expectations.empty(), "No expectations given.");

        StringStreamT ss{};

        ss << "Unmatched ";
        detail::stringify_call_report_from(std::ostreambuf_iterator{ss}, call);
        detail::stringify_call_report_target(std::ostreambuf_iterator{ss}, call);
        detail::stringify_call_report_arguments(std::ostreambuf_iterator{ss}, call, "\t");

        ss << expectations.size() << " inapplicable but otherwise matching Expectation(s):";

        for (int i{};
             auto& expReport : expectations)
        {
            ss << "\n\t#" << ++i << " ";
            detail::stringify_expectation_report_from(std::ostreambuf_iterator{ss}, expReport);

            ss << "\tBecause ";
            std::visit(
                std::bind_front(detail::inapplicable_reason_printer{}, std::ostreambuf_iterator{ss}),
                expReport.controlReport);
            ss << "\n";

            std::ranges::sort(expReport.requirementDescriptions);
            detail::stringify_expectation_report_requirement_adherences(
                std::ostreambuf_iterator{ss},
                expReport.requirementDescriptions,
                "\t");
        }

        detail::stringify_stacktrace(
            std::ostreambuf_iterator{ss},
            call.stacktrace);

        return std::move(ss).str();
    }

    [[nodiscard]]
    inline StringT stringify_no_matches(CallReport const& call, std::span<NoMatchReport> noMatchReports)
    {
        StringStreamT ss{};

        ss << "Unmatched ";
        detail::stringify_call_report_from(std::ostreambuf_iterator{ss}, call);
        detail::stringify_call_report_target(std::ostreambuf_iterator{ss}, call);
        detail::stringify_call_report_arguments(std::ostreambuf_iterator{ss}, call, "\t");

        std::vector<NoMatchReport*> applicableReports{};
        for (auto& noMatch : noMatchReports)
        {
            if (std::holds_alternative<state_applicable>(noMatch.expectationReport.controlReport))
            {
                applicableReports.emplace_back(&noMatch);
            }
        }

        if (applicableReports.empty())
        {
            ss << "No applicable Expectations available!\n";
        }
        else
        {
            ss << applicableReports.size() << " applicable non-matching Expectation(s):";

            for (int i{};
                 auto* report : applicableReports)
            {
                auto& [expReport, outcomes] = *report;

                ss << "\n\t#" << ++i << " ";
                detail::stringify_expectation_report_from(std::ostreambuf_iterator{ss}, expReport);

                std::span const violations = util::partition_by(
                    expReport.requirementDescriptions,
                    outcomes.outcomes,
                    std::bind_front(std::equal_to{}, true));
                MIMICPP_ASSERT(!violations.empty(), "Zero violations do not denote a no-match.");
                std::ranges::sort(violations);
                detail::stringify_expectation_report_requirement_violations(
                    std::ostreambuf_iterator{ss},
                    violations,
                    "\t");

                std::span const adherences{expReport.requirementDescriptions.data(), violations.data()};
                std::ranges::sort(adherences);
                detail::stringify_expectation_report_requirement_adherences(
                    std::ostreambuf_iterator{ss},
                    adherences,
                    "\t");
            }
        }

        detail::stringify_stacktrace(
            std::ostreambuf_iterator{ss},
            call.stacktrace);

        return std::move(ss).str();
    }

    [[nodiscard]]
    inline StringT stringify_unfulfilled_expectation(ExpectationReport const& expectationReport)
    {
        StringStreamT ss{};

        ss << "Unfulfilled ";
        detail::stringify_expectation_report_from(std::ostreambuf_iterator{ss}, expectationReport);
        detail::stringify_expectation_report_target(std::ostreambuf_iterator{ss}, expectationReport);
        ss << "\tBecause ";
        std::visit(
            std::bind_front(detail::unfulfilled_reason_printer{}, std::ostreambuf_iterator{ss}),
            expectationReport.controlReport);

        return std::move(ss).str();
    }

    [[nodiscard]]
    inline StringT stringify_unhandled_exception(
        CallReport const& call,
        ExpectationReport const& expectationReport,
        std::exception_ptr const& exception)
    {
        StringStreamT ss{};
        ss << "Unhandled Exception ";

        try
        {
            std::rethrow_exception(exception);
        }
        catch (const std::exception& e)
        {
            ss << "with message `" << e.what() << "`\n";
        }
        catch (...)
        {
            ss << "of unknown type.\n";
        }

        ss << "\t" << "While checking ";
        detail::stringify_expectation_report_from(std::ostreambuf_iterator{ss}, expectationReport);

        ss << "For ";
        detail::stringify_call_report_from(std::ostreambuf_iterator{ss}, call);
        detail::stringify_call_report_target(std::ostreambuf_iterator{ss}, call);
        detail::stringify_call_report_arguments(std::ostreambuf_iterator{ss}, call, "\t");

        detail::stringify_stacktrace(
            std::ostreambuf_iterator{ss},
            call.stacktrace);

        return std::move(ss).str();
    }
}

#endif
