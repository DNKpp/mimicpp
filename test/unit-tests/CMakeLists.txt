#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-tests)

add_executable(${TARGET_NAME}
    "Config.cpp"
    "Expectation.cpp"
    "ExpectationBuilder.cpp"
    "InterfaceMock.cpp"
    "mimic++.cpp"
    "Mock.cpp"
    "ObjectWatcher.cpp"
    "Sequence.cpp"
    "Stacktrace.cpp"
    "String.cpp"
    "TypeTraits.cpp"
)

add_subdirectory(call-conventions)
add_subdirectory(matchers)
add_subdirectory(policies)
add_subdirectory(printing)
add_subdirectory(reporting)
add_subdirectory(utilities)

target_include_directories(${TARGET_NAME}
    PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

target_compile_options(${TARGET_NAME}
    PRIVATE
    # some test files exceed some limits
    # see: https://learn.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/fatal-error-c1128?view=msvc-170
    "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
)

include(EnableAdditionalTestFlags)
include(EnableWarnings)
include(LinkStdStacktrace)
find_package(Catch2 REQUIRED)
find_package(trompeloeil REQUIRED)
target_link_libraries(${TARGET_NAME}
    PRIVATE
    mimicpp::mimicpp
    mimicpp::internal::additional-test-flags
    mimicpp::internal::warnings
    mimicpp::internal::link-std-stacktrace
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)

target_precompile_headers(${TARGET_NAME}
    PRIVATE
    <catch2/catch_all.hpp>
    <catch2/trompeloeil.hpp>
)

catch_discover_tests(${TARGET_NAME})
