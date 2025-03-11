//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_ALGORITHM_HPP
#define MIMICPP_UTILITIES_ALGORITHM_HPP

#pragma once

#include "mimic++/config/Config.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <string_view>
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
}

#endif
