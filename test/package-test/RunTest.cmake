#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(PACKAGE_PREFIX "${TEST_BINARY_DIR}/package")

# Clear any previous attempt
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf "${TEST_BINARY_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)

# Install mimic++
execute_process(COMMAND ${CMAKE_COMMAND} --install "${CUR_BINARY_DIR}" --prefix "${PACKAGE_PREFIX}/origin"
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)

# Relocate the package, to test that it is properly relocatable
execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf "${PACKAGE_PREFIX}/other-location"
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${PACKAGE_PREFIX}/other/location"
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)
execute_process(COMMAND ${CMAKE_COMMAND} -E rename "${PACKAGE_PREFIX}/origin" "${PACKAGE_PREFIX}/other/location/mimicpp"
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)

# Configure, build and run the test
set(PACKAGE_TEST_BINARY_DIR "${TEST_BINARY_DIR}/package-consumer")
execute_process(COMMAND ${CMAKE_COMMAND}
    -S "${TEST_SOURCE_DIR}/package-consumer"
    -B "${PACKAGE_TEST_BINARY_DIR}"
    --fresh
    --log-level=DEBUG
    -D
    CMAKE_PREFIX_PATH=${PACKAGE_PREFIX}/other/location
    -D
    CMAKE_MODULE_PATH=${PROJECT_SOURCE_DIR}/cmake
    -D MIMICPP_WITH_FMT=${MIMICPP_WITH_FMT}
    -D MIMICPP_WITH_UNICODE_STR_MATCHER=${MIMICPP_WITH_UNICODE_STR_MATCHER}
    -D MIMICPP_WITH_STACKTRACE=${MIMICPP_WITH_STACKTRACE}
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)
execute_process(COMMAND ${CMAKE_COMMAND} --build "${PACKAGE_TEST_BINARY_DIR}" -j
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)
execute_process(COMMAND ${CMAKE_CTEST_COMMAND} --test-dir "${PACKAGE_TEST_BINARY_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)
