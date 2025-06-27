#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-test-basics)

add_library(${TARGET_NAME} INTERFACE)
add_library(mimicpp::test::basics ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME} INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)

target_compile_options(${TARGET_NAME} INTERFACE
    # some test files exceed some limits
    # see: https://learn.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/fatal-error-c1128?view=msvc-170
    "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
    "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>"
)

target_compile_definitions(${TARGET_NAME} INTERFACE
    "LIBASSERT_BREAK_ON_FAIL"
)

include(Mimic++-EnableAdditionalTestFlags)
include(Mimic++-EnableWarnings)
include(LinkStdStacktrace)
find_package(libassert REQUIRED)
target_link_libraries(${TARGET_NAME} INTERFACE
    mimicpp::internal::additional-test-flags
    mimicpp::internal::warnings
    mimicpp::internal::link-std-stacktrace
    libassert::assert
)

option(MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION "" OFF)
message(DEBUG "${MESSAGE_PREFIX} MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION: ${MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION}")
if (MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION)

    set(SOURCE_LOCATION_BUILD_TESTS OFF)
    CPMAddPackage("gh:paweldac/source_location@0.4")

    target_link_libraries(${TARGET_NAME} INTERFACE
        nostd::source_location
    )

    target_compile_definitions(${TARGET_NAME} INTERFACE
        MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION
    )

    set(MIMICPP_TESTING_COMPAT_SOURCE_LOCATION_HEADER "\"mimic++_ext/compatibility/paweldac-source_location.hpp\"" CACHE STRING "" FORCE)
endif ()
