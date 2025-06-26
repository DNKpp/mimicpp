#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_library(mimicpp-module)
add_library(mimicpp::module ALIAS mimicpp-module)

target_sources(mimicpp-module PUBLIC
    FILE_SET CXX_MODULES
    TYPE CXX_MODULES FILES mimic++.cppm
)

target_include_directories(mimicpp-module PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(mimicpp-module PUBLIC
    mimicpp::header-only
)
