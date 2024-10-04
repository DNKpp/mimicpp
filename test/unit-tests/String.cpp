// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/String.hpp"

#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE_SIG(
	"is_character(_v) determines, whether the given type is a character type.",
	"[string][string::traits]",
	((bool expected, typename T), expected, T),
	(false, int),
	(false, bool),
	(false, std::string),
	(false, const char),
	(false, char&),
	(false, char&&),
	(false, char*),

	(true, char),
	(true, unsigned char),
	(true, signed char),
	(true, wchar_t),
	(true, char8_t),
	(true, char16_t),
	(true, char32_t)
)
{
	STATIC_REQUIRE(expected == mimicpp::is_character<T>::value);
	STATIC_REQUIRE(expected == mimicpp::is_character_v<T>);
}

TEMPLATE_TEST_CASE_SIG(
	"string_traits contains properties for the provided string type.",
	"[type_traits]",
	((bool dummy, typename T, typename Char, typename String), dummy, T, Char, String),
	(true, char*, char, std::string_view),
	(true, const char*, char, std::string_view),
	(true, wchar_t*, wchar_t, std::wstring_view),
	(true, const wchar_t*, wchar_t, std::wstring_view),
	(true, char8_t*, char8_t, std::u8string_view),
	(true, const char8_t*, char8_t, std::u8string_view),
	(true, char16_t*, char16_t, std::u16string_view),
	(true, const char16_t*, char16_t, std::u16string_view),
	(true, char32_t*, char32_t, std::u32string_view),
	(true, const char32_t*, char32_t, std::u32string_view),

	(true, char[], char, std::string_view),
	(true, const char[], char, std::string_view),
	(true, char[42], char, std::string_view),
	(true, const char[42], char, std::string_view),

	(true, std::string, char, std::string),
	(true, std::wstring, wchar_t, std::wstring),
	(true, std::u8string, char8_t, std::u8string),
	(true, std::u16string, char16_t, std::u16string),
	(true, std::u32string, char32_t, std::u32string),

	(true, std::string_view, char, std::string_view),
	(true, std::wstring_view, wchar_t, std::wstring_view),
	(true, std::u8string_view, char8_t, std::u8string_view),
	(true, std::u16string_view, char16_t, std::u16string_view),
	(true, std::u32string_view, char32_t, std::u32string_view)
)
{
	using traits_t = mimicpp::string_traits<T>;

	STATIC_REQUIRE(std::same_as<Char, typename traits_t::char_t>);
	STATIC_REQUIRE(std::same_as<String, typename traits_t::string_t>);
}

TEMPLATE_TEST_CASE(
	"string_traits does not work for actual non-string-types.",
	"[type_traits]",
	char,
	const char,
	int,
	std::optional<std::string>,
	std::string&
)
{
	using traits_t = mimicpp::string_traits<TestType>;

	STATIC_REQUIRE(!requires{ traits_t{}; });
}
