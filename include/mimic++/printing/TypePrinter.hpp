//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PRINTER_HPP
#define MIMICPP_PRINTING_TYPE_PRINTER_HPP

#include "mimic++/Printer.hpp"

#include <concepts>
#include <functional>
#include <iterator>
#include <typeinfo>
#include <utility>

namespace mimicpp::detail
{
    template <typename T>
    struct CommonTypePrinter;

    template <typename T, print_iterator OutIter>
        requires requires {
            typename CommonTypePrinter<T>;
            { CommonTypePrinter<T>::value } -> std::convertible_to<StringViewT>;
        }
    OutIter print_type([[maybe_unused]] const priority_tag<1u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            CommonTypePrinter<T>::value);
    }

    template <typename T, print_iterator OutIter>
    OutIter print_type([[maybe_unused]] const priority_tag<0u>, OutIter out)
    {
        return format::format_to(
            std::move(out),
            "{}",
            typeid(T).name());
    }

    constexpr priority_tag<1u> maxPrintTypePriority{};

    template <typename T>
    class PrintTypeFn
    {
    public:
        template <print_iterator OutIter>
        OutIter operator()(OutIter out) const
        {
            return print_type<T>(maxPrintTypePriority, std::move(out));
        }

        StringT operator()() const
        {
            StringStreamT stream{};
            std::invoke(*this, std::ostreambuf_iterator{stream});
            return std::move(stream).str();
        }
    };
}

namespace mimicpp
{
    template <typename T>
    [[maybe_unused]] inline constexpr detail::PrintTypeFn<T> print_type{};
}

namespace mimicpp::detail
{
    template <>
    struct CommonTypePrinter<void>
    {
        static constexpr StringViewT value{"void"};
    };

    template <>
    struct CommonTypePrinter<bool>
    {
        static constexpr StringViewT value{"bool"};
    };

    template <>
    struct CommonTypePrinter<std::nullptr_t>
    {
        static constexpr StringViewT value{"std::nullptr_t"};
    };

    template <>
    struct CommonTypePrinter<std::byte>
    {
        static constexpr StringViewT value{"std::byte"};
    };

    template <>
    struct CommonTypePrinter<char>
    {
        static constexpr StringViewT value{"char"};
    };

    template <>
    struct CommonTypePrinter<signed char>
    {
        static constexpr StringViewT value{"signed char"};
    };

    template <>
    struct CommonTypePrinter<unsigned char>
    {
        static constexpr StringViewT value{"unsigned char"};
    };

    template <>
    struct CommonTypePrinter<char8_t>
    {
        static constexpr StringViewT value{"char8_t"};
    };

    template <>
    struct CommonTypePrinter<char16_t>
    {
        static constexpr StringViewT value{"char16_t"};
    };

    template <>
    struct CommonTypePrinter<char32_t>
    {
        static constexpr StringViewT value{"char32_t"};
    };

    template <>
    struct CommonTypePrinter<wchar_t>
    {
        static constexpr StringViewT value{"wchar_t"};
    };

    template <>
    struct CommonTypePrinter<float>
    {
        static constexpr StringViewT value{"float"};
    };

    template <>
    struct CommonTypePrinter<double>
    {
        static constexpr StringViewT value{"double"};
    };

    template <>
    struct CommonTypePrinter<long double>
    {
        static constexpr StringViewT value{"long double"};
    };

    template <>
    struct CommonTypePrinter<short>
    {
        static constexpr StringViewT value{"short"};
    };

    template <>
    struct CommonTypePrinter<unsigned short>
    {
        static constexpr StringViewT value{"unsigned short"};
    };

    template <>
    struct CommonTypePrinter<int>
    {
        static constexpr StringViewT value{"int"};
    };

    template <>
    struct CommonTypePrinter<unsigned int>
    {
        static constexpr StringViewT value{"unsigned int"};
    };

    template <>
    struct CommonTypePrinter<long>
    {
        static constexpr StringViewT value{"long"};
    };

    template <>
    struct CommonTypePrinter<unsigned long>
    {
        static constexpr StringViewT value{"unsigned long"};
    };

    template <>
    struct CommonTypePrinter<long long>
    {
        static constexpr StringViewT value{"long long"};
    };

    template <>
    struct CommonTypePrinter<unsigned long long>
    {
        static constexpr StringViewT value{"unsigned long long"};
    };
}

#endif
