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
message(DEBUG "${MESSAGE_PREFIX} CXXFLAGS: $ENV{CXXFLAGS}")
message(DEBUG "${MESSAGE_PREFIX} CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(DEBUG "${MESSAGE_PREFIX} CMAKE_CXX_FLAGS_INIT: ${CMAKE_CXX_FLAGS_INIT}")

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${mimicpp_SOURCE_DIR}" IS_TOP_LEVEL_PROJECT)

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
	message(DEBUG "${MESSAGE_PREFIX} forced c++-standard c++${MIMICPP_FORCED_CXX_STANDARD}")
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
