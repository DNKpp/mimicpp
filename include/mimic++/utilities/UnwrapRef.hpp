//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_UNWRAP_REF_HPP
#define MIMICPP_UTILITIES_UNWRAP_REF_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/Concepts.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <functional>
    #include <type_traits>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \brief Unwraps the given `std::reference_wrapper` and returns the referenced object.
     * \tparam T The referenced type.
     * \param ref The `std::reference_wrapper` to unwrap.
     * \return A reference to the object, referenced by `ref`.
     */
    template <typename T>
    [[nodiscard]]
    constexpr T& unwrap_ref(std::reference_wrapper<T> const ref) noexcept
    {
        return ref.get();
    }

    /**
     * \brief Overload accepting arbitrary lvalues, simply forwarding the reference.
     * \tparam T The (deduced) reference type.
     * \param ref The lvalue-reference to forward.
     * \return A reference to `ref`.
     * \note This overload is deliberately restricted to lvalue-references,
     * as returning a reference bound to an rvalue-argument would result in a dangling reference.
     */
    template <satisfies<std::is_lvalue_reference> T>
    [[nodiscard]]
    constexpr T& unwrap_ref(T && ref) noexcept
    {
        return ref;
    }
}

#endif

