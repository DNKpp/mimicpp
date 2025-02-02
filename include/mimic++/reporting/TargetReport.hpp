//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_TARGET_REPORT_HPP
#define MIMICPP_REPORTING_TARGET_REPORT_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/reporting/TypeReport.hpp"

namespace mimicpp::reporting
{
    /**
     * \brief Contains the extracted mock info.
     * \ingroup REPORTING_REPORTS
     */
    class TargetReport
    {
    public:
        StringT name;
        TypeReport overloadReport;

        [[nodiscard]]
        friend bool operator==(const TargetReport&, const TargetReport&) = default;
    };
}

#endif
