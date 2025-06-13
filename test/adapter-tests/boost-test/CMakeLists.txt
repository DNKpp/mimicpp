#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-adapter-tests-boost-test)
add_executable(${TARGET_NAME}
    "main.cpp"
)

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

set(BOOST_INCLUDE_LIBRARIES "test")
CPMAddPackage(
    NAME Boost
    URL "https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.tar.gz"
    URL_HASH "SHA256=dcea50f40ba1ecfc448fdf886c0165cf3e525fef2c9e3e080b9804e8117b9694"
    VERSION 1.88.0
    EXCLUDE_FROM_ALL YES
    SYSTEM YES
)

target_link_libraries(${TARGET_NAME}
    PRIVATE
    mimicpp::mimicpp
    mimicpp::test::basics
    Boost::unit_test_framework
)

target_precompile_headers(${TARGET_NAME}
    PRIVATE
    <TestAssert.hpp>
    $<$<BOOL:${MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION}>:\"mimic++/compatibility/paweldac-source_location.hpp\">
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
