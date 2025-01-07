#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET enable-additional-test-flags)

    add_library(enable-additional-test-flags INTERFACE)
    add_library(mimicpp::internal::additional-test-flags ALIAS enable-additional-test-flags)

    if (MIMICPP_TEST_ADDITIONAL_COMPILER_FLAGS)

        message(DEBUG "${MESSAGE_PREFIX} enabled additional compiler-flags: ${MIMICPP_TEST_ADDITIONAL_COMPILER_FLAGS}")

        target_compile_options(enable-additional-test-flags
            INTERFACE
            ${MIMICPP_TEST_ADDITIONAL_COMPILER_FLAGS}
        )

    endif ()

    if (MIMICPP_TEST_ADDITIONAL_LINKER_FLAGS)

        message(DEBUG "${MESSAGE_PREFIX} enabled additional linker-flags: ${MIMICPP_TEST_ADDITIONAL_LINKER_FLAGS}")

        target_link_options(enable-additional-test-flags
            INTERFACE
            ${MIMICPP_TEST_ADDITIONAL_LINKER_FLAGS}
        )

    endif ()
endif ()
