//          Copyright Dominic (DNKpp) Koepke 2024 - 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_FORMAT_HPP
#define MIMICPP_PRINTING_FORMAT_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Fwd.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <functional>
    #include <sstream>
    #include <type_traits>
    #include <utility>

    #ifndef MIMICPP_CONFIG_USE_FMT
        #include <format>
    #else

        #if __has_include(<fmt/format.h>)
            #include <fmt/format.h>
        #else
            #error "The fmt formatting backend is explicitly enabled, but the include <fmt/format.h> can not be found."
        #endif

    #endif
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp
{
    using StringStreamT = std::basic_ostringstream<CharT, CharTraitsT>;

    template <typename T>
    concept print_iterator = std::output_iterator<T, const CharT&>;

    template <typename Printer, typename OutIter, typename T>
    concept printer_for = print_iterator<OutIter>
                       && requires(OutIter out) {
                              {
                                  Printer::print(out, std::declval<T&>())
                              } -> std::convertible_to<OutIter>;
                          };
}

#ifndef MIMICPP_CONFIG_USE_FMT

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::format::detail::fmt
{
    template <typename... Args>
    using format_string = std::basic_format_string<CharT, std::type_identity_t<Args>...>;
    using std::format_args;
    using std::formatter;
    using std::make_format_args;
    using std::vformat;
    using std::vformat_to;
}

namespace mimicpp::format::detail::fmt
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
     * \brief Determines whether a complete specialization of `std::formatter` for the given (possibly cv-ref qualified) type exists.
     * \tparam T Type to check.
     * \tparam Char Used character-type.
     * \details This is an adapted implementation of the `std::formattable` concept, which is added c++23.
     * \note This implementation takes a simple but reasonable shortcut in assuming that `Char` is either `char` or `wchar_t`,
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

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::format::detail::fmt
{
    template <typename... Args>
    using format_string = ::fmt::format_string<Args...>;
    using ::fmt::format_args;
    using ::fmt::formatter;
    using ::fmt::make_format_args;
    using ::fmt::vformat;
    using ::fmt::vformat_to;

    template <class T, class Char>
    concept formattable = requires {
        requires ::fmt::is_formattable<std::remove_reference_t<T>, Char>::value;
    };
}

#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::format
{
    template <class T, typename Char = CharT>
    concept formattable = detail::fmt::formattable<T, Char>;

    using detail::fmt::format_args;
    using detail::fmt::format_string;
    using detail::fmt::formatter;
    using detail::fmt::make_format_args;
    using detail::fmt::vformat_to;
    using detail::fmt::vformat;

    template <print_iterator OutIter, typename... Args>
    OutIter format_to(OutIter out, format_string<Args...> const fmt, Args&&... args)
    {
        return detail::fmt::vformat_to(
            std::move(out),
            fmt.get(),
            format::make_format_args(args...));
    }

    template <typename... Args>
    StringT format(format_string<Args...> const fmt, Args && ... args)
    {
        return detail::fmt::vformat(
            fmt.get(),
            format::make_format_args(args...));
    }
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::format
{
    namespace detail
    {
        template <typename T>
        struct fallback_formattable
        {
            using printer_type = printing::PrintFn;
            std::remove_reference_t<T> value;

            [[nodiscard]]
            constexpr auto& get() const noexcept
            {
                return value;
            }
        };

        template <typename T>
            requires std::is_lvalue_reference_v<T>
        struct fallback_formattable<T>
        {
            using printer_type = printing::PrintFn;
            std::reference_wrapper<std::remove_reference_t<T>> ref;

            [[nodiscard]]
            constexpr auto& get() const noexcept
            {
                return ref.get();
            }
        };
    }

    template <typename T>
    constexpr decltype(auto) fallback_formattable(T&& target)
    {
        if constexpr (formattable<T&>)
        {
            return std::forward<T>(target);
        }
        else
        {
            return detail::fallback_formattable<T>{std::forward<T>(target)};
        }
    }
}

template <typename T>
struct mimicpp::format::formatter<mimicpp::format::detail::fallback_formattable<T>, mimicpp::CharT>
{
    using Target = mimicpp::format::detail::fallback_formattable<T>;
    using Printer = typename Target::printer_type;

    static constexpr auto parse(auto& ctx)
    {
        return ctx.begin();
    }

    static constexpr auto format(Target const& target, auto& ctx)
    {
        return std::invoke(Printer{}, ctx.out(), target.get());
    }
};

#endif
