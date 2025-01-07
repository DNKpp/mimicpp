#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

message(TRACE "Begin amalgamate headers")

set(AMALGAMATE_VERSION "v0.99.0")

if (WIN32)
	set(AMALGAMATE_URL_SUFFIX "windows-amd64.zip")
elseif(UNIX AND NOT APPLE)
	set(AMALGAMATE_URL_SUFFIX "linux-amd64.zip")
else()
	message(FATAL_ERROR "Unsupported platform.")
endif()

include(get_cpm)
CPMAddPackage(
	NAME amalgamate
	URL "https://github.com/rindeal/Amalgamate/releases/download/${AMALGAMATE_VERSION}/amalgamate-${AMALGAMATE_VERSION}-${AMALGAMATE_URL_SUFFIX}"
	DOWNLOAD_ONLY		YES
	EXCLUDE_FROM_ALL	YES
)

set(AMALGAMATE_COMMAND "${amalgamate_SOURCE_DIR}/amalgamate")
add_custom_target(
	mimicpp-amalgamate-headers
	COMMAND "${AMALGAMATE_COMMAND}" -v -i .. mimic++.hpp "${CMAKE_CURRENT_BINARY_DIR}/mimic++-amalgamated.hpp"
	COMMENT "Amalgamate headers"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../include/mimic++"
	COMMAND_EXPAND_LISTS
	VERBATIM
	USES_TERMINAL
)

message(TRACE "End amalgamate headers")
