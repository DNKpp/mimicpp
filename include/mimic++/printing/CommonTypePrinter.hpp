//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_COMMON_TYPE_PRINTER_HPP
#define MIMICPP_PRINTING_COMMON_TYPE_PRINTER_HPP

#include "mimic++/printing/TypePrinter.hpp"

#ifndef MIMICPP_CONFIG_DISABLE_PRETTY_TYPE_PRINTING

#include <cstddef>
#include <string>
// ReSharper disable once CppUnusedIncludeDirective
#include <string_view> // false-positive

namespace mimicpp::printing::detail
{
    // required for msvc
    template <>
    struct common_type_printer<long long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"long long"};
        }
    };

    // required for msvc
    template <>
    struct common_type_printer<unsigned long long>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"unsigned long long"};
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

    // required for AppleClang
    template <>
    struct common_type_printer<char8_t>
    {
        [[nodiscard]]
        static consteval StringViewT name() noexcept
        {
            return {"char8_t"};
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

#endif
