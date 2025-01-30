//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_EXPECTATION_REPORT_HPP
#define MIMICPP_REPORTING_EXPECTATION_REPORT_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/StatePrinter.hpp"

#include <algorithm>
#include <optional>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

namespace mimicpp::reporting
{
    struct state_inapplicable
    {
        int min{};
        int max{};
        int count{};
        std::vector<sequence::rating> sequenceRatings{};
        std::vector<sequence::Tag> inapplicableSequences{};

        [[nodiscard]]
        friend bool operator==(const state_inapplicable&, const state_inapplicable&) = default;
    };

    struct state_applicable
    {
        int min{};
        int max{};
        int count{};
        std::vector<sequence::rating> sequenceRatings{};

        [[nodiscard]]
        friend bool operator==(const state_applicable&, const state_applicable&) = default;
    };

    struct state_saturated
    {
        int min{};
        int max{};
        int count{};
        std::vector<sequence::Tag> sequences{};

        [[nodiscard]]
        friend bool operator==(const state_saturated&, const state_saturated&) = default;
    };

    using control_state_t = std::variant<
        state_inapplicable,
        state_applicable,
        state_saturated>;

    namespace detail
    {
        using mimicpp::detail::expectation_info;

        template <print_iterator OutIter>
        OutIter print_times_state(OutIter out, const std::size_t current, const std::size_t min, const std::size_t max)
        {
            const auto verbalizeValue = [](OutIter o, const std::size_t value) {
                switch (value)
                {
                case 0:  return format::format_to(std::move(o), "never");
                case 1:  return format::format_to(std::move(o), "once");
                case 2:  return format::format_to(std::move(o), "twice");
                default: return format::format_to(std::move(o), "{} times", value);
                }
            };

            if (current == max)
            {
                out = format::format_to(
                    std::move(out),
                    "already saturated (matched ");
                out = verbalizeValue(std::move(out), current);
                return format::format_to(std::move(out), ")");
            }

            if (min <= current)
            {
                return format::format_to(
                    std::move(out),
                    "accepts further matches (matched {} out of {} times)",
                    current,
                    max);
            }

            const auto verbalizeInterval = [verbalizeValue](OutIter o, const std::size_t start, const std::size_t end) {
                if (start < end)
                {
                    return format::format_to(
                        std::move(o),
                        "between {} and {} times",
                        start,
                        end);
                }

                o = format::format_to(std::move(o), "exactly ");
                return verbalizeValue(std::move(o), end);
            };

            out = format::format_to(std::move(out), "matched ");
            out = verbalizeValue(std::move(out), current);
            out = format::format_to(std::move(out), " - ");
            out = verbalizeInterval(std::move(out), min, max);
            return format::format_to(std::move(out), " is expected");
        }

        struct control_state_printer
        {
            template <print_iterator OutIter>
            OutIter operator()(OutIter out, const state_applicable& state) const
            {
                out = print_times_state(
                    std::move(out),
                    state.count,
                    state.min,
                    state.max);

                if (const auto sequenceCount = std::ranges::ssize(state.sequenceRatings);
                    0 < sequenceCount)
                {
                    out = format::format_to(
                        std::move(out),
                        ",\n\tand is the current element of {} sequence(s).",
                        sequenceCount);
                }

                return out;
            }

            template <print_iterator OutIter>
            OutIter operator()(OutIter out, const state_inapplicable& state) const
            {
                out = print_times_state(
                    std::move(out),
                    state.count,
                    state.min,
                    state.max);

                const auto totalSequences = std::ranges::ssize(state.sequenceRatings)
                                          + std::ranges::ssize(state.inapplicableSequences);
                return format::format_to(
                    std::move(out),
                    ",\n\tbut is not the current element of {} sequence(s) ({} total).",
                    std::ranges::ssize(state.inapplicableSequences),
                    totalSequences);
            }

            template <print_iterator OutIter>
            OutIter operator()(OutIter out, const state_saturated& state) const
            {
                out = print_times_state(
                    std::move(out),
                    state.count,
                    state.min,
                    state.max);

                if (const auto sequenceCount = std::ranges::ssize(state.sequences);
                    0 < sequenceCount)
                {
                    out = format::format_to(
                        std::move(out),
                        ",\n\tand is part of {} sequence(s).",
                        sequenceCount);
                }

                return out;
            }
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
        detail::expectation_info info{};
        control_state_t controlReport{};
        std::optional<StringT> finalizerDescription{};
        std::vector<std::optional<StringT>> requirementDescriptions{};

        [[nodiscard]]
        friend bool operator==(const ExpectationReport&, const ExpectationReport&) = default;
    };

    class RequirementOutcomes
    {
    public:
        std::vector<bool> outcomes{};

        [[nodiscard]]
        friend bool operator==(const RequirementOutcomes&, const RequirementOutcomes&) = default;
    };
}

template <>
struct mimicpp::printing::detail::state::common_type_printer<mimicpp::reporting::ExpectationReport>
{
    using ExpectationReport = reporting::ExpectationReport;

    template <print_iterator OutIter>
    static OutIter print(OutIter out, const ExpectationReport& report)
    {
        out = format::format_to(
            std::move(out),
            "Expectation report:\n");
        out = format::format_to(
            std::move(out),
            "mock: {}\n",
            report.expectationInfo.mockName);
        out = format::format_to(
            std::move(out),
            "from: ");
        out = mimicpp::print(
            std::move(out),
            report.expectationInfo.sourceLocation);
        out = format::format_to(
            std::move(out),
            "\n");

        if (report.timesDescription)
        {
            out = format::format_to(
                out,
                "times: {}\n",
                *report.timesDescription);
        }

        if (std::ranges::any_of(
                report.expectationDescriptions,
                [](const auto& desc) { return desc.has_value(); }))
        {
            out = format::format_to(
                std::move(out),
                "expects:\n");
            for (const auto& desc : report.expectationDescriptions
                                        | std::views::filter([](const auto& desc) { return desc.has_value(); }))
            {
                out = format::format_to(
                    std::move(out),
                    "\t{},\n",
                    *desc);
            }
        }

        if (report.finalizerDescription)
        {
            out = format::format_to(
                std::move(out),
                "finally: {}\n",
                *report.finalizerDescription);
        }

        return out;
    }
};

#endif
