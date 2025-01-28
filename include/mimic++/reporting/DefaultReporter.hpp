//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_DEFAULT_REPORTER_HPP
#define MIMICPP_REPORTING_DEFAULT_REPORTER_HPP

#pragma once

#include "mimic++/reporting/BasicReporter.hpp"
#include "mimic++/reporting/CallReport.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/IReporter.hpp"
#include "mimic++/reporting/MatchReport.hpp"

#include <algorithm>
#include <cassert>
#include <exception>
#include <functional>
#include <ostream>
#include <source_location>
#include <utility>

namespace mimicpp::reporting
{
    using UnmatchedCallT = Error<std::tuple<CallReport, std::vector<MatchReport>>>;
    using UnfulfilledExpectationT = Error<ExpectationReport>;

    /**
     * \brief The default reporter.
     */
    class DefaultReporter final
        : public IReporter
    {
    public:
        [[nodiscard]]
        DefaultReporter() = default;

        [[nodiscard]]
        explicit DefaultReporter(std::ostream& out) noexcept
            : m_Out{std::addressof(out)}
        {
        }

        [[noreturn]]
        void report_no_matches(
            CallReport call,
            std::vector<MatchReport> matchReports) override
        {
            assert(
                std::ranges::all_of(
                    matchReports,
                    std::bind_front(std::equal_to{}, MatchResult::none),
                    &evaluate_match_report));

            const auto msg = detail::stringify_no_match_report(call, matchReports);
            if (m_Out)
            {
                *m_Out << msg << '\n';
            }

            const std::source_location loc{call.fromLoc};
            throw UnmatchedCallT{
                msg,
                {std::move(call), std::move(matchReports)},
                loc
            };
        }

        [[noreturn]]
        void report_inapplicable_matches(
            CallReport call,
            std::vector<MatchReport> matchReports) override
        {
            assert(
                std::ranges::all_of(
                    matchReports,
                    std::bind_front(std::equal_to{}, MatchResult::inapplicable),
                    &evaluate_match_report));

            const auto msg = detail::stringify_inapplicable_match_report(call, matchReports);
            if (m_Out)
            {
                *m_Out << msg << '\n';
            }

            const std::source_location loc{call.fromLoc};
            throw UnmatchedCallT{
                msg,
                {std::move(call), std::move(matchReports)},
                loc
            };
        }

        void report_full_match(
            [[maybe_unused]] const CallReport call,
            [[maybe_unused]] const MatchReport matchReport) noexcept override
        {
            assert(MatchResult::full == evaluate_match_report(matchReport));
        }

        void report_unfulfilled_expectation(
            ExpectationReport expectationReport) override
        {
            if (0 == std::uncaught_exceptions())
            {
                const auto msg = detail::stringify_unfulfilled_expectation(expectationReport);
                if (m_Out)
                {
                    *m_Out << msg << '\n';
                }

                throw UnfulfilledExpectationT{
                    msg,
                    std::move(expectationReport)};
            }
        }

        void report_error(const StringT message) override
        {
            if (0 == std::uncaught_exceptions())
            {
                if (m_Out)
                {
                    *m_Out << message << '\n';
                }

                throw Error{message};
            }
        }

        void report_unhandled_exception(
            const CallReport call,
            const ExpectationReport expectationReport,
            const std::exception_ptr exception) override
        {
            if (m_Out)
            {
                *m_Out << detail::stringify_unhandled_exception(call, expectationReport, exception)
                       << '\n';
            }
        }

    private:
        std::ostream* m_Out{};
    };
}

#endif
