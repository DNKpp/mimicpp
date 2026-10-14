#          Copyright Dominic (DNKpp) Koepke 2024-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(test-basics)
add_subdirectory(compile-errors)

option(MIMICPP_ENABLE_UNIT_TESTS "Determines, whether the unit-tests shall be built." ON)
if (MIMICPP_ENABLE_UNIT_TESTS)
    add_subdirectory(unit-tests)

    if (MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES)
        add_subdirectory(type-name-print-tests)
    endif ()
endif ()

if (MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__)
    add_subdirectory(module-test)
endif ()

option(MIMICPP_ENABLE_ADAPTER_TESTS "Determines, whether the adapter tests shall be built." OFF)
if (MIMICPP_ENABLE_ADAPTER_TESTS)
    add_subdirectory(adapter-tests)
endif ()

option(MIMICPP_ENABLE_STACKTRACE_TESTS "Determines, whether the stacktrace tests shall be built." OFF)
if (MIMICPP_ENABLE_STACKTRACE_TESTS)
    add_subdirectory(stacktrace-tests)
endif ()

option(MIMICPP_ENABLE_UNICODE_STR_MATCHER_TESTS "Determines, whether the unicode-str-matcher tests shall be built." OFF)
if (MIMICPP_ENABLE_UNICODE_STR_MATCHER_TESTS)
    add_subdirectory(unicode-str-matcher-tests)
endif ()

cmake_dependent_option(MIMICPP_ENABLE_PACKAGE_TEST "Tests the generated package." ON "MIMICPP_INSTALL" OFF)
if (MIMICPP_ENABLE_PACKAGE_TEST)
    add_subdirectory(package-test)
endif ()
