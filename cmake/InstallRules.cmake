#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(CMakePackageConfigHelpers)

set(MIMICPP_LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/mimicpp")
set(MIMICPP_CMAKE_INSTALL_DIR "${MIMICPP_LIB_INSTALL_DIR}/cmake")
set(MIMICPP_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/mimicpp")

write_basic_package_version_file(
    "mimicpp-version.cmake"
    VERSION         ${PROJECT_VERSION}
    COMPATIBILITY   AnyNewerVersion
    ARCH_INDEPENDENT
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/mimicpp-config.cmake.in"
    "mimicpp-config.cmake"
    INSTALL_DESTINATION "${MIMICPP_CMAKE_INSTALL_DIR}"
)

install(
	TARGETS						mimicpp enable-config-options
	EXPORT						mimicpp-targets
	PUBLIC_HEADER DESTINATION	"${MIMICPP_INCLUDE_INSTALL_DIR}"
)

install(
    DIRECTORY				"include/"
    TYPE					INCLUDE
    FILES_MATCHING PATTERN	"*.hpp"
)

install(
    EXPORT                  mimicpp-targets
    FILE                    mimicpp-targets.cmake
    DESTINATION             "${MIMICPP_CMAKE_INSTALL_DIR}"
    NAMESPACE               mimicpp::
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/mimicpp-config.cmake"
        "${PROJECT_BINARY_DIR}/mimicpp-version.cmake"
    DESTINATION "${MIMICPP_CMAKE_INSTALL_DIR}"
)
