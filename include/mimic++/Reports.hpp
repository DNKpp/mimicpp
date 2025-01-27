//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTS_HPP
#define MIMICPP_REPORTS_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/Printer.hpp"
#include "mimic++/Utility.hpp"

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
     * Each report type can be printed via ``mimicpp::print`` function, but users may customize that by adding a specialization for
     * ``mimicpp::custom::Printer``.
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
     * \brief Contains information about a specific (potentially cv-ref-qualified) type.
     */
    class TypeReport
    {
    public:
        template <typename T>
        [[nodiscard]]
        static TypeReport make()
        {
            return TypeReport{typeid(T), print_type<T>()};
        }

        [[nodiscard]]
        std::type_index type_index() const noexcept
        {
            return m_TypeIndex;
        }

        [[nodiscard]]
        constexpr StringT const& name() const noexcept
        {
            return m_Name;
        }

        [[nodiscard]]
        friend bool operator==(TypeReport const& lhs, TypeReport const& rhs) noexcept
        {
            return lhs.m_TypeIndex == rhs.m_TypeIndex
                // typeid decays cv-qualifications.
                // So, we must compare the name, too.
                && lhs.m_Name == rhs.m_Name;
        }

    private:
        std::type_index m_TypeIndex;
        StringT m_Name;

        explicit TypeReport(std::type_index&& index, StringT&& name) noexcept
            : m_TypeIndex{std::move(index)},
              m_Name{std::move(name)}
        {
            assert(!m_Name.empty() && "Type name must not be empty.");
        }
    };

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
            TypeReport typeInfo;
            StringT stateString;

            [[nodiscard]]
            friend bool operator==(const Arg&, const Arg&) = default;
        };

        TypeReport returnTypeInfo;
        std::vector<Arg> argDetails{};
        std::source_location fromLoc{};
        Stacktrace stacktrace{stacktrace::NullBackend{}};
        ValueCategory fromCategory{};
        Constness fromConstness{};

        [[nodiscard]]
        friend bool operator==(const CallReport& lhs, const CallReport& rhs)
        {
            return lhs.returnTypeInfo == rhs.returnTypeInfo
                && lhs.argDetails == rhs.argDetails
                && is_same_source_location(lhs.fromLoc, rhs.fromLoc)
                && lhs.fromCategory == rhs.fromCategory
                && lhs.fromConstness == rhs.fromConstness
                && lhs.stacktrace == rhs.stacktrace;
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
    CallReport make_call_report(call::Info<Return, Params...> callInfo)
    {
        return CallReport{
            .returnTypeInfo = TypeReport::make<Return>(),
            .argDetails = std::apply(
                [](auto&... args) {
                    return std::vector<CallReport::Arg>{
                        CallReport::Arg{
                                        .typeInfo = TypeReport::make<Params>(),
                                        .stateString = mimicpp::print(args.get())}
                        ...
                    };
                },
                callInfo.args),
            .fromLoc = std::move(callInfo.fromSourceLocation),
            .stacktrace = std::move(callInfo.stacktrace),
            .fromCategory = callInfo.fromCategory,
            .fromConstness = callInfo.fromConstness};
    }

    template <>
    struct printing::detail::state::common_type_printer<CallReport>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out, const CallReport& report)
        {
            out = format::format_to(
                std::move(out),
                "call from ");
            if (!report.stacktrace.empty())
            {
                out = stacktrace::detail::print_entry(
                    std::move(out),
                    report.stacktrace,
                    0u);
            }
            else
            {
                out = mimicpp::print(
                    std::move(out),
                    report.fromLoc);
            }
            out = format::format_to(
                std::move(out),
                "\n");

            out = format::format_to(
                std::move(out),
                "constness: {}\n"
                "value category: {}\n"
                "return type: {}\n",
                report.fromConstness,
                report.fromCategory,
                report.returnTypeInfo.name());

            if (!std::ranges::empty(report.argDetails))
            {
                out = format::format_to(
                    std::move(out),
                    "args:\n");
                for (const std::size_t i : std::views::iota(0u, std::ranges::size(report.argDetails)))
                {
                    auto const& arg = report.argDetails[i];
                    out = format::format_to(
                        std::move(out),
                        "\targ[{}]: {{\n"
                        "\t\ttype: {},\n"
                        "\t\tvalue: {}\n"
                        "\t}},\n",
                        i,
                        arg.typeInfo.name(),
                        arg.stateString);
                }
            }

            return out;
        }
    };

    /**
     * \brief Contains the extracted info from a typed expectation.
     * \details This type is meant to be used to communicate with independent domains via the reporter interface and thus contains
     * the generic information as plain ``std`` types.
     */
    class ExpectationReport
    {
    public:
        detail::expectation_info expectationInfo{};
        std::optional<StringT> finalizerDescription{};
        std::optional<StringT> timesDescription{};
        std::vector<std::optional<StringT>> expectationDescriptions{};

        [[nodiscard]]
        friend bool operator==(const ExpectationReport& lhs, const ExpectationReport& rhs) = default;
    };

    template <>
    struct printing::detail::state::common_type_printer<ExpectationReport>
    {
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

    template <>
    struct printing::detail::state::common_type_printer<MatchReport>
    {
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
                    std::bind_front(mimicpp::detail::control_state_printer{}, std::move(out)),
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

    /**
     * \}
     */
}

#endif
