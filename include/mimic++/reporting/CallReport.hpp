//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_CALL_REPORT_HPP
#define MIMICPP_REPORTING_CALL_REPORT_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/Stacktrace.hpp"
#include "mimic++/printing/StatePrinter.hpp"
#include "mimic++/reporting/TargetReport.hpp"
#include "mimic++/reporting/TypeReport.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

#include <tuple>
#include <utility>
#include <vector>

namespace mimicpp::reporting
{
    /**
     * \defgroup REPORTING_REPORTS reports
     * \ingroup REPORTING
     * \brief Contains reports of ``mimic++`` types.
     * \details Reports are simplified object representations of ``mimic++`` types. In fact, reports are used to communicate with
     * independent domains (e.g. unit-test frameworks) over the ``IReporter`` interface and are thus designed to provide as much
     * transparent information as possible, without requiring them to be a generic type.
     *
     * \{
     */

    /**
     * \brief Contains the extracted info from a typed ``call::Info``.
     */
    class CallReport
    {
    public:
        class Arg
        {
        public:
            TypeReport typeInfo;
            StringT stateString;

            [[nodiscard]]
            friend bool operator==(const Arg&, const Arg&) = default;
        };

        TargetReport target;
        TypeReport returnTypeInfo;
        std::vector<Arg> argDetails{};
        util::SourceLocation fromLoc{};
        Stacktrace stacktrace{stacktrace::NullBackend{}};
        ValueCategory fromCategory{};
        Constness fromConstness{};

        [[nodiscard]]
        friend bool operator==(CallReport const&, CallReport const&) = default;
    };

    /**
     * \brief Generates the call report for a given call info.
     * \tparam Return The function return type.
     * \tparam Params The function parameter types.
     * \param target The mock-target report.
     * \param callInfo The call info.
     * \param stacktrace The stacktrace, from where the call originates.
     * \return The call report.
     * \relatesalso CallReport
     */
    template <typename Return, typename... Params>
    [[nodiscard]]
    CallReport make_call_report(TargetReport target, call::Info<Return, Params...> callInfo, Stacktrace stacktrace)
    {
        return CallReport{
            .target{std::move(target)},
            .returnTypeInfo{TypeReport::make<Return>()},
            .argDetails = std::apply(
                [](auto&... args) {
                    return std::vector<CallReport::Arg>{
                        CallReport::Arg{
                                        .typeInfo = TypeReport::make<Params>(),
                                        .stateString = mimicpp::print(args.get())}
                        ...
                    };
                },
                callInfo.args),
            .fromLoc{std::move(callInfo.fromSourceLocation)},
            .stacktrace{std::move(stacktrace)},
            .fromCategory{callInfo.fromCategory},
            .fromConstness{callInfo.fromConstness}};
    }

    /**
     * \}
     */
}

#endif
