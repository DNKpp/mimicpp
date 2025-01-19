//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_ADAPTERS_BOOST_TEST_HPP
#define MIMICPP_ADAPTERS_BOOST_TEST_HPP

#pragma once

#if __has_include("mimic++/Reporter.hpp")
    #include "mimic++/Reporter.hpp"
#elif not defined(MIMICPP_VERSION)
    #error "It appears that the test-adapter is not included in the mimic++ project or package." \
        "If you plan to use it alongside the mimic++-amalgamated header, please ensure to include the adapter-header afterwards."
#endif

#if __has_include(<boost/test/unit_test.hpp>)
    #include <boost/test/unit_test.hpp>
#else
    #error "Unable to find Boost.Test includes."
#endif

namespace mimicpp::detail::boost_test
{
    struct failure
    {
    };

    [[noreturn]]
    inline void send_fail(const StringViewT msg)
    {
        BOOST_TEST_FAIL(msg);
        unreachable();
    }

    inline void send_success(const StringViewT msg)
    {
        BOOST_TEST_MESSAGE(msg);
    }

    inline void send_warning(const StringViewT msg)
    {
        BOOST_TEST_MESSAGE(
            format::format(
                "warning: {}",
                msg));
    }
}

namespace mimicpp
{
    // GCOVR_EXCL_START

    /**
     * \brief Reporter for the integration into ``Boost.Test``.
     * \ingroup REPORTING_ADAPTERS
     * \details This reporter enables the integration of ``mimic++`` into ``Boost.Test`` and prefixes the headers
     * of ``Boost.Test`` with ``boost/test``.
     *
     * This reporter installs itself by simply including this header file into any source file of the test executable.
     */
    using BoostTestReporterT = BasicReporter<
        &detail::boost_test::send_success,
        &detail::boost_test::send_warning,
        &detail::boost_test::send_fail>;

    // GCOVR_EXCL_STOP
}

namespace mimicpp::detail::boost_test
{
    [[maybe_unused]]
    inline const ReporterInstaller<BoostTestReporterT> installer{};
}

#endif
