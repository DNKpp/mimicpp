#          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

find_package(Catch2 REQUIRED)

add_executable(
	mimicpp-adapter-tests-catch2
	"main.cpp"
)

include(EnableWarnings)
include(EnableSanitizers)
enable_sanitizers(mimicpp-adapter-tests-catch2)

target_link_libraries(
	mimicpp-adapter-tests-catch2
	PRIVATE
	mimicpp::mimicpp
	mimicpp::internal::warnings
	Catch2::Catch2WithMain
)

catch_discover_tests(mimicpp-adapter-tests-catch2)
