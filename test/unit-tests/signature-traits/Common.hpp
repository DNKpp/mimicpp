//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

//
// Created by dnkpp on 04.10.25.
//

#ifndef MIMICPP_UNIT_TESTS_SIGNATURE_TRAITS_COMMON_HPP
#define MIMICPP_UNIT_TESTS_SIGNATURE_TRAITS_COMMON_HPP

template <typename Signature>
inline constexpr std::type_identity<Signature> type_v{};

#define TEST_SIGNATURE_COLLECTION              \
    (true, void),                              \
        (true, void, int),                     \
        (true, void, float, int),              \
        (true, void, float&),                  \
        (true, void, const float&),            \
        (true, void, float&&),                 \
        (true, void, const float&&),           \
        (true, void, float*),                  \
        (true, void, const float*),            \
                                               \
        (true, double),                        \
        (true, double, int),                   \
        (true, double, float, int),            \
        (true, double, float&),                \
        (true, double, const float&),          \
        (true, double, float&&),               \
        (true, double, const float&&),         \
        (true, double, float*),                \
        (true, double, const float*),          \
                                               \
        (true, double&),                       \
        (true, double&, int),                  \
        (true, double&, float, int),           \
        (true, double&, float&),               \
        (true, double&, const float&),         \
        (true, double&, float&&),              \
        (true, double&, const float&&),        \
        (true, double&, float*),               \
        (true, double&, const float*),         \
                                               \
        (true, const double&),                 \
        (true, const double&, int),            \
        (true, const double&, float, int),     \
        (true, const double&, float&),         \
        (true, const double&, const float&),   \
        (true, const double&, float&&),        \
        (true, const double&, const float&&),  \
        (true, const double&, float*),         \
        (true, const double&, const float*),   \
                                               \
        (true, double&&),                      \
        (true, double&&, int),                 \
        (true, double&&, float, int),          \
        (true, double&&, float&),              \
        (true, double&&, const float&),        \
        (true, double&&, float&&),             \
        (true, double&&, const float&&),       \
        (true, double&&, float*),              \
        (true, double&&, const float*),        \
                                               \
        (true, const double&&),                \
        (true, const double&&, int),           \
        (true, const double&&, float, int),    \
        (true, const double&&, float&),        \
        (true, const double&&, const float&),  \
        (true, const double&&, float&&),       \
        (true, const double&&, const float&&), \
        (true, const double&&, float*),        \
        (true, const double&&, const float*),  \
                                               \
        (true, void*),                         \
        (true, void*, int),                    \
        (true, void*, float, int),             \
        (true, void*, float&),                 \
        (true, void*, const float&),           \
        (true, void*, float&&),                \
        (true, void*, const float&&),          \
        (true, void*, float*),                 \
        (true, void*, const float*),           \
                                               \
        (true, const void*),                   \
        (true, const void*, int),              \
        (true, const void*, float, int),       \
        (true, const void*, float&),           \
        (true, const void*, const float&),     \
        (true, const void*, float&&),          \
        (true, const void*, const float&&),    \
        (true, const void*, float*),           \
        (true, const void*, const float*)

#endif
