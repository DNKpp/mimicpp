find_package(Catch2 REQUIRED)

find_package(sanitizers-cmake REQUIRED)

add_executable(
	mimicpp-examples
	"CustomPrinter.cpp"
	"Finalizers.cpp"
	"InterfaceMock.cpp"
	"Mock.cpp"
	"Requirements.cpp"
	"Sequences.cpp"
	"SideEffects.cpp"
	"Times.cpp"
	"Watcher.cpp"
)

include(EnableSanitizers)
enable_sanitizers(mimicpp-examples)

target_link_libraries(
	mimicpp-examples
	PRIVATE
	enable-warnings
	mimicpp::mimicpp
	Catch2::Catch2WithMain
)

catch_discover_tests(mimicpp-examples)
