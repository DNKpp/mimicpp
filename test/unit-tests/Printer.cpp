// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Printer.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <sstream>

using namespace mimicpp;

namespace
{
	using StringPrintIteratorT = std::string::iterator;
	using WStringPrintIteratorT = std::wstring::iterator;

	class NonPrinter
	{
	};

	class CustomIntPrinter
	{
	public:
		template <typename OutIter>
		static OutIter print(OutIter out, [[maybe_unused]] int& value)
		{
			return out;
		}
	};

	class CustomConstIntPrinter
	{
	public:
		template <typename OutIter>
		static OutIter print(OutIter out, [[maybe_unused]] const int& value)
		{
			return out;
		}
	};
}

TEMPLATE_TEST_CASE_SIG(
	"printer_for determines whether the given type satisfies the requirements.",
	"[print]",
	((bool expected, typename Printer, typename OutIter, typename T), expected, Printer, OutIter, T),
	(false, NonPrinter, StringPrintIteratorT, int&),
	(false, NonPrinter, WStringPrintIteratorT, int&),

	(false, CustomIntPrinter, StringPrintIteratorT, const float&),
	(false, CustomIntPrinter, WStringPrintIteratorT, const float&),
	(true, CustomIntPrinter, StringPrintIteratorT, int&),
	(true, CustomIntPrinter, WStringPrintIteratorT, int&),
	(false, CustomIntPrinter, StringPrintIteratorT, const int&),
	(false, CustomIntPrinter, WStringPrintIteratorT, const int&),
	(false, CustomIntPrinter, StringPrintIteratorT, int&&),
	(false, CustomIntPrinter, WStringPrintIteratorT, int&&),
	(false, CustomIntPrinter, StringPrintIteratorT, const int&&),
	(false, CustomIntPrinter, WStringPrintIteratorT, const int&&),

	(false, CustomConstIntPrinter, StringPrintIteratorT, const std::string&),
	(false, CustomConstIntPrinter, WStringPrintIteratorT, const std::wstring&),
	(true, CustomConstIntPrinter, StringPrintIteratorT, int&),
	(true, CustomConstIntPrinter, WStringPrintIteratorT, int&),
	(true, CustomConstIntPrinter, StringPrintIteratorT, const int&),
	(true, CustomConstIntPrinter, WStringPrintIteratorT, const int&),
	(true, CustomConstIntPrinter, StringPrintIteratorT, int&&),
	(true, CustomConstIntPrinter, WStringPrintIteratorT, int&&),
	(true, CustomConstIntPrinter, StringPrintIteratorT, const int&&),
	(true, CustomConstIntPrinter, WStringPrintIteratorT, const int&&)
)
{
	STATIC_REQUIRE(expected == printer_for<Printer, OutIter, T>);
}

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
	STATIC_REQUIRE(expected == format::detail::formattable<T, Char>);
}

namespace
{
	class CustomPrintable
	{
	};

	class InternalPrintable
	{
	};

	class CustomAndInternalPrintable
	{
	};

	class FormatPrintable
	{
	};

	class FormatAndCustomPrintable
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

template <>
class detail::Printer<InternalPrintable>
{
public:
	inline static int printCallCounter{0};

	static auto print(print_iterator auto out, const InternalPrintable&)
	{
		++printCallCounter;
		return format::format_to(out, "InternalPrintable");
	}
};

template <>
class custom::Printer<CustomAndInternalPrintable>
{
public:
	inline static int printCallCounter{0};

	static auto print(print_iterator auto out, const CustomAndInternalPrintable&)
	{
		++printCallCounter;
		return format::format_to(out, "CustomAndInternalPrintable - custom::Printer");
	}
};

template <>
class detail::Printer<CustomAndInternalPrintable>
{
public:
	inline static int printCallCounter{0};

	static auto print(print_iterator auto out, const CustomAndInternalPrintable&)
	{
		++printCallCounter;
		return format::format_to(out, "CustomAndInternalPrintable - detail::Printer");
	}
};

template <typename Char>
struct format::formatter<FormatPrintable, Char>
{
	inline static int printCallCounter{0};

	static constexpr auto parse(auto& ctx)
	{
		return ctx.begin();
	}

	static auto format(const FormatPrintable&, auto& ctx)
	{
		++printCallCounter;
		return format::format_to(ctx.out(), "FormatPrintable");
	}
};

template <>
class custom::Printer<FormatAndCustomPrintable>
{
public:
	inline static int printCallCounter{0};

	static auto print(print_iterator auto out, const FormatAndCustomPrintable&)
	{
		++printCallCounter;
		return format::format_to(out, "FormatAndCustomPrintable");
	}
};

template <typename Char>
struct format::formatter<FormatAndCustomPrintable, Char>
{
	inline static int printCallCounter{0};

	static constexpr auto parse(auto& ctx)
	{
		return ctx.begin();
	}

	static auto format(const FormatAndCustomPrintable&, auto& ctx)
	{
		++printCallCounter;
		return format::format_to(ctx.out(), "FormatAndCustomPrintable");
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

	SECTION("detail::Printer specialization is supported.")
	{
		using PrinterT = detail::Printer<InternalPrintable>;
		PrinterT::printCallCounter = 0;

		constexpr InternalPrintable value{};

		print(std::ostreambuf_iterator{stream}, value);
		REQUIRE(PrinterT::printCallCounter == 1);

		REQUIRE_THAT(
			std::move(stream).str(),
			Catch::Matchers::Equals("InternalPrintable"));
	}

	SECTION("custom::Printer is preferred, if both printers have valid specializations.")
	{
		using CustomPrinterT = custom::Printer<CustomAndInternalPrintable>;
		using DetailPrinterT = detail::Printer<CustomAndInternalPrintable>;
		CustomPrinterT::printCallCounter = 0;
		DetailPrinterT::printCallCounter = 0;

		constexpr CustomAndInternalPrintable value{};

		print(std::ostreambuf_iterator{stream}, value);
		REQUIRE(CustomPrinterT::printCallCounter == 1);
		REQUIRE(DetailPrinterT::printCallCounter == 0);

		REQUIRE_THAT(
			stream.str(),
			Catch::Matchers::Equals("CustomAndInternalPrintable - custom::Printer"));
	}

	SECTION("std::formatter specialization is supported.")
	{
		using PrinterT = format::formatter<FormatPrintable, CharT>;
		PrinterT::printCallCounter = 0;

		constexpr FormatPrintable value{};

		print(std::ostreambuf_iterator{stream}, value);
		REQUIRE(PrinterT::printCallCounter == 1);
		REQUIRE_THAT(
			std::move(stream).str(),
			Catch::Matchers::Equals("FormatPrintable"));
	}

	SECTION("custom::Printer specialization is preferred.")
	{
		using PrinterT = custom::Printer<FormatAndCustomPrintable>;
		PrinterT::printCallCounter = 0;

		constexpr FormatAndCustomPrintable value{};

		print(std::ostreambuf_iterator{stream}, value);
		REQUIRE(PrinterT::printCallCounter == 1);
		REQUIRE_THAT(
			std::move(stream).str(),
			Catch::Matchers::Equals("FormatAndCustomPrintable"));
	}

	SECTION("Strings have special treatment.")
	{
		SECTION("Empty strings are supported.")
		{
			const std::string str{};

			print(std::ostreambuf_iterator{stream}, str);
			REQUIRE_THAT(
				std::move(stream).str(),
				Catch::Matchers::Equals("\"\""));
		}

		SECTION("Strings with arbitrary size are supported.")
		{
			const std::string str{"Hello, World!"};

			print(std::ostreambuf_iterator{stream}, str);
			REQUIRE_THAT(
				std::move(stream).str(),
				Catch::Matchers::Equals("\"Hello, World!\""));
		}

		SECTION("Raw string literals are supported.")
		{
			print(std::ostreambuf_iterator{stream}, "Hello, World!");
			REQUIRE_THAT(
				std::move(stream).str(),
				Catch::Matchers::Equals("\"Hello, World!\""));
		}

		SECTION("std::string_views are supported.")
		{
			print(std::ostreambuf_iterator{stream}, StringViewT{"Hello, World!"});
			REQUIRE_THAT(
				std::move(stream).str(),
				Catch::Matchers::Equals("\"Hello, World!\""));
		}
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

	SECTION("std::source_location has specialized printer.")
	{
		const std::source_location loc = std::source_location::current();

		REQUIRE_THAT(
			mimicpp::print(loc),
			Catch::Matchers::Matches(".+\\[\\d+:\\d+\\], .+"));
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

TEST_CASE(
	"ValueCategory is formattable.",
	"[print]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("When valid ValueCategory is given.")
	{
		const auto [expected, category] = GENERATE(
			(table<StringT, ValueCategory>)({
				{"any", ValueCategory::any},
				{"rvalue",ValueCategory::rvalue},
				{"lvalue", ValueCategory::lvalue},
				}));

		REQUIRE_THAT(
			format::format("{}", category),
			Matches::Equals(expected));
	}

	SECTION("When an invalid ValueCategory is given, std::invalid_argument is thrown.")
	{
		REQUIRE_THROWS_AS(
			format::format("{}", ValueCategory{42}),
			std::invalid_argument);
	}
}

TEST_CASE(
	"Constness is formattable.",
	"[print]"
)
{
	namespace Matches = Catch::Matchers;

	SECTION("When valid Constness is given.")
	{
		const auto [expected, category] = GENERATE(
			(table<StringT, Constness>)({
				{"any", Constness::any},
				{"const",Constness::as_const},
				{"mutable", Constness::non_const},
				}));

		REQUIRE_THAT(
			format::format("{}", category),
			Matches::Equals(expected));
	}

	SECTION("When an invalid Constness is given, std::invalid_argument is thrown.")
	{
		REQUIRE_THROWS_AS(
			format::format("{}", Constness{42}),
			std::invalid_argument);
	}
}
