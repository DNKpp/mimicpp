//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_SOURCE_LOCATION_HPP
#define MIMICPP_UTILITIES_SOURCE_LOCATION_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/PathPrinter.hpp"
#include "mimic++/printing/type/PrintType.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <cstddef>
    #include <string_view>
    #include <utility>

    #ifdef __cpp_lib_source_location
        #include <source_location>
    #endif
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \brief A thin wrapper around general source-location info.
     * \ingroup UTILITIES
     * \details By default, *mimic++* uses the C++20 `std::source_location` feature.
     * However, this feature is not available on all platforms or with all compilers,
     * for which it then automatically falls back to specific compiler intrinsics.
     */
    class SourceLocation
    {
    public:
        ~SourceLocation() = default;

#ifdef __cpp_lib_source_location
        /**
         * \brief Default constructor, deducing the source-location info via `std::source_location`.
         * \param canary Parameter-pack, preventing users modifying the relevant default argument.
         * \param loc The deduced source-location.
         */
        [[nodiscard]] //
        explicit(false) constexpr SourceLocation(
            [[maybe_unused]] auto&&... canary,
            std::source_location const& loc = std::source_location::current()) noexcept
            : m_FileName{loc.file_name()},
              m_FunctionName{loc.function_name()},
              m_Line{loc.line()}
        {
            static_assert(0 == sizeof...(canary), "Do not supply custom arguments to util::SourceLocation.");
        }
#else
        /**
         * \brief Compatibility constructor, used in cases where `std::source_location` is not available.
         * \param canary Parameter-pack, preventing users modifying the relevant default arguments.
         * \param fileName The deduced file name.
         * \param functionName The deduced function name.
         * \param line The deduced line number.
         */
        [[nodiscard]] //
        explicit(false) constexpr SourceLocation(
            [[maybe_unused]] auto&&... canary,
            std::string_view const fileName = __builtin_FILE(),
            std::string_view const functionName = __builtin_FUNCTION(),
            std::size_t const line = __builtin_LINE()) noexcept
            : m_FileName{fileName},
              m_FunctionName{functionName},
              m_Line{line}
        {
            static_assert(0 == sizeof...(canary), "Do not supply custom arguments to util::SourceLocation.");
        }
#endif

        SourceLocation(SourceLocation const&) = default;
        SourceLocation& operator=(SourceLocation const&) = default;
        SourceLocation(SourceLocation&&) = default;
        SourceLocation& operator=(SourceLocation&&) = default;

        [[nodiscard]]
        constexpr std::string_view file_name() const noexcept
        {
            return m_FileName;
        }

        [[nodiscard]]
        constexpr std::string_view function_name() const noexcept
        {
            return m_FunctionName;
        }

        [[nodiscard]]
        constexpr std::size_t line() const noexcept
        {
            return m_Line;
        }

        [[nodiscard]]
        friend constexpr bool operator==(SourceLocation const& lhs, SourceLocation const& rhs) noexcept
        {
            return lhs.line() == rhs.line()
                && lhs.file_name() == rhs.file_name()
                && lhs.function_name() == rhs.function_name();
        }

    private:
        std::string_view m_FileName;
        std::string_view m_FunctionName;
        std::size_t m_Line;
    };
}

template <>
struct mimicpp::printing::detail::state::common_type_printer<mimicpp::util::SourceLocation>
{
    template <print_iterator OutIter>
    static constexpr OutIter print(OutIter out, util::SourceLocation const& loc)
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
