//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_SEQUENCE_REPORT_HPP
#define MIMICPP_REPORTING_SEQUENCE_REPORT_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::reporting
{
    /**
     * \addtogroup REPORTING_REPORTS
     *
     * \{
     */

    class SequenceReport
    {
    public:
        sequence::Tag tag{};
        util::SourceLocation from{};

        [[nodiscard]]
        friend bool operator==(SequenceReport const&, SequenceReport const&) = default;
    };

    template <typename Id, auto priorityStrategy>
    [[nodiscard]]
    constexpr SequenceReport make_sequence_report(sequence::detail::BasicSequence<Id, priorityStrategy> const& seq)
    {
        return SequenceReport{seq.tag(), seq.from()};
    }

    template <typename Id, auto priorityStrategy>
    [[nodiscard]]
    constexpr SequenceReport make_sequence_report(sequence::detail::BasicSequenceInterface<Id, priorityStrategy> const& seq)
    {
        return SequenceReport{seq.tag(), seq.from()};
    }

    /**
     * \}
     */
}

#endif
