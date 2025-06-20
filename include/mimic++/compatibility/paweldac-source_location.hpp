//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_COMPATIBILITY_PAWELDAC_SOURCE_LOCATION_HPP
#define MIMICPP_COMPATIBILITY_PAWELDAC_SOURCE_LOCATION_HPP

#pragma once

#include "mimic++/Fwd.hpp"

#if __has_include(<source_location/source_location.hpp>)
    #include <source_location/source_location.hpp>
#else
    #error "Unable to find source_location/source_location.hpp of github.com/paweldac/source_location."
#endif

#define MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND nostd::source_location

namespace mimicpp::util::source_location
{
    template <>
    struct backend_traits<nostd::source_location>
    {
        [[nodiscard]]
        static constexpr nostd::source_location current(nostd::source_location const loc = nostd::source_location::current()) noexcept
        {
            return loc;
        }

        [[nodiscard]]
        static constexpr std::string_view file_name(nostd::source_location const& loc) noexcept
        {
            return loc.file_name();
        }

        [[nodiscard]]
        static constexpr std::string_view function_name(nostd::source_location const& loc) noexcept
        {
            return loc.function_name();
        }

        [[nodiscard]]
        static constexpr std::size_t line(nostd::source_location const& loc) noexcept
        {
            return std::size_t{loc.line()};
        }
    };

    using InstalledBackend = nostd::source_location;
}

#include "mimic++/utilities/SourceLocation.hpp"

static_assert(
    mimicpp::util::source_location::backend<nostd::source_location>,
    "nostd::source_location isn't suitable as a source-location backend.");

#endif
