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
    cmake_dependent_option(MIMICPP_CONFIG_ENABLE_CXX20_MODULES "" OFF "CMAKE_VERSION VERSION_GREATER_EQUAL 3.28" OFF)
    set(CMAKE_CXX_SCAN_FOR_MODULES ${MIMICPP_CONFIG_ENABLE_CXX20_MODULES})
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_ENABLE_CXX20_MODULES: " ${MIMICPP_CONFIG_ENABLE_CXX20_MODULES})

    # Config option, to utilize fmt instead of std formatting.
    # Checks, whether fmt is already available. Fetches it instead.
    # Eventually defines the macro MIMICPP_CONFIG_USE_FMT.
    option(MIMICPP_CONFIG_USE_FMT "When enabled, uses fmt instead of std formatting." OFF)
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_USE_FMT: ${MIMICPP_CONFIG_USE_FMT}")
    if (MIMICPP_CONFIG_USE_FMT)

        message(DEBUG "${MESSAGE_PREFIX} Searching for installed {fmt}-package.")
        cmake_dependent_option(MIMICPP_CONFIG_IMPORT_FMT "Determines, whether fmt will be consumed as a c++20 module." ON "MIMICPP_CONFIG_ENABLE_CXX20_MODULES" OFF)
        set(FMT_MODULE ${MIMICPP_CONFIG_IMPORT_FMT})

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

        set(UNI_ALGO_INSTALL ON)

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

    # Config option to enable full stacktrace support.
    # Eventually defines the macro MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE.
    option(MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE "When enabled, experimental stacktrace feature is enabled (requires either c++23 or cpptrace)." OFF)
    message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE: ${MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE}")
    if (MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE)

        message(DEBUG "${MESSAGE_PREFIX} Stacktrace feature enabled.")

        # Config option to enable cpptrace as stacktrace-backend.
        # This will download the cpptrace source if not found.
        # Eventually defines the macro MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE.
        option(MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE "When enabled, registers cpptrace library as stacktrace-backend." OFF)
        message(DEBUG "${MESSAGE_PREFIX} MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE: ${MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE}")
        if (MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE)

            message(DEBUG "${MESSAGE_PREFIX} Searching for installed {cpptrace}-package.")

            cmake_dependent_option(
                MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE
                "Determines, whether cpptrace will be consumed as a c++20 module."
                ON
                "MIMICPP_CONFIG_ENABLE_CXX20_MODULES"
                OFF
            )
            if (NOT MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE)
                set(CPPTRACE_DISABLE_CXX_20_MODULES ON)
            else ()
                set(CPPTRACE_DISABLE_CXX_20_MODULES OFF)
            endif ()

            find_package(cpptrace QUIET)
            if (NOT cpptrace_FOUND)
                message(STATUS "${MESSAGE_PREFIX} No installed {cpptrace}-package found. Fetching via cpm.")

                include(get_cpm)
                CPMAddPackage("gh:jeremy-rifkin/cpptrace@1.0.2")
            endif ()

            find_package(cpptrace REQUIRED)
            message(STATUS "${MESSAGE_PREFIX} Using {cpptrace}-package from: ${cpptrace_SOURCE_DIR}")
            target_link_libraries(mimicpp-enable-config-options
                INTERFACE
                cpptrace::cpptrace
            )

            target_compile_definitions(mimicpp-enable-config-options
                INTERFACE
                MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE=1
                $<$<BOOL:${MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE}>:MIMICPP_CONFIG_EXPERIMENTAL_IMPORT_CPPTRACE=1>
            )

        else ()
            message(DEBUG "${MESSAGE_PREFIX} Selected std::stacktrace.")
        endif ()

        target_compile_definitions(mimicpp-enable-config-options
            INTERFACE
            MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE=1
        )

    endif ()

endif ()
