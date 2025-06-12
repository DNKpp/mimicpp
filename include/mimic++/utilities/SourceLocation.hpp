//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_SOURCE_LOCATION_HPP
#define MIMICPP_UTILITIES_SOURCE_LOCATION_HPP

#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/state/Print.hpp"

#include <cstring>
#include <source_location>
#include <utility>

namespace mimicpp::util::source_location
{
    template <typename Backend>
    struct backend_traits;

    template <typename T>
    concept backend =
        std::copyable<T>
        && requires(backend_traits<std::remove_cvref_t<T>> traits, std::remove_cvref_t<T> const& backend) {
               { decltype(traits)::current() } noexcept -> std::convertible_to<std::remove_cvref_t<T>>;
               { decltype(traits)::file_name(backend) } -> std::convertible_to<std::string_view>;
               { decltype(traits)::function_name(backend) } -> std::convertible_to<std::string_view>;
               { decltype(traits)::line(backend) } -> std::convertible_to<std::size_t>;
               { decltype(traits)::column(backend) } -> std::convertible_to<std::size_t>;
           };


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
        constexpr char const* file_name() const noexcept
        {
            return m_SourceLocation.file_name();
        }

        [[nodiscard]]
        constexpr char const* function_name() const noexcept
        {
            return m_SourceLocation.function_name();
        }

        [[nodiscard]]
        constexpr std::size_t line() const noexcept
        {
            return std::size_t{m_SourceLocation.line()};
        }

        [[nodiscard]]
        constexpr std::size_t column() const noexcept
        {
            return std::size_t{m_SourceLocation.column()};
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

template <>
struct mimicpp::printing::detail::state::common_type_printer<mimicpp::util::SourceLocation>
{
    template <print_iterator OutIter>
    static OutIter print(OutIter out, util::SourceLocation const& loc)
    {
        out = format::format_to(std::move(out), "`");
        out = print_path(std::move(out), loc.file_name());
        out = format::format_to(std::move(out), "`");

        out = format::format_to(
            std::move(out),
            "#L{}, `",
            loc.line());
        out = type::prettify_function(std::move(out), loc.function_name());
        out = format::format_to(std::move(out), "`");

        return out;
    }
};

#endif
