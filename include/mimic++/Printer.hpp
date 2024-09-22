// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTER_HPP
#define MIMICPP_PRINTER_HPP

#pragma once

#include "Fwd.hpp"

#include <format>
#include <iterator>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

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
#ifndef _LIBCPP_VERSION

	using std::format;
	using std::format_to;
	using std::vformat;
	using std::vformat_to;
	using std::make_format_args;

#else

	// libc++ has some serious trouble when using its std::format implementation.
	// Let's simply redirect any calls to std::vformat instead.

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
			out,
			fmt,
			std::make_format_args(args...));
	}

#endif
}

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
	template <std::size_t priority>
	struct priority_tag
		: public priority_tag<priority - 1>
	{
	};

	template <>
	struct priority_tag<0>
	{
	};

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
			out,
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
			out,
			std::forward<T>(value));
	}

	template <typename OutIter, std::convertible_to<StringViewT> String>
	OutIter print(
		OutIter out,
		String&& str,
		[[maybe_unused]] const priority_tag<3>
	)
	{
		return format::format_to(
			out,
			"\"{}\"",
			static_cast<StringViewT>(std::forward<String>(str)));
	}

	template <typename OutIter, std::ranges::forward_range Range>
	OutIter print(
		OutIter out,
		Range&& range,
		priority_tag<2>
	);

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

	template <typename OutIter, formattable<CharT> T>
	OutIter print(
		OutIter out,
		T& value,
		[[maybe_unused]] const priority_tag<1>
	)
	{
		return format::format_to(out, "{}", value);
	}

	template <typename OutIter>
	OutIter print(
		OutIter out,
		auto&,
		[[maybe_unused]] const priority_tag<0>
	)
	{
		return format::format_to(out, "{{?}}");
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
				out,
				std::forward<T>(value),
				priority_tag<6>{});
		}

		template <typename T>
		StringT operator ()(T&& value) const
		{
			StringStreamT stream{};
			operator()(
				std::ostreambuf_iterator{stream},
				std::forward<T>(value));
			return std::move(stream).str();
		}
	};

	template <typename OutIter, std::ranges::forward_range Range>
	OutIter print(
		OutIter out,
		Range&& range,  // NOLINT(cppcoreguidelines-missing-std-forward)
		const priority_tag<2>
	)
	{
		out = format::format_to(out, "{{ ");
		auto iter = std::ranges::begin(range);
		if (const auto end = std::ranges::end(range);
			iter != end)
		{
			constexpr PrintFn print{};
			out = print(out, *iter++);

			for (; iter != end; ++iter)
			{
				out = print(
					format::format_to(out, ", "),
					*iter);
			}
		}

		return format::format_to(out, " }}");
	}

	template <>
	class Printer<std::source_location>
	{
	public:
		template <typename OutIter>
		static OutIter print(OutIter out, const std::source_location& loc)
		{
			return format::format_to(
				out,
				"{}[{}:{}], {}",
				loc.file_name(),
				loc.line(),
				loc.column(),
				loc.function_name());
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
	 * - convertible to ``std::string_view``
	 * - satisfies ``std::ranges::forward_range``
	 * - is ``std::source_location``
	 *
	 * If no valid alternative has been found, the default is chosen.
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
