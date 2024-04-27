CPMAddPackage("gh:catchorg/Catch2@3.5.4")
include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")

add_executable(
	mimicpp-examples
	"Finalizers.cpp"
	"SideEffects.cpp"
)
if (MSVC)

	# When using github ci, exceptions seems to be disabled by default.
	target_compile_options(
		mimicpp-examples
		PRIVATE
		/EHsc
	)

endif()

if (SANITIZE_ADDRESS)

	# workaround linker errors on msvc
	# see: https://learn.microsoft.com/en-us/answers/questions/864574/enabling-address-sanitizer-results-in-error-lnk203
	target_compile_definitions(
		mimicpp-examples
		PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:_DISABLE_VECTOR_ANNOTATION>
		$<$<CXX_COMPILER_ID:MSVC>:_DISABLE_STRING_ANNOTATION>
	)

endif()

add_sanitizers(mimicpp-tests)

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
