//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_COMMON_TYPE_PRINTER_HPP
#define MIMICPP_PRINTING_COMMON_TYPE_PRINTER_HPP

#include "mimic++/printing/TypePrinter.hpp"

#include <cstddef>
#include <string>
// ReSharper disable once CppUnusedIncludeDirective
#include <string_view> // false-positive

namespace mimicpp::printing::detail
{
    // general types
    template <>
    struct common_type_printer<void>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"void"};
        }
    };

    template <>
    struct common_type_printer<bool>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"bool"};
        }
    };

    template <>
    struct common_type_printer<std::byte>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::byte"};
        }
    };

    // required for gcc
    template <>
    struct common_type_printer<std::nullptr_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::nullptr_t"};
        }
    };

    // integer types
    template <>
    struct common_type_printer<short>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"short"};
        }
    };

    template <>
    struct common_type_printer<unsigned short>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned short"};
        }
    };

    template <>
    struct common_type_printer<int>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"int"};
        }
    };

    template <>
    struct common_type_printer<unsigned int>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned int"};
        }
    };

    template <>
    struct common_type_printer<long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"long"};
        }
    };

    template <>
    struct common_type_printer<unsigned long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned long"};
        }
    };

    template <>
    struct common_type_printer<long long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"long long"};
        }
    };

    template <>
    struct common_type_printer<unsigned long long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned long long"};
        }
    };

    // floating-point types
    template <>
    struct common_type_printer<float>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"float"};
        }
    };

    template <>
    struct common_type_printer<double>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"double"};
        }
    };

    template <>
    struct common_type_printer<long double>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"long double"};
        }
    };

    // char-types
    template <>
    struct common_type_printer<char>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char"};
        }
    };

    template <>
    struct common_type_printer<signed char>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"signed char"};
        }
    };

    template <>
    struct common_type_printer<unsigned char>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned char"};
        }
    };

    template <>
    struct common_type_printer<char8_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char8_t"};
        }
    };

    template <>
    struct common_type_printer<char16_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char16_t"};
        }
    };

    template <>
    struct common_type_printer<char32_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char32_t"};
        }
    };

    template <>
    struct common_type_printer<wchar_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"wchar_t"};
        }
    };

    // std::basic_string
    template <>
    struct common_type_printer<std::string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::string"};
        }
    };

    template <>
    struct common_type_printer<std::pmr::string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::pmr::string"};
        }
    };

    template <>
    struct common_type_printer<std::u8string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u8string"};
        }
    };

    template <>
    struct common_type_printer<std::pmr::u8string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::pmr::u8string"};
        }
    };

    template <>
    struct common_type_printer<std::u16string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u16string"};
        }
    };

    template <>
    struct common_type_printer<std::pmr::u16string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::pmr::u16string"};
        }
    };

    template <>
    struct common_type_printer<std::u32string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u32string"};
        }
    };

    template <>
    struct common_type_printer<std::pmr::u32string>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::pmr::u32string"};
        }
    };

    template <>
    struct common_type_printer<std::wstring>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::wstring"};
        }
    };

    template <>
    struct common_type_printer<std::pmr::wstring>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::pmr::wstring"};
        }
    };

    // std::basic_string_view
    template <>
    struct common_type_printer<std::string_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::string_view"};
        }
    };

    template <>
    struct common_type_printer<std::u8string_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u8string_view"};
        }
    };

    template <>
    struct common_type_printer<std::u16string_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u16string_view"};
        }
    };

    template <>
    struct common_type_printer<std::u32string_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::u32string_view"};
        }
    };

    template <>
    struct common_type_printer<std::wstring_view>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"std::wstring_view"};
        }
    };
}

#endif
