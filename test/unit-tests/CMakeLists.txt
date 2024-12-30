#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

find_package(Catch2 REQUIRED)
find_package(trompeloeil REQUIRED)

set(TARGET_NAME mimicpp-tests)

add_executable(${TARGET_NAME}
    "Config.cpp"
    "Expectation.cpp"
    "ExpectationBuilder.cpp"
    "InterfaceMock.cpp"
    "Matcher.cpp"
    "mimic++.cpp"
    "Mock.cpp"
    "ObjectWatcher.cpp"
    "Printer.cpp"
    "Reports.cpp"
    "Reporter.cpp"
    "Sequence.cpp"
    "Stacktrace.cpp"
    "String.cpp"
    "TypeTraits.cpp"
    "Utility.cpp"
)

add_subdirectory(call-conventions)
add_subdirectory(matchers)
add_subdirectory(policies)

target_include_directories(${TARGET_NAME}
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)

include(EnableWarnings)
include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

target_compile_options(${TARGET_NAME}
    PRIVATE
    # some test files exceed some limits
    # see: https://learn.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/fatal-error-c1128?view=msvc-170
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
    mimicpp::mimicpp
    mimicpp::internal::warnings
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil

    # required to make stacktrace work on gcc
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,14>>:stdc++_libbacktrace>
    $<$<CXX_COMPILER_ID:GNU>:stdc++exp>
)

target_precompile_headers(${TARGET_NAME}
    PRIVATE
    <catch2/catch_all.hpp>
    <catch2/trompeloeil.hpp>
)

catch_discover_tests(${TARGET_NAME})
