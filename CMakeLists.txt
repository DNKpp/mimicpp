#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(
	mimicpp
	LANGUAGES		CXX
	VERSION			4
	DESCRIPTION		"A modern and (mostly) macro free mocking-framework"
	HOMEPAGE_URL	"https://github.com/DNKpp/mimicpp"
)

message(STATUS "mimic++ version: ${PROJECT_VERSION} from: ${mimicpp_SOURCE_DIR}")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(STATUS "targeting 64-bit architecture")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(STATUS "targeting 32-bit architecture")
else ()
	message(WARNING "unable to determine architecture")
endif ()

include(GNUInstallDirs)

add_library(mimicpp INTERFACE)
add_library(mimicpp::mimicpp ALIAS mimicpp)
target_include_directories(
	mimicpp
	INTERFACE
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
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

include(EnableConfigOptions)
target_link_libraries(
	mimicpp
	INTERFACE
	mimicpp::internal::config-options
)

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

if(NOT CMAKE_SKIP_INSTALL_RULES)

  include(InstallRules)

endif()
