#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.15...3.31)
set(MESSAGE_PREFIX "mimic++:")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ReadProjectVersion)
read_project_version("include/mimic++/config/Version.hpp" MIMICPP_VERSION)

project(
	mimicpp
	LANGUAGES		CXX
    VERSION ${MIMICPP_VERSION}
	DESCRIPTION		"A modern and (mostly) macro free mocking-framework"
	HOMEPAGE_URL	"https://github.com/DNKpp/mimicpp"
)

message(STATUS "${MESSAGE_PREFIX} version: v${PROJECT_VERSION} from: ${mimicpp_SOURCE_DIR}")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(STATUS "${MESSAGE_PREFIX} targeting 64-bit architecture")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(STATUS "${MESSAGE_PREFIX} targeting 32-bit architecture")
else ()
	message(WARNING "${MESSAGE_PREFIX} unable to determine architecture")
endif ()

include(CMakeDependentOption)
# Enable cmake c++20 modules support when possible,
# but give users the option to explicitly disable it in cases where e.g. an insufficient generator or compiler is used.
cmake_dependent_option(MIMICPP_CMAKE_HAS_CXX20_MODULES "" OFF "CMAKE_VERSION VERSION_GREATER_EQUAL 3.28" OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ${MIMICPP_CMAKE_HAS_CXX20_MODULES})
message(DEBUG "${MESSAGE_PREFIX} CMake has c++20 modules: " ${MIMICPP_CMAKE_HAS_CXX20_MODULES})

message(DEBUG "${MESSAGE_PREFIX} CXXFLAGS: $ENV{CXXFLAGS}")
message(DEBUG "${MESSAGE_PREFIX} CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(DEBUG "${MESSAGE_PREFIX} CMAKE_CXX_FLAGS_INIT: ${CMAKE_CXX_FLAGS_INIT}")

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${mimicpp_SOURCE_DIR}" IS_TOP_LEVEL_PROJECT)

set(CMAKE_CXX_STANDARD 20)
if (MIMICPP_FORCED_CXX_STANDARD)
	message(DEBUG "${MESSAGE_PREFIX} forced c++-standard c++${MIMICPP_FORCED_CXX_STANDARD}")
	set(CMAKE_CXX_STANDARD ${MIMICPP_FORCED_CXX_STANDARD})
endif ()

add_library(mimicpp-header-only INTERFACE)
add_library(mimicpp::header-only ALIAS mimicpp-header-only)

include(GNUInstallDirs)
target_include_directories(mimicpp-header-only INTERFACE
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_compile_options(mimicpp-header-only INTERFACE
	# this option is required to make __VA_OPT__ work on msvc
	"$<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>"
)

target_compile_features(mimicpp-header-only INTERFACE
	cxx_std_${CMAKE_CXX_STANDARD}
)

include(Mimic++-EnableConfigOptions)
target_link_libraries(mimicpp-header-only INTERFACE
	mimicpp::internal::config-options
)

if (MIMICPP_CMAKE_HAS_CXX20_MODULES)
	add_library(mimicpp-module)
	add_library(mimicpp::module ALIAS mimicpp-module)
	add_library(mimicpp::mimicpp ALIAS mimicpp-module)

	target_link_libraries(mimicpp-module PUBLIC
		mimicpp::header-only
	)

	target_sources(mimicpp-module PUBLIC
		FILE_SET CXX_MODULES
		TYPE CXX_MODULES FILES mimic++.cppm
	)
else ()
	add_library(mimicpp::mimicpp ALIAS mimicpp-header-only)
endif ()

option(MIMICPP_BUILD_TESTS "Determines, whether the tests shall be built." ${IS_TOP_LEVEL_PROJECT})
if (MIMICPP_BUILD_TESTS)
	include(CTest)
	add_subdirectory("test")
endif()

option(MIMICPP_BUILD_EXAMPLES "Determines, whether the examples shall be built." ${IS_TOP_LEVEL_PROJECT})
if (MIMICPP_BUILD_EXAMPLES)
	include(CTest)
	add_subdirectory("examples")
endif()

option(MIMICPP_CONFIGURE_DOXYGEN "Determines, whether the doxyfile shall be configured." OFF)
if (MIMICPP_CONFIGURE_DOXYGEN)
	option(MIMICPP_ENABLE_GENERATE_DOCS "Enable the doxygen documentation target." OFF)
	add_subdirectory("docs")
endif()

option(MIMICPP_ENABLE_AMALGAMATE_HEADERS "Enables the amalgamation target." OFF)
if (MIMICPP_ENABLE_AMALGAMATE_HEADERS)
	add_subdirectory("tools/amalgamate-headers")
endif()

if(NOT CMAKE_SKIP_INSTALL_RULES)
	include(InstallRules)
endif()
