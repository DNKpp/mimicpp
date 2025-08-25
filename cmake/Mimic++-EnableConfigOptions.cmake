#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET mimicpp-enable-config-options)

    add_library(mimicpp-enable-config-options INTERFACE)
    add_library(mimicpp::internal::config-options ALIAS mimicpp-enable-config-options)

    option(MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES "When enabled, all types will be printed prettified." OFF)
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES: ${MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES}")
    option(MIMICPP_CONFIG_ONLY_PREFIXED_MACROS "When enabled, all macros will be prefixed with MIMICPP_." OFF)
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_ONLY_PREFIXED_MACROS: ${MIMICPP_CONFIG_ONLY_PREFIXED_MACROS}")
    option(
        MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION
        "When enabled, catch2 matchers integration will be enabled, if catch2 adapter is used (experimental)."
        OFF
    )
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION: ${MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION}")

    # Config option, to utilize a custom source-location backend.
    set(MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND "" CACHE STRING "Defines the utilized source-location type.")
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND: ${MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND}")

    target_compile_definitions(mimicpp-enable-config-options
        INTERFACE
        $<$<BOOL:${MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES}>:MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES>
        $<$<BOOL:${MIMICPP_CONFIG_ONLY_PREFIXED_MACROS}>:MIMICPP_CONFIG_ONLY_PREFIXED_MACROS>
        $<$<BOOL:${MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION}>:MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION>
        $<$<BOOL:${MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND}>:MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND=${MIMICPP_CONFIG_ALTERNATIVE_SOURCE_LOCATION_BACKEND}>
    )

    # Make this option available, when CMake actually supports C++20 modules.
    # As portability is very limited, users have to explicitly opt-in.
    cmake_dependent_option(MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__ "" OFF "CMAKE_VERSION VERSION_GREATER_EQUAL 3.28" OFF)
    set(CMAKE_CXX_SCAN_FOR_MODULES ${MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__})
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__: " ${MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__})

    # Config option, to utilize fmt instead of std formatting.
    # Checks, whether fmt is already available. Fetches it instead.
    # Eventually defines the macro MIMICPP_CONFIG_USE_FMT.
    option(MIMICPP_CONFIG_USE_FMT "When enabled, uses fmt instead of std formatting." OFF)
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_USE_FMT: ${MIMICPP_CONFIG_USE_FMT}")
    if (MIMICPP_CONFIG_USE_FMT)

        message(DEBUG "${MESSAGE_PREFIX} Searching for installed {fmt}-package.")
        cmake_dependent_option(
            MIMICPP_CONFIG_IMPORT_FMT
            "Determines, whether fmt will be consumed as a c++20 module."
            ON
            "MIMICPP_CONFIG_EXPERIMENTAL_ENABLE_CXX20_MODULES__UNPORTABLE__"
            OFF
        )
        set(FMT_MODULE ${MIMICPP_CONFIG_IMPORT_FMT} CACHE BOOL "")

        find_package(fmt QUIET)
        if (NOT fmt_FOUND)
            message(STATUS "${MESSAGE_PREFIX} No installed {fmt}-package found. Fetching via cpm.")

            include(get_cpm)
            CPMAddPackage("gh:fmtlib/fmt#11.1.4")
        endif ()

        find_package(fmt REQUIRED)
        message(STATUS "${MESSAGE_PREFIX} Using {fmt}-package from: ${fmt_SOURCE_DIR}")
        target_link_libraries(mimicpp-enable-config-options
            INTERFACE
            fmt::fmt
        )

        target_compile_definitions(mimicpp-enable-config-options
            INTERFACE
            MIMICPP_CONFIG_USE_FMT=1
            $<$<BOOL:${MIMICPP_CONFIG_IMPORT_FMT}>:MIMICPP_CONFIG_IMPORT_FMT=1>
        )

    endif ()

    # Config option, to enable unicode support for string matchers.
    # This will download the uni-algo/uni-algo source.
    # Eventually defines the macro MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER.
    option(MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER "When enabled, all case-insensitive string matchers are available." OFF)
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER: ${MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER}")
    if (MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER)

        set(UNI_ALGO_INSTALL ON CACHE BOOL "")

        message(DEBUG "${MESSAGE_PREFIX} Searching for installed {uni-algo}-package.")
        find_package(uni-algo QUIET)
        if (NOT uni-algo_FOUND)
            message(STATUS "${MESSAGE_PREFIX} No installed {uni-algo}-package found. Fetching via cpm.")

            include(get_cpm)
            CPMAddPackage("gh:uni-algo/uni-algo@1.2.0")
        endif ()

        find_package(uni-algo REQUIRED)
        message(STATUS "${MESSAGE_PREFIX} Using {uni-algo}-package from: ${uni-algo_SOURCE_DIR}")
        target_link_libraries(mimicpp-enable-config-options
            INTERFACE
            uni-algo::uni-algo
        )

        target_compile_definitions(mimicpp-enable-config-options
            INTERFACE
            MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER=1
        )

    endif ()

    # Config option regarding stacktrace support.
    set(MIMICPP_DETAIL_STACKTRACE_FEATURES "off;c++23;cpptrace;custom")
    set(MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE "off" CACHE STRING "Which stacktrace backend to use.")
    set(CACHE MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE PROPERTY
        STRINGS ${MIMICPP_DETAIL_STACKTRACE_FEATURES}
    )
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE: ${MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE}")
    if (MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE STREQUAL "c++23")

        target_compile_features(mimicpp-enable-config-options INTERFACE
            cxx_std_23
        )
        target_compile_definitions(mimicpp-enable-config-options INTERFACE
            MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE=1
            MIMICPP_CONFIG_EXPERIMENTAL_USE_CXX23_STACKTRACE=1
        )

    elseif (MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE STREQUAL "cpptrace")

        find_package(cpptrace REQUIRED)
        target_link_libraries(mimicpp-enable-config-options INTERFACE
            cpptrace::cpptrace
        )
        target_compile_definitions(mimicpp-enable-config-options INTERFACE
            MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE=1
            MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE=1
            $<$<BOOL:${MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE}>:MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE=1>
        )

    elseif (MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE STREQUAL "custom")

        target_compile_definitions(mimicpp-enable-config-options INTERFACE
            MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE=1
            MIMICPP_CONFIG_EXPERIMENTAL_USE_CUSTOM_STACKTRACE=1
        )

    elseif (MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE)
        message(FATAL_ERROR "${MESSAGE_PREFIX} Invalid value for MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE: ${MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE}\
            \tOptions are: ${MIMICPP_DETAIL_STACKTRACE_FEATURES}"
        )
    endif ()

endif ()
