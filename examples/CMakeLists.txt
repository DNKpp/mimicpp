#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-examples)
add_executable(${TARGET_NAME}
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
    "VariadicMocks.cpp"
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
    OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))

    message(DEBUG "${MESSAGE_PREFIX} Adding RegisterCallConvention example.")
    target_sources(${TARGET_NAME}
        PRIVATE
        "RegisterCallConvention.cpp"
    )

endif ()

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

include(EnableAdditionalTestFlags)
include(EnableWarnings)
include(LinkStdStacktrace)
find_package(Catch2 REQUIRED)
target_link_libraries(${TARGET_NAME}
    PRIVATE
    mimicpp::mimicpp
    mimicpp::internal::additional-test-flags
    mimicpp::internal::warnings
    mimicpp::internal::link-std-stacktrace
    Catch2::Catch2WithMain
)

catch_discover_tests(${TARGET_NAME})
