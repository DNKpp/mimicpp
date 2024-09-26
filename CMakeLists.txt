cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(get_cpm)

project(
	mimicpp
	LANGUAGES CXX
	VERSION 1
)

add_library(mimicpp INTERFACE)
add_library(mimicpp::mimicpp ALIAS mimicpp)
target_include_directories(
	mimicpp
	INTERFACE
	"include"
)

target_compile_options(
	mimicpp
	INTERFACE
	# this option is required to make __VA_OPT__ work on msvc
	"$<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>"
)

set(CMAKE_CXX_STANDARD 20)
if (MIMICPP_FORCED_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD ${MIMICPP_FORCED_CXX_STANDARD})
endif()

target_compile_features(
	mimicpp
	INTERFACE
	cxx_std_${CMAKE_CXX_STANDARD}
)

OPTION(MIMICPP_CONFIG_ONLY_PREFIXED_MACROS "When enabled, all macros will be prefixed with MIMICPP_." OFF)
if (MIMICPP_CONFIG_ONLY_PREFIXED_MACROS)
	target_compile_definitions(
		mimicpp
		INTERFACE
		MIMICPP_CONFIG_ONLY_PREFIXED_MACROS
	)
endif()

if (CMAKE_SOURCE_DIR STREQUAL mimicpp_SOURCE_DIR)
	set(IS_TOP_LEVEL_PROJECT ON)
else()
	set(IS_TOP_LEVEL_PROJECT OFF)
endif()

OPTION(MIMICPP_BUILD_TESTS "Determines, whether the tests shall be built." ${IS_TOP_LEVEL_PROJECT})
if (MIMICPP_BUILD_TESTS)
	include(CTest)
	add_subdirectory("test")
endif()

OPTION(MIMICPP_BUILD_EXAMPLES "Determines, whether the examples shall be built." ${IS_TOP_LEVEL_PROJECT})
if (MIMICPP_BUILD_EXAMPLES)
	include(CTest)
	add_subdirectory("examples")
endif()

OPTION(MIMICPP_CONFIGURE_DOXYGEN "Determines, whether the doxyfile shall be configured." OFF)
if (MIMICPP_CONFIGURE_DOXYGEN)

	OPTION(MIMICPP_ENABLE_GENERATE_DOCS "Enable the doxygen documentation target." OFF)

	add_subdirectory("docs")

endif()

OPTION(MIMICPP_ENABLE_AMALGAMATE_HEADERS "Enables the amalgamation target." OFF)
if (MIMICPP_ENABLE_AMALGAMATE_HEADERS)

	add_subdirectory("tools/amalgamate-headers")

endif()
