//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_REPORTING_TYPE_REPORT_HPP
#define MIMICPP_REPORTING_TYPE_REPORT_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/TypePrinter.hpp"

#include <functional>

namespace mimicpp::reporting
{
    /**
     * \brief Contains information about a specific (potentially cv-ref-qualified) type.
     */
    class TypeReport
    {
    private:
        using make_name_fn = StringT (*)();

        template <typename T>
        [[nodiscard]]
        static constexpr StringT make_type_name()
        {
            return mimicpp::print_type<T>();
        }

    public:
        template <typename T>
        [[nodiscard]]
        static constexpr TypeReport make() noexcept
        {
            return TypeReport{&TypeReport::make_type_name<T>};
        }

        [[nodiscard]]
        constexpr StringT name() const
        {
            return std::invoke(m_MakeNameFn);
        }

        [[nodiscard]]
        friend bool operator==(TypeReport const&, TypeReport const&) = default;

    private:
        make_name_fn m_MakeNameFn;

        explicit constexpr TypeReport(make_name_fn const makeFn) noexcept
            : m_MakeNameFn{makeFn}
        {
            MIMICPP_ASSERT(m_MakeNameFn, "Null make-function is not allowed.");
        }
    };
}

#endif
