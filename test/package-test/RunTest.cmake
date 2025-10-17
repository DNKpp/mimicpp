#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(PACKAGE_PREFIX "${TEST_BINARY_DIR}/package")

function(unwrap_args LIST OUT_LIST)
    string(LENGTH ${LIST} STR_LENGTH)
    math(EXPR STR_LENGTH "${STR_LENGTH} - 4")
    string(SUBSTRING ${LIST} 2 ${STR_LENGTH} LIST)
    set(${OUT_LIST} "${LIST}" PARENT_SCOPE)
endfunction()

unwrap_args(${CONFIG_ARGS} CONFIG_ARGS)
unwrap_args(${BUILD_ARGS} BUILD_ARGS)
message(STATUS CONFIG_ARGS: "${CONFIG_ARGS}")
message(STATUS BUILD_ARGS: "${BUILD_ARGS}")

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
        ${CONFIG_ARGS}
        -D CMAKE_PREFIX_PATH=${PACKAGE_PREFIX}/other/location
        -D CMAKE_MODULE_PATH=${PROJECT_SOURCE_DIR}/cmake
        COMMAND_ERROR_IS_FATAL ANY
        COMMAND_ECHO STDOUT
)
execute_process(COMMAND ${CMAKE_COMMAND} --build "${PACKAGE_TEST_BINARY_DIR}" ${BUILD_ARGS} -j
        COMMAND_ERROR_IS_FATAL ANY
        COMMAND_ECHO STDOUT
)
execute_process(COMMAND ${CMAKE_CTEST_COMMAND} --test-dir "${PACKAGE_TEST_BINARY_DIR}" -C ${CMAKE_BUILD_TYPE}
    COMMAND_ERROR_IS_FATAL ANY
    COMMAND_ECHO STDOUT
)
