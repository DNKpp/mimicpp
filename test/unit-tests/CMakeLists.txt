find_package(Catch2 REQUIRED)
CPMAddPackage("gh:rollbear/trompeloeil@48")

add_executable(
	mimicpp-tests
	"Call.cpp"
	"ControlPolicy.cpp"
	"Expectation.cpp"
	"ExpectationBuilder.cpp"
	"ExpectationPolicies.cpp"
	"InterfaceMock.cpp"
	"Matcher.cpp"
	"Mock.cpp"
	"Printer.cpp"
	"Reports.cpp"
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
