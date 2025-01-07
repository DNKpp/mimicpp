#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
    OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT APPLE))

    message(STATUS "${MESSAGE_PREFIX} Adding __vectorcall tests.")
    target_sources(mimicpp-tests PRIVATE
        "Vectorcall.cpp"
    )

endif ()


if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    message(STATUS "${MESSAGE_PREFIX} Adding gcc call-convention tests.")
    target_sources(mimicpp-tests PRIVATE
        "GccCallConvention.cpp"
    )

endif ()
