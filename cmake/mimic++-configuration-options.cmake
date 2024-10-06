#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(get_cpm)

OPTION(MIMICPP_CONFIG_ONLY_PREFIXED_MACROS "When enabled, all macros will be prefixed with MIMICPP_." OFF)
OPTION(
	MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION
	"When enabled, catch2 matchers integration will be enabled, if catch2 adapter is used (experimental)."
	OFF
)

target_compile_definitions(
	mimicpp
	INTERFACE
	$<$<BOOL:${MIMICPP_CONFIG_ONLY_PREFIXED_MACROS}>:MIMICPP_CONFIG_ONLY_PREFIXED_MACROS>
	$<$<BOOL:${MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION}>:MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION>
)

# Config option, to utilize fmt instead of std formatting.
# Checks, whether fmt is already available. Fetches it instead.
# Eventually defines the macro MIMICPP_CONFIG_USE_FMT. 
OPTION(MIMICPP_CONFIG_USE_FMT "When enabled, uses fmt instead of std formatting." OFF)
if (MIMICPP_CONFIG_USE_FMT)

	find_package(fmt QUIET)
	if (NOT fmt_FOUND)
		CPMAddPackage("gh:fmtlib/fmt#11.0.2")
	endif()

	find_package(fmt REQUIRED)
	target_link_libraries(
		mimicpp
		INTERFACE
		fmt::fmt
	)

	target_compile_definitions(
		mimicpp
		INTERFACE
		MIMICPP_CONFIG_USE_FMT
	)

endif()

# Config option, to enable unicode support for string matchers.
# This will download the cpp-unicodelib source and create an import target
# Eventually defines the macro MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER. 
OPTION(MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER "When enabled, all case-insensitive string matchers are available." OFF)
if (MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER)

	find_package(uni-algo QUIET)
	if (NOT uni-algo_FOUND)
		CPMAddPackage(
			NAME				uni-algo
			GITHUB_REPOSITORY	uni-algo/uni-algo
			GIT_TAG				v1.2.0
			EXCLUDE_FROM_ALL	YES
			SYSTEM				YES
			OPTIONS
				UNI_ALGO_INSTALL YES
		)
	endif()

	find_package(uni-algo REQUIRED)
	target_link_libraries(
		mimicpp
		INTERFACE
		uni-algo::uni-algo
	)

	target_compile_definitions(
		mimicpp
		INTERFACE
		MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER
	)

endif()
