#          Copyright Dominic (DNKpp) Koepke 2024-2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-adapter-tests-boost-test)
add_executable(${TARGET_NAME}
    "main.cpp"
)

include(Mimic++-EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

find_package(Boost COMPONENTS test)
target_link_libraries(${TARGET_NAME} PRIVATE
    mimicpp::header-only
    mimicpp::test::basics
    Boost::unit_test_framework
)

# @formatter:off
string(CONCAT DISABLE_C2Y_EXTENSION_WARNING
    "$<"
        "$<AND:"
            "$<CXX_COMPILER_ID:Clang>,"
            "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,22>>"
        ":-Wno-c2y-extensions>"
)
# @formatter:on

target_compile_options(${TARGET_NAME} PRIVATE
    ${DISABLE_C2Y_EXTENSION_WARNING}
)

target_precompile_headers(${TARGET_NAME} PRIVATE
    <TestAssert.hpp>
)

add_test(
    NAME boost_adapter_test
    COMMAND ${TARGET_NAME} --run_test=SuccessSuite
)

# executes test which are expected to fail
add_test(
    NAME failing_boost_adapter_test
    COMMAND ${TARGET_NAME} --run_test=FailureSuite
)
set_property(
    TEST failing_boost_adapter_test
    PROPERTY WILL_FAIL TRUE
)
