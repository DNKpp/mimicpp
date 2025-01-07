//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#define MIMICPP_CONFIG_ONLY_PREFIXED_MACROS

// ReSharper disable CppUnusedIncludeDirective
#include "mimic++/ExpectationBuilder.hpp"
#include "mimic++/InterfaceMock.hpp"
// ReSharper restore CppUnusedIncludeDirective

#ifdef MOCK_METHOD
	static_assert(
		mimicpp::always_false<>::value,
		"MOCK_METHOD must be undefined when MIMICPP_CONFIG_ONLY_PREFIXED_MACROS is defined.");
#endif

#ifdef MOCK_OVERLOADED_METHOD
	static_assert(
		mimicpp::always_false<>::value,
		"MOCK_OVERLOADED_METHOD must be undefined when MIMICPP_CONFIG_ONLY_PREFIXED_MACROS is defined.");
#endif

#ifdef ADD_OVERLOAD
	static_assert(
		mimicpp::always_false<>::value,
		"ADD_OVERLOAD must be undefined when MIMICPP_CONFIG_ONLY_PREFIXED_MACROS is defined.");
#endif

#ifdef SCOPED_EXP
	static_assert(
		mimicpp::always_false<>::value,
		"SCOPED_EXP must be undefined when MIMICPP_CONFIG_ONLY_PREFIXED_MACROS is defined.");
#endif
