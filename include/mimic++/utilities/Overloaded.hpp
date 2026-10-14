//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_OVERLOADED_HPP
#define MIMICPP_UTILITIES_OVERLOADED_HPP

#pragma once

namespace mimicpp::util
{
    /**
     * \brief Famous overloaded helper type used for convenient variant visiting.
     * \ingroup UTILITIES
     * \see https://www.cppstories.com/2019/02/2lines3featuresoverload.html/
     */
    template <typename... Ts>
    struct Overloaded
        : public Ts...
    {
        using Ts::operator()...;
    };

    template <typename... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;
}

#endif
