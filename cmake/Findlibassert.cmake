#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(get_cpm)

if (MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__)
    set(LIBASSERT_DISABLE_CXX_20_MODULES OFF CACHE BOOL "")
else ()
    set(LIBASSERT_DISABLE_CXX_20_MODULES ON CACHE BOOL "")
endif ()

find_package(cpptrace REQUIRED)
set(LIBASSERT_USE_EXTERNAL_CPPTRACE ON CACHE BOOL "")
CPMAddPackage("gh:jeremy-rifkin/libassert@2.2.0")
