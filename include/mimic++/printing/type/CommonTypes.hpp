//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_COMMON_TYPES_HPP
#define MIMICPP_PRINTING_TYPE_COMMON_TYPES_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"

#include <cstddef>
#include <string>
// ReSharper disable once CppUnusedIncludeDirective
#include <string_view> // false-positive

namespace mimicpp::printing::detail::type
{
    // general types
    template <>
    struct common_type_printer<void>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "void");
        }
    };

    template <>
    struct common_type_printer<bool>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "bool");
        }
    };

    template <>
    struct common_type_printer<std::byte>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::byte");
        }
    };

    // required for gcc
    template <>
    struct common_type_printer<std::nullptr_t>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::nullptr_t");
        }
    };

    // integer types
    template <>
    struct common_type_printer<short>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "short");
        }
    };

    template <>
    struct common_type_printer<unsigned short>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "unsigned short");
        }
    };

    template <>
    struct common_type_printer<int>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "int");
        }
    };

    template <>
    struct common_type_printer<unsigned int>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "unsigned int");
        }
    };

    template <>
    struct common_type_printer<long>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "long");
        }
    };

    template <>
    struct common_type_printer<unsigned long>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "unsigned long");
        }
    };

    template <>
    struct common_type_printer<long long>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "long long");
        }
    };

    template <>
    struct common_type_printer<unsigned long long>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "unsigned long long");
        }
    };

    // floating-point types
    template <>
    struct common_type_printer<float>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "float");
        }
    };

    template <>
    struct common_type_printer<double>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "double");
        }
    };

    template <>
    struct common_type_printer<long double>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "long double");
        }
    };

    // char-types
    template <>
    struct common_type_printer<char>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "char");
        }
    };

    template <>
    struct common_type_printer<signed char>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "signed char");
        }
    };

    template <>
    struct common_type_printer<unsigned char>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "unsigned char");
        }
    };

    template <>
    struct common_type_printer<char8_t>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "char8_t");
        }
    };

    template <>
    struct common_type_printer<char16_t>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "char16_t");
        }
    };

    template <>
    struct common_type_printer<char32_t>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "char32_t");
        }
    };

    template <>
    struct common_type_printer<wchar_t>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "wchar_t");
        }
    };

    // std::basic_string
    template <>
    struct common_type_printer<std::string>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::string");
        }
    };

    template <>
    struct common_type_printer<std::pmr::string>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::pmr::string");
        }
    };

    template <>
    struct common_type_printer<std::u8string>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::u8string");
        }
    };

    template <>
    struct common_type_printer<std::pmr::u8string>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::pmr::u8string");
        }
    };

    template <>
    struct common_type_printer<std::u16string>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::u16string");
        }
    };

    template <>
    struct common_type_printer<std::pmr::u16string>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::pmr::u16string");
        }
    };

    template <>
    struct common_type_printer<std::u32string>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::u32string");
        }
    };

    template <>
    struct common_type_printer<std::pmr::u32string>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::pmr::u32string");
        }
    };

    template <>
    struct common_type_printer<std::wstring>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::wstring");
        }
    };

    template <>
    struct common_type_printer<std::pmr::wstring>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::pmr::wstring");
        }
    };

    // std::basic_string_view
    template <>
    struct common_type_printer<std::string_view>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::string_view");
        }
    };

    template <>
    struct common_type_printer<std::u8string_view>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::u8string_view");
        }
    };

    template <>
    struct common_type_printer<std::u16string_view>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::u16string_view");
        }
    };

    template <>
    struct common_type_printer<std::u32string_view>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::u32string_view");
        }
    };

    template <>
    struct common_type_printer<std::wstring_view>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out)
        {
            return format::format_to(std::move(out), "std::wstring_view");
        }
    };
}

#endif
