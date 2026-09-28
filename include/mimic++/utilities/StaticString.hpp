//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_STATIC_STRING_HPP
#define MIMICPP_UTILITIES_STATIC_STRING_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <array>
    #include <concepts>
    #include <cstddef>
    #include <string>
    #include <string_view>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    template <typename Char, std::size_t length>
    class StaticString
    {
    public:
        // We intentionally keep the null-terminator.
        std::array<Char, length + 1u> data;

        [[nodiscard]] //
        explicit(false) consteval StaticString(Char const (&arr)[length + 1u]) noexcept
        {
            std::ranges::copy(arr, data.begin());
        }

        [[nodiscard]]
        static constexpr bool empty() noexcept
        {
            return length == 0u;
        }

        [[nodiscard]]
        static constexpr std::size_t size() noexcept
        {
            return length;
        }

        [[nodiscard]]
        constexpr auto begin() const noexcept
        {
            return data.cbegin();
        }

        [[nodiscard]]
        constexpr auto end() const noexcept
        {
            return begin() + size();
        }

        [[nodiscard]]
        constexpr std::basic_string_view<Char> view() const noexcept
        {
            return {begin(), end()};
        }

        [[nodiscard]]
        constexpr std::basic_string<Char> str() const noexcept
        {
            return {begin(), end()};
        }

        [[nodiscard]]
        friend bool operator==(StaticString const&, StaticString const&) = default;

        template <std::convertible_to<std::basic_string_view<Char>> String>
        [[nodiscard]]
        constexpr bool operator==(String const& other) const noexcept
        {
            return view() == std::basic_string_view<Char>{other};
        }
    };

    template <typename Char, std::size_t length>
    StaticString(Char const(&)[length]) -> StaticString<Char, length - 1u>;
}

#endif
