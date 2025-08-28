#          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(Mimic++-LinkStdStacktrace)
include(CheckCXXSourceCompiles)

function(check_std_stacktrace_support)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    get_target_property(LIBS mimicpp::internal::link-std-stacktrace INTERFACE_LINK_LIBRARIES)
    set(CMAKE_REQUIRED_LIBRARIES "${LIBS}")

    set(CODE
        "
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
