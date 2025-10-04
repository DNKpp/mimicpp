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
        explicit(false) consteval StaticString(Char const (&arr)[length]) noexcept
        {
            std::ranges::copy(arr, std::ranges::begin(data));
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
}

#endif
