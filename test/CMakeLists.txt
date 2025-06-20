#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_subdirectory("test-basics")
add_subdirectory("unit-tests")
add_subdirectory("custom-stacktrace-tests")
add_subdirectory("custom-source-location-tests")

if (MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__)
    add_subdirectory("module-test")
endif ()

option(MIMICPP_ENABLE_ADAPTER_TESTS "Determines, whether the adapter tests shall be built." OFF)
if (MIMICPP_ENABLE_ADAPTER_TESTS)
    add_subdirectory("adapter-tests")
endif ()
