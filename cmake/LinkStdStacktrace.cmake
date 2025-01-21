#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET link-std-stacktrace)

    add_library(link-std-stacktrace INTERFACE)
    add_library(mimicpp::internal::link-std-stacktrace ALIAS link-std-stacktrace)

    # @formatter:off
    string(CONCAT GCC_LINK_LIBBACKTRACE
        "$<"
            "$<AND:"
                "$<PLATFORM_ID:Linux>,"
                "$<CXX_COMPILER_ID:GNU>,"
                "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,13>,"
                "$<VERSION_LESS:$<CXX_COMPILER_VERSION>,14>>"
            ":stdc++_libbacktrace>"
    )
    string(CONCAT GCC_LINK_EXP
        "$<"
            "$<AND:"
                "$<PLATFORM_ID:Linux>,"
                "$<CXX_COMPILER_ID:GNU>,"
                "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,14>>"
            ":stdc++exp>"
    )
    string(CONCAT CLANG_LINK_LIBBACKTRACE
        "$<"
            "$<AND:"
                "$<PLATFORM_ID:Linux>,"
                "$<CXX_COMPILER_ID:Clang>,"
                "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,17>,"
                "$<VERSION_LESS:$<CXX_COMPILER_VERSION>,19>>"
            ":stdc++_libbacktrace>"
    )
    string(CONCAT CLANG_LINK_EXP
        "$<"
            "$<AND:"
                "$<PLATFORM_ID:Linux>,"
                "$<CXX_COMPILER_ID:Clang>,"
                "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,19>>"
            ":stdc++exp>"
    )
    # @formatter:on

    target_link_libraries(link-std-stacktrace
        INTERFACE
        ${GCC_LINK_LIBBACKTRACE}
        ${GCC_LINK_EXP}
        ${CLANG_LINK_LIBBACKTRACE}
        ${CLANG_LINK_EXP}
    )

endif ()
