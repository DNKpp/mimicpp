#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

function(read_project_version FILE_PATH OUT_VERSION)

    file(READ ${FILE_PATH} FILE_CONTENT)
    if (NOT FILE_CONTENT)
        message(FATAL_ERROR "${MESSAGE_PREFIX} read_project_version failed - Unable to read file from: ${FILE_PATH}")
    endif ()

    set(VERSION_REGEX "#define[ \t]+MIMICPP_VERSION[ \t]+([0-9]+)")
    string(REGEX MATCH ${VERSION_REGEX} _ ${FILE_CONTENT})
    set(TOKEN ${CMAKE_MATCH_1})

    if (NOT TOKEN VERSION_GREATER 0)
        message(FATAL_ERROR "${MESSAGE_PREFIX} read_project_version failed - Unable to read version.")
    endif ()

    message(DEBUG "${MESSAGE_PREFIX} read_project_version succeeded - Version is ${TOKEN}.")
    set(${OUT_VERSION} ${TOKEN} PARENT_SCOPE)

endfunction()
