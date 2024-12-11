// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <version>

#if defined(_MSC_VER) \
    && !defined(__clang__)

    #define START_WARNING_SUPPRESSION __pragma(warning(push))
    #define STOP_WARNING_SUPPRESSION  __pragma(warning(pop))

    #define SUPPRESS_UNREACHABLE_CODE    __pragma(warning(disable: 4702))
    #define SUPPRESS_SELF_MOVE           // seems not required on msvc
    #define SUPPRESS_SELF_ASSIGN         // seems not required on msvc
    #define SUPPRESS_MAYBE_UNINITIALIZED // seems not required on msvc

#else

    // clang accepts GCC diagnostic
    #define START_WARNING_SUPPRESSION _Pragma("GCC diagnostic push")
    #define STOP_WARNING_SUPPRESSION  _Pragma("GCC diagnostic pop")

    #define SUPPRESS_UNREACHABLE_CODE _Pragma("GCC diagnostic ignored \"-Wunreachable-code\"")

    // clang doesn't know -Wmaybe-uninitialized,
    // but gcc doesn't know -Wunknown-warning-option
    // but this combination works
    #define SUPPRESS_MAYBE_UNINITIALIZED                                   \
        _Pragma("GCC diagnostic ignored \"-Wpragmas\"")                    \
            _Pragma("GCC diagnostic ignored \"-Wunknown-warning-option\"") \
                _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")

    // gcc 12 doesn't know -Wself-move option
    #if !defined(__clang__)  \
        && defined(__GNUC__) \
        && 12 >= __GNUC__
        #define SUPPRESS_SELF_MOVE
    #else
        #define SUPPRESS_SELF_MOVE _Pragma("GCC diagnostic ignored \"-Wself-move\"")
    #endif

    // gcc doesn't know -Wself-assign-overloaded option
    #if defined(__clang__)
        #define SUPPRESS_SELF_ASSIGN _Pragma("GCC diagnostic ignored \"-Wself-assign-overloaded\"")
    #else
        #define SUPPRESS_SELF_ASSIGN
    #endif

#endif
