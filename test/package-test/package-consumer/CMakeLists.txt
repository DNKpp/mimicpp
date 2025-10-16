#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.15...3.31)

project(package-consumer
    LANGUAGES CXX
)

message(DEBUG "MIMICPP_PACKAGE_PREFIX: ${MIMICPP_PACKAGE_PREFIX}")
set(TARGET_NAME package-consumer)
add_executable(${TARGET_NAME}
    "main.cpp"
)

find_package(mimicpp REQUIRED
    PATHS ${MIMICPP_PACKAGE_PREFIX}
)
target_link_libraries(${TARGET_NAME} PRIVATE
    mimicpp::mimicpp
)

include(CTest)
add_test(NAME package-test
    COMMAND ${TARGET_NAME}
)
