//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_FORMAT_HPP
#define MIMICPP_PRINTING_FORMAT_HPP

#pragma once

#include "mimic++/Config.hpp"
#include "mimic++/Fwd.hpp"

#include <concepts>
#include <sstream>
#include <utility>
#include <type_traits>

#ifndef MIMICPP_CONFIG_USE_FMT
    #include <format>
#else

    #if __has_include(<fmt/format.h>)
        #include <fmt/format.h>
    #else
        #error "The fmt formatting backend is explicitly enabled, but the include <fmt/format.h> can not be found."
    #endif

#endif

namespace mimicpp
{
    using StringStreamT = std::basic_ostringstream<CharT, CharTraitsT>;

    template <typename T>
    concept print_iterator = std::output_iterator<T, const CharT&>;

    template <typename Printer, typename OutIter, typename T>
    concept printer_for = print_iterator<OutIter>
                       && requires(OutIter out) {
        {
            Printer::print(out, std::declval<T&&>())
        } -> std::convertible_to<OutIter>;
                       };
}

namespace mimicpp::format
{
#ifndef MIMICPP_CONFIG_USE_FMT

        // use std format
    #if not MIMICPP_DETAIL_USES_LIBCXX

    using std::format;
    using std::format_to;
    using std::formatter;
    using std::make_format_args;
    using std::vformat;
    using std::vformat_to;

    #else

    // libc++ has some serious trouble when using its std::format implementation.
    // Let's simply redirect any calls to std::vformat instead.

    using std::formatter;
    using std::make_format_args;
    using std::vformat;
    using std::vformat_to;

    template <typename... Args>
    [[nodiscard]]
    StringT format(const StringViewT fmt, Args&&... args) // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        return format::vformat(
            fmt,
            std::make_format_args(args...));
    }

    template <class OutputIt, typename... Args>
    OutputIt format_to(const OutputIt out, const StringViewT fmt, Args&&... args) // NOLINT(cppcoreguidelines-missing-std-forward)
    {
        return format::vformat_to(
            std::move(out),
            fmt,
            std::make_format_args(args...));
    }

    #endif

    namespace detail
    {
        template <typename Char>
        struct format_context;

        template <typename Char>
        using format_context_t = typename format_context<Char>::type;

        template <>
        struct format_context<char>
        {
            using type = std::format_context;
        };

        template <>
        struct format_context<wchar_t>
        {
            using type = std::wformat_context;
        };

        /**
         * \brief Determines, whether a complete specialization of ``std::formatter`` for the given (possibly cv-ref qualified) type exists.
         * \tparam T Type to check.
         * \tparam Char Used character type.
         * \details This is an adapted implementation of the ``std::formattable`` concept, which is added c++23.
         * \note This implementation takes a simple but reasonable shortcut in assuming, that ```Char`` is either ``char`` or ``wchar_t``,
         * which must not necessarily true.
         * \see Adapted from here: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2286r8.html#concept-formattable
         * \see https://en.cppreference.com/w/cpp/utility/format/formattable
         */
        template <class T, class Char>
        concept formattable =
            std::semiregular<std::formatter<std::remove_cvref_t<T>, Char>>
            && requires(
                std::formatter<std::remove_cvref_t<T>, Char> formatter,
                T t,
                format_context_t<Char> formatContext,
                std::basic_format_parse_context<Char> parseContext) {
                   { formatter.parse(parseContext) } -> std::same_as<typename std::basic_format_parse_context<Char>::iterator>;
                   {
                       std::as_const(formatter).format(t, formatContext)
                   } -> std::same_as<typename std::remove_reference_t<decltype(formatContext)>::iterator>;
               };
    }

        // use fmt format
#else

    using fmt::format;
    using fmt::format_to;
    using fmt::formatter;
    using fmt::make_format_args;
    using fmt::vformat;
    using fmt::vformat_to;

    namespace detail
    {
        template <class T, class Char>
        concept formattable = fmt::is_formattable<std::remove_reference_t<T>, Char>::value;
    }

#endif
}

#endif
