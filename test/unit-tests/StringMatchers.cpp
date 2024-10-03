// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Matcher.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace matches = mimicpp::matches;

TEST_CASE(
	"matches::str::eq does support empty strings.",
	"[matcher][matcher::str]"
)
{
	const auto matcher = matches::str::eq("");
	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("is equal to \"\""));
	REQUIRE(matcher.matches(""));
	REQUIRE(!matcher.matches(" "));
}

namespace
{
	template <mimicpp::string String, typename Source>
	void generic_str_eq_test(
		Source&& pattern,
		std::string descriptionPartExpectation,
		auto match,
		auto mismatch
	)
	{
		const auto matcher = matches::str::eq(static_cast<String>(pattern));

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is equal to " + descriptionPartExpectation));

		SECTION("When target is equal, they match.")
		{
			REQUIRE(matcher.matches(match));
		}

		SECTION("When target is not equal, they do not match.")
		{
			REQUIRE(!matcher.matches(mismatch));
		}

		SECTION("Matcher can be inverted.")
		{
			const auto invertedMatcher = !matches::str::eq(static_cast<String>(pattern));

			REQUIRE_THAT(
				invertedMatcher.describe(),
				Catch::Matchers::Equals("is not equal to " + descriptionPartExpectation));

			SECTION("When target is equal, they do not match.")
			{
				REQUIRE(!invertedMatcher.matches(match));
			}

			SECTION("When target is not equal, they do match.")
			{
				REQUIRE(invertedMatcher.matches(mismatch));
			}
		}
	}
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target char-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	char*,
	const char*,
	char[14],
	const char[14],
	std::string,
	const std::string,
	std::string_view,
	const std::string_view
)
{
	char pattern[] = "Hello, World!";

	generic_str_eq_test<TestType>(
		pattern,
		"\"Hello, World!\"",
		std::string{"Hello, World!"},
		std::string{"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target wchar_t-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	wchar_t*,
	const wchar_t*,
	wchar_t[14],
	const wchar_t[14],
	std::wstring,
	const std::wstring,
	std::wstring_view,
	const std::wstring_view
)
{
	wchar_t pattern[] = L"Hello, World!";
	generic_str_eq_test<TestType>(
		pattern,
		"L\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
		std::wstring{L"Hello, World!"},
		std::wstring{L"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target char8_t-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	char8_t*,
	const char8_t*,
	char8_t[14],
	const char8_t[14],
	std::u8string,
	const std::u8string,
	std::u8string_view,
	const std::u8string_view
)
{
	char8_t pattern[] = u8"Hello, World!";
	generic_str_eq_test<TestType>(
		pattern,
		"u8\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
		std::u8string{u8"Hello, World!"},
		std::u8string{u8"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target char16_t-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	char16_t*,
	const char16_t*,
	char16_t[14],
	const char16_t[14],
	std::u16string,
	const std::u16string,
	std::u16string_view,
	const std::u16string_view
)
{
	char16_t pattern[] = u"Hello, World!";
	generic_str_eq_test<TestType>(
		pattern,
		"u\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
		std::u16string{u"Hello, World!"},
		std::u16string{u"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target char32_t-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	char32_t*,
	const char32_t*,
	char32_t[14],
	const char32_t[14],
	std::u32string,
	const std::u32string,
	std::u32string_view,
	const std::u32string_view
)
{
	char32_t pattern[] = U"Hello, World!";
	generic_str_eq_test<TestType>(
		pattern,
		"U\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
		std::u32string{U"Hello, World!"},
		std::u32string{U"Hello, WOrld!"});
}
