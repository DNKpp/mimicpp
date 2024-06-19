add_executable(
	mimicpp-adapter-tests-boost-test
	"main.cpp"
)

include(SetupTestTarget)
setup_test_target(mimicpp-adapter-tests-boost-test)

CPMAddPackage(
	NAME Boost
	URL	"https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.zip"
	VERSION 1.85.0
	SYSTEM YES
)

target_link_libraries(
	mimicpp-adapter-tests-boost-test
	PRIVATE
	mimicpp::mimicpp
	Boost::unit_test_framework
)

add_test(
	NAME boost_adapter_test
	COMMAND mimicpp-adapter-tests-boost-test --run_test=SuccessSuite
)

# executes test which are expected to fail
add_test(
	NAME failing_boost_adapter_test
	COMMAND mimicpp-adapter-tests-boost-test --run_test=FailureSuite
)
set_property(
	TEST failing_boost_adapter_test
	PROPERTY WILL_FAIL TRUE
)
