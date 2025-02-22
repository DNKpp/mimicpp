//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_ALGORITHM_HPP
#define MIMICPP_UTILITIES_ALGORITHM_HPP

#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <ranges>
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
        assert(std::ranges::size(targetRange) == std::ranges::size(controlRange) && "Size mismatch.");

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

    template <std::ranges::borrowed_range Range>
    [[nodiscard]]
    constexpr std::ranges::borrowed_iterator_t<Range> find_next_comma(Range&& str)
    {
        constexpr std::array opening{'<', '(', '[', '{'};
        constexpr std::array closing{'>', ')', ']', '}'};
        constexpr auto countAllOf = [](auto const& source, auto const& collection) {
            int count{};
            for (auto const c : collection)
            {
                count += std::ranges::count(source, c);
            }

            return count;
        };

        int openScopes{};
        auto begin = std::ranges::begin(str);
        auto const end = std::ranges::end(str);
        for (auto commaIter = std::ranges::find(str, ',');
             commaIter != end;
             commaIter = std::ranges::find(commaIter + 1, end, ','))
        {
            std::ranges::subrange const part{begin, commaIter};

            openScopes += countAllOf(part, opening)
                        - countAllOf(part, closing);
            assert(0 <= openScopes && "More scopes closed than opened.");
            if (0 == openScopes)
            {
                return commaIter;
            }

            begin = commaIter + 1;
        }

        return end;
    }
}

#endif
