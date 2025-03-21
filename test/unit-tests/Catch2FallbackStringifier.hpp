//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mimic++/printing/StatePrinter.hpp"

#include <string>

// see: https://github.com/catchorg/Catch2/blob/devel/docs/configuration.md#fallback-stringifier
[[nodiscard]]
std::string catch2_fallback_stringifier(auto const& object)
{
    return mimicpp::print(object);
}

#define CATCH_CONFIG_FALLBACK_STRINGIFIER catch2_fallback_stringifier
