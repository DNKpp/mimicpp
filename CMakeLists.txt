cmake_minimum_required(VERSION 3.14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(get_cpm)

project(
	mimicpp
	LANGUAGES CXX
)

add_library(mimicpp INTERFACE)
add_library(mimicpp::mimicpp ALIAS mimicpp)
target_include_directories(
	mimicpp
	INTERFACE
	"include"
)

set(CMAKE_CXX_STANDARD 20)
target_compile_features(
	mimicpp
	INTERFACE
	cxx_std_20
)

if (CMAKE_SOURCE_DIR STREQUAL mimicpp_SOURCE_DIR)
	set(IS_TOP_LEVEL_PROJECT ON)
else()
	set(IS_TOP_LEVEL_PROJECT OFF)
endif()

if (mimicpp_FORCED_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD ${mimicpp_FORCED_CXX_STANDARD})
endif()

OPTION(mimicpp_BUILD_TESTS "Determines whether tests shall be built." ${IS_TOP_LEVEL_PROJECT})
if (mimicpp_BUILD_TESTS)
	include(CTest)
	add_subdirectory("test")
endif()
