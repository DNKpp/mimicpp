//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_CONCEPTS_HPP
#define MIMICPP_UTILITIES_CONCEPTS_HPP

#pragma once

#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <utility>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \defgroup CONCEPTS concepts
     * \brief Contains common concept definitions.
     *
     * \{
     */

    /**
     * \brief Determines, whether `From` can be explicitly converted to `To`.
     * \note In fact, this is a more relaxed version of `std::convertible_to`, as this also requires implicit-convertibility.
     * \see https://en.cppreference.com/w/cpp/concepts/convertible_to
     */
    template <typename From, typename To>
    concept explicitly_convertible_to =
        requires {
            static_cast<To>(std::declval<From>());
        };

    /**
     * \brief Determines, whether `From` can be explicitly converted to `To`, without throwing.
     */
    template <typename From, typename To>
    concept nothrow_explicitly_convertible_to =
        explicitly_convertible_to<From, To>
        && requires {
               { static_cast<To>(std::declval<From>()) } noexcept;
           };

    /**
     * \brief Determines, whether `T` is the same as any type of the pack `Others`.
     */
    template <typename T, typename... Others>
    concept same_as_any = (... || std::same_as<T, Others>);

    /**
     * \brief Determines, whether `T` satisfies the specified trait type.
     */
    template <typename T, template <typename> typename Trait>
    concept satisfies = Trait<T>::value;

    /**
     * \brief Determines, whether `B` behaves as a the builtin type `bool`.
     * \see https://en.cppreference.com/w/cpp/concepts/boolean-testable
     */
    template <typename B>
    concept boolean_testable =
        std::convertible_to<B, bool>
        && requires(B&& b) {
               { !std::forward<B>(b) } -> std::convertible_to<bool>;
           };

    /**
     * \brief Determines, whether `T` can be equality compared with `U`.
     * \note In fact, this is a more relaxed version of the `std::equality_comparable_with` concept.
     * \see https://en.cppreference.com/w/cpp/concepts/equality_comparable
     */
    template <typename T, typename U>
    concept weakly_equality_comparable_with =
        requires(std::remove_reference_t<T> const& t, std::remove_reference_t<U> const& u) {
            { t == u } -> boolean_testable;
            { t != u } -> boolean_testable;
            { u == t } -> boolean_testable;
            { u != t } -> boolean_testable;
        };

    /**
     * \}
     */
}

#endif
