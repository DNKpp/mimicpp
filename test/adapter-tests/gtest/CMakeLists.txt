#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-adapter-tests-gtest)
add_executable(${TARGET_NAME}
	"main.cpp"
)

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

CPMAddPackage(
	NAME GTest
	GITHUB_REPOSITORY	"google/googletest"
	GIT_TAG				"v1.15.2"
	EXCLUDE_FROM_ALL	YES
	SYSTEM				YES
	OPTIONS
						"gtest_force_shared_crt ON"
)

include(EnableAdditionalTestFlags)
include(EnableWarnings)
target_link_libraries(${TARGET_NAME}
	PRIVATE
	mimicpp::mimicpp
	mimicpp::internal::additional-test-flags
	mimicpp::internal::warnings
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
