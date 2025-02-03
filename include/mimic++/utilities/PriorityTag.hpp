//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_PRIORITY_TAG_HPP
#define MIMICPP_UTILITIES_PRIORITY_TAG_HPP

#pragma once

#include <cstddef>

namespace mimicpp::util
{
    template <std::size_t priority>
    struct priority_tag
        /** \cond Help doxygen with recursion.*/
        : public priority_tag<priority - 1>
    /** \endcond */
    {
    };

    template <>
    struct priority_tag<0u>
    {
    };
}

#endif
