//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_ALGORITHM_HPP
#define MIMICPP_UTILITIES_ALGORITHM_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/C++26Backports.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <iterator>
#include <ranges>
#include <string_view>
#include <tuple>
#include <utility>

namespace mimicpp::util
{
    /**
     * \brief Partitions the target range by the predicate results on the control range.
     * \tparam Target The target range.
     * \tparam Control The control range.
     * \tparam Projection The projection type.
     * \tparam Predicate The predicate type.
     * \param targetRange The range to be partitioned.
     * \param controlRange The range, which controls the partition.
     * \param predicate The predicate for the projected control-elements.
     * \param projection The projection into the control-elements.
     * \return A subrange to the elements for which the predicate returned ``false``.
     *
     * \details This algorithm partitions the target range by evaluating the associated elements of the control range.
     * This means, that if the predicate evaluates the control-element to ``false``, the associated target-element moves
     * into the second half; otherwise it stays at its position.
     * The association is simply index based => ``target[i]`` is associated with ``control[i]``.
     *
     * \note Due to ``std::vector<bool>::iterator`` (and other Proxy-Iterators) not satisfying ``std::permutable``,
     * this requirement is dropped.
     * Nevertheless, the range must semantically still be permutable.
     * \see https://stackoverflow.com/questions/63412623/should-c20-stdrangessort-not-need-to-support-stdvectorbool
     */
    template <
        std::ranges::random_access_range Target,
        std::ranges::random_access_range Control,
        typename Projection = std::identity,
        std::indirect_unary_predicate<
            std::projected<std::ranges::iterator_t<Control>, Projection>> Predicate>
        requires std::ranges::sized_range<Target>
              && std::ranges::sized_range<Control>
    /*&& std::permutable<std::ranges::iterator_t<Target>>
    && std::permutable<std::ranges::iterator_t<Control>>*/
    [[nodiscard]]
    constexpr std::ranges::borrowed_subrange_t<Target> partition_by(
        Target&& targetRange,
        Control&& controlRange,
        Predicate predicate,
        Projection projection = {})
    {
        MIMICPP_ASSERT(std::ranges::size(targetRange) == std::ranges::size(controlRange), "Size mismatch.");

        auto const targetBegin = std::ranges::begin(targetRange);
        auto const controlBegin = std::ranges::begin(controlRange);
        auto midpoint = std::ranges::end(targetRange);
        auto end = std::ranges::end(controlRange);
        for (auto iter = std::ranges::find_if_not(controlRange, std::ref(predicate), std::ref(projection));
             iter != end;
             iter = std::ranges::find_if_not(iter, end, std::ref(predicate), std::ref(projection)))
        {
            auto const index = std::ranges::distance(controlBegin, iter);
            std::ranges::iter_swap(iter, --end);
            std::ranges::iter_swap(std::ranges::next(targetBegin, index), --midpoint);
        }

        return {std::move(midpoint), std::ranges::end(targetRange)};
    }

    /**
     * \brief Finds the next closing token in the given string.
     * \tparam Range The string type.
     * \param str The string to operate on.
     * \param openingToken The symmetrical opening token.
     * \param closingToken The symmetrical closing token.
     * \return Returns the iterator to the next closing token, or `std::ranges::end(str)` if no such token exist.
     *
     * \details This algorithm determines the first closing-token, for which no corresponding opening-token where found.
     * E.g. for the input `())` an iterator to the second `)` will be returned.
     * \note The algorithm assumes, that the corresponding opening-token is not part of the string.
     */
    template <std::ranges::borrowed_range Range>
    [[nodiscard]]
    constexpr std::ranges::borrowed_iterator_t<Range> find_closing_token(
        Range&& str,
        std::ranges::range_value_t<Range> const& openingToken,
        std::ranges::range_value_t<Range> const& closingToken)
    {
        auto const begin = std::ranges::begin(str);
        auto const end = std::ranges::end(str);
        auto closingIter = std::ranges::find(str, closingToken);
        if (closingIter == end)
        {
            return closingIter;
        }

        for (auto openingIter = std::ranges::find(begin, closingIter, openingToken);
             openingIter != closingIter
             && closingIter != end;
             openingIter = std::ranges::find(openingIter + 1, closingIter, openingToken))
        {
            closingIter = std::ranges::find(closingIter + 1, end, closingToken);
        }

        return closingIter;
    }

    /**
     * \brief Finds the next unwrapped token in the given string.
     * \tparam Range The string type.
     * \param str The string to operate on.
     * \param token The token to be found.
     * \param opening Character-range, which will be treated as wrapping start.
     * \param closing Character-range, which will be treated as wrapping end.
     * \return Returns a subview to the next unwrapped token,
     * or `std::ranges::borrowed_subrange_t<Range>{std::ranges::end(str),std::ranges::end(str)}` if no such token exist.
     *
     * \details This algorithm determines the first unwrapped token.
     * Unwrapped means, that the token does not appear between elements of the opening- and closing-character-range.
     * E.g. the input `<t>t` will return the second `t` when `<` and `>` are part of the opening- and closing-range.
     */
    template <std::ranges::borrowed_range Range>
    [[nodiscard]]
    constexpr std::ranges::borrowed_subrange_t<Range> find_next_unwrapped_token(
        Range&& str,
        std::string_view const token,
        std::ranges::forward_range auto&& opening,
        std::ranges::forward_range auto&& closing)
    {
        using count_t = std::ranges::range_difference_t<Range>;
        constexpr auto countAllOf = [](auto const& source, auto const& collection) {
            count_t count{};
            for (auto const c : collection)
            {
                count += std::ranges::count(source, c);
            }

            return count;
        };

        count_t openScopes{};
        std::ranges::borrowed_subrange_t<Range> pending{str};
        while (auto const match = std::ranges::search(pending, token))
        {
            // It's important to count the matched part, because the token may also be part of either the opening-
            // or closing-collection. But count the match part with the opening collection after the test.
            openScopes += countAllOf(std::ranges::subrange{pending.begin(), match.begin()}, opening);
            openScopes -= countAllOf(std::ranges::subrange{pending.begin(), match.end()}, closing);
            MIMICPP_ASSERT(0 <= openScopes, "More scopes closed than opened.");
            if (0 == openScopes)
            {
                return match;
            }

            openScopes += countAllOf(match, opening);

            pending = {match.end(), pending.end()};
        }

        return {std::ranges::end(str), std::ranges::end(str)};
    }

    /**
     * \brief Returns a view containing all elements, which start with the given prefix.
     * \tparam Range The range type, which holds elements comparable with `Prefix`.
     * \tparam Prefix The prefix type.
     * \param range The range.
     * \param prefix The prefix.
     * \return A subrange view to `range`.
     *
     * \attention The behaviour is undefined, when `range` is not sorted.
     */
    template <std::ranges::forward_range Range, std::ranges::forward_range Prefix>
        requires std::totally_ordered_with<std::ranges::range_value_t<Range>, Prefix>
    [[nodiscard]]
    constexpr std::ranges::borrowed_subrange_t<Range> prefix_range(Range&& range, Prefix&& prefix)
    {
        auto const lower = std::ranges::lower_bound(range, prefix);
        auto const end = std::ranges::lower_bound(
            lower,
            std::ranges::end(range),
            prefix,
            [](auto const& element, auto const& p) {
                auto const iter = std::ranges::mismatch(element, p).in2;
                return iter == std::ranges::end(p);
            });

        return {lower, end};
    }

    /**
     * \brief Concatenates the given arrays by copying all elements into a new array.
     * \tparam T The element type.
     * \tparam firstN The size of the first array.
     * \tparam secondN The size of the second array.
     * \param first The first array.
     * \param second The second array.
     * \return A newly constructed arrays with copied elements.
     */
    template <std::copyable T, std::size_t firstN, std::size_t secondN>
    [[nodiscard]]
    constexpr std::array<T, firstN + secondN> concat_arrays(std::array<T, firstN> const& first, std::array<T, secondN> const& second)
    {
        return std::invoke(
            [&]<std::size_t... firstIs, std::size_t... secondIs>(
                [[maybe_unused]] std::index_sequence<firstIs...> const,
                [[maybe_unused]] std::index_sequence<secondIs...> const) {
                return std::array<T, firstN + secondN>{
                    std::get<firstIs>(first)...,
                    std::get<secondIs>(second)...};
            },
            std::make_index_sequence<firstN>{},
            std::make_index_sequence<secondN>{});
    }

    /**
     * \copybrief concat_arrays
     * \tparam T The element type.
     * \tparam firstN The size of the first array.
     * \tparam Others Other array types which share the same element-type.
     * \param first The first array.
     * \param others The second array.
     * \return A newly constructed arrays with copied elements.
     */
    template <std::copyable T, std::size_t firstN, typename... Others>
        requires(... && std::same_as<T, std::ranges::range_value_t<Others>>)
    [[nodiscard]]
    constexpr std::array<T, (firstN + ... + std::tuple_size_v<Others>)> concat_arrays(std::array<T, firstN> const& first, Others const&... others)
    {
        if constexpr (0u == sizeof...(Others))
        {
            return first;
        }
        else
        {
            return concat_arrays(
                first,
                concat_arrays(others...));
        }
    }

    namespace detail
    {
        struct binary_find_fn
        {
            template <
                std::forward_iterator Iterator,
                std::sentinel_for<Iterator> Sentinel,
                typename Projection = std::identity,
                typename T = util::projected_value_t<Iterator, Projection>,
                std::indirect_strict_weak_order<
                    T const*,
                    std::projected<Iterator, Projection>> Comparator = std::ranges::less>
            [[nodiscard]]
            constexpr Iterator operator()(
                Iterator const first,
                Sentinel const last,
                T const& value,
                Comparator compare = {},
                Projection projection = {}) const
            {
                if (auto const iter = std::ranges::lower_bound(first, last, value, compare, projection);
                    iter != last
                    && !std::invoke(compare, value, std::invoke(projection, *iter)))
                {
                    return iter;
                }

                return last;
            }

            template <
                std::ranges::forward_range Range,
                typename Projection = std::identity,
                typename T = util::projected_value_t<std::ranges::iterator_t<Range>, Projection>,
                std::indirect_strict_weak_order<
                    T const*,
                    std::projected<std::ranges::iterator_t<Range>, Projection>> Comparator = std::ranges::less>
            [[nodiscard]]
            constexpr std::ranges::borrowed_iterator_t<Range> operator()(
                Range&& range,
                T const& value,
                Comparator compare = {},
                Projection projection = {}) const
            {
                return std::invoke(
                    *this,
                    std::ranges::begin(range),
                    std::ranges::end(range),
                    value,
                    std::move(compare),
                    std::move(projection));
            }
        };
    }

    /**
     * \brief Finds the specified value within the container and returns an iterator pointing to it.
     * If the value is not found, it returns an iterator to the end of the container.
     * \return A borrowed iterator to the element (or end).
     */
    inline constexpr detail::binary_find_fn binary_find{};
}

#endif
