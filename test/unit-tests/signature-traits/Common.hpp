//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UNIT_TESTS_SIGNATURE_TRAITS_COMMON_HPP
#define MIMICPP_UNIT_TESTS_SIGNATURE_TRAITS_COMMON_HPP

template <typename Signature>
inline constexpr std::type_identity<Signature> type_v{};

#define TEST_SIGNATURE_COLLECTION      \
    (void),                            \
        (void, int),                   \
        (void, float, int),            \
        (void, float&),                \
        (void, const float&),          \
        (void, float&&),               \
        (void, const float&&),         \
        (void, float*),                \
        (void, const float*),          \
                                       \
        (double),                      \
        (double, int),                 \
        (double, float, int),          \
        (double, float&),              \
        (double, const float&),        \
        (double, float&&),             \
        (double, const float&&),       \
        (double, float*),              \
        (double, const float*),        \
                                       \
        (double&),                     \
        (double&, int),                \
        (double&, float, int),         \
        (double&, float&),             \
        (double&, const float&),       \
        (double&, float&&),            \
        (double&, const float&&),      \
        (double&, float*),             \
        (double&, const float*),       \
                                       \
        (const double&),               \
        (const double&, int),          \
        (const double&, float, int),   \
        (const double&, float&),       \
        (const double&, const float&), \
        (const double&, float&&),      \
        (const double&, const float&&), \
        (const double&, float*),       \
        (const double&, const float*), \
                                       \
        (double&&),                    \
        (double&&, int),               \
        (double&&, float, int),        \
        (double&&, float&),            \
        (double&&, const float&),      \
        (double&&, float&&),           \
        (double&&, const float&&),     \
        (double&&, float*),            \
        (double&&, const float*),      \
                                       \
        (const double&&),              \
        (const double&&, int),         \
        (const double&&, float, int),  \
        (const double&&, float&),      \
        (const double&&, const float&), \
        (const double&&, float&&),     \
        (const double&&, const float&&), \
        (const double&&, float*),      \
        (const double&&, const float*), \
                                       \
        (void*),                       \
        (void*, int),                  \
        (void*, float, int),           \
        (void*, float&),               \
        (void*, const float&),         \
        (void*, float&&),              \
        (void*, const float&&),        \
        (void*, float*),               \
        (void*, const float*),         \
                                       \
        (const void*),                 \
        (const void*, int),            \
        (const void*, float, int),     \
        (const void*, float&),         \
        (const void*, const float&),   \
        (const void*, float&&),        \
        (const void*, const float&&),  \
        (const void*, float*),         \
        (const void*, const float*)

#endif
