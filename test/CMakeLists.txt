#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_subdirectory("unit-tests")

option(MIMICPP_ENABLE_ADAPTER_TESTS "Determines, whether the adapter tests shall be built." OFF)
if (MIMICPP_ENABLE_ADAPTER_TESTS)
	add_subdirectory("adapter-tests")
endif()
