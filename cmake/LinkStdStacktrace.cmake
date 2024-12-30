#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET link-std-stacktrace)

    add_library(link-std-stacktrace INTERFACE)
    add_library(mimicpp::internal::link-std-stacktrace ALIAS link-std-stacktrace)

    string(CONCAT GCC_LINK_LIBBACKTRACE
        "$<"
        "$<AND:"
        "$<CXX_COMPILER_ID:GNU>,"
        "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,13>,"
        "$<VERSION_LESS:$<CXX_COMPILER_VERSION>,14>>"
        ":stdc++_libbacktrace>"
    )
    message(DEBUG "${MESSAGE_PREFIX} GCC_LINK_LIBBACKTRACE: ${GCC_LINK_LIBBACKTRACE}")
    string(CONCAT GCC_LINK_EXP
        "$<"
        "$<AND:"
        "$<CXX_COMPILER_ID:GNU>,"
        "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,14>>"
        ":stdc++exp>"
    )
    message(DEBUG "${MESSAGE_PREFIX} GCC_LINK_EXP: ${GCC_LINK_EXP}")

    target_link_libraries(link-std-stacktrace
        INTERFACE
        # required to make stacktrace work on gcc
        ${GCC_LINK_LIBBACKTRACE}
        ${GCC_LINK_EXP}
    )

endif ()
