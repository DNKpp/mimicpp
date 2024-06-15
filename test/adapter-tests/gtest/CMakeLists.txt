add_executable(
	mimicpp-adapter-tests-gtest
	"main.cpp"
)

include(SetupTestTarget)
setup_test_target(mimicpp-adapter-tests-gtest)

CPMAddPackage(
	NAME GTest
	GITHUB_REPOSITORY	"google/googletest"
	GIT_TAG				"1d17ea141d2c11b8917d2c7d029f1c4e2b9769b2"
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
