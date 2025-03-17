//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_CONCEPTS_HPP
#define MIMICPP_UTILITIES_CONCEPTS_HPP

#pragma once

#include <concepts>
#include <utility>

namespace mimicpp::util
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
}

#endif
