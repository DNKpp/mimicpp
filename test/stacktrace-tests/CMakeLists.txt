#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

find_package(Catch2 REQUIRED)
add_subdirectory("custom")

function(create_stacktrace_test BACKEND LIBS)
    set(TARGET_NAME mimicpp-stacktrace-${BACKEND}-tests)

    add_executable(${TARGET_NAME}
        "test.cpp"
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
        mimicpp::header-only
        mimicpp::test::basics
        mimicpp::internal::enable-${BACKEND}
        Catch2::Catch2WithMain
        ${LIBS}
    )

    target_precompile_headers(${TARGET_NAME} PRIVATE
        <TestAssert.hpp>
        ${MIMICPP_TESTING_COMPAT_SOURCE_LOCATION_HEADER}
        <catch2/catch_all.hpp>
    )

    catch_discover_tests(${TARGET_NAME})
endfunction()

include(Mimic++-HasStdStacktrace)
include(Mimic++-InternalTargets)

if (HAS_STD_STACKTRACE)
    message(DEBUG "${MESSAGE_PREFIX} adding std-stacktrace test.")
    create_stacktrace_test(std-stacktrace "")
else ()
    message(DEBUG "${MESSAGE_PREFIX} skipping std-stacktrace test.")
endif ()

find_package(cpptrace REQUIRED)
create_stacktrace_test(cpptrace cpptrace::cpptrace)

find_package(Boost REQUIRED COMPONENTS stacktrace)
create_stacktrace_test(boost-stacktrace Boost::stacktrace)

