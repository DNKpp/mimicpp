#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(get_cpm)

# fixes reporting in clion
set(CATCH_CONFIG_CONSOLE_WIDTH 800 CACHE STRING "")

CPMAddPackage("gh:catchorg/Catch2@3.7.1")

if (Catch2_ADDED)
	include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")
endif()
