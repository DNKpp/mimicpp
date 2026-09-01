//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_MATCH_REPORT_HPP
#define MIMICPP_REPORTING_MATCH_REPORT_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/expectation/Common.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::reporting
{
    /**
     * \brief Contains the expectation-report and the match-results to a specific call.
     * \ingroup REPORTING_REPORTS
     */
    class MatchReport
    {
    public:
        ExpectationReport expectationReport;
        expectation::MatchResults matchResults;

        [[nodiscard]]
        friend bool operator==(MatchReport const&, MatchReport const&) = default;
    };
}

#endif
