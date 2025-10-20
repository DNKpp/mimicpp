//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CONFIG_VERSION_HPP
#define MIMICPP_CONFIG_VERSION_HPP

#pragma once

#include "mimic++/macros/Common.hpp"

#define MIMICPP_VERSION_MAJOR 9
#define MIMICPP_VERSION_MINOR 1
#define MIMICPP_VERSION_PATCH 0

#define MIMICPP_VERSION \
    MIMICPP_DETAIL_STRINGIFY(MIMICPP_VERSION_MAJOR) "." \
    MIMICPP_DETAIL_STRINGIFY(MIMICPP_VERSION_MINOR) "." \
    MIMICPP_DETAIL_STRINGIFY(MIMICPP_VERSION_PATCH)

#endif
