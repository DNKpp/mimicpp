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
    template <typename From, typename To>
    concept explicitly_convertible_to =
        requires {
            static_cast<To>(std::declval<From>());
        };

    template <typename From, typename To>
    concept nothrow_explicitly_convertible_to =
        explicitly_convertible_to<From, To>
        && requires {
               { static_cast<To>(std::declval<From>()) } noexcept;
           };

    template <typename T, typename... Others>
    concept same_as_any = (... || std::same_as<T, Others>);

    template <typename T, template <typename> typename Trait>
    concept satisfies = Trait<T>::value;

    // see: https://en.cppreference.com/w/cpp/concepts/boolean-testable
    template <typename B>
    concept boolean_testable =
        std::convertible_to<B, bool>
        && requires(B&& b) {
               { !std::forward<B>(b) } -> std::convertible_to<bool>;
           };

    // see: https://en.cppreference.com/w/cpp/concepts/equality_comparable
    template <typename T, typename U>
    concept weakly_equality_comparable_with =
        requires(std::remove_reference_t<T> const& t, std::remove_reference_t<U> const& u) {
            { t == u } -> boolean_testable;
            { t != u } -> boolean_testable;
            { u == t } -> boolean_testable;
            { u != t } -> boolean_testable;
        };
}

#endif
