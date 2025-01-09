//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MATCHERS_RANGE_MATCHERS_HPP
#define MIMICPP_MATCHERS_RANGE_MATCHERS_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/matchers/Common.hpp"
#include "mimic++/matchers/GeneralMatchers.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <ranges>
#include <tuple>
#include <utility>

namespace mimicpp::matches::range
{
    /**
     * \defgroup MATCHERS_RANGE range matchers
     * \ingroup MATCHERS
     * \brief Range specific matchers.
     * \snippet Requirements.cpp matcher range
     *\{
     */

    /**
     * \brief Tests, whether the target range compares equal to the expected range, by comparing them element-wise.
     * \tparam Range Expected range type.
     * \tparam Comparator Comparator type.
     * \param expected The expected range.
     * \param comparator The comparator.
     */
    template <std::ranges::forward_range Range, typename Comparator = std::equal_to<>>
    [[nodiscard]]
    constexpr auto eq(Range&& expected, Comparator comparator = Comparator{})
    {
        return PredicateMatcher{
            [comp = std::move(comparator)]<typename Target>(Target&& target, auto& range) // NOLINT(cppcoreguidelines-missing-std-forward)
                requires std::predicate<
                             const Comparator&,
                             std::ranges::range_reference_t<Target>,
                             std::ranges::range_reference_t<Range>>
            {
                return std::ranges::equal(
                    target,
                    range,
                    std::ref(comp));
            },
            "elements are {}",
            "elements are not {}",
            std::make_tuple(std::views::all(std::forward<Range>(expected)))};
    }

    /**
     * \brief Tests, whether the target range is a permutation of the expected range, by comparing them element-wise.
     * \tparam Range Expected range type.
     * \tparam Comparator Comparator type.
     * \param expected The expected range.
     * \param comparator The comparator.
     */
    template <std::ranges::forward_range Range, typename Comparator = std::equal_to<>>
    [[nodiscard]]
    constexpr auto unordered_eq(Range&& expected, Comparator comparator = Comparator{})
    {
        return PredicateMatcher{
            [comp = std::move(comparator)]<typename Target>(Target&& target, auto& range) // NOLINT(cppcoreguidelines-missing-std-forward)
                requires std::predicate<
                             const Comparator&,
                             std::ranges::range_reference_t<Target>,
                             std::ranges::range_reference_t<Range>>
            {
                return std::ranges::is_permutation(
                    target,
                    range,
                    std::ref(comp));
            },
            "is a permutation of {}",
            "is not a permutation of {}",
            std::make_tuple(std::views::all(std::forward<Range>(expected)))};
    }

    /**
     * \brief Tests, whether the target range is sorted, by applying the relation on each adjacent elements.
     * \tparam Relation Relation type.
     * \param relation The relation.
     */
    template <typename Relation = std::ranges::less>
    [[nodiscard]]
    constexpr auto is_sorted(Relation relation = Relation{})
    {
        return PredicateMatcher{
            [rel = std::move(relation)]<typename Target>(Target&& target) // NOLINT(cppcoreguidelines-missing-std-forward)
                requires std::equivalence_relation<
                             const Relation&,
                             std::ranges::range_reference_t<Target>,
                             std::ranges::range_reference_t<Target>>
            {
                return std::ranges::is_sorted(
                    target,
                    std::ref(rel));
            },
            "is a sorted range",
            "is an unsorted range"};
    }

    /**
     * \brief Tests, whether the target range is empty.
     */
    [[nodiscard]]
    constexpr auto is_empty()
    {
        return PredicateMatcher{
            [](std::ranges::range auto&& target) {
                return std::ranges::empty(target);
            },
            "is an empty range",
            "is not an empty range"};
    }

    /**
     * \brief Tests, whether the target range has the expected size.
     * \param expected The expected size.
     */
    [[nodiscard]]
    constexpr auto has_size(const std::integral auto expected)
    {
        return PredicateMatcher{
            [](std::ranges::range auto&& target, const std::integral auto size) {
                return std::cmp_equal(
                    size,
                    std::ranges::size(target));
            },
            "has size of {}",
            "has different size than {}",
            std::make_tuple(expected)};
    }

    /**
     * \brief Tests, whether each element of the target range matches the specified matcher.
     * \param matcher The matcher.
     */
    template <typename Matcher>
    [[nodiscard]]
    constexpr auto each_element(Matcher&& matcher)
    {
        using MatcherT = std::remove_cvref_t<Matcher>;
        return PredicateMatcher{
            [](std::ranges::range auto&& target, const MatcherT& m) {
                return std::ranges::all_of(
                    target,
                    [&](const auto& element) { return m.matches(element); });
            },
            "each el in range: el {}",
            "not each el in range: el {}",
            std::make_tuple(
                mimicpp::detail::arg_storage<
                    MatcherT,
                    std::identity,
                    decltype([](const auto& m) { return mimicpp::detail::describe_hook::describe(m); })>{
                    std::forward<MatcherT>(matcher)})};
    }

    /**
     * \brief Tests, whether any element of the target range matches the specified matcher.
     * \param matcher The matcher.
     */
    template <typename Matcher>
    [[nodiscard]]
    constexpr auto any_element(Matcher&& matcher)
    {
        using MatcherT = std::remove_cvref_t<Matcher>;
        return PredicateMatcher{
            [](std::ranges::range auto&& target, const MatcherT& m) {
                return std::ranges::any_of(
                    target,
                    [&](const auto& element) { return m.matches(element); });
            },
            "any el in range: el {}",
            "none el in range: el {}",
            std::make_tuple(
                mimicpp::detail::arg_storage<
                    MatcherT,
                    std::identity,
                    decltype([](const auto& m) { return mimicpp::detail::describe_hook::describe(m); })>{
                    std::forward<MatcherT>(matcher)})};
    }

    /**
     * \}
     */
}

#endif
