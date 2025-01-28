//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_ADAPTERS_DOCTEST_HPP
#define MIMICPP_ADAPTERS_DOCTEST_HPP

#pragma once

#if __has_include("mimic++/Reporting.hpp")
    #include "mimic++/Reporting.hpp"
    #include "mimic++/Utility.hpp"
#elif not defined(MIMICPP_VERSION)
    #error "It appears that the test-adapter is not included in the mimic++ project or package." \
        "If you plan to use it alongside the mimic++-amalgamated header, please ensure to include the adapter-header afterwards."
#endif

#if __has_include(<doctest/doctest.h>)
    #include <doctest/doctest.h>
#else
    #error "Unable to find Doctest includes."
#endif

namespace mimicpp::reporting::detail::doctest
{
    using namespace ::doctest;

    [[noreturn]]
    inline void send_fail(const StringViewT msg)
    {
        DOCTEST_FAIL(msg);
        unreachable();
    }

    inline void send_success(const StringViewT msg)
    {
        DOCTEST_REQUIRE_MESSAGE(true, msg);
    }

    inline void send_warning(const StringViewT msg)
    {
        DOCTEST_MESSAGE(msg);
    }
}

namespace mimicpp::reporting
{
    // GCOVR_EXCL_START

    /**
     * \brief Reporter for the integration into Doctest.
     * \ingroup REPORTING_ADAPTERS
     * \details This reporter enables the integration of ``mimic++`` into ``Doctest`` and prefixes the headers
     * of ``Doctest`` with ``doctest/``.
     *
     * This reporter installs itself by simply including this header file into any source file of the test executable.
     */
    using DoctestReporterT = BasicReporter<
        &detail::doctest::send_success,
        &detail::doctest::send_warning,
        &detail::doctest::send_fail>;

    // GCOVR_EXCL_STOP
}

namespace mimicpp::reporting::detail::doctest
{
    [[maybe_unused]]
    inline const ReporterInstaller<DoctestReporterT> installer{};
}

#endif
