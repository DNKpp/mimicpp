#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET enable-config-options)

	add_library(enable-config-options INTERFACE)
	add_library(mimicpp::internal::config-options ALIAS enable-config-options)

	OPTION(MIMICPP_CONFIG_DISABLE_PRETTY_TYPE_PRINTING "Disables pretty type-printing." OFF)
	message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_DISABLE_PRETTY_TYPE_PRINTING: ${MIMICPP_CONFIG_DISABLE_PRETTY_TYPE_PRINTING}")
	OPTION(MIMICPP_CONFIG_ONLY_PREFIXED_MACROS "When enabled, all macros will be prefixed with MIMICPP_." OFF)
	message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_ONLY_PREFIXED_MACROS: ${MIMICPP_CONFIG_ONLY_PREFIXED_MACROS}")
	OPTION(
		MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION
		"When enabled, catch2 matchers integration will be enabled, if catch2 adapter is used (experimental)."
		OFF
	)
	message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION: ${MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION}")

	target_compile_definitions(
		enable-config-options
		INTERFACE
		$<$<BOOL:${MIMICPP_CONFIG_DISABLE_PRETTY_TYPE_PRINTING}>:MIMICPP_CONFIG_DISABLE_PRETTY_TYPE_PRINTING>
		$<$<BOOL:${MIMICPP_CONFIG_ONLY_PREFIXED_MACROS}>:MIMICPP_CONFIG_ONLY_PREFIXED_MACROS>
		$<$<BOOL:${MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION}>:MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION>
	)

	# Config option, to utilize fmt instead of std formatting.
	# Checks, whether fmt is already available. Fetches it instead.
	# Eventually defines the macro MIMICPP_CONFIG_USE_FMT.
	OPTION(MIMICPP_CONFIG_USE_FMT "When enabled, uses fmt instead of std formatting." OFF)
	message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_USE_FMT: ${MIMICPP_CONFIG_USE_FMT}")
	if (MIMICPP_CONFIG_USE_FMT)

		message(DEBUG "${MESSAGE_PREFIX} Searching for installed {fmt}-package.")
		find_package(fmt QUIET)
		if (NOT fmt_FOUND)
			message(STATUS "${MESSAGE_PREFIX} No installed {fmt}-package found. Fetching via cpm.")

			include(get_cpm)
			CPMAddPackage("gh:fmtlib/fmt#11.0.2")
		endif()

		find_package(fmt REQUIRED)
		message(STATUS "${MESSAGE_PREFIX} Using {fmt}-package from: ${fmt_SOURCE_DIR}")
		target_link_libraries(
			enable-config-options
			INTERFACE
			fmt::fmt
		)

		target_compile_definitions(
			enable-config-options
			INTERFACE
			MIMICPP_CONFIG_USE_FMT
		)

	endif()

	# Config option, to enable unicode support for string matchers.
	# This will download the cpp-unicodelib source and create an import target
	# Eventually defines the macro MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER.
	OPTION(MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER "When enabled, all case-insensitive string matchers are available." OFF)
	message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER: ${MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER}")
	if (MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER)

		# on clang-builds this somehow emits an error, if not explicitly disabled
		# Got the info, to turn this of from here:
		# https://discourse.cmake.org/t/cmake-3-28-cmake-cxx-compiler-clang-scan-deps-notfound-not-found/9244/3
		set(OLD_CMAKE_CXX_SCAN_FOR_MODULES ${CMAKE_CXX_SCAN_FOR_MODULES})
		set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

		message(DEBUG "${MESSAGE_PREFIX} Searching for installed {uni-algo}-package.")
		find_package(uni-algo QUIET)
		if (NOT uni-algo_FOUND)
			message(STATUS "${MESSAGE_PREFIX} No installed {uni-algo}-package found. Fetching via cpm.")

			include(get_cpm)
			CPMAddPackage(
				NAME				uni-algo
				VERSION 1.2.0
				GITHUB_REPOSITORY	uni-algo/uni-algo
				EXCLUDE_FROM_ALL	YES
				SYSTEM				YES
				OPTIONS
					UNI_ALGO_INSTALL YES
			)
		endif()

		find_package(uni-algo REQUIRED)
		message(STATUS "${MESSAGE_PREFIX} Using {uni-algo}-package from: ${uni-algo_SOURCE_DIR}")
		target_link_libraries(
			enable-config-options
			INTERFACE
			uni-algo::uni-algo
		)

		target_compile_definitions(
			enable-config-options
			INTERFACE
			MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER
		)

		set(CMAKE_CXX_SCAN_FOR_MODULES ${OLD_CMAKE_CXX_SCAN_FOR_MODULES})

	endif()

	# Config option to enable full stacktrace support.
	# Eventually defines the macro MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE.
	OPTION(MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE "When enabled, experimental stacktrace feature is enabled (requires either c++23 or cpptrace)." OFF)
	message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE: ${MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE}")
	if (MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE)

		message(DEBUG "${MESSAGE_PREFIX} Stacktrace feature enabled.")

		# Config option to enable cpptrace as stacktrace-backend.
		# This will download the cpptrace source if not found.
		# Eventually defines the macro MIMICPP_CONFIG_USE_CPPTRACE.
		OPTION(MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE "When enabled, registers cpptrace library as stacktrace-backend." OFF)
		message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE: ${MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE}")
		if (MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE)

			# on clang-builds this somehow emits an error, if not explicitly disabled
			# Got the info, to turn this of from here:
			# https://discourse.cmake.org/t/cmake-3-28-cmake-cxx-compiler-clang-scan-deps-notfound-not-found/9244/3
			set(OLD_CMAKE_CXX_SCAN_FOR_MODULES ${CMAKE_CXX_SCAN_FOR_MODULES})
			set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

			message(DEBUG "${MESSAGE_PREFIX} Searching for installed {cpptrace}-package.")
			find_package(cpptrace QUIET)
			if (NOT cpptrace_FOUND)
				message(STATUS "${MESSAGE_PREFIX} No installed {cpptrace}-package found. Fetching via cpm.")

				include(get_cpm)
				CPMAddPackage(
					NAME cpptrace
					VERSION 0.7.5
					GITHUB_REPOSITORY jeremy-rifkin/cpptrace
					EXCLUDE_FROM_ALL YES
					SYSTEM YES
				)
			endif ()

			find_package(cpptrace REQUIRED)
			message(STATUS "${MESSAGE_PREFIX} Using {cpptrace}-package from: ${cpptrace_SOURCE_DIR}")
			target_link_libraries(
				enable-config-options
				INTERFACE
				cpptrace::cpptrace
			)

			target_compile_definitions(
				enable-config-options
				INTERFACE
				MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE
			)

			set(CMAKE_CXX_SCAN_FOR_MODULES ${OLD_CMAKE_CXX_SCAN_FOR_MODULES})

		else ()
			message(DEBUG "${MESSAGE_PREFIX} Selected std::stacktrace.")
		endif ()

		target_compile_definitions(
			enable-config-options
			INTERFACE
			MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE
		)

	endif ()

endif()
