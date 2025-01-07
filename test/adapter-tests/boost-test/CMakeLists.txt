#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME mimicpp-adapter-tests-boost-test)
add_executable(${TARGET_NAME}
	"main.cpp"
)

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

if (WIN32)
	set(BOOST_ARCHIVE_URL "https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.zip")
else()
	set(BOOST_ARCHIVE_URL "https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.tar.gz")
endif()

CPMAddPackage(
	NAME				Boost
	URL					${BOOST_ARCHIVE_URL}
	VERSION				1.85.0
	EXCLUDE_FROM_ALL	YES
	SYSTEM				YES
)

include(EnableAdditionalTestFlags)
include(EnableWarnings)
target_link_libraries(${TARGET_NAME}
	PRIVATE
	mimicpp::mimicpp
	mimicpp::internal::additional-test-flags
	mimicpp::internal::warnings
	Boost::unit_test_framework
)

add_test(
	NAME boost_adapter_test
	COMMAND ${TARGET_NAME} --run_test=SuccessSuite
)

# executes test which are expected to fail
add_test(
	NAME failing_boost_adapter_test
	COMMAND ${TARGET_NAME} --run_test=FailureSuite
)
set_property(
	TEST failing_boost_adapter_test
	PROPERTY WILL_FAIL TRUE
)
