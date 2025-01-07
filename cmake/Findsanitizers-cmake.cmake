#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(get_cpm)

CPMAddPackage(
	NAME sanitizers-cmake
	GITHUB_REPOSITORY "arsenm/sanitizers-cmake"
	GIT_TAG "3f0542e4e034aab417c51b2b22c94f83355dee15"
	SYSTEM YES
	EXCLUDE_FROM_ALL YES
	DOWNLOAD_ONLY YES
)

list(APPEND CMAKE_MODULE_PATH "${sanitizers-cmake_SOURCE_DIR}/cmake")

find_package(Sanitizers REQUIRED)
