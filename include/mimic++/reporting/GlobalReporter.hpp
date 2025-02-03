//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_GLOBAL_REPORTER_HPP
#define MIMICPP_REPORTING_GLOBAL_REPORTER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/reporting/CallReport.hpp"
#include "mimic++/reporting/DefaultReporter.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/IReporter.hpp"
#include "mimic++/utilities/C++23Backports.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace mimicpp::reporting::detail
{
#ifdef __cpp_lib_atomic_shared_ptr

    [[nodiscard]]
    inline std::atomic<std::shared_ptr<IReporter>>& reporter() noexcept
    {
        static std::atomic<std::shared_ptr<IReporter>> reporter{
            std::make_shared<DefaultReporter>(std::cerr)};
        return reporter;
    }

    inline void set_reporter(std::shared_ptr<IReporter> newReporter) noexcept
    {
        reporter().store(std::move(newReporter));
    }

    [[nodiscard]]
    inline std::shared_ptr<IReporter> get_reporter() noexcept
    {
        return reporter().load();
    }

#else // libc++ doesn't support std::atomic<std::shared_ptr> yet, so fallback to older free-functions approach instead

    [[nodiscard]]
    inline std::shared_ptr<IReporter>* reporter() noexcept
    {
        static std::shared_ptr<IReporter> reporter{
            std::make_shared<DefaultReporter>(std::cerr)};
        return std::addressof(reporter);
    }

    inline void set_reporter(std::shared_ptr<IReporter> newReporter)
    {
        std::atomic_store(reporter(), std::move(newReporter));
    }

    [[nodiscard]]
    inline std::shared_ptr<IReporter> get_reporter() noexcept
    {
        return std::atomic_load(reporter());
    }

#endif

    [[noreturn]]
    inline void report_no_matches(
        CallReport callReport,
        std::vector<NoMatchReport> noMatchReports)
    {
        get_reporter()
            // GCOVR_EXCL_START
            ->report_no_matches(
                // GCOVR_EXCL_STOP
                std::move(callReport),
                std::move(noMatchReports));

        // GCOVR_EXCL_START
        // ReSharper disable once CppDFAUnreachableCode
        util::unreachable();
        // GCOVR_EXCL_STOP
    }

    [[noreturn]]
    inline void report_inapplicable_matches(
        CallReport callReport,
        std::vector<ExpectationReport> expectationReports)
    {
        get_reporter()
            // GCOVR_EXCL_START
            ->report_inapplicable_matches(
                // GCOVR_EXCL_STOP
                std::move(callReport),
                std::move(expectationReports));

        // GCOVR_EXCL_START
        // ReSharper disable once CppDFAUnreachableCode
        util::unreachable();
        // GCOVR_EXCL_STOP
    }

    inline void report_full_match(
        CallReport callReport,
        ExpectationReport expectationReport) noexcept
    {
        get_reporter()
            ->report_full_match(
                std::move(callReport),
                std::move(expectationReport));
    }

    inline void report_unfulfilled_expectation(
        ExpectationReport expectationReport)
    {
        get_reporter()
            ->report_unfulfilled_expectation(std::move(expectationReport));
    }

    inline void report_error(StringT message)
    {
        get_reporter()
            ->report_error(std::move(message));
    }

    inline void report_unhandled_exception(
        CallReport callReport,
        ExpectationReport expectationReport,
        std::exception_ptr const& exception)
    {
        get_reporter()
            ->report_unhandled_exception(
                std::move(callReport),
                std::move(expectationReport),
                exception);
    }
}

namespace mimicpp::reporting
{
    /**
     * \brief Replaces the previous reporter with a newly constructed one.
     * \tparam T The desired reporter type.
     * \tparam Args The constructor argument types for ``T``.
     * \param args The constructor arguments.
     * \ingroup REPORTING
     * \details This function accesses the globally available reporter and replaces it with a new instance.
     */
    template <std::derived_from<IReporter> T, typename... Args>
        requires std::constructible_from<T, Args...>
    void install_reporter(Args&&... args)
    {
        detail::set_reporter(
            std::make_shared<T>(std::forward<Args>(args)...));
    }

    namespace detail
    {
        template <typename T>
        class ReporterInstaller
        {
        public:
            template <typename... Args>
            explicit ReporterInstaller(Args&&... args)
            {
                install_reporter<T>(
                    std::forward<Args>(args)...);
            }
        };
    }
}

#endif
