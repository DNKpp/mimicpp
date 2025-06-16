//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_MACRO_EXPORTS_HPP
#define MIMICPP_MACRO_EXPORTS_HPP

#define MIMICPP_DETAIL_UNIQUE_NAME(prefix, counter) prefix##counter
#define MIMICPP_DETAIL_SCOPED_EXPECTATION_IMPL(counter) \
    [[maybe_unused]]                                    \
    ::mimicpp::ScopedExpectation const MIMICPP_DETAIL_UNIQUE_NAME(_mimicpp_expectation_, counter) =

/**
 * \brief Convenience macro, which creates a ScopedExpectation with a unique name.
 * \ingroup MOCK
 */
#define MIMICPP_SCOPED_EXPECTATION MIMICPP_DETAIL_SCOPED_EXPECTATION_IMPL(__COUNTER__)

#ifndef MIMICPP_CONFIG_ONLY_PREFIXED_MACROS

    /**
     * \brief Shorthand variant of \ref MIMICPP_SCOPED_EXPECTATION.
     * \ingroup MOCK
     */
    #define SCOPED_EXP MIMICPP_SCOPED_EXPECTATION

#endif

#endif
