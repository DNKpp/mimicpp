find_package(Catch2 REQUIRED)

find_package(sanitizers-cmake REQUIRED)

add_executable(
	mimicpp-examples
	"Mock.cpp"
	"Finalizers.cpp"
	"Requirements.cpp"
	"Sequences.cpp"
	"SideEffects.cpp"
	"Times.cpp"
	"CustomPrinter.cpp"
)

include(SetupTestTarget)
setup_test_target(mimicpp-examples)

target_link_libraries(
	mimicpp-examples
	PRIVATE
	mimicpp::mimicpp
	Catch2::Catch2WithMain
)

catch_discover_tests(mimicpp-examples)
