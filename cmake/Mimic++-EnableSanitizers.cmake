#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

function(enable_sanitizers TARGET_NAME)
	find_package(sanitizers-cmake)

	if (SANITIZE_ADDRESS)
		# workaround linker errors on msvc
		# see: https://learn.microsoft.com/en-us/answers/questions/864574/enabling-address-sanitizer-results-in-error-lnk203
		target_compile_definitions(${TARGET_NAME}
			PRIVATE
			"$<$<CXX_COMPILER_ID:MSVC>:_DISABLE_VECTOR_ANNOTATION;_DISABLE_STRING_ANNOTATION>"
		)

		target_compile_options(${TARGET_NAME}
			PRIVATE
			# see: https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html#index-fsanitize_003dbuiltin
			"$<$<CXX_COMPILER_ID:GNU>:-g;-fno-omit-frame-pointer;-fno-optimize-sibling-calls;-fno-ipa-icf>"
			# see: https://clang.llvm.org/docs/AddressSanitizer.html#id3
			"$<$<CXX_COMPILER_ID:Clang>:-g;-fno-omit-frame-pointer;-fno-optimize-sibling-calls>"
		)
	endif()

	add_sanitizers(${TARGET_NAME})
endfunction()