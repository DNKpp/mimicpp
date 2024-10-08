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
	Catch2::Catch2WithMain
	enable-warnings
)

catch_discover_tests(mimicpp-adapter-tests-catch2)
