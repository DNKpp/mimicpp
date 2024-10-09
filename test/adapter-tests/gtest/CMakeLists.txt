#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

add_executable(
	mimicpp-adapter-tests-gtest
	"main.cpp"
)

include(EnableWarnings)
include(EnableSanitizers)
enable_sanitizers(mimicpp-adapter-tests-gtest)

CPMAddPackage(
	NAME GTest
	GITHUB_REPOSITORY	"google/googletest"
	GIT_TAG				"v1.15.2"
	EXCLUDE_FROM_ALL	YES
	SYSTEM				YES
	OPTIONS
						"gtest_force_shared_crt ON"
)

target_link_libraries(
	mimicpp-adapter-tests-gtest
	PRIVATE
	mimicpp::mimicpp
	mimicpp::internal::warnings
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(mimicpp-adapter-tests-gtest)
