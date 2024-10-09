find_package(Catch2 REQUIRED)

add_executable(
	mimicpp-adapter-tests-catch2
	"main.cpp"
)

include(EnableSanitizers)
enable_sanitizers(mimicpp-adapter-tests-catch2)

target_link_libraries(
	mimicpp-adapter-tests-catch2
	PRIVATE
	mimicpp::mimicpp
	mimicpp::internal::warnings
	Catch2::Catch2WithMain
)

catch_discover_tests(mimicpp-adapter-tests-catch2)
