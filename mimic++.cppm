//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

module;

#include <version>

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdlib>
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
#include <source_location>
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

#if MIMICPP_CONFIG_USE_FMT

    #if MIMICPP_CONFIG_IMPORT_FMT
import fmt;
    #else
        #include <fmt/format.h>
    #endif
#else
    #include <format>

#endif

#if MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE

    #if MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE

        #if MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE
import cpptrace;
        #else

            #if __has_include(<cpptrace/basic.hpp>)
                #include <cpptrace/basic.hpp>
            #elif __has_include(<cpptrace/cpptrace.hpp>)
                // This is necessary for older cpptrace versions.
                // see: https://github.com/jeremy-rifkin/libassert/issues/110
                #include <cpptrace/cpptrace.hpp>
            #endif

        #endif

    #elif defined(__cpp_lib_stacktrace)
        #include <stacktrace>
    #endif

#endif

export module mimicpp;

#define MIMICPP_DETAIL_IS_MODULE 1

#include "mimic++/mimic++.hpp"
