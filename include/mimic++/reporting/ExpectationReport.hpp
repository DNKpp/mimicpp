//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_EXPECTATION_REPORT_HPP
#define MIMICPP_REPORTING_EXPECTATION_REPORT_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/reporting/SequenceReport.hpp"
#include "mimic++/reporting/TargetReport.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <optional>
    #include <variant>
    #include <vector>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::reporting
{
    /**
     * \brief Denotes an inapplicable expectation state.
     * \ingroup REPORTING_REPORTS
     */
    struct state_inapplicable
    {
        int min{};
        int max{};
        int count{};
        std::vector<sequence::rating> sequences{};
        std::vector<SequenceReport> inapplicableSequences{};

        [[nodiscard]]
        friend bool operator==(state_inapplicable const&, state_inapplicable const&) = default;
    };

    /**
     * \brief Denotes an applicable expectation state.
     * \ingroup REPORTING_REPORTS
     */
    struct state_applicable
    {
        int min{};
        int max{};
        int count{};
        std::vector<sequence::rating> sequenceRatings{};

        [[nodiscard]]
        friend bool operator==(state_applicable const&, state_applicable const&) = default;
    };

    /**
     * \brief Denotes a saturated expectation state.
     * \ingroup REPORTING_REPORTS
     */
    struct state_saturated
    {
        int min{};
        int max{};
        int count{};
        std::vector<SequenceReport> sequences{};

        [[nodiscard]]
        friend bool operator==(state_saturated const&, state_saturated const&) = default;
    };

    /**
     * \brief Denotes an expectation state.
     * \ingroup REPORTING_REPORTS
     */
    using control_state_t = std::variant<
        state_inapplicable,
        state_applicable,
        state_saturated>;

    /**
     * \brief Contains the extracted info from a typed expectation.
     * \ingroup REPORTING_REPORTS
     */
    class ExpectationReport
    {
    public:
        util::SourceLocation from{};
        TargetReport target;
        control_state_t controlReport{};
        std::optional<StringT> finalizerDescription{};
        std::vector<std::optional<StringT>> requirementDescriptions{};

        [[nodiscard]]
        friend bool operator==(ExpectationReport const&, ExpectationReport const&) = default;
    };

    /**
     * \brief Contains the boolean outcomes of a match-test.
     * \ingroup REPORTING_REPORTS
     */
    class RequirementOutcomes
    {
    public:
        std::vector<bool> outcomes{};

        [[nodiscard]]
        friend bool operator==(RequirementOutcomes const&, RequirementOutcomes const&) = default;
    };
}

#endif
