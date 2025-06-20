//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CONFIG_SETTINGS_HPP
#define MIMICPP_CONFIG_SETTINGS_HPP

#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <atomic>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::settings
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
     * \returns a mutable reference to the actual settings value.
     */
    [[nodiscard]]
    inline std::atomic_bool& report_success() noexcept
    {
        static std::atomic_bool value{false};

        return value;
    }

    /**
     * \}
     */
}

#endif
