// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Matcher.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <array>

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
	template <typename Source>
	void generic_str_eq_test(
		Source&& pattern,
		std::string descriptionPartExpectation,
		auto match,
		auto mismatch
	)
	{
		const auto matcher = matches::str::eq(pattern);

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
			const auto invertedMatcher = !matches::str::eq(pattern);

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
	const char*,
	const char(&)[14],
	std::string,
	std::string_view
)
{
	constexpr char pattern[] = "Hello, World!";
	generic_str_eq_test(
		static_cast<TestType>(pattern),
		"\"Hello, World!\"",
		std::string{"Hello, World!"},
		std::string{"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target wchar_t-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	const wchar_t*,
	const wchar_t(&)[14],
	std::wstring,
	std::wstring_view
)
{
	constexpr wchar_t pattern[] = L"Hello, World!";
	generic_str_eq_test(
		static_cast<TestType>(pattern),
		"L\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
		std::wstring{L"Hello, World!"},
		std::wstring{L"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target char8_t-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	const char8_t*,
	const char8_t(&)[14],
	std::u8string,
	std::u8string_view
)
{
	constexpr char8_t pattern[] = u8"Hello, World!";
	generic_str_eq_test<TestType>(
		static_cast<TestType>(pattern),
		"u8\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
		std::u8string{u8"Hello, World!"},
		std::u8string{u8"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target char16_t-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	const char16_t*,
	const char16_t(&)[14],
	std::u16string,
	std::u16string_view
)
{
	constexpr char16_t pattern[] = u"Hello, World!";
	generic_str_eq_test<TestType>(
		static_cast<TestType>(pattern),
		"u\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
		std::u16string{u"Hello, World!"},
		std::u16string{u"Hello, WOrld!"});
}

TEMPLATE_TEST_CASE(
	"matches::str::eq matches when target char32_t-string compares equal to the stored one.",
	"[matcher][matcher::str]",
	const char32_t*,
	const char32_t(&)[14],
	std::u32string,
	std::u32string_view
)
{
	constexpr char32_t pattern[] = U"Hello, World!";
	generic_str_eq_test<TestType>(
		static_cast<TestType>(pattern),
		"U\"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21\"",
		std::u32string{U"Hello, World!"},
		std::u32string{U"Hello, WOrld!"});
}

TEST_CASE(
	"matches::str::eq supports case-insensitive comparison for char-strings.",
	"[matcher][matcher::str]"
)
{
	static constexpr std::array matches = std::to_array<const char*>(
		{
			"Hello, World!",
			"hello, World!",
			"Hello, world!",
			"HelLo, world!"
		});

	static constexpr std::array mismatches = std::to_array<const char*>(
		{
			" hello, world!",
			"hello, world! ",
			"hello,world!"
		});

	const auto matcher = matches::str::eq(
		"Hello, World!",
		mimicpp::case_insensitive);
	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("is case-insensitively equal to \"hello, world!\""));

	SECTION("When target is equal, they match.")
	{
		const std::string target = GENERATE(from_range(matches));

		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is not equal, they do not match.")
	{
		const std::string target = GENERATE(from_range(mismatches));

		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::str::eq("Hello, World!");

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("is case-insensitively not equal to \"hello, world!\""));

		SECTION("When target is equal, they do not match.")
		{
			const std::string target = GENERATE(from_range(matches));

			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is not equal, they do match.")
		{
			const std::string target = GENERATE(from_range(mismatches));

			REQUIRE(invertedMatcher.matches(target));
		}
	}
}
