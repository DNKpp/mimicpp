#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(Mimic++-LinkStdStacktrace)
include(CheckCXXSourceCompiles)
include(CheckCXXCompilerFlag)

function(check_std_stacktrace_support)

    if (MSVC)
        set(CXX23_FLAG "/std:c++latest")
    else ()
        set(CXX23_FLAG "-std=c++23")
    endif ()

    check_cxx_compiler_flag(${CXX23_FLAG} COMPILER_SUPPORTS_CXX23)
    if (NOT COMPILER_SUPPORTS_CXX23)
        return()
    endif ()

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    get_target_property(LIBS mimicpp::internal::link-std-stacktrace INTERFACE_LINK_LIBRARIES)
    set(CMAKE_REQUIRED_LIBRARIES "${LIBS}")

    set(CODE "
#include <stacktrace>
int main()
{
  [[maybe_unused]] auto const st = std::stacktrace::current();
  return 0;
}
")
    check_cxx_source_compiles("${CODE}" HAS_STD_STACKTRACE)
    set(HAS_STD_STACKTRACE ${HAS_STD_STACKTRACE} PARENT_SCOPE)
endfunction()

check_std_stacktrace_support()
