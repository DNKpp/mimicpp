CPMAddPackage("gh:catchorg/Catch2@3.7.1")

if (Catch2_ADDED)
	include("${Catch2_SOURCE_DIR}/extras/Catch.cmake")
endif()
