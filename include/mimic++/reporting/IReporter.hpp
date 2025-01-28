//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_IREPORTER_HPP
#define MIMICPP_REPORTING_IREPORTER_HPP

#pragma once

#include "mimic++/reporting/Fwd.hpp"

#include <exception>
#include <vector>

namespace mimicpp::reporting
{
    /**
     * \defgroup REPORTING reporting
     * \brief Contains reporting related symbols
     * \details Reporting is executed, when something notably has been detected by ``mimic++``; often it is expected, that the reporter
     * reacts to such a report in a specific manner (e.g. aborting the test case). For example the ``DefaultReporter`` simply throws
     * exceptions on error reports, while other more specialized reporters handle such cases slightly different (but still abort the
     * current test).
     * These specialized Reporters are used to send reports to a specific destination (e.g. the utilized unit-test framework),
     * which often provide more advanced mechanics for printing failed tests to the users.
     *
     * Users may provide their own reporter implementation; e.g. if there is no reporter for the desired unit-test framework.
     *
     * At any time there exists exactly one global reporter, which may be directly or indirectly exchanged by users.
     * Reports are sent to the currently installed reporter via the ``report_xyz`` free-functions. Most of those functions require, that
     * reports are handled in a specific manner (e.g. ``report_no_matches`` is expected to never return) and custom reporters **must**
     * follow that specification, otherwise this will lead to undefined behavior. For further details, have a look at the specific
     * function documentation.
     *
     * \note In general users shall not directly interact with the installed reporter, except when they want to replace it.
     *
     * \{
     */

    /**
     * \brief The reporter interface.
     * \details This is the central interface to be used, when creating reporters for external domains.
     */
    class IReporter
    {
    public:
        /**
         * \brief Defaulted virtual destructor.
         */
        virtual ~IReporter() = default;

        /**
         * \brief Expects reports about all ``none`` matching expectations. This is only called, if there are no better options available.
         * \param call The call report.
         * \param matchReports Reports of all ``none`` matching expectations.
         * \details This function is called, when no match has been found and there are no other expectations, which are matching but
         * inapplicable. In fact, this is the fallback reporting mechanism, for unmatched calls.
         * \note ``matchReports`` may be empty.
         *
         * \attention Derived reporter implementations must never return and shall instead leave the function via a thrown exception or
         * a terminating mechanism (e.g. ``std::terminate``). Otherwise, this will result in undefined behavior.
         */
        [[noreturn]]
        virtual void report_no_matches(
            CallReport call,
            std::vector<MatchReport> matchReports) = 0;

        /**
         * \brief Expects reports about all ``inapplicable`` matching expectations. This is only called, if there are no better options available.
         * \param call The call report.
         * \param matchReports Reports of all ``inapplicable`` matching expectations.
         * \details This function is called, when no applicable match has been found, but actually the call expectations are fulfilled. This in fact
         * happens, when the ``times`` policy is already saturated (e.g. it was once expected and already matched once) or otherwise not applicable
         * (e.g. a sequence element is not the current element).
         *
         * \attention Derived reporter implementations must never return and shall instead leave the function via a thrown exception or
         * a terminating mechanism (e.g. ``std::terminate``). Otherwise, this will result in undefined behavior.
         */
        [[noreturn]]
        virtual void report_inapplicable_matches(
            CallReport call,
            std::vector<MatchReport> matchReports) = 0;

        /**
         * \brief Expects the report about a ``full`` matching expectation.
         * \param call The call report.
         * \param matchReport Report of the ``full`` matching expectation.
         * \details This function is called, when a match has been found. There are no other expectations on the behavior of this function;
         * except the ``noexcept`` guarantee. Implementations shall simply return to the caller.
         */
        virtual void report_full_match(
            CallReport call,
            MatchReport matchReport) noexcept = 0;

        /**
         * \brief Expects the report of an unfulfilled expectation.
         * \param expectationReport The expectation report.
         * \details This function is called, when an unfulfilled expectation goes out of scope. In fact this happens, when the ``times`` policy is not
         * satisfied.
         *
         * \note In general, it is expected that this function does not return, but throws an exception instead. But, as this function is always called
         * when an unfulfilled expectation goes out of scope, implementations shall check whether an uncaught exception already exists (e.g. via
         * ``std::uncaught_exceptions``) before throwing by themselves.
         * \see ``DefaultReporter::report_unfulfilled_expectation`` for an example.
         */
        virtual void report_unfulfilled_expectation(
            ExpectationReport expectationReport) = 0;

        /**
         * \brief Expects rather unspecific errors.
         * \param message The error message.
         * \details This function is called, when an unspecific error occurs.
         *
         * \note In general, it is expected that this function does not return, but throws an exception instead. But, as this function may be called
         * due to any reason, implementations shall check whether an uncaught exception already exists (e.g. via ``std::uncaught_exceptions``) before
         * throwing by themselves.
         * \see ``DefaultReporter::report_error`` for an example.
         */
        virtual void report_error(StringT message) = 0;

        /**
         * \brief Expects reports about unhandled exceptions, during ``handle_call``.
         * \param call The call report.
         * \param expectationReport The expectation report.
         * \param exception The exception.
         * \details This function is called, when an expectation throws during a ``matches`` call. There are no expectations on the behavior of this
         * function. As this function is called inside a ``catch`` block, throwing exceptions will result in a terminate call.
         */
        virtual void report_unhandled_exception(
            CallReport call,
            ExpectationReport expectationReport,
            std::exception_ptr exception) = 0;

    protected:
        [[nodiscard]]
        IReporter() = default;

        IReporter(const IReporter&) = default;
        IReporter& operator=(const IReporter&) = default;
        IReporter(IReporter&&) = default;
        IReporter& operator=(IReporter&&) = default;
    };

    /**
     * \}
     */

    /**
     * \defgroup REPORTING_ADAPTERS test framework adapters
     * \ingroup REPORTING
     * \brief Reporter integrations for various third-party frameworks.
     * \details These reporters are specialized implementations, which provide seamless integrations of ``mimic++`` into the desired
     * unit-test framework. Integrations are enabled by simply including the specific header into any source file. The include order
     * doesn't matter.
     *
     * \note Including multiple headers of the ``adapters`` subdirectory into one executable is possible, but with caveats. It's unspecified
     * which reporter will be active at the program start. So, if you need multiple reporters in one executable, you should explicitly
     * install the desired reporter on a per test case basis.
     */
}

#endif
