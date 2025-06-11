//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CONFIG_CONFIG_HPP
#define MIMICPP_CONFIG_CONFIG_HPP

#pragma once

#include <cstdint>
#include <version>

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

#if 201'907L <= __cpp_lib_constexpr_string
    #define MIMICPP_DETAIL_CONSTEXPR_STRING constexpr
#else
    #define MIMICPP_DETAIL_CONSTEXPR_STRING inline
#endif

#endif
