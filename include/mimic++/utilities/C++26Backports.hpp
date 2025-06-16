//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_CXX26_BACKPORTS_HPP
#define MIMICPP_UTILITIES_CXX26_BACKPORTS_HPP

#pragma once

#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <iterator>
    #include <type_traits>
    #include <version>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \brief The alias template projected_value_t obtains the value type by stripping any reference and its topmost
     * cv-qualifiers of the result type of applying Proj to `std::iter_value_t<I>&`.
     * \tparam I an indirectly readable type.
     * \tparam Projection projection applied to an lvalue reference to value type of `I`.
     * \see https://en.cppreference.com/w/cpp/iterator/projected_value_t
     * \note Implementation directly taken from https://en.cppreference.com/w/cpp/iterator/projected_value_t
     */
    template <std::indirectly_readable I, std::indirectly_regular_unary_invocable<I> Projection>
    using projected_value_t = std::remove_cvref_t<
        std::invoke_result_t<Projection&, std::iter_value_t<I>&>>;
}

#endif
