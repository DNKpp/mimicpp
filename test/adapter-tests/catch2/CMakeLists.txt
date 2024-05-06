find_package(Catch2 REQUIRED)
include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")

add_executable(
	mimicpp-adapter-tests-catch2
	"main.cpp"
)

target_link_libraries(
	mimicpp-adapter-tests-catch2
	PRIVATE
	mimicpp::mimicpp
	Catch2::Catch2WithMain
)

target_compile_features(
	mimicpp
	INTERFACE
	cxx_std_${CMAKE_CXX_STANDARD}
)

catch_discover_tests(mimicpp-adapter-tests-catch2)
