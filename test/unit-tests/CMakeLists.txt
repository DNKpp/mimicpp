CPMAddPackage("gh:catchorg/Catch2@3.6.0")
CPMAddPackage("gh:rollbear/trompeloeil@47")
include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")

add_executable(
	mimicpp-tests
	"Call.cpp"
	"Expectation.cpp"
	"ExpectationBuilder.cpp"
	"ExpectationPolicies.cpp"
	"Matcher.cpp"
	"Mock.cpp"
	"Printer.cpp"
	"Reporter.cpp"
	"Sequence.cpp"
	"TypeTraits.cpp"
	"Utility.cpp"
)

include(SetupTestTarget)
setup_test_target(mimicpp-tests)

target_link_libraries(
	mimicpp-tests
	PRIVATE
	mimicpp::mimicpp
	Catch2::Catch2WithMain
	trompeloeil::trompeloeil
)

catch_discover_tests(mimicpp-tests)
