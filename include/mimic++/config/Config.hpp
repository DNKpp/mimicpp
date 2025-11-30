//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CONFIG_CONFIG_HPP
#define MIMICPP_CONFIG_CONFIG_HPP

#pragma once

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <cstdint>
    #include <version>
#endif

#ifdef MIMICPP_DETAIL_IS_MODULE
    #define MIMICPP_DETAIL_MODULE_EXPORT export
#else
    #define MIMICPP_DETAIL_MODULE_EXPORT
#endif

#if INTPTR_MAX < INT64_MAX
    #define MIMICPP_DETAIL_IS_32BIT 1
#endif

#ifdef _LIBCPP_VERSION
    #define MIMICPP_DETAIL_USES_LIBCXX 1
#endif

#ifdef __GNUC__
    #ifdef __clang__
        #define MIMICPP_DETAIL_IS_CLANG 1
    #else
        #define MIMICPP_DETAIL_IS_GCC 1
        #define MIMICPP_DETAIL_GCC_VERSION (__GNUC__ * 1'000'000 + __GNUC_MINOR__ * 1000 + __GNUC_PATCHLEVEL__)
    #endif
#endif

#ifdef _WIN32
    #define MIMICPP_DETAIL_IS_WINDOWS 1
#endif

#ifdef _MSC_VER
    #ifdef __clang__
        #define MIMICPP_DETAIL_IS_CLANG_CL 1
    #else
        #define MIMICPP_DETAIL_IS_MSVC 1
    #endif
#endif

#ifndef MIMICPP_ASSERT
    #define MIMICPP_ASSERT(condition, msg) (void(0))
#endif

// clang-format off
// Prevent number from getting decorated with '.
#if 201907L <= __cpp_lib_constexpr_string
    // clang-format on
    #define MIMICPP_DETAIL_CONSTEXPR_STRING constexpr
#else
    #define MIMICPP_DETAIL_CONSTEXPR_STRING inline
#endif

// clang-format off
// Prevent number from getting decorated with '.
#if 201907L <= __cpp_lib_constexpr_vector
    // clang-format on
    #define MIMICPP_DETAIL_CONSTEXPR_VECTOR constexpr
#else
    #define MIMICPP_DETAIL_CONSTEXPR_VECTOR inline
#endif

// gcc 10.2 requires a workaround, due to some ambiguities.
// see: https://github.com/DNKpp/mimicpp/issues/151
#if MIMICPP_DETAIL_IS_GCC \
    && MIMICPP_DETAIL_GCC_VERSION < 10'003'000
    #define MIMICPP_DETAIL_STD_GET_WORKAROUND 1
#endif

#endif
