#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

message(DEBUG "${MESSAGE_PREFIX} Searching for {fmt}-package.")

include(CMakeDependentOption)
cmake_dependent_option(
    MIMICPP_CONFIG_IMPORT_FMT
    "Determines, whether fmt will be consumed as a c++20 module."
    ON
    "MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__"
    OFF
)
set(FMT_MODULE ${MIMICPP_CONFIG_IMPORT_FMT} CACHE BOOL "" FORCE)

find_package(fmt QUIET)
if (NOT fmt_FOUND)
    include(get_cpm)
    CPMAddPackage("gh:fmtlib/fmt#12.1.0")
endif ()
