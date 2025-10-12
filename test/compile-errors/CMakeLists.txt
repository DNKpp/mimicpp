#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(CTest)

function(read_compile_error_definitions SOURCE_FILE OUT_ERROR_DEFINITIONS)
    file(READ ${SOURCE_FILE} FILE_CONTENT)
    if (NOT FILE_CONTENT)
        message(FATAL_ERROR "${MESSAGE_PREFIX} Unable to read source-file: `${SOURCE_FILE}`")
    endif ()
    # matches everything between the tags and also newlines
    set(BEGIN_TOKEN "<begin-expected-compile-error>")
    string(LENGTH "${BEGIN_TOKEN}" BEGIN_TOKEN_LENGTH)
    string(FIND "${FILE_CONTENT}" "${BEGIN_TOKEN}" DEF_BEGIN)
    string(FIND "${FILE_CONTENT}" "<end-expected-compile-error>" DEF_END)
    math(EXPR ERROR_START "${DEF_BEGIN} + ${BEGIN_TOKEN_LENGTH}")
    math(EXPR ERROR_LENGTH "${DEF_END} - ${ERROR_START}")
    string(SUBSTRING "${FILE_CONTENT}" ${ERROR_START} ${ERROR_LENGTH} ERROR_DEFINITIONS)
    string(STRIP "${ERROR_DEFINITIONS}" ERROR_DEFINITIONS)

    if (NOT ERROR_DEFINITIONS)
        message(FATAL_ERROR "${MESSAGE_PREFIX} Could not read compile-error definition(s) from `${SOURCE_FILE}`.")
    endif ()
    message(DEBUG "${MESSAGE_PREFIX} Expected compile-errors for case `${SOURCE_FILE}` are:\n${ERROR_DEFINITIONS}")

    set(${OUT_ERROR_DEFINITIONS} ${ERROR_DEFINITIONS} PARENT_SCOPE)
endfunction()

function(check_compile_error SOURCE_FILE)
    cmake_path(GET SOURCE_FILE STEM NAME)
    set(TARGET_NAME mimicpp-compile-error-${NAME})
    set(TEST_NAME compile-error-${NAME})
    read_compile_error_definitions(${SOURCE_FILE} EXPECTED_COMPILE_ERRORS)

    add_library(${TARGET_NAME} EXCLUDE_FROM_ALL
        ${SOURCE_FILE}
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
        mimicpp::header-only
    )

    add_test(NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${TARGET_NAME}
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        PASS_REGULAR_EXPRESSION "${EXPECTED_COMPILE_ERRORS}"
    )
endfunction()

check_compile_error("missing-finalizer-for-non-void.cpp")
