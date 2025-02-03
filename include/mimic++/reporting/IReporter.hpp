//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_IREPORTER_HPP
#define MIMICPP_REPORTING_IREPORTER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/reporting/Fwd.hpp"

#include <exception>
#include <vector>

namespace mimicpp::reporting
{
    /**
     * \defgroup REPORTING reporting
     * \brief Contains reporting related symbols
     * \details Reporting in ``mimic++`` is triggered when notable events are detected.
     * The reporter is expected to respond to these reports in specific ways, often by aborting the test case.
     * For instance, the DefaultReporter throws exceptions on error reports, while more specialized reporters may handle such cases differently,
     * though still typically aborting the current test.
     * These specialized reporters are designed to send reports to specific destinations (e.g., the unit-test framework in use),
     * often providing more advanced mechanisms for displaying failed tests to users.
     *
     * Users can implement their own reporters, which is particularly useful when there's no existing reporter for their preferred unit-test framework.
     *
     * At any given time, there is exactly one active global reporter.
     * Users can exchange this reporter either directly or indirectly (i.e. by including a certain adapter-header).
     * Reports are sent to the currently installed reporter via the ``report_xyz`` free-functions.
     * Most of these functions require reports to be handled in specific ways (e.g., ``report_no_matches`` is expected to never return).
     * Custom reporters must adhere to these specifications to avoid undefined behavior.
     * For more detailed information, please refer to the documentation of each specific function.
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
         * \brief Expects reports on all non-matching expectations. This is only called when no better options are available.
         * \param call The call report.
         * \param noMatchReports Reports of all ``none`` matching expectations.
         * \details This function is called when no match has been found and there are no other expectations that are matching but inapplicable.
         * It serves as the fallback reporting mechanism for unmatched calls.
         * \note ``noMatchReports`` may be empty.
         *
         * \attention Derived reporter implementations must never return normally and should instead exit the function
         * either by throwing an exception or using a terminating mechanism (e.g., ``std::abort``).
         * Failing to do so will result in undefined behavior.
         */
        [[noreturn]]
        virtual void report_no_matches(
            CallReport call,
            std::vector<NoMatchReport> noMatchReports) = 0;

        /**
         * \brief Handles reports for all *inapplicable* but otherwise matching expectations. This function is called only when no better options are available.
         * \param call The call report.
         * \param expectationReports Reports of all *inapplicable* expectations.
         * \details This function is called when no applicable match has been found, but the call expectations are actually fulfilled.
         * This occurs when the "times" policy is already saturated (e.g., it was expected once and has already been matched once)
         * or is otherwise not applicable (e.g., a sequence element is not the current element).
         *
         * \attention Derived reporter implementations must never return normally and should instead exit the function
         * either by throwing an exception or using a terminating mechanism (e.g., ``std::abort``).
         * Failing to do so will result in undefined behavior.
         */
        [[noreturn]]
        virtual void report_inapplicable_matches(
            CallReport call,
            std::vector<ExpectationReport> expectationReports) = 0;

        /**
         * \brief Handles the report for a fully matching expectation.
         * \param call The call report.
         * \param expectationReport Report of the fully matched expectation.
         * \details This function is called when a match has been found.
         * There are no other expectations for the behavior of this function, except for the ``noexcept`` guarantee.
         * Implementations should simply return to the caller.
         */
        virtual void report_full_match(
            CallReport call,
            ExpectationReport expectationReport) noexcept = 0;

        /**
         * \brief Handles the report of an unfulfilled expectation.
         * \param expectationReport The expectation report.
         * \details This function is called when an unfulfilled expectation goes out of scope.
         * This occurs when the "times" policy is not satisfied.
         *
         * \note In general, this function is expected to not return but throw an exception instead.
         * However, since it is always called when an unfulfilled expectation goes out of scope,
         * implementations shall check whether an uncaught exception already exists (e.g., via ``std::uncaught_exceptions``)
         * before throwing their own exception.
         * \see ``DefaultReporter::report_unfulfilled_expectation`` for an example.
         */
        virtual void report_unfulfilled_expectation(
            ExpectationReport expectationReport) = 0;

        /**
         * \brief Handles general or unspecified errors.
         * \param message The error message.
         * \details This function is called when a non-specific error occurs.
         *
         * \note In general, this function is expected to throw an exception rather than return.
         * However, as it may be called for various reasons, implementations should check for existing uncaught exceptions before throwing their own.
         * This can be done using ``std::uncaught_exceptions()``.
         * \see ``DefaultReporter::report_error`` for an example.
         */
        virtual void report_error(StringT message) = 0;

        /**
         * \brief Handles reports about unhandled exceptions during ``handle_call``.
         * \param call The call report.
         * \param expectationReport The expectation report.
         * \param exception The exception.
         * \details This function is called when an expectation throws an exception during a ``matches`` call.
         * There are no specific requirements for the behavior of this function.
         * Since this function is called inside a catch-block, throwing exceptions will result in a terminate call.
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
     * \brief Reporter integrations for various third-party testing frameworks.
     * \details These reporters are specialized implementations that provide seamless integrations of ``mimic++`` into the desired unit-test framework.
     * Integrations are enabled by simply including the specific header into any source file.
     * The include order does not matter.
     *
     * \note Including multiple headers from the adapters subdirectory into one executable is possible, but with caveats.
     * It's unspecified which reporter will be active at program start.
     * So, if you need multiple reporters in one executable, you should explicitly install the desired reporter on a per-test case basis.
     */
}

#endif
