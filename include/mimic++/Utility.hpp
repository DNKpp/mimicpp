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

namespace mimicpp::detail
{
    template <std::default_initializable FillElement, std::size_t n, typename... Elements>
        requires(sizeof...(Elements) <= n)
    [[nodiscard]]
    constexpr auto expand_tuple(std::tuple<Elements...>&& tuple)
    {
        // prior to c++23, tuple_cat does not officially support tuple-like types,
        // thus we transform the generated array manually
        return std::tuple_cat(
            std::move(tuple),
            std::apply(
                [](auto&&... elements) { return std::make_tuple(std::move(elements)...); },
                std::array<FillElement, n - sizeof...(Elements)>{}));
    }
}

#endif
