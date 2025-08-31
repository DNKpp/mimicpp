#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

# This separate executable is required, because in this case the ``mimicpp::find_stacktrace_backend``
# trait is specialized. As this shall only be for the duration of these tests, I've extracted it
# into a separate test-executable.
# see: https://eel.is/c++draft/temp.spec#general-5 for the technical reason

set(TARGET_NAME mimicpp-stacktrace-custom-tests)
add_executable(${TARGET_NAME}
    "CustomStacktrace.cpp"
    "EqualityComparison.cpp"
    "Printing.cpp"
)

include(Mimic++-EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

find_package(Catch2 REQUIRED)
find_package(trompeloeil REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE
    mimicpp::header-only
    mimicpp::test::basics
    mimicpp::internal::enable-custom-stacktrace
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)

target_precompile_headers(${TARGET_NAME} PRIVATE
    <TestAssert.hpp>
    <catch2/catch_all.hpp>
    <catch2/trompeloeil.hpp>
)

catch_discover_tests(${TARGET_NAME})
