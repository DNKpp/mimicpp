//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_CXX23_BACKPORTS_HPP
#define MIMICPP_UTILITIES_CXX23_BACKPORTS_HPP

#pragma once

#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <type_traits>
    #include <version>
#endif

// GCOVR_EXCL_START
#ifdef __cpp_lib_unreachable

    #ifndef MIMICPP_DETAIL_IS_MODULE
        #include <utility>
    #endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    using std::unreachable;
}

#else

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \brief Invokes undefined behavior
     * \see https://en.cppreference.com/w/cpp/utility/unreachable
     * \note Implementation directly taken from https://en.cppreference.com/w/cpp/utility/unreachable
     */
    [[noreturn]]
    inline void unreachable()
    {
        MIMICPP_ASSERT(false, "Reached the unreachable.");

    // Uses compiler specific extensions if possible.
    // Even if no extension is used, undefined behavior is still raised by
    // an empty function body and the noreturn attribute.
    #if MIMICPP_DETAIL_IS_MSVC // MSVC
        __assume(false);
    #else                      // GCC, Clang
        __builtin_unreachable();
    #endif
    }
}
#endif
// GCOVR_EXCL_STOP

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    template <typename T>
        requires std::is_enum_v<T>
    [[nodiscard]]
    constexpr std::underlying_type_t<T> to_underlying(T const value) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(value);
    }
}

#endif
