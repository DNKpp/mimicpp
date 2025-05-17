//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CONFIG_SETTINGS_HPP
#define MIMICPP_CONFIG_SETTINGS_HPP

#include "mimic++/config/Config.hpp"

#include <atomic>

namespace mimicpp::settings
{
    /**
     * \defgroup SETTINGS settings
     * \brief Contains global settings, which can be changed at runtime.
     * \{
     */

    /**
     * \brief Controls whether successful matches are reported.
     * \details Reporting can be an expensive operation, particularly when stacktraces are collected.
     * To reduce overhead, `mimic++` reports only violations by default.
     * \note This setting affects only the behavior of mimic++.
     * When using a test adapter, additional configuration in the test framework may be required to receive such reports.
     */
    inline std::atomic_bool reportSuccess{false};

    /**
     * \}
     */
}

#endif
