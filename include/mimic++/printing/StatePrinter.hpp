//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_STATE_PRINTER_HPP
#define MIMICPP_PRINTING_STATE_PRINTER_HPP

#pragma once

#include "mimic++/Config.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/String.hpp"
// ReSharper disable once CppUnusedIncludeDirective
#include "mimic++/TypeTraits.hpp" // uint_with_size
#include "mimic++/Utility.hpp"

#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/PathPrinter.hpp"

#include <bit>
#include <concepts>
#include <cstddef>
// ReSharper disable once CppUnusedIncludeDirective
#include <functional> // false-positive
#include <iterator>
#include <optional>
#include <ranges>
#include <source_location>
#include <stdexcept>
#include <string>
// ReSharper disable once CppUnusedIncludeDirective
#include <string_view> // false-positive
#include <tuple>
#include <type_traits>
#include <utility>

template <>
struct mimicpp::format::formatter<mimicpp::ValueCategory, mimicpp::CharT>
    : public formatter<std::string_view, mimicpp::CharT>
{
    using ValueCategoryT = mimicpp::ValueCategory;

    auto format(
        const ValueCategoryT category,
        auto& ctx) const
    {
        constexpr auto toString = [](const ValueCategoryT cat) {
            switch (cat)
            {
            case ValueCategoryT::lvalue:
                return "lvalue";
            case ValueCategoryT::rvalue:
                return "rvalue";
            case ValueCategoryT::any:
                return "any";
            }

            throw std::invalid_argument{"Unknown category value."};
        };

        return formatter<std::string_view, mimicpp::CharT>::format(
            toString(category),
            ctx);
    }
};

template <>
struct mimicpp::format::formatter<mimicpp::Constness, mimicpp::CharT>
    : public formatter<std::string_view, mimicpp::CharT>
{
    using ConstnessT = mimicpp::Constness;

    auto format(
        const ConstnessT category,
        auto& ctx) const
    {
        constexpr auto toString = [](const ConstnessT value) {
            switch (value)
            {
            case ConstnessT::non_const:
                return "mutable";
            case ConstnessT::as_const:
                return "const";
            case ConstnessT::any:
                return "any";
            }

            throw std::invalid_argument{"Unknown constness value."};
        };

        return formatter<std::string_view, mimicpp::CharT>::format(
            toString(category),
            ctx);
    }
};

namespace mimicpp::detail
{
    template <
        print_iterator OutIter,
        typename T,
        printer_for<OutIter, T> Printer = custom::Printer<std::remove_const_t<T>>>
    OutIter print(
        [[maybe_unused]] priority_tag<4> const,
        OutIter out,
        T& value)
    {
        return Printer::print(
            std::move(out),
            value);
    }

    template <
        print_iterator OutIter,
        typename T,
        printer_for<OutIter, T> Printer = printing::detail::state::cxx23_backport_printer<std::remove_const_t<T>>>
    OutIter print(
        [[maybe_unused]] priority_tag<3> const,
        OutIter out,
        T& value)
    {
        return Printer::print(
            std::move(out),
            value);
    }

    template <
        print_iterator OutIter,
        typename T,
        printer_for<OutIter, T> Printer = printing::detail::state::common_type_printer<std::remove_const_t<T>>>
    OutIter print(
        [[maybe_unused]] priority_tag<2> const,
        OutIter out,
        T& value)
    {
        return Printer::print(
            std::move(out),
            value);
    }

    template <print_iterator OutIter, format::detail::formattable<CharT> T>
    OutIter print(
        [[maybe_unused]] priority_tag<1> const,
        OutIter out,
        T& value)
    {
        return format::format_to(
            std::move(out),
            "{}",
            value);
    }

    template <print_iterator OutIter>
    OutIter print(
        [[maybe_unused]] priority_tag<0> const,
        OutIter out,
        auto&)
    {
        return format::format_to(
            std::move(out),
            "{{?}}");
    }

    constexpr priority_tag<4> maxStatePrinterTag{};

    class PrintFn
    {
    public:
        template <print_iterator OutIter>
        OutIter operator()(OutIter out, auto&& value) const
        {
            static_assert(
                requires {{ print(maxStatePrinterTag, out, value) } -> std::convertible_to<OutIter>; },
                "The given type is not printable. ");

            return print(
                maxStatePrinterTag,
                std::move(out),
                value);
        }

        [[nodiscard]]
        StringT operator()(auto&& value) const
        {
            StringStreamT stream{};
            std::invoke(*this, std::ostreambuf_iterator{stream}, value);

            return std::move(stream).str();
        }
    };
}

namespace mimicpp
{
    /**
     * \defgroup PRINTING printing
     * \brief Contains several printing related functionalities.
     */

    /**
     * \defgroup PRINTING_STATE object-state stringification
     * \ingroup PRINTING
     * \brief State stringification occurs when an object's state is transformed into a textual representation.
     * \details ``mimic++`` often aims to provide users with a textual representation of their tested objects,
     * particularly when a test case has failed.
     * It employs the ``print`` function for this purpose.
     *
     * This function internally checks for the first available option in the following order:
     * - ``mimicpp::custom::Printer`` specialization
     * - internal printer specializations (which handle strings, ``std::source_location``, ``std::optional``, etc.)
     * - Types that satisfy ``std::ranges::forward_range``
     * - Formattable types (based on the installed format backend)
     *
     * If no valid alternative is found, the default option is used, which simply prints "{?}").
     *
     * ### Override existing printings or print custom types
     *
     * As ``mimic++``  cannot automatically convert every custom type, it employs a simple yet effective mechanism.
     * Users can create a specialization of ``mimicpp::custom::Printer`` for their own or third-party types.
     * ``mimic++`` will always prioritize these specializations over any internal alternatives,
     * even if it already has specific handling for that particular type (e.g. ``std::source_location``).
     *
     * Consider the following type.
     * \snippet CustomPrinter.cpp my_type
     * Users can then create a specialization as follows:
     * \snippet CustomPrinter.cpp my_type printer
     *
     * When an object of ``my_type`` is then passed to ``print``, that specification will be used:
     * \snippet CustomPrinter.cpp my_type print
     *
     * ### String printing
     *
     * All char-strings (like ``const char*`` or ``std::string``) will be printed as they are.
     * However, other character types (e.g. ``wchar_t`` or ``char8_t``) require careful handling.
     * Achieving 100% compatibility with all existing character types in ``mimic++`` would either involve significant workload
     * or necessitate reliance on an external dependency.
     * Currently, ``mimic++`` takes a different approach:
     * if a string of a non-printable character type (as determined by the type trait ``is_character``) is detected,
     * it prints the string literal, converting all elements to their value representation
     * and displaying them as comma-separated hexadecimal values.
     *
     * For example, the string ``u8"Hello, World!"`` will then be printed as:
     * ``u8"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21"``.
     *
     *\{
     */

    /**
     * \brief Functional object, converting the given object to its textual representation.
     */
    [[maybe_unused]] inline constexpr detail::PrintFn print{};

    /**
     * \}
     */
}

#endif
