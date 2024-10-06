add_executable(
	mimicpp-adapter-tests-gtest
	"main.cpp"
)

include(SetupTestTarget)
setup_test_target(mimicpp-adapter-tests-gtest)

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
	GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(mimicpp-adapter-tests-gtest)
