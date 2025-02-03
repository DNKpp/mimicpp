//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_SOURCE_LOCATION_HPP
#define MIMICPP_UTILITIES_SOURCE_LOCATION_HPP

#include <cstring>
#include <source_location>
#include <utility>

namespace mimicpp::util
{
    /**
     * \brief A thin wrapper around a ``std::source_location`` with additional ``operator ==``.
     */
    class SourceLocation
    {
    public:
        [[nodiscard]]
        explicit(false) constexpr SourceLocation(std::source_location loc = std::source_location::current()) noexcept
            : m_SourceLocation{std::move(loc)}
        {
        }

        [[nodiscard]]
        constexpr std::source_location const* operator->() const noexcept
        {
            return &m_SourceLocation;
        }

        [[nodiscard]]
        constexpr std::source_location const& operator*() const noexcept
        {
            return m_SourceLocation;
        }

        [[nodiscard]]
        friend bool operator==(SourceLocation const& lhs, SourceLocation const& rhs) noexcept
        {
            auto const& lhsLoc = lhs.m_SourceLocation;
            auto const& rhsLoc = rhs.m_SourceLocation;

            return 0 == std::strcmp(lhsLoc.file_name(), rhsLoc.file_name())
                && 0 == std::strcmp(lhsLoc.function_name(), rhsLoc.function_name())
                && lhsLoc.line() == rhsLoc.line()
                && lhsLoc.column() == rhsLoc.column();
        }

    private:
        std::source_location m_SourceLocation;
    };
}

#endif
