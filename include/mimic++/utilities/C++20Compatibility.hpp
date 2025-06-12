//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_CXX20_COMPATIBILITY_HPP
#define MIMICPP_UTILITIES_CXX20_COMPATIBILITY_HPP

#pragma once

#include "mimic++/config/Config.hpp"

#include <version>

#ifdef __cpp_lib_bit_cast

    #include <bit>

namespace mimicpp::util
{
    using std::bit_cast;
}

#else

namespace mimicpp::util
{
    // taken from: https://www.en.cppreference.com/w/cpp/numeric/bit_cast.html
    template <typename To, typename From>
    std::enable_if_t<
        sizeof(To) == sizeof(From)
            && std::is_trivially_copyable_v<From>
            && std::is_trivially_copyable_v<To>,
        To>
    // constexpr support needs compiler magic
    bit_cast(From const& src) noexcept
    {
        static_assert(std::is_trivially_constructible_v<To>, "This implementation additionally requires destination type to be trivially constructible");

        To dst;
        std::memcpy(&dst, &src, sizeof(To));

        return dst;
    }
}

#endif

#endif
