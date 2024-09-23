find_package(Catch2 REQUIRED)

add_executable(
	mimicpp-adapter-tests-catch2
	"main.cpp"
)

include(SetupTestTarget)
setup_test_target(mimicpp-adapter-tests-catch2)

target_link_libraries(
	mimicpp-adapter-tests-catch2
	PRIVATE
	mimicpp::mimicpp
	Catch2::Catch2WithMain
)

catch_discover_tests(mimicpp-adapter-tests-catch2)
