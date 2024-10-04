// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTER_HPP
#define MIMICPP_PRINTER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/String.hpp"
#include "mimic++/TypeTraits.hpp"
#include "mimic++/Utility.hpp"

#include <cstdint>
#include <functional>
#include <iterator>
#include <ranges>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#ifndef MIMICPP_CONFIG_USE_FMT
	#include <format>
#else

#if __has_include(<fmt/format.h>)
#include <fmt/format.h>
#else
		#error "The fmt formatting backend is explicitly enabled, but the include <fmt/format.h> can not be found."
#endif

#endif

namespace mimicpp
{
	using StringViewT = std::basic_string_view<CharT, CharTraitsT>;
	using StringStreamT = std::basic_ostringstream<CharT, CharTraitsT>;

	template <typename T>
	concept print_iterator = std::output_iterator<T, const CharT&>;

	template <typename Printer, typename OutIter, typename T>
	concept printer_for = print_iterator<OutIter>
						&& requires(OutIter out)
						{
							{
								Printer::print(out, std::declval<T&&>())
							} -> std::convertible_to<OutIter>;
						};
}

namespace mimicpp::format
{
#ifndef MIMICPP_CONFIG_USE_FMT

	// use std format
#ifndef _LIBCPP_VERSION

	using std::formatter;
	using std::format;
	using std::format_to;
	using std::vformat;
	using std::vformat_to;
	using std::make_format_args;

#else

	// libc++ has some serious trouble when using its std::format implementation.
	// Let's simply redirect any calls to std::vformat instead.

	using std::formatter;
	using std::vformat;
	using std::vformat_to;
	using std::make_format_args;

	template <typename... Args>
	[[nodiscard]]
	StringT format(const StringViewT fmt, Args&&... args)  // NOLINT(cppcoreguidelines-missing-std-forward)
	{
		return format::vformat(
			fmt,
			std::make_format_args(args...));
	}

	template <class OutputIt, typename... Args>
	OutputIt format_to(const OutputIt out, const StringViewT fmt, Args&&... args)  // NOLINT(cppcoreguidelines-missing-std-forward)
	{
		return format::vformat_to(
			std::move(out),
			fmt,
			std::make_format_args(args...));
	}

#endif

	namespace detail
	{
		template <typename Char>
		struct format_context;

		template <typename Char>
		using format_context_t = typename format_context<Char>::type;

		template <>
		struct format_context<char>
		{
			using type = std::format_context;
		};

		template <>
		struct format_context<wchar_t>
		{
			using type = std::wformat_context;
		};

		/**
		* \brief Determines, whether a complete specialization of ``std::formatter`` for the given (possibly cv-ref qualified) type exists.
		* \tparam T Type to check.
		* \tparam Char Used character type.
		* \details This is an adapted implementation of the ``std::formattable`` concept, which is added c++23.
		* \note This implementation takes a simple but reasonable shortcut in assuming, that ```Char`` is either ``char`` or ``wchar_t``,
		* which must not necessarily true.
		* \see Adapted from here: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2286r8.html#concept-formattable
		* \see https://en.cppreference.com/w/cpp/utility/format/formattable
		*/
		template <class T, class Char>
		concept formattable =
			std::semiregular<std::formatter<std::remove_cvref_t<T>, Char>>
			&& requires(
			std::formatter<std::remove_cvref_t<T>, Char> formatter,
			T t,
			format_context_t<Char> formatContext,
			std::basic_format_parse_context<Char> parseContext
		)
			{
				{ formatter.parse(parseContext) } -> std::same_as<typename std::basic_format_parse_context<Char>::iterator>;
				{
					std::as_const(formatter).format(t, formatContext)
				} -> std::same_as<typename std::remove_reference_t<decltype(formatContext)>::iterator>;
			};
	}

	// use fmt format
#else

	using fmt::formatter;
	using fmt::format;
	using fmt::format_to;
	using fmt::vformat;
	using fmt::vformat_to;
	using fmt::make_format_args;

	namespace detail
	{
		template <class T, class Char>
		concept formattable = fmt::is_formattable<std::remove_reference_t<T>, Char>::value;
	}

#endif
}

template <>
struct mimicpp::format::formatter<mimicpp::ValueCategory, mimicpp::CharT>
	: public formatter<std::string_view, mimicpp::CharT>
{
	using ValueCategoryT = mimicpp::ValueCategory;

	auto format(
		const ValueCategoryT category,
		auto& ctx
	) const
	{
		constexpr auto toString = [](const ValueCategoryT cat)
		{
			switch (cat)
			{
			case ValueCategoryT::lvalue: return "lvalue";
			case ValueCategoryT::rvalue: return "rvalue";
			case ValueCategoryT::any: return "any";
			}

			throw std::invalid_argument{"Unknown category value."};
		};

		return formatter<std::string_view, mimicpp::CharT>::format(
			toString(category),
			ctx);
	}
};

template <>
struct mimicpp::format::formatter<mimicpp::Constness, mimicpp::CharT>
	: public formatter<std::string_view, mimicpp::CharT>
{
	using ConstnessT = mimicpp::Constness;

	auto format(
		const ConstnessT category,
		auto& ctx
	) const
	{
		constexpr auto toString = [](const ConstnessT value)
		{
			switch (value)
			{
			case ConstnessT::non_const: return "mutable";
			case ConstnessT::as_const: return "const";
			case ConstnessT::any: return "any";
			}

			throw std::invalid_argument{"Unknown constness value."};
		};

		return formatter<std::string_view, mimicpp::CharT>::format(
			toString(category),
			ctx);
	}
};

namespace mimicpp::custom
{
	/**
	 * \brief User may add specializations, which will then be used during ``print`` calls.
	 * \ingroup STRINGIFICATION
	 */
	template <typename>
	class Printer;
}

namespace mimicpp::detail
{
	template <
		print_iterator OutIter,
		typename T,
		printer_for<OutIter, T> Printer = custom::Printer<std::remove_cvref_t<T>>>
	OutIter print(
		OutIter out,
		T&& value,
		[[maybe_unused]] const priority_tag<5>
	)
	{
		return Printer::print(
			std::move(out),
			std::forward<T>(value));
	}

	template <typename>
	class Printer;

	template <
		print_iterator OutIter,
		typename T,
		printer_for<OutIter, T> Printer = Printer<std::remove_cvref_t<T>>>
	OutIter print(
		OutIter out,
		T&& value,
		[[maybe_unused]] const priority_tag<4>
	)
	{
		return Printer::print(
			std::move(out),
			std::forward<T>(value));
	}

	template <print_iterator OutIter, std::convertible_to<StringViewT> String>
	OutIter print(
		OutIter out,
		String&& str,
		[[maybe_unused]] const priority_tag<3>
	)
	{
		return format::format_to(
			std::move(out),
			"\"{}\"",
			static_cast<StringViewT>(std::forward<String>(str)));
	}

	template <print_iterator OutIter, std::ranges::forward_range Range>
	OutIter print(
		OutIter out,
		Range&& range,
		priority_tag<2>
	);

	template <print_iterator OutIter, format::detail::formattable<CharT> T>
	OutIter print(
		OutIter out,
		T&& value,
		[[maybe_unused]] const priority_tag<1>
	)
	{
		return format::format_to(
			std::move(out),
			"{}",
			std::forward<T>(value));
	}

	template <print_iterator OutIter>
	OutIter print(
		OutIter out,
		auto&&,
		[[maybe_unused]] const priority_tag<0>
	)
	{
		return format::format_to(
			std::move(out),
			"{{?}}");
	}

	class PrintFn
	{
	public:
		template <print_iterator OutIter, typename T>
		OutIter operator ()(
			OutIter out,
			T&& value
		) const
		{
			static_assert(
				requires(const priority_tag<6> tag)
				{
					{ print(out, std::forward<T>(value), tag) } -> std::convertible_to<OutIter>;
				},
				"The given type is not printable. ");

			return print(
				std::move(out),
				std::forward<T>(value),
				priority_tag<6>{});
		}

		template <typename T>
		StringT operator ()(T&& value) const
		{
			StringStreamT stream{};
			std::invoke(
				*this,
				std::ostreambuf_iterator{stream},
				std::forward<T>(value));
			return std::move(stream).str();
		}
	};

	template <print_iterator OutIter, std::ranges::forward_range Range>
	OutIter print(
		OutIter out,
		Range&& range,  // NOLINT(cppcoreguidelines-missing-std-forward)
		const priority_tag<2>
	)
	{
		out = format::format_to(
			std::move(out),
			"{{ ");
		auto iter = std::ranges::begin(range);
		if (const auto end = std::ranges::end(range);
			iter != end)
		{
			constexpr PrintFn print{};
			out = print(std::move(out), *iter++);

			for (; iter != end; ++iter)
			{
				out = print(
					format::format_to(std::move(out), ", "),
					*iter);
			}
		}

		return format::format_to(
			std::move(out),
			" }}");
	}

	template <>
	class Printer<std::source_location>
	{
	public:
		template <print_iterator OutIter>
		static OutIter print(OutIter out, const std::source_location& loc)
		{
			return format::format_to(
				std::move(out),
				"{}[{}:{}], {}",
				loc.file_name(),
				loc.line(),
				loc.column(),
				loc.function_name());
		}
	};

	template <typename Char>
		requires is_character_v<Char>
	struct character_literal_printer;

	template <>
	struct character_literal_printer<char>
	{
		template <print_iterator OutIter>
		static OutIter print(OutIter out) noexcept
		{
			// no special character-literal
			return out;
		}
	};

	template <>
	struct character_literal_printer<wchar_t>
	{
		template <print_iterator OutIter>
		static OutIter print(OutIter out)
		{
			return format::format_to(std::move(out), "L");
		}
	};

	template <>
	struct character_literal_printer<char8_t>
	{
		template <print_iterator OutIter>
		static OutIter print(OutIter out)
		{
			return format::format_to(std::move(out), "u8");
		}
	};

	template <>
	struct character_literal_printer<char16_t>
	{
		template <print_iterator OutIter>
		static OutIter print(OutIter out)
		{
			return format::format_to(std::move(out), "u");
		}
	};

	template <>
	struct character_literal_printer<char32_t>
	{
		template <print_iterator OutIter>
		static OutIter print(OutIter out)
		{
			return format::format_to(std::move(out), "U");
		}
	};

	template <string String>
		requires (!std::same_as<CharT, string_char_t<String>>)
	class Printer<String>
	{
	public:
		template <std::common_reference_with<String> T, print_iterator OutIter>
		static OutIter print(OutIter out, T&& str)
		{
			using intermediate_t = std::uint32_t;
			static_assert(sizeof(string_char_t<String>) <= sizeof(intermediate_t));

			out = character_literal_printer<string_char_t<String>>::print(std::move(out));
			out = format::format_to(std::move(out), "\"");

			auto view = string_traits<std::remove_cvref_t<T>>::view(std::forward<T>(str));
			auto iter = std::ranges::begin(view);
			if (const auto end = std::ranges::end(view);
				iter != end)
			{
				out = format::format_to(
					std::move(out),
					"{:#x}",
					static_cast<intermediate_t>(*iter++));

				for (; iter != end; ++iter)
				{
					out = format::format_to(
						std::move(out),
						", {:#x}",
						static_cast<intermediate_t>(*iter));
				}
			}

			return format::format_to(std::move(out), "\"");
		}
	};
}

namespace mimicpp
{
	/**
	 * \defgroup STRINGIFICATION stringification
	 * \brief Stringification describes the process of converting an object state into its textual representation.
	 * \details ``mimic++`` often wants to present users a textual representation of their tested objects,
	 * because they might have failed a test case. It utilizes the ``print`` function for that purpose.
	 *
	 * That function internally checks for the first available option (in that order):
	 * - ``mimicpp::custom::Printer`` specialization
	 * - internal printer specializations
	 * - convertible to ``std::string_view``
	 * - satisfies ``std::ranges::forward_range``
	 * - formattable type (in terms of the installed format-backend)
	 *
	 * If no valid alternative has been found, the default is chosen (which just prints "{?}").
	 *
	 * ## Override existing printings or print custom types
	 *
	 * As ``mimic++`` can not know how to convert any custom type out there, a simple but effective mechanism is used.
	 * Users can add a specialization of ``mimicpp::custom::Printer`` for their own or third-party types.
	 * ``mimic++`` will always prefer such a specialization over any other internal alternative, even if ``mimic++`` already
	 * has special treatment that particular type (e.g. ``std::source_location``).
	 *
	 * Given the following type.
	 * \snippet CustomPrinter.cpp my_type
	 * Users can then create a specialization as follows:
	 * \snippet CustomPrinter.cpp my_type printer
	 *
	 * When an object of ``my_type`` is then passed to ``print``, that specification will be used:
	 * \snippet CustomPrinter.cpp my_type print
	 *
	 * ## String printing
	 *
	 * All char-strings (like ``const char*`` or ``std::string``) will be printed as they are.
	 * Other character-types (e.g. ``wchar_t`` or ``char8_t``) must be treated with care. Making ``mimic++`` 100% compatible with any
	 * existing character-type is either a major work-load or has to be outsourced to a dependency.
	 * Currently, ``mimic++`` chooses another option: If a string of a non-printable character-type (in terms of the type-trait ``is_character``)
	 * is detected, it prints the string-literal and all elements are converted to their value-representation and printed as comma separated hex-values.
	 *
	 * For example, the string ``u8"Hello, World!"`` will then be printed as
	 * ``u8"0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21"``.
	 *
	 *\{
	 */

	/**
	 * \brief Functional object, converting the given object to its textual representation.
	 */
	inline constexpr detail::PrintFn print{};

	/**
	 * \}
	 */
}

#endif
