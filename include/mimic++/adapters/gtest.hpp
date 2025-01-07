// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_ADAPTERS_GTEST_HPP
#define MIMICPP_ADAPTERS_GTEST_HPP

#pragma once

#include "mimic++/Reporter.hpp"

#if __has_include(<gtest/gtest.h>)
    #include <gtest/gtest.h>
#else
    #error "Unable to find gtest includes."
#endif

namespace mimicpp::detail::gtest
{
    struct failure
    {
    };

    [[noreturn]]
    inline void send_fail(const StringViewT& msg)
    {
        // GTEST_FAIL has an immediate return
        std::invoke(
            [&] {
                GTEST_FAIL() << msg;
            });

        throw failure{};
    }

    inline void send_success(const StringViewT& msg)
    {
        GTEST_SUCCEED() << msg;
    }

    inline void send_warning([[maybe_unused]] const StringViewT& msg)
    {
        // seems unsupported
    }
}

namespace mimicpp
{
    // GCOVR_EXCL_START

    /**
     * \brief Reporter for the integration into gtest.
     * \ingroup REPORTING_ADAPTERS
     * \details This reporter enables the integration of ``mimic++`` into ``gtest`` and prefixes the headers
     * of ``gtest`` with ``gtest/``.
     *
     * This reporter installs itself by simply including this header file into any source file of the test executable.
     */
    using GTestReporterT = BasicReporter<
        &detail::gtest::send_success,
        &detail::gtest::send_warning,
        &detail::gtest::send_fail>;

    // GCOVR_EXCL_STOP
}

namespace mimicpp::detail::gtest
{
    [[maybe_unused]]
    inline const ReporterInstaller<GTestReporterT> installer{};
}

#endif
