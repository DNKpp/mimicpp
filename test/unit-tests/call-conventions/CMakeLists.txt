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
