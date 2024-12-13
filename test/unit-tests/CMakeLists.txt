#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

# fixes reporting in clion
set(CATCH_CONFIG_CONSOLE_WIDTH 800 CACHE STRING "")
find_package(Catch2 REQUIRED)
CPMAddPackage("gh:rollbear/trompeloeil@48")

add_executable(
    mimicpp-tests
    "Call.cpp"
    "Config.cpp"
    "ControlPolicy.cpp"
    "Expectation.cpp"
    "ExpectationBuilder.cpp"
    "ExpectationPolicies.cpp"
    "InterfaceMock.cpp"
    "Matcher.cpp"
    "Mock.cpp"
    "ObjectWatcher.cpp"
    "Printer.cpp"
    "Reports.cpp"
    "Reporter.cpp"
    "Sequence.cpp"
    "String.cpp"
    "TypeTraits.cpp"
    "Utility.cpp"
)

add_subdirectory(call-conventions)
add_subdirectory(matchers)

include(EnableWarnings)
include(EnableSanitizers)
enable_sanitizers(mimicpp-tests)

target_compile_options(
    mimicpp-tests
    PRIVATE
    # some test files exceed some limits
    # see: https://learn.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/fatal-error-c1128?view=msvc-170
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

target_link_libraries(
    mimicpp-tests
    PRIVATE
    mimicpp::mimicpp
    mimicpp::internal::warnings
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)

catch_discover_tests(mimicpp-tests)
