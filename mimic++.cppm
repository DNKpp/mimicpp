//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

module;

#include <version>

#ifdef __cpp_lib_modules
import std;
#else
    #include <algorithm>
    #include <any>
    #include <array>
    #include <atomic>
    #include <cmath>
    #include <concepts>
    #include <cstddef>
    #include <deque>
    #include <filesystem>
    #include <functional>
    #include <iostream>
    #include <iterator>
    #include <limits>
    #include <memory>
    #include <mutex>
    #include <optional>
    #include <ranges>
    #include <span>
    #include <sstream>
    #include <stdexcept>
    #include <string>
    #include <string_view>
    #include <tuple>
    #include <type_traits>
    #include <unordered_map>
    #include <unordered_set>
    #include <utility>
    #include <variant>
    #include <vector>

    // We support gcc-10, which doesn't know source-location.
    #ifdef __cpp_lib_source_location
        #include <source_location>
    #endif
#endif

export module mimicpp;

#define MIMICPP_DETAIL_IS_MODULE 1

#include "mimic++/mimic++.hpp"
