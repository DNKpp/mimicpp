#[[ Adds a custom target, which generates the doxygen documentation.
	The working directory is set to the project origin, as this is an effective way to use the relative paths for the documentation.
#]]

message(TRACE "Begin generating docs")

find_package(Doxygen REQUIRED COMPONENTS dot)
if (NOT DOXYGEN_FOUND)
	message(FATAL_ERROR "Doxygen not found.")
endif()

get_target_property(DOXY_DOT_PATH Doxygen::dot LOCATION)
message(VERBOSE "Using dot at: ${DOXY_DOT_PATH}")

CPMAddPackage("gh:jothepro/doxygen-awesome-css@2.3.2")

set(DOXY_PROJECT_VERSION ${PROJECT_VERSION})
set(DOXY_INPUT_DIR "include")
set(DOXY_EXAMPLE_DIR "examples")
set(DOXY_README_PATH "README.md")
set(DOXY_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(DOXY_HTML_EXTRA_STYLESHEET "${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css")
	
configure_file(Doxyfile.in Doxyfile @ONLY)

add_custom_target(
	mimicpp-generate-docs
	COMMAND "$<TARGET_FILE:Doxygen::doxygen>" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
	COMMENT "Generate Documentation"
	WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
	COMMAND_EXPAND_LISTS
	VERBATIM
	USES_TERMINAL
)
	
message(TRACE "Finished generating docs")