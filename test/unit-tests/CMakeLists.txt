find_package(Catch2 REQUIRED)
CPMAddPackage("gh:rollbear/trompeloeil@48")

add_executable(
	mimicpp-tests
	"Call.cpp"
	"Config.cpp"
	"ControlPolicy.cpp"
	"Expectation.cpp"
	"ExpectationBuilder.cpp"
	"ExpectationPolicies.cpp"
	"InterfaceMock.cpp"
	"LifetimeWatcher.cpp"
	"Matcher.cpp"
	"Mock.cpp"
	"Printer.cpp"
	"Reports.cpp"
	"Reporter.cpp"
	"Sequence.cpp"
	"String.cpp"
	"TypeTraits.cpp"
	"Utility.cpp"
)

add_subdirectory(matchers)

include(EnableSanitizers)
enable_sanitizers(mimicpp-tests)

target_compile_options(
	mimicpp
	INTERFACE
	# some test files exceed some limits
	# see: https://learn.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/fatal-error-c1128?view=msvc-170
	"$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
)

target_link_libraries(
	mimicpp-tests
	PRIVATE
	mimicpp::mimicpp
	Catch2::Catch2WithMain
	trompeloeil::trompeloeil
	enable-warnings
)

catch_discover_tests(mimicpp-tests)
