//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_EXPECTATION_REPORT_HPP
#define MIMICPP_REPORTING_EXPECTATION_REPORT_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Utility.hpp"

#include <optional>
#include <variant>
#include <vector>

namespace mimicpp::reporting
{
    struct state_inapplicable
    {
        int min{};
        int max{};
        int count{};
        std::vector<sequence::rating> sequenceRatings{};
        std::vector<sequence::Tag> inapplicableSequences{};

        [[nodiscard]]
        friend bool operator==(const state_inapplicable&, const state_inapplicable&) = default;
    };

    struct state_applicable
    {
        int min{};
        int max{};
        int count{};
        std::vector<sequence::rating> sequenceRatings{};

        [[nodiscard]]
        friend bool operator==(const state_applicable&, const state_applicable&) = default;
    };

    struct state_saturated
    {
        int min{};
        int max{};
        int count{};
        std::vector<sequence::Tag> sequences{};

        [[nodiscard]]
        friend bool operator==(const state_saturated&, const state_saturated&) = default;
    };

    using control_state_t = std::variant<
        state_inapplicable,
        state_applicable,
        state_saturated>;

    /**
     * \brief Contains the extracted info from a typed expectation.
     * \details This type is meant to be used to communicate with independent domains via the reporter interface and thus contains
     * the generic information as plain ``std`` types.
     */
    class ExpectationReport
    {
    public:
        detail::expectation_info info{};
        control_state_t controlReport{};
        std::optional<StringT> finalizerDescription{};
        std::vector<std::optional<StringT>> requirementDescriptions{};

        [[nodiscard]]
        friend bool operator==(const ExpectationReport&, const ExpectationReport&) = default;
    };

    class RequirementOutcomes
    {
    public:
        std::vector<bool> outcomes{};

        [[nodiscard]]
        friend bool operator==(const RequirementOutcomes&, const RequirementOutcomes&) = default;
    };
}

#endif
