//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITY_HPP
#define MIMICPP_UTILITY_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/Concepts.hpp"

#include <array>
#include <cassert>
#include <ranges>
#include <tuple>
#include <utility>

namespace mimicpp
{
    [[nodiscard]]
    constexpr bool is_matching(const Constness lhs, const Constness rhs) noexcept
    {
        return std::cmp_not_equal(0, util::to_underlying(lhs) & util::to_underlying(rhs));
    }

    [[nodiscard]]
    constexpr bool is_matching(const ValueCategory lhs, const ValueCategory rhs) noexcept
    {
        return std::cmp_not_equal(0, util::to_underlying(lhs) & util::to_underlying(rhs));
    }
}

#endif
