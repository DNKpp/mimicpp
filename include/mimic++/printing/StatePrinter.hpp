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

namespace mimicpp::custom
{
    /**
     * \brief User may add specializations, which will then be used during ``print`` calls.
     * \ingroup STRINGIFICATION
     */
    template <typename>
    class Printer;
}

namespace mimicpp::detail
{
    template <
        print_iterator OutIter,
        typename T,
        printer_for<OutIter, T> Printer = custom::Printer<std::remove_cvref_t<T>>>
    OutIter print(
        OutIter out,
        T&& value,
        [[maybe_unused]] const priority_tag<4>)
    {
        return Printer::print(
            std::move(out),
            std::forward<T>(value));
    }

    template <
        print_iterator OutIter,
        typename T,
        printer_for<OutIter, T> Printer = Printer<std::remove_cvref_t<T>>>
    OutIter print(
        OutIter out,
        T&& value,
        [[maybe_unused]] const priority_tag<3>)
    {
        return Printer::print(
            std::move(out),
            std::forward<T>(value));
    }

    template <print_iterator OutIter, std::ranges::forward_range Range>
    OutIter print(
        OutIter out,
        Range&& range,
        priority_tag<2>);

    template <print_iterator OutIter, format::detail::formattable<CharT> T>
    OutIter print(
        OutIter out,
        T&& value,
        [[maybe_unused]] const priority_tag<1>)
    {
        return format::format_to(
            std::move(out),
            "{}",
            std::forward<T>(value));
    }

    template <print_iterator OutIter>
    OutIter print(
        OutIter out,
        auto&&,
        [[maybe_unused]] const priority_tag<0>)
    {
        return format::format_to(
            std::move(out),
            "{{?}}");
    }

    class PrintFn
    {
    public:
        template <print_iterator OutIter, typename T>
        OutIter operator()(
            OutIter out,
            T&& value) const
        {
            static_assert(
                requires(const priority_tag<4> tag) {
                    { print(out, std::forward<T>(value), tag) } -> std::convertible_to<OutIter>;
                },
                "The given type is not printable. ");

            return print(
                std::move(out),
                std::forward<T>(value),
                priority_tag<4>{});
        }

        template <typename T>
        StringT operator()(T&& value) const
        {
            StringStreamT stream{};
            std::invoke(
                *this,
                std::ostreambuf_iterator{stream},
                std::forward<T>(value));
            return std::move(stream).str();
        }
    };

    template <print_iterator OutIter, std::ranges::forward_range Range>
    OutIter print(
        OutIter out,
        Range&& range, // NOLINT(cppcoreguidelines-missing-std-forward)
        const priority_tag<2>)
    {
        out = format::format_to(
            std::move(out),
            "{{ ");
        auto iter = std::ranges::begin(range);
        if (const auto end = std::ranges::end(range);
            iter != end)
        {
            constexpr PrintFn print{};
            out = print(std::move(out), *iter++);

            for (; iter != end; ++iter)
            {
                out = print(
                    format::format_to(std::move(out), ", "),
                    *iter);
            }
        }

        return format::format_to(
            std::move(out),
            " }}");
    }

    template <>
    class Printer<std::source_location>
    {
    public:
        template <print_iterator OutIter>
        static OutIter print(OutIter out, const std::source_location& loc)
        {
            return format::format_to(
                std::move(out),
                "{}[{}:{}], {}",
                loc.file_name(),
                loc.line(),
                loc.column(),
                loc.function_name());
        }
    };

    template <>
    class Printer<std::nullopt_t>
    {
    public:
        template <print_iterator OutIter>
        static OutIter print(OutIter out, [[maybe_unused]] const std::nullopt_t)
        {
            return format::format_to(
                std::move(out),
                "nullopt");
        }
    };

    template <typename T>
    class Printer<std::optional<T>>
    {
    public:
        template <print_iterator OutIter>
        static OutIter print(OutIter out, const std::optional<T>& opt)
        {
            constexpr PrintFn print{};

            if (opt)
            {
                out = format::format_to(std::move(out), "{{ value: ");
                out = print(std::move(out), *opt);
                return format::format_to(std::move(out), " }}");
            }
            return print(std::move(out), std::nullopt);
        }
    };

    template <std::size_t index, print_iterator OutIter, typename Tuple>
    OutIter tuple_element_print(OutIter out, Tuple&& tuple)
    {
        if constexpr (0u != index)
        {
            out = format::format_to(std::move(out), ", ");
        }

        constexpr PrintFn printer{};
        return printer(
            std::move(out),
            std::get<index>(std::forward<Tuple>(tuple)));
    }

    template <typename T>
        requires requires {
            typename std::tuple_size<T>::type;
            requires std::convertible_to<typename std::tuple_size<T>::type, std::size_t>;
            requires 0u <= std::tuple_size_v<T>;
        }
    class Printer<T>
    {
    public:
        template <print_iterator OutIter>
        static OutIter print(OutIter out, const T& tuple)
        {
            out = format::format_to(std::move(out), "{{ ");

            std::invoke(
                [&]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>) {
                    (...,
                     (out = tuple_element_print<indices>(std::move(out), tuple)));
                },
                std::make_index_sequence<std::tuple_size_v<T>>{});

            return format::format_to(std::move(out), " }}");
        }
    };

    template <typename T>
    concept pointer_like = std::is_pointer_v<T>
                        || std::same_as<std::nullptr_t, T>;

    template <pointer_like T>
        requires(!string<T>)
    class Printer<T>
    {
    public:
        template <print_iterator OutIter>
        static OutIter print(OutIter out, T ptr)
        {
            if constexpr (4u < sizeof(T))
            {
                return format::format_to(
                    std::move(out),
                    "0x{:0>16x}",
                    std::bit_cast<std::uintptr_t>(ptr));
            }
            else
            {
                return format::format_to(
                    std::move(out),
                    "0x{:0>8x}",
                    std::bit_cast<std::uintptr_t>(ptr));
            }
        }
    };

    template <string String>
    class Printer<String>
    {
    public:
        template <std::common_reference_with<String> T, print_iterator OutIter>
        static OutIter print(OutIter out, T&& str)
        {
            if constexpr (constexpr auto prefix = string_literal_prefix<string_char_t<String>>;
                          !std::ranges::empty(prefix))
            {
                out = out = format::format_to(
                    std::move(out),
                    "{}",
                    prefix);
            }

            out = format::format_to(std::move(out), "\"");

            // By definition, the string concepts requires the view type to be sized and contiguous.
            // For simplicity, let's always manually convert the view type to an actual std::string_view,
            // which is formattable for sure.
            if constexpr (std::same_as<CharT, string_char_t<String>>)
            {
                auto view = string_traits<String>::view(str);
                out = format::format_to(
                    std::move(out),
                    "{}",
                    StringViewT{
                        std::ranges::data(view),
                        std::ranges::size(view)});
            }
            // required for custom char types
            else if constexpr (printer_for<custom::Printer<string_char_t<String>>, OutIter, string_char_t<String>>)
            {
                for (custom::Printer<string_char_t<String>> printer{};
                     const string_char_t<String>& c : string_traits<String>::view(str))
                {
                    out = printer.print(std::move(out), c);
                }
            }
            else
            {
                constexpr auto to_dump = [](const string_char_t<String>& c) noexcept {
                    using intermediate_t = uint_with_size_t<sizeof c>;
                    return std::bit_cast<intermediate_t>(c);
                };

                auto view = string_traits<std::remove_cvref_t<T>>::view(std::forward<T>(str));
                auto iter = std::ranges::begin(view);
                if (const auto end = std::ranges::end(view);
                    iter != end)
                {
                    out = format::format_to(
                        std::move(out),
                        "{:#x}",
                        to_dump(*iter++));

                    for (; iter != end; ++iter)
                    {
                        out = format::format_to(
                            std::move(out),
                            ", {:#x}",
                            to_dump(*iter));
                    }
                }
            }

            return format::format_to(std::move(out), "\"");
        }
    };
}

namespace mimicpp
{
    /**
     * \defgroup STRINGIFICATION stringification
     * \brief Stringification describes the process of converting an object state into its textual representation.
     * \details ``mimic++`` often wants to present users a textual representation of their tested objects,
     * because they might have failed a test case. It utilizes the ``print`` function for that purpose.
     *
     * That function internally checks for the first available option (in that order):
     * - ``mimicpp::custom::Printer`` specialization
     * - internal printer specializations (handles strings, ``std::source_location``, ``std::optional``, etc.)
     * - satisfies ``std::ranges::forward_range``
     * - formattable type (in terms of the installed format-backend)
     *
     * If no valid alternative has been found, the default is chosen (which just prints "{?}").
     *
     * ## Override existing printings or print custom types
     *
     * As ``mimic++`` can not know how to convert any custom type out there, a simple but effective mechanism is used.
     * Users can add a specialization of ``mimicpp::custom::Printer`` for their own or third-party types.
     * ``mimic++`` will always prefer such a specialization over any other internal alternative, even if ``mimic++`` already
     * has special treatment that particular type (e.g. ``std::source_location``).
     *
     * Given the following type.
     * \snippet CustomPrinter.cpp my_type
     * Users can then create a specialization as follows:
     * \snippet CustomPrinter.cpp my_type printer
     *
     * When an object of ``my_type`` is then passed to ``print``, that specification will be used:
     * \snippet CustomPrinter.cpp my_type print
     *
     * ## String printing
     *
     * All char-strings (like ``const char*`` or ``std::string``) will be printed as they are.
     * Other character-types (e.g. ``wchar_t`` or ``char8_t``) must be treated with care. Making ``mimic++`` 100% compatible with any
     * existing character-type is either a major work-load or has to be outsourced to a dependency.
     * Currently, ``mimic++`` chooses another option: If a string of a non-printable character-type (in terms of the type-trait ``is_character``)
     * is detected, it prints the string-literal and all elements are converted to their value-representation and printed as comma separated hex-values.
     *
     * For example, the string ``u8"Hello, World!"`` will then be printed as
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
