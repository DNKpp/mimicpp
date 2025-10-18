#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

function(collect_cmake_env outConfigArgs outBuildArgs)
    set(configArgs "")
    set(buildArgs "")

    list(APPEND configArgs "-D CMAKE_BUILD_TYPE=$<CONFIG>")
    list(APPEND configArgs "-D MIMICPP_WITH_FMT=${MIMICPP_CONFIG_USE_FMT}")
    list(APPEND configArgs "-D MIMICPP_WITH_UNICODE_STR_MATCHER=${MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER}")
    list(APPEND configArgs "-D MIMICPP_WITH_STACKTRACE=${MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE}")

    if(MSVC)
        list(APPEND buildArgs "--config $<CONFIG>")
    endif ()

    set(${outConfigArgs} ${configArgs} PARENT_SCOPE)
    set(${outBuildArgs} ${buildArgs} PARENT_SCOPE)
endfunction()

collect_cmake_env(CONFIG_ARGS BUILD_ARGS)
# We propagate `CONFIG_ARGS` directly to the script and also pass it as literal string
add_test(NAME package-test
    COMMAND ${CMAKE_COMMAND}
        ${CONFIG_ARGS}
        -D CONFIG_ARGS=[[${CONFIG_ARGS}]]
        -D BUILD_ARGS=[[${BUILD_ARGS}]]
        -D CUR_BINARY_DIR=${CMAKE_BINARY_DIR}
        -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
        -D TEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -D TEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/test-run
        -P "${CMAKE_CURRENT_SOURCE_DIR}/RunTest.cmake"
)
