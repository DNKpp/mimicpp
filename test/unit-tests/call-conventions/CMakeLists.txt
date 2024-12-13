if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
    OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
)
    message(STATUS "Adding __vectorcall tests.")
    target_sources(mimicpp-tests PRIVATE
        "Vectorcall.cpp"
    )
endif ()
