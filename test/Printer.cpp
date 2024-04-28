// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Printer.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>

using namespace mimicpp;

namespace
{
	class NonPrintable
	{
	};
}

TEMPLATE_TEST_CASE_SIG(
	"detail::formattable determines whether the given type has a std::formatter specialization.",
	"[print]",
	((bool expected, typename T, typename Char), expected, T, Char),
	(true, int, char),
	(true, const int, char),
	(true, int&, char),
	(true, const int&, char),
	(true, int, wchar_t),
	(false, NonPrintable, char)
)
{
	STATIC_REQUIRE(expected == detail::formattable<T, Char>);
}

namespace
{
	class CustomPrintable
	{
	};

	class StdFormatPrintable
	{
	};

	class StdFormatAndCustomPrintable
	{
	};
}

template <>
class custom::Printer<CustomPrintable>
{
public:
	inline static int printCallCounter{0};

	static auto print(print_iterator auto out, const CustomPrintable&)
	{
		++printCallCounter;
		return format::format_to(out, "CustomPrintable");
	}
};

template <typename Char>
struct std::formatter<StdFormatPrintable, Char>
{
	inline static int printCallCounter{0};

	static constexpr auto parse(auto& ctx)
	{
		return ctx.begin();
	}

	static auto format(const StdFormatPrintable&, auto& ctx)
	{
		++printCallCounter;
		return format::format_to(ctx.out(), "StdFormatPrintable");
	}
};

template <>
class custom::Printer<StdFormatAndCustomPrintable>
{
public:
	inline static int printCallCounter{0};

	static auto print(print_iterator auto out, const StdFormatAndCustomPrintable&)
	{
		++printCallCounter;
		return format::format_to(out, "StdFormatAndCustomPrintable");
	}
};

template <typename Char>
struct std::formatter<StdFormatAndCustomPrintable, Char>
{
	inline static int printCallCounter{0};

	static constexpr auto parse(auto& ctx)
	{
		return ctx.begin();
	}

	static auto format(const StdFormatAndCustomPrintable&, auto& ctx)
	{
		++printCallCounter;
		return format::format_to(ctx.out(), "StdFormatAndCustomPrintable");
	}
};

TEST_CASE(
	"print selects the best option to print a given value.",
	"[print]"
)
{
	std::basic_stringstream<CharT> stream{};

	SECTION("custom::Printer specialization is supported.")
	{
		using PrinterT = custom::Printer<CustomPrintable>;
		PrinterT::printCallCounter = 0;

		constexpr CustomPrintable value{};

		print(std::ostreambuf_iterator{stream}, value);
		REQUIRE(PrinterT::printCallCounter == 1);

		REQUIRE_THAT(
			std::move(stream).str(),
			Catch::Matchers::Equals("CustomPrintable"));
	}

	SECTION("std::formatter specialization is supported.")
	{
		using PrinterT = std::formatter<StdFormatPrintable, CharT>;
		PrinterT::printCallCounter = 0;

		constexpr StdFormatPrintable value{};

		print(std::ostreambuf_iterator{stream}, value);
		REQUIRE(PrinterT::printCallCounter == 1);
		REQUIRE_THAT(
			std::move(stream).str(),
			Catch::Matchers::Equals("StdFormatPrintable"));
	}

	SECTION("custom::Printer specialization is prefered.")
	{
		using PrinterT = custom::Printer<StdFormatAndCustomPrintable>;
		PrinterT::printCallCounter = 0;

		constexpr StdFormatAndCustomPrintable value{};

		print(std::ostreambuf_iterator{stream}, value);
		REQUIRE(PrinterT::printCallCounter == 1);
		REQUIRE_THAT(
			std::move(stream).str(),
			Catch::Matchers::Equals("StdFormatAndCustomPrintable"));
	}

	SECTION("Ranges have special treatment.")
	{
		SECTION("Empty ranges are supported.")
		{
			const std::vector<int> vec{};

			print(std::ostreambuf_iterator{stream}, vec);
			REQUIRE_THAT(
				std::move(stream).str(),
				Catch::Matchers::Equals("{  }"));
		}

		SECTION("Ranges with single element are supported.")
		{
			const std::vector vec{42};

			print(std::ostreambuf_iterator{stream}, vec);
			REQUIRE_THAT(
				std::move(stream).str(),
				Catch::Matchers::Equals("{ 42 }"));
		}

		SECTION("Ranges with multiple elements are supported.")
		{
			const std::vector vec{42, 1337};

			print(std::ostreambuf_iterator{stream}, vec);
			REQUIRE_THAT(
				std::move(stream).str(),
				Catch::Matchers::Equals("{ 42, 1337 }"));
		}

		SECTION("Range elements are forwarded to the print function..")
		{
			const std::vector vec{NonPrintable{}, NonPrintable{}, NonPrintable{}};

			print(std::ostreambuf_iterator{stream}, vec);
			REQUIRE_THAT(
				std::move(stream).str(),
				Catch::Matchers::Equals("{ {?}, {?}, {?} }"));
		}
	}

	SECTION("When nothing matches, a default token is inserted.")
	{
		constexpr NonPrintable value{};
		print(std::ostreambuf_iterator{stream}, value);
		REQUIRE_THAT(
			std::move(stream).str(),
			Catch::Matchers::Equals("{?}"));
	}
}
