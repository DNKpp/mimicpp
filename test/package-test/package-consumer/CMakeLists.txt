#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.15...3.31)

project(package-consumer
    LANGUAGES CXX
)

message(DEBUG "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(DEBUG "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
message(DEBUG "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
set(TARGET_NAME package-consumer)
add_executable(${TARGET_NAME}
    "main.cpp"
)

if (MIMICPP_WITH_STACKTRACE STREQUAL "cpptrace")
    find_package(mimicpp-cpptrace MODULE REQUIRED)
endif ()

if (MIMICPP_WITH_FMT)
    find_package(mimicpp-fmt MODULE REQUIRED)
endif ()

if (MIMICPP_WITH_UNICODE_STR_MATCHER)
    find_package(mimicpp-uni-algo MODULE REQUIRED)
endif ()

include("../../cmake/Mimic++-LinkStdStacktrace.cmake")
find_package(mimicpp REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE
    mimicpp::mimicpp
    mimicpp::internal::link-std-stacktrace
)

include(CTest)
add_test(NAME package-test
    COMMAND ${TARGET_NAME}
)
