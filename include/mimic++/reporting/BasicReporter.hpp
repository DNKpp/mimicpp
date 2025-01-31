//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_BASIC_REPORTER_HPP
#define MIMICPP_REPORTING_BASIC_REPORTER_HPP

#pragma once

#include "mimic++/Stacktrace.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/reporting/CallReport.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/IReporter.hpp"
#include "mimic++/reporting/MatchReport.hpp"
#include "mimic++/reporting/StringifyReports.hpp"

#include <algorithm>
#include <concepts>
#include <exception>
#include <functional>
#include <iterator>
#include <string>
#include <utility>

namespace mimicpp::reporting
{
    namespace detail
    {
        [[nodiscard]]
        inline StringT stringify_unfulfilled_expectation(ExpectationReport const& expectationReport)
        {
            StringStreamT ss{};
            ss << "Unfulfilled expectation:\n";
            mimicpp::print(
                std::ostreambuf_iterator{ss},
                expectationReport);
            ss << "\n";

            return std::move(ss).str();
        }

        [[nodiscard]]
        inline StringT stringify_unhandled_exception(
            CallReport const& call,
            ExpectationReport const& expectationReport,
            std::exception_ptr const& exception)
        {
            StringStreamT ss{};
            ss << "Unhandled exception: ";

            try
            {
                std::rethrow_exception(exception);
            }
            catch (const std::exception& e)
            {
                ss << "what: "
                   << e.what()
                   << "\n";
            }
            catch (...)
            {
                ss << "Unknown exception type.\n";
            }

            ss << "while checking expectation:\n";
            mimicpp::print(
                std::ostreambuf_iterator{ss},
                expectationReport);
            ss << "\n";

            ss << "For ";
            mimicpp::print(
                std::ostreambuf_iterator{ss},
                call);
            ss << "\n";

            return std::move(ss).str();
        }
    }

    /**
     * \brief A reporter, which creates text messages and reports them via the provided callbacks.
     * \tparam successReporter The success reporter callback.
     * \tparam warningReporter The warning reporter callback.
     * \tparam failReporter The fail reporter callback. This reporter must never return!
     */
    template <
        std::invocable<const StringT&> auto successReporter,
        std::invocable<const StringT&> auto warningReporter,
        std::invocable<const StringT&> auto failReporter>
    class BasicReporter
        : public IReporter
    {
    public:
        [[noreturn]]
        void report_no_matches(CallReport call, std::vector<NoMatchReport> noMatchReports) override
        {
            send_fail(
                detail::stringify_no_matches(std::move(call), noMatchReports));
        }

        [[noreturn]]
        void report_inapplicable_matches(CallReport call, std::vector<ExpectationReport> expectationReports) override
        {
            send_fail(
                detail::stringify_inapplicable_matches(std::move(call), expectationReports));
        }

        void report_full_match(CallReport call, ExpectationReport expectationReport) noexcept override
        {
            send_success(
                detail::stringify_full_match(std::move(call), std::move(expectationReport)));
        }

        void report_unfulfilled_expectation(const ExpectationReport expectationReport) override
        {
            if (0 == std::uncaught_exceptions())
            {
                send_fail(
                    detail::stringify_unfulfilled_expectation(expectationReport));
            }
        }

        void report_error(const StringT message) override
        {
            if (0 == std::uncaught_exceptions())
            {
                send_fail(message);
            }
        }

        void report_unhandled_exception(
            const CallReport call,
            const ExpectationReport expectationReport,
            const std::exception_ptr exception) override
        {
            send_warning(
                detail::stringify_unhandled_exception(call, expectationReport, exception));
        }

    private:
        void send_success(const StringT& msg)
        {
            std::invoke(successReporter, msg);
        }

        void send_warning(const StringT& msg)
        {
            std::invoke(warningReporter, msg);
        }

        [[noreturn]]
        void send_fail(const StringT& msg)
        {
            // GCOVR_EXCL_START
            std::invoke(failReporter, msg);
            unreachable();
            // GCOVR_EXCL_STOP
        }
    };
}

#endif
