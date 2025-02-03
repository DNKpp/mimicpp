//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_DEFAULT_REPORTER_HPP
#define MIMICPP_REPORTING_DEFAULT_REPORTER_HPP

#pragma once

#include "mimic++/reporting/CallReport.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/IReporter.hpp"
#include "mimic++/reporting/NoMatchReport.hpp"
#include "mimic++/reporting/StringifyReports.hpp"

#include <cassert>
#include <cstddef>
#include <exception>
#include <ostream>
#include <source_location>
#include <stdexcept>
#include <utility>

namespace mimicpp::reporting
{
    template <typename Data = std::nullptr_t>
    class Error final
        : public std::logic_error
    {
    public:
        [[nodiscard]]
        explicit Error(
            const std::string& what,
            Data&& data = {},
            const std::source_location& loc = std::source_location::current())
            : std::logic_error{what},
              m_Data{std::move(data)},
              m_Loc{loc}
        {
        }

        [[nodiscard]]
        Data const& data() const noexcept
        {
            return m_Data;
        }

        [[nodiscard]]
        std::source_location const& where() const noexcept
        {
            return m_Loc;
        }

    private:
        Data m_Data;
        std::source_location m_Loc;
    };

    using UnmatchedCallT = Error<CallReport>;
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
            std::vector<NoMatchReport> noMatchReports) override
        {
            auto const msg = stringify_no_matches(call, noMatchReports);
            if (m_Out)
            {
                *m_Out << msg << '\n';
            }

            std::source_location const loc{*call.fromLoc};
            throw UnmatchedCallT{msg, std::move(call), loc};
        }

        [[noreturn]]
        void report_inapplicable_matches(
            CallReport call,
            std::vector<ExpectationReport> expectationReports) override

        {
            const auto msg = stringify_inapplicable_matches(call, expectationReports);
            if (m_Out)
            {
                *m_Out << msg << '\n';
            }

            std::source_location const loc{*call.fromLoc};
            throw UnmatchedCallT{msg, std::move(call), loc};
        }

        void report_full_match(
            [[maybe_unused]] CallReport const call,
            [[maybe_unused]] ExpectationReport const expectationReport) noexcept override
        {
            assert(std::holds_alternative<state_applicable>(expectationReport.controlReport) && "Report denotes inapplicable expectation.");
        }

        void report_unfulfilled_expectation(ExpectationReport expectationReport) override
        {
            if (0 == std::uncaught_exceptions())
            {
                auto const msg = stringify_unfulfilled_expectation(expectationReport);
                if (m_Out)
                {
                    *m_Out << msg << '\n';
                }

                throw UnfulfilledExpectationT{msg, std::move(expectationReport)};
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
                *m_Out << stringify_unhandled_exception(call, expectationReport, exception)
                       << '\n';
            }
        }

    private:
        std::ostream* m_Out{};
    };
}

#endif
