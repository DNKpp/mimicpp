// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Utility.hpp"
#include "mimic++/Printer.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstdint>

using namespace mimicpp;

TEMPLATE_TEST_CASE(
	"to_underlying converts the enum value to its underlying representation.",
	"[utility]",
	std::int8_t,
	std::uint8_t,
	std::int16_t,
	std::uint16_t,
	std::int32_t,
	std::uint32_t,
	std::int64_t,
	std::uint64_t
)
{
	using UnderlyingT = TestType;

	SECTION("When an enum value is given.")
	{
		enum Test
			: UnderlyingT
		{
		};

		const UnderlyingT value = GENERATE(
			std::numeric_limits<UnderlyingT>::min(),
			0,
			1,
			std::numeric_limits<UnderlyingT>::max());

		STATIC_REQUIRE(std::same_as<UnderlyingT, decltype(to_underlying(Test{value}))>);
		REQUIRE(value == to_underlying(Test{value}));
	}

	SECTION("When an class enum value is given.")
	{
		enum class Test
			: UnderlyingT
		{
		};

		const UnderlyingT value = GENERATE(
			std::numeric_limits<UnderlyingT>::min(),
			0,
			1,
			std::numeric_limits<UnderlyingT>::max());

		STATIC_REQUIRE(std::same_as<UnderlyingT, decltype(to_underlying(Test{value}))>);
		REQUIRE(value == to_underlying(Test{value}));
	}
}

TEMPLATE_TEST_CASE_SIG(
	"same_as_any determines, whether T is the same as any other given type.",
	"[utility]",
	((bool expected, typename T, typename... Others), expected, T, Others...),
	(false, int),
	(false, int, int&),
	(false, int, int&, int&),
	(false, int, int&, double, float),
	(true, int, int),
	(true, int, int&, int),
	(true, int, double, int, int&)
)
{
	STATIC_REQUIRE(expected == same_as_any<T, Others...>);
}

TEMPLATE_TEST_CASE_SIG(
	"unique_list_t is an alias to a tuple with just unique types.",
	"[utility]",
	((bool dummy, typename Expected, typename... Types), dummy, Expected, Types...),
	(true, std::tuple<>),
	(true, std::tuple<int, int&>, int, int&),
	(true, std::tuple<int, int&>, int, int&, int&),
	(true, std::tuple<int, int&, double, float>, int, int&, double, float),
	(true, std::tuple<int>, int, int),
	(true, std::tuple<int, int&>, int, int&, int),
	(true, std::tuple<int, double, int&>, int, double, int, int&)
)
{
	STATIC_REQUIRE(std::same_as<Expected, detail::unique_list_t<Types...>>);
}

namespace
{
	class CustomString
	{
	public:
		[[nodiscard]]
		CustomString(std::string str) noexcept
			: m_Internal{std::move(str)}
		{
		}

		[[nodiscard]]
		explicit(false) operator std::string_view() const
		{
			return std::string_view{m_Internal};
		}

	private:
		std::string m_Internal;
	};
}

template <>
struct mimicpp::string_traits<CustomString>
{
	using char_t = char;
	using string_t = std::string_view;
};

TEMPLATE_TEST_CASE_SIG(
	"string concept determines, whether the given type can be used as a string-type.",
	"[utility]",
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
	(true, char(&)[]),
	(true, const char(&)[]),
	(true, char[42]),
	(true, const char[42]),
	(true, char(&)[42]),
	(true, const char(&)[42]),

	(true, std::string),
	(true, const std::string),
	(true, std::string&),
	(true, const std::string&),
	(true, std::string&&),
	(true, const std::string&&),

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
}

template <>
struct custom::normalize_string_converter<CustomString>
{
	[[nodiscard]]
	std::string operator()([[maybe_unused]] const CustomString& str) const
	{
		return "custom::normalize_string_converter<CustomString>";
	}
};

TEST_CASE(
	"normalize_string selects the correct converter.",
	"[utility]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("custom::normalize_string_converter specializations are preferred.")
	{
		const std::string result = normalize_string(CustomString{"Hello, World!"});

		REQUIRE_THAT(
			result,
			Matches::Equals("custom::normalize_string_converter<CustomString>"));
	}

	SECTION("Otherwise, normalize_string_hook::normalize_string_converter is chosen.")
	{
		const std::string result = normalize_string("Hello, World!");

		REQUIRE_THAT(
			result,
			Matches::Equals("HELLO, WORLD!"));
	}
}

TEMPLATE_TEST_CASE(
	"normalize_string converts the given string to its normalized representation.",
	"[utility]",
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

	const std::string result = normalize_string(static_cast<TestType>(source));

	REQUIRE_THAT(
		result,
		Matches::Equals(expected));
}

TEMPLATE_TEST_CASE_SIG(
	"normalizable_string determines, whether the given string supports normalize conversions.",
	"[utility]",
	((bool expected, typename T), expected, T),
	(false, char),

	(true, char*),
	(true, const char*),
	(true, char[]),
	(true, const char[]),
	(true, char(&)[]),
	(true, const char(&)[]),
	(true, char[42]),
	(true, const char[42]),
	(true, char(&)[42]),
	(true, const char(&)[42]),
	(true, std::string),
	(true, const std::string),
	(true, std::string&),
	(true, const std::string&),
	(true, std::string&&),
	(true, const std::string&&),
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
	STATIC_REQUIRE(expected == mimicpp::normalizable_string<T>);
}
