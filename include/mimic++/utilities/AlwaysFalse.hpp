//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_ALWAYS_FALSE_HPP
#define MIMICPP_UTILITIES_ALWAYS_FALSE_HPP

#pragma once

#include <type_traits>

namespace mimicpp::util
{
    template <typename...>
    struct always_false
        : public std::bool_constant<false>
    {
    };
}

#endif
