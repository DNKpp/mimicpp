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

#ifndef __cpp_lib_source_location
    #error "The std::source_location feature is unknown. Please setup a custom source-location backend."
#endif

#include <source_location>

    template <>
    struct backend_traits<std::source_location>
    {
        [[nodiscard]]
        static constexpr std::source_location current(std::source_location const loc = std::source_location::current()) noexcept
        {
            return loc;
        }

        [[nodiscard]]
        static constexpr std::string_view file_name(std::source_location const& loc) noexcept
        {
            return loc.file_name();
        }

        [[nodiscard]]
        static constexpr std::string_view function_name(std::source_location const& loc) noexcept
        {
            return loc.function_name();
        }

        [[nodiscard]]
        static constexpr std::size_t line(std::source_location const& loc) noexcept
        {
            return std::size_t{loc.line()};
        }

        [[nodiscard]]
        static constexpr std::size_t column(std::source_location const& loc) noexcept
        {
            return std::size_t{loc.column()};
        }
    };

    static_assert(backend<std::source_location>, "std::source_location isn't suitable as a source-location backend. Blame the mimic++ maintainer!");

    using InstalledBackend = std::source_location;
}

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
        constexpr std::string_view file_name() const noexcept
        {
            return m_SourceLocation.file_name();
        }

        [[nodiscard]]
        constexpr std::string_view function_name() const noexcept
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
            return lhs.line() == rhs.line()
                && lhs.column() == rhs.column()
                && lhs.file_name() == rhs.file_name()
                && lhs.function_name() == rhs.function_name();
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
        out = type::prettify_function(std::move(out), StringT{loc.function_name()});
        out = format::format_to(std::move(out), "`");

        return out;
    }
};

#endif
