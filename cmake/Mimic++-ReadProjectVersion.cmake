#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

function(mimicpp_read_project_version FILE_PATH OUT_VERSION)

    file(READ ${FILE_PATH} FILE_CONTENT)
    if (NOT FILE_CONTENT)
        message(FATAL_ERROR "${MESSAGE_PREFIX} mimicpp_read_project_version failed - Unable to read file from: ${FILE_PATH}")
    endif ()

    set(VERSION_MAJOR_REGEX "#define[ \t]+MIMICPP_VERSION_MAJOR[ \t]+([0-9]+)")
    string(REGEX MATCH ${VERSION_MAJOR_REGEX} _ ${FILE_CONTENT})
    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    if (NOT VERSION_MAJOR GREATER_EQUAL 0)
        message(FATAL_ERROR "${MESSAGE_PREFIX} mimicpp_read_project_version failed - Major version not found.")
    endif ()

    set(VERSION_MINOR_REGEX "#define[ \t]+MIMICPP_VERSION_MINOR[ \t]+([0-9]+)")
    string(REGEX MATCH ${VERSION_MINOR_REGEX} _ ${FILE_CONTENT})
    set(VERSION_MINOR ${CMAKE_MATCH_1})
    if (NOT VERSION_MINOR GREATER_EQUAL 0)
        message(FATAL_ERROR "${MESSAGE_PREFIX} mimicpp_read_project_version failed - Minor version not found.")
    endif ()

    set(VERSION_PATCH_REGEX "#define[ \t]+MIMICPP_VERSION_PATCH[ \t]+([0-9]+)")
    string(REGEX MATCH ${VERSION_PATCH_REGEX} _ ${FILE_CONTENT})
    set(VERSION_PATCH ${CMAKE_MATCH_1})
    if (NOT VERSION_PATCH GREATER_EQUAL 0)
        message(FATAL_ERROR "${MESSAGE_PREFIX} mimicpp_read_project_version failed - Patch version not found.")
    endif ()

    set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    if (NOT VERSION VERSION_GREATER 0)
        message(FATAL_ERROR "${MESSAGE_PREFIX} mimicpp_read_project_version failed - Unable to read version.")
    endif ()

    message(DEBUG "${MESSAGE_PREFIX} mimicpp_read_project_version succeeded - Version is ${VERSION}.")
    set(${OUT_VERSION} ${VERSION} PARENT_SCOPE)

endfunction()
