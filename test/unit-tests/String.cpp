// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/String.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

using namespace mimicpp;

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

namespace
{
	class CustomString
	{
	public:
		std::string inner{};
	};
}

template <>
struct mimicpp::string_traits<CustomString>
{
	using char_t = char;

	[[nodiscard]]
	static constexpr std::string_view view(const CustomString& str) noexcept
	{
		return std::string_view{str.inner};
	}
};

TEMPLATE_TEST_CASE_SIG(
	"string_traits contains properties for the provided string type.",
	"[string][string::traits]",
	((bool dummy, typename T, typename Char, typename ViewType), dummy, T, Char, ViewType),
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
	(true, std::u32string_view, char32_t, std::u32string_view),

	(true, CustomString, char, std::string_view)
)
{
	STATIC_REQUIRE(std::same_as<Char, typename string_traits<T>::char_t>);
	STATIC_REQUIRE(std::same_as<Char, string_char_t<T>>);
	STATIC_REQUIRE(std::same_as<Char, string_char_t<const T>>);
	STATIC_REQUIRE(std::same_as<Char, string_char_t<T&>>);
	STATIC_REQUIRE(std::same_as<Char, string_char_t<const T&>>);
	STATIC_REQUIRE(std::same_as<Char, string_char_t<T&&>>);
	STATIC_REQUIRE(std::same_as<Char, string_char_t<const T&&>>);

	STATIC_REQUIRE(std::ranges::forward_range<string_view_t<T>>);
	STATIC_REQUIRE(std::ranges::forward_range<string_view_t<const T>>);
	STATIC_REQUIRE(std::ranges::forward_range<string_view_t<T&>>);
	STATIC_REQUIRE(std::ranges::forward_range<string_view_t<const T&>>);
	STATIC_REQUIRE(std::ranges::forward_range<string_view_t<T&&>>);
	STATIC_REQUIRE(std::ranges::forward_range<string_view_t<const T&&>>);

	STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<T>>, Char>);
	STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<const T>>, Char>);
	STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<T&>>, Char>);
	STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<const T&>>, Char>);
	STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<T&&>>, Char>);
	STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<const T&&>>, Char>);
}

TEMPLATE_TEST_CASE(
	"string_traits does not work for actual non-string-types.",
	"[string][string::traits]",
	char,
	const char,
	int,
	std::optional<std::string>,
	std::string&
)
{
	using traits_t = string_traits<TestType>;

	STATIC_REQUIRE(!requires{ traits_t{}; });
}

TEMPLATE_TEST_CASE(
	"Common char-strings support string_traits::view.",
	"[string][string::traits]",
	char*,
	const char*,
	char(&)[14],
	const char(&)[14],
	std::string,
	const std::string,
	std::string_view,
	const std::string_view
)
{
	namespace Matches = Catch::Matchers;
	using traits_t = string_traits<std::remove_cvref_t<TestType>>;

	char str[] = "Hello, World!";

	REQUIRE_THAT(
		traits_t::view(static_cast<TestType>(str)),
		Matches::RangeEquals(std::string_view{str}));
}

TEMPLATE_TEST_CASE(
	"Common wchar_t-strings support string_traits::view.",
	"[string][string::traits]",
	wchar_t*,
	const wchar_t*,
	wchar_t(&)[14],
	const wchar_t(&)[14],
	std::wstring,
	const std::wstring,
	std::wstring_view,
	const std::wstring_view
)
{
	namespace Matches = Catch::Matchers;
	using traits_t = string_traits<std::remove_cvref_t<TestType>>;

	wchar_t str[] = L"Hello, World!";

	REQUIRE_THAT(
		traits_t::view(static_cast<TestType>(str)),
		Matches::RangeEquals(std::wstring_view{str}));
}

TEMPLATE_TEST_CASE(
	"Common char8_t-strings support string_traits::view.",
	"[string][string::traits]",
	char8_t*,
	const char8_t*,
	char8_t(&)[14],
	const char8_t(&)[14],
	std::u8string,
	const std::u8string,
	std::u8string_view,
	const std::u8string_view
)
{
	namespace Matches = Catch::Matchers;
	using traits_t = string_traits<std::remove_cvref_t<TestType>>;

	char8_t str[] = u8"Hello, World!";

	REQUIRE_THAT(
		traits_t::view(static_cast<TestType>(str)),
		Matches::RangeEquals(std::u8string_view{str}));
}

TEMPLATE_TEST_CASE(
	"Common char16_t-strings support string_traits::view.",
	"[string][string::traits]",
	char16_t*,
	const char16_t*,
	char16_t(&)[14],
	const char16_t(&)[14],
	std::u16string,
	const std::u16string,
	std::u16string_view,
	const std::u16string_view
)
{
	namespace Matches = Catch::Matchers;
	using traits_t = string_traits<std::remove_cvref_t<TestType>>;

	char16_t str[] = u"Hello, World!";

	REQUIRE_THAT(
		traits_t::view(static_cast<TestType>(str)),
		Matches::RangeEquals(std::u16string_view{str}));
}

TEMPLATE_TEST_CASE(
	"Common char32_t-strings support string_traits::view.",
	"[string][string::traits]",
	char32_t*,
	const char32_t*,
	char32_t(&)[14],
	const char32_t(&)[14],
	std::u32string,
	const std::u32string,
	std::u32string_view,
	const std::u32string_view
)
{
	namespace Matches = Catch::Matchers;
	using traits_t = string_traits<std::remove_cvref_t<TestType>>;

	char32_t str[] = U"Hello, World!";

	REQUIRE_THAT(
		traits_t::view(static_cast<TestType>(str)),
		Matches::RangeEquals(std::u32string_view{str}));
}

TEMPLATE_TEST_CASE_SIG(
	"string concept determines, whether the given type can be used as a string-type.",
	"[string]",
	((bool expected, typename T), expected, T),
	(false, char),

	(true, CustomString),

	(true, char*),
	(true, const char*),
	(true, wchar_t*),
	(true, const wchar_t*),
	(true, char8_t*),
	(true, const char8_t*),
	(true, char16_t*),
	(true, const char16_t*),
	(true, char32_t*),
	(true, const char32_t*),

	(true, char[]),
	(true, const char[]),
	(true, char[42]),
	(true, const char[42]),
	(true, wchar_t[]),
	(true, const wchar_t[]),
	(true, wchar_t[42]),
	(true, const wchar_t[42]),
	(true, char8_t[]),
	(true, const char8_t[]),
	(true, char8_t[42]),
	(true, const char8_t[42]),
	(true, char16_t[]),
	(true, const char16_t[]),
	(true, char16_t[42]),
	(true, const char16_t[42]),
	(true, char32_t[]),
	(true, const char32_t[]),
	(true, char32_t[42]),
	(true, const char32_t[42]),

	(true, std::string),
	(true, std::wstring),
	(true, std::u8string),
	(true, std::u16string),
	(true, std::u32string),

	(true, std::string_view),
	(true, std::wstring_view),
	(true, std::u8string_view),
	(true, std::u16string_view),
	(true, std::u32string_view)
)
{
	STATIC_REQUIRE(expected == mimicpp::string<T>);
	STATIC_REQUIRE(expected == mimicpp::string<const T>);
	STATIC_REQUIRE(expected == mimicpp::string<T&>);
	STATIC_REQUIRE(expected == mimicpp::string<const T&>);
	STATIC_REQUIRE(expected == mimicpp::string<T&&>);
	STATIC_REQUIRE(expected == mimicpp::string<const T&&>);
}

TEMPLATE_TEST_CASE(
	"string_normalize_converter<char> can be used with all char-strings.",
	"[string]",
	const char*,
	std::string,
	std::string_view
)
{
	namespace Matches = Catch::Matchers;

	const auto [expected, source] = GENERATE(
		(table<std::string, const char*>)({
			{"", ""},
			{" !1337\t", " !1337\t"},
			{"HELLO, WORLD!", "HeLlO, WoRlD!"},
			}));

	auto converted = static_cast<TestType>(source);

	REQUIRE_THAT(
		std::invoke(
			string_normalize_converter<char>{},
			string_traits<TestType>::view(converted)),
		Matches::RangeEquals(expected));
}

TEMPLATE_TEST_CASE_SIG(
	"normalizable_string determines, whether the given string supports normalize conversions.",
	"[string]",
	((bool expected, typename T), expected, T),
	(false, char),

	(true, char*),
	(true, const char*),
	(true, char[]),
	(true, const char[]),
	(true, char[42]),
	(true, const char[42]),
	(true, std::string),
	(true, std::string_view),

	// all below are just temporarily not supported. Would be nice to do so!
	(false, wchar_t*),
	(false, const wchar_t*),
	(false, char8_t*),
	(false, const char8_t*),
	(false, char16_t*),
	(false, const char16_t*),
	(false, char32_t*),
	(false, const char32_t*),

	(false, std::wstring),
	(false, std::u8string),
	(false, std::u16string),
	(false, std::u32string),
	(false, std::wstring_view),
	(false, std::u8string_view),
	(false, std::u16string_view),
	(false, std::u32string_view)
)
{
	STATIC_REQUIRE(expected == normalizable_string<T>);
	STATIC_REQUIRE(expected == normalizable_string<const T>);
	STATIC_REQUIRE(expected == normalizable_string<T&>);
	STATIC_REQUIRE(expected == normalizable_string<const T&>);
	STATIC_REQUIRE(expected == normalizable_string<T&&>);
	STATIC_REQUIRE(expected == normalizable_string<const T&&>);
}