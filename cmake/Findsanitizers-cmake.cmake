CPMAddPackage(
	NAME sanitizers-cmake
	GITHUB_REPOSITORY "arsenm/sanitizers-cmake"
	GIT_TAG "3f0542e4e034aab417c51b2b22c94f83355dee15"
	SYSTEM YES
	EXCLUDE_FROM_ALL YES
	DOWNLOAD_ONLY YES
)

if (sanitizers-cmake_ADDED)

	list(APPEND CMAKE_MODULE_PATH "${sanitizers-cmake_SOURCE_DIR}/cmake")

endif()

find_package(Sanitizers REQUIRED)
