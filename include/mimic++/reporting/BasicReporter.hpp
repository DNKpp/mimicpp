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

#include <concepts>
#include <cstddef>
#include <exception>
#include <functional>
#include <iterator>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>

namespace mimicpp::reporting
{
    template <typename Data = std::nullptr_t>
    class Error final
        : public std::runtime_error
    {
    public:
        [[nodiscard]]
        explicit Error(
            const std::string& what,
            Data&& data = Data{},
            const std::source_location& loc = std::source_location::current())
            : std::runtime_error{what},
              m_Data{std::move(data)},
              m_Loc{loc}
        {
        }

        [[nodiscard]]
        const Data& data() const noexcept
        {
            return m_Data;
        }

        [[nodiscard]]
        const std::source_location& where() const noexcept
        {
            return m_Loc;
        }

    private:
        Data m_Data;
        std::source_location m_Loc;
    };

    namespace detail
    {
        template <print_iterator OutIter>
        OutIter stringify_stacktrace(OutIter out, const Stacktrace& stacktrace)
        {
            if (!stacktrace.empty())
            {
                out = format::format_to(
                    std::move(out),
                    "Stacktrace:\n");
                out = mimicpp::print(
                    std::move(out),
                    stacktrace);
            }

            return out;
        }

        [[nodiscard]]
        inline StringT stringify_no_match_report(const CallReport& call, const std::span<const MatchReport> matchReports)
        {
            StringStreamT ss{};
            ss << "No match for ";
            mimicpp::print(
                std::ostreambuf_iterator{ss},
                call);
            ss << "\n";

            if (std::ranges::empty(matchReports))
            {
                ss << "No expectations available.\n";
            }
            else
            {
                format::format_to(
                    std::ostreambuf_iterator{ss},
                    "{} available expectation(s):\n",
                    std::ranges::size(matchReports));

                for (const auto& report : matchReports)
                {
                    mimicpp::print(
                        std::ostreambuf_iterator{ss},
                        report);
                    ss << "\n";
                }
            }

            stringify_stacktrace(
                std::ostreambuf_iterator{ss},
                call.stacktrace);

            return std::move(ss).str();
        }

        [[nodiscard]]
        inline StringT stringify_inapplicable_match_report(const CallReport& call, const std::span<const MatchReport> matchReports)
        {
            StringStreamT ss{};
            ss << "No applicable match for ";
            mimicpp::print(
                std::ostreambuf_iterator{ss},
                call);
            ss << "\n";

            ss << "Tested expectations:\n";
            for (const auto& report : matchReports)
            {
                mimicpp::print(
                    std::ostreambuf_iterator{ss},
                    report);
                ss << "\n";
            }

            stringify_stacktrace(
                std::ostreambuf_iterator{ss},
                call.stacktrace);

            return std::move(ss).str();
        }

        [[nodiscard]]
        inline StringT stringify_report(const CallReport& call, const MatchReport& matchReport)
        {
            StringStreamT ss{};
            ss << "Found match for ";
            mimicpp::print(
                std::ostreambuf_iterator{ss},
                call);
            ss << "\n";

            mimicpp::print(
                std::ostreambuf_iterator{ss},
                matchReport);
            ss << "\n";

            stringify_stacktrace(
                std::ostreambuf_iterator{ss},
                call.stacktrace);

            return std::move(ss).str();
        }

        [[nodiscard]]
        inline StringT stringify_unfulfilled_expectation(const ExpectationReport& expectationReport)
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
            const CallReport& call,
            const ExpectationReport& expectationReport,
            const std::exception_ptr& exception)
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
        void report_no_matches(const CallReport call, const std::vector<MatchReport> matchReports) override
        {
            send_fail(
                detail::stringify_no_match_report(call, matchReports));
        }

        [[noreturn]]
        void report_inapplicable_matches(const CallReport call, const std::vector<MatchReport> matchReports) override
        {
            send_fail(
                detail::stringify_inapplicable_match_report(call, matchReports));
        }

        void report_full_match(const CallReport call, const MatchReport matchReport) noexcept override
        {
            send_success(
                detail::stringify_report(call, matchReport));
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
