#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-adapter-tests-doctest)
add_executable(${TARGET_NAME}
	"main.cpp"
)

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

CPMAddPackage("gh:doctest/doctest@2.4.11")
target_link_libraries(${TARGET_NAME} PRIVATE
	mimicpp::header-only
	mimicpp::test::basics
	doctest::doctest
)

target_compile_options(${TARGET_NAME} PRIVATE
	"$<$<CXX_COMPILER_ID:Clang>:-Wno-#warnings>"
)

target_precompile_headers(${TARGET_NAME} PRIVATE
	<TestAssert.hpp>
	$<$<BOOL:${MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION}>:\"mimic++/compatibility/paweldac-source_location.hpp\">
)

list(APPEND CMAKE_MODULE_PATH "${doctest_SOURCE_DIR}/scripts/cmake")
include(doctest)
doctest_discover_tests(${TARGET_NAME})
