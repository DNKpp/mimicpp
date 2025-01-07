#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET enable-coverage-flags)

    add_library(enable-coverage-flags INTERFACE)
    add_library(mimicpp::internal::coverage-flags ALIAS enable-coverage-flags)

    OPTION(MIMICPP_ENABLE_COVERAGE_FLAGS "When enabled, coverage flags will be applied." OFF)
    if (MIMICPP_ENABLE_COVERAGE_FLAGS)

        message(DEBUG "${MESSAGE_PREFIX} enabled coverage-flags.")

        if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            message(WARNING "${MESSAGE_PREFIX} MIMICPP_ENABLE_COVERAGE is only supported for gcc.")
        endif ()

        target_compile_options(
            enable-coverage-flags
            INTERFACE
            "$<$<CXX_COMPILER_ID:GNU>:--coverage;-fno-inline;-fprofile-abs-path;-fkeep-inline-functions;-fkeep-static-functions>"
        )

        target_link_options(
            enable-coverage-flags
            INTERFACE
            "$<$<CXX_COMPILER_ID:GNU>:--coverage>"
        )

    endif ()

endif ()
