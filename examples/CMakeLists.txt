#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

find_package(Catch2 REQUIRED)

add_executable(
    mimicpp-examples
    "CustomPrinter.cpp"
    "CustomString.cpp"
    "Finalizers.cpp"
    "InterfaceMock.cpp"
    "Mock.cpp"
    "Requirements.cpp"
    "Sequences.cpp"
    "SideEffects.cpp"
    "Times.cpp"
    "Watcher.cpp"
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
    OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))

    message(STATUS "${MESSAGE_PREFIX} Adding RegisterCallConvention example.")
    target_sources(mimicpp-examples PRIVATE
        "RegisterCallConvention.cpp"
    )

endif ()

include(EnableWarnings)
include(EnableSanitizers)
enable_sanitizers(mimicpp-examples)

target_link_libraries(
    mimicpp-examples
    PRIVATE
    mimicpp::mimicpp
    mimicpp::internal::warnings
    Catch2::Catch2WithMain
)

catch_discover_tests(mimicpp-examples)
