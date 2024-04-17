CPMAddPackage("gh:catchorg/Catch2@3.5.4")
CPMAddPackage("gh:rollbear/trompeloeil@47")
include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")

add_executable(
	mimicpp-tests
	"Call.cpp"
	"CallProperties.cpp"
	"Expectation.cpp"
	"Printer.cpp"
	"TypeTraits.cpp"
)

target_link_libraries(
	mimicpp-tests
	PRIVATE
	mimicpp::mimicpp
	Catch2::Catch2WithMain
	trompeloeil::trompeloeil
)

catch_discover_tests(mimicpp-tests)
