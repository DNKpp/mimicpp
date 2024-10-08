#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#         https://www.boost.org/LICENSE_1_0.txt)


function(setup_test_target TARGET_NAME)

	find_package(sanitizers-cmake)

	if (SANITIZE_ADDRESS)

		# workaround linker errors on msvc
		# see: https://learn.microsoft.com/en-us/answers/questions/864574/enabling-address-sanitizer-results-in-error-lnk203
		target_compile_definitions(
			${TARGET_NAME}
			PRIVATE
			$<$<CXX_COMPILER_ID:MSVC>:_DISABLE_VECTOR_ANNOTATION>
			$<$<CXX_COMPILER_ID:MSVC>:_DISABLE_STRING_ANNOTATION>
		)

	endif()

	add_sanitizers(${TARGET_NAME})

	# We need to circumvent the huge nonsense warnings from clang-cl
	# see: https://discourse.cmake.org/t/wall-with-visual-studio-clang-toolchain-results-in-way-too-many-warnings/7927
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
		AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")

		set(WARNING_FLAGS /W4 -Wextra -Wpedantic -Werror -Wno-unknown-attributes)

	else()

		set(WARNING_FLAGS
			$<IF:$<CXX_COMPILER_ID:MSVC>,
				/W4 /WX,
				-Wall -Wextra -Wpedantic -Werror>
		)

	endif()

	target_compile_options(
		${TARGET_NAME}
		PRIVATE
		${WARNING_FLAGS}
	)

endfunction()