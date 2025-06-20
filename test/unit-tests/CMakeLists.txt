#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-tests)

add_executable(${TARGET_NAME}
    "Expectation.cpp"
    "ExpectationBuilder.cpp"
    "InterfaceMock.cpp"
    "mimic++.cpp"
    "Mock.cpp"
    "ObjectWatcher.cpp"
    "Sequence.cpp"
    "ScopedSequence.cpp"
    "Stacktrace.cpp"
    "String.cpp"
    "TypeTraits.cpp"
)

add_subdirectory(call-conventions)
add_subdirectory(config)
add_subdirectory(matchers)
add_subdirectory(policies)
add_subdirectory(printing)
add_subdirectory(reporting)
add_subdirectory(utilities)

target_include_directories(${TARGET_NAME} PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

find_package(Catch2 REQUIRED)
find_package(trompeloeil REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE
    mimicpp::header-only
    mimicpp::test::basics
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)

target_precompile_headers(${TARGET_NAME} PRIVATE
    <TestAssert.hpp>
    $<$<BOOL:${MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION}>:\"mimic++/compatibility/paweldac-source_location.hpp\">
    "Catch2FallbackStringifier.hpp"
    <catch2/catch_all.hpp>
    <catch2/trompeloeil.hpp>
)

catch_discover_tests(${TARGET_NAME})
