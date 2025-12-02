#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

message(DEBUG "${MESSAGE_PREFIX} Searching for `cpptrace`-package.")

include(CMakeDependentOption)
cmake_dependent_option(
    MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE
    "Determines whether cpptrace will be consumed as a c++20 module."
    OFF
    "MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__"
    OFF
)

if (NOT MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE)
    set(CPPTRACE_DISABLE_CXX_20_MODULES ON CACHE BOOL "" FORCE)
else ()
    set(CPPTRACE_DISABLE_CXX_20_MODULES OFF CACHE BOOL "" FORCE)
endif ()

find_package(cpptrace QUIET)
if (NOT cpptrace_FOUND)
    message(DEBUG "${MESSAGE_PREFIX} No installed `cpptrace`-package found. Fetching via cpm.")

    include(get_cpm)
    CPMAddPackage("gh:jeremy-rifkin/cpptrace@1.0.4")
endif ()
