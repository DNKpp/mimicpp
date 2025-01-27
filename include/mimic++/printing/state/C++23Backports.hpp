//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_STATE_CXX23_BACKPORTS_HPP
#define MIMICPP_PRINTING_STATE_CXX23_BACKPORTS_HPP

#pragma once

#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/state/Print.hpp"

#include <concepts>
#include <functional>
#include <ranges>
#include <tuple>
#include <utility>

namespace mimicpp::printing::detail::state
{
    template <std::ranges::forward_range Range>
    struct cxx23_backport_printer<Range>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out, auto& range)
        {
            out = format::format_to(std::move(out), "[");
            auto iter = std::ranges::begin(range);
            if (const auto end = std::ranges::end(range);
                iter != end)
            {
                out = mimicpp::print(std::move(out), *iter++);

                for (; iter != end; ++iter)
                {
                    out = format::format_to(std::move(out), ", ");
                    out = mimicpp::print(std::move(out), *iter);
                }
            }

            return format::format_to(std::move(out), "]");
        }
    };

    template <std::size_t index, print_iterator OutIter>
    OutIter print_tuple_element(OutIter out, auto& tuple)
    {
        if constexpr (0u != index)
        {
            out = format::format_to(std::move(out), ", ");
        }

        return mimicpp::print(std::move(out), std::get<index>(tuple));
    }

    template <typename T>
    concept tuple_like = requires {
        typename std::tuple_size<T>::type;
        { std::tuple_size_v<T> } -> std::convertible_to<std::size_t>;
        requires 0u <= std::tuple_size_v<T>;
    };

    template <tuple_like T>
    struct cxx23_backport_printer<T>
    {
        template <print_iterator OutIter>
        static OutIter print(OutIter out, auto& tuple)
        {
            out = format::format_to(std::move(out), "(");
            std::invoke(
                [&]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>) {
                    (...,
                     (out = state::print_tuple_element<indices>(std::move(out), tuple)));
                },
                std::make_index_sequence<std::tuple_size_v<T>>{});

            return format::format_to(std::move(out), ")");
        }
    };
}

#endif
