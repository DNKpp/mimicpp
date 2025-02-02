//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_BASIC_REPORTER_HPP
#define MIMICPP_REPORTING_BASIC_REPORTER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/reporting/CallReport.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/IReporter.hpp"
#include "mimic++/reporting/StringifyReports.hpp"

#include <concepts>
#include <exception>
#include <utility>

namespace mimicpp::reporting
{
    /**
     * \brief A reporter, which creates text messages and reports them via the provided callbacks.
     * \tparam successReporter The success reporter callback.
     * \tparam warningReporter The warning reporter callback.
     * \tparam failReporter The fail reporter callback. This reporter must never return!
     */
    template <
        std::invocable<StringT const&> auto successReporter,
        std::invocable<StringT const&> auto warningReporter,
        std::invocable<StringT const&> auto failReporter>
    class BasicReporter
        : public IReporter
    {
    public:
        [[noreturn]]
        void report_no_matches(CallReport call, std::vector<NoMatchReport> noMatchReports) override
        {
            send_fail(stringify_no_matches(std::move(call), noMatchReports));
        }

        [[noreturn]]
        void report_inapplicable_matches(CallReport call, std::vector<ExpectationReport> expectationReports) override
        {
            send_fail(stringify_inapplicable_matches(std::move(call), expectationReports));
        }

        void report_full_match(CallReport call, ExpectationReport expectationReport) noexcept override
        {
            send_success(stringify_full_match(std::move(call), std::move(expectationReport)));
        }

        void report_unfulfilled_expectation(const ExpectationReport expectationReport) override
        {
            if (0 == std::uncaught_exceptions())
            {
                send_fail(stringify_unfulfilled_expectation(expectationReport));
            }
        }

        void report_error(StringT const message) override
        {
            if (0 == std::uncaught_exceptions())
            {
                send_fail(message);
            }
        }

        void report_unhandled_exception(
            CallReport const call,
            ExpectationReport const expectationReport,
            std::exception_ptr const exception) override
        {
            send_warning(stringify_unhandled_exception(call, expectationReport, exception));
        }

    private:
        void send_success(StringT const& msg)
        {
            std::invoke(successReporter, msg);
        }

        void send_warning(StringT const& msg)
        {
            std::invoke(warningReporter, msg);
        }

        [[noreturn]]
        void send_fail(StringT const& msg)
        {
            // GCOVR_EXCL_START
            std::invoke(failReporter, msg);
            unreachable();
            // GCOVR_EXCL_STOP
        }
    };
}

#endif
