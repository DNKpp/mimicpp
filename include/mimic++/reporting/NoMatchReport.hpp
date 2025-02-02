//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_NO_MATCH_REPORT_HPP
#define MIMICPP_REPORTING_NO_MATCH_REPORT_HPP

#pragma once

#include "mimic++/reporting/ExpectationReport.hpp"

namespace mimicpp::reporting
{
    /**
     * \brief Contains the requirement-outcomes (where at least one is negative) and the related expectation-report.
     * \ingroup REPORTING_REPORTS
     */
    class NoMatchReport
    {
    public:
        ExpectationReport expectationReport;
        RequirementOutcomes requirementOutcomes;

        [[nodiscard]]
        friend bool operator==(NoMatchReport const&, NoMatchReport const&) = default;
    };
}

#endif
