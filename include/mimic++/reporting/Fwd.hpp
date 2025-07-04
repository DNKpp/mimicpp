//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_FWD_HPP
#define MIMICPP_REPORTING_FWD_HPP

#pragma once

#include "mimic++/config/Config.hpp"

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::reporting
{
    class IReporter;
    class DefaultReporter;

    class TypeReport;
    class TargetReport;
    class CallReport;
    class ExpectationReport;
    class NoMatchReport;
    class RequirementOutcomes;
}

#endif
