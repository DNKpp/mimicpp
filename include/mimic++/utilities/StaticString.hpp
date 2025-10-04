//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_STATIC_STRING_HPP
#define MIMICPP_UTILITIES_STATIC_STRING_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <cstddef>
    #include <iterator>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    template <typename Char, std::size_t length>
    class StaticString
    {
    public:
        Char data[length];

        [[nodiscard]] //
        explicit(false) consteval StaticString(Char const (&arr)[length + 1]) noexcept
        {
            std::ranges::copy_n(arr, length, std::ranges::begin(data));
        }

        [[nodiscard]]
        static constexpr bool empty() noexcept
        {
            return false;
        }

        [[nodiscard]]
        static constexpr std::size_t size() noexcept
        {
            return length;
        }

        [[nodiscard]]
        constexpr auto begin() const noexcept
        {
            return std::ranges::begin(data);
        }

        [[nodiscard]]
        constexpr auto end() const noexcept
        {
            return std::ranges::end(data);
        }
    };

    template <typename Char>
    class StaticString<Char, 0u>
    {
    public:
        [[nodiscard]] //
        explicit(false) consteval StaticString([[maybe_unused]] Char const (&arr)[1]) noexcept
        {
        }

        [[nodiscard]]
        static constexpr bool empty() noexcept
        {
            return true;
        }

        [[nodiscard]]
        static constexpr std::size_t size() noexcept
        {
            return 0u;
        }

        [[nodiscard]]
        static constexpr Char const* begin() noexcept
        {
            return nullptr;
        }

        [[nodiscard]]
        static constexpr Char const* end() noexcept
        {
            return nullptr;
        }
    };

    template <typename Char, std::size_t length>
    StaticString(Char const(&)[length]) -> StaticString<Char, length - 1u>;
}

#endif
