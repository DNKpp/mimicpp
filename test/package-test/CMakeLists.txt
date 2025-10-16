#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_test(NAME package-test
    COMMAND ${CMAKE_COMMAND}
    -D CUR_BINARY_DIR=${CMAKE_BINARY_DIR}
    -D TEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -D TEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/test-run
    -P "${CMAKE_CURRENT_SOURCE_DIR}/RunTest.cmake"
)
