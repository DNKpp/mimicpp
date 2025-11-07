#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-unicode-str-matcher-tests)

add_executable(${TARGET_NAME}
    "StringContains.cpp"
    "../unit-tests/matchers/StringContains.cpp"
)

include(Mimic++-EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

find_package(Catch2 REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE
    mimicpp::header-only
    mimicpp::test::basics
    Catch2::Catch2WithMain
)

target_precompile_headers(${TARGET_NAME} PRIVATE
    <TestAssert.hpp>
    "../unit-tests/Catch2FallbackStringifier.hpp"
    <catch2/catch_all.hpp>
)

catch_discover_tests(${TARGET_NAME})
