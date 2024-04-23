CPMAddPackage("gh:catchorg/Catch2@3.5.4")
CPMAddPackage("gh:rollbear/trompeloeil@47")
include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")

add_executable(
	mimicpp-tests
	"Call.cpp"
	#"ComplexMock.cpp"
	"Expectation.cpp"
	"ExpectationBuilder.cpp"
	"ExpectationPolicies.cpp"
	"Matcher.cpp"
	"Mock.cpp"
	"Printer.cpp"
	"TypeTraits.cpp"
)

if (MSVC)

	# When using github ci clang-cl, exceptions seems to be disabled by default.
	target_compile_options(
		mimicpp-tests
		PRIVATE
		/EHsc
	)

endif()

if (SANITIZE_ADDRESS)

	# workaround linker errors on msvc
	# see: https://learn.microsoft.com/en-us/answers/questions/864574/enabling-address-sanitizer-results-in-error-lnk203
	target_compile_definitions(
		mimicpp-tests
		PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:_DISABLE_VECTOR_ANNOTATION>
		$<$<CXX_COMPILER_ID:MSVC>:_DISABLE_STRING_ANNOTATION>
	)

endif()

add_sanitizers(mimicpp-tests)

target_link_libraries(
	mimicpp-tests
	PRIVATE
	mimicpp::mimicpp
	Catch2::Catch2WithMain
	trompeloeil::trompeloeil
)

target_compile_features(
	mimicpp
	INTERFACE
	cxx_std_${CMAKE_CXX_STANDARD}
)

catch_discover_tests(mimicpp-tests)
