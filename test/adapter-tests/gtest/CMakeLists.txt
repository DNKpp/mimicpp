#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-adapter-tests-gtest)
add_executable(${TARGET_NAME}
	"main.cpp"
)

include(Mimic++-EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

CPMAddPackage(
	NAME GTest
	VERSION 1.15.2
	GITHUB_REPOSITORY	"google/googletest"
	EXCLUDE_FROM_ALL	YES
	SYSTEM				YES
	OPTIONS
						"gtest_force_shared_crt ON"
)

target_link_libraries(${TARGET_NAME} PRIVATE
	mimicpp::header-only
	mimicpp::test::basics
	GTest::gtest_main
)

target_precompile_headers(${TARGET_NAME} PRIVATE
	<TestAssert.hpp>
	${MIMICPP_TESTING_COMPAT_SOURCE_LOCATION_HEADER}
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
