//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_DIAGNOSTICS_HELPER_HPP
#define MIMICPP_UTILITIES_DIAGNOSTICS_HELPER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"

namespace mimicpp::diagnostics
{
    template <bool outcome>
    consteval bool verify_constraint([[maybe_unused]] char const* const diagnostic) noexcept
    {
        return outcome;
    }

    template <>
    consteval bool verify_constraint<false>([[maybe_unused]] char const* diagnostic) noexcept
    {
        return false;
    }
}

#endif
