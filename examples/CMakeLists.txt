CPMAddPackage("gh:catchorg/Catch2@3.5.4")
include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")

add_executable(
	mimicpp-examples
	"Finalizers.cpp"
	"Requirements.cpp"
	"Sequences.cpp"
	"SideEffects.cpp"
)

include(SetupTestTarget)
setup_test_target(mimicpp-examples)

target_link_libraries(
	mimicpp-examples
	PRIVATE
	mimicpp::mimicpp
	Catch2::Catch2WithMain
)

target_compile_features(
	mimicpp
	INTERFACE
	cxx_std_${CMAKE_CXX_STANDARD}
)

catch_discover_tests(mimicpp-examples)
