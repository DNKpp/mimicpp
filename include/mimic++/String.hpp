// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_STRING_HPP
#define MIMICPP_STRING_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Utility.hpp"

#include <concepts>
#include <functional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

namespace mimicpp
{
	/**
	 * \defgroup STRING string
	 * \brief Contains symbols for generic string processing.
	 * \details There are plenty of different character-types and even more string-types out there.
	 * They mostly differ in their details, but in general some common operations shall be applied (like equality- or
	 * prefix-tests).
	 * Therefore, ``mimic++`` offers various abstractions, which users can utilize to make their char- and string-types compatible with the framework,
	 * and thus with all string-matchers.
	 *
	 * ## Custom Strings with common char-type
	 *
	 * Custom strings, which do use any of the common char-types (``char``, ``wchar_t``, ``char16_t``, etc.), can be made compatible in no time.
	 * Users do just have to provide a specialization for the ``string_traits`` template:
	 * Given the following string-type, which in fact is just an abstraction around a ``std::string``:
	 * \snippet CustomString.cpp MyString
	 *
	 * To make it compatible with ``mimic++``, just that simple specialization is necessary:
	 * \snippet CustomString.cpp MyString trait
	 *
	 * \note These types of strings are already printable by default, if the underlying char-type is ``formattable``.
	 * \details
	 * \snippet CustomString.cpp MyString example
	 *
	 * ## Custom char-types and related strings
	 *
	 * It is possible to make custom char-types (and strings which use those) compatible with ``mimic++``, but this requires quite some effort,
	 * depending on the features you are planning to use.
	 *
	 * Given the following (``regular``) char-type
	 * \snippet CustomString.cpp custom_char
	 * and this string type:
	 * \snippet CustomString.cpp custom_string
	 *
	 * At first, we have to tell ``mimic++``, that ``my_char`` is an actual char-type. This can be done by specializing the ``is_character``-trait.
	 * \snippet CustomString.cpp custom_char trait
	 *
	 * Next, the ``string_traits`` must be specialized:
	 * \snippet CustomString.cpp custom_string traits
	 *
	 * After that, the string can be used with any string matcher, but just with case-sensitive matching:
	 * \snippet CustomString.cpp custom_string example
	 *
	 * ### Support for printing
	 *
	 * The current setup is already enough, that these strings can be printed. Unfortunately this will print the whole string
	 * just as comma separated hex-values, because ``mimic++`` doesn't know yet, how to print the underlying char-type.
	 * This can easily be fixed by proving a ``custom::Printer`` specialization for that particular char-type.
	 * \note Proving a custom printer for the char-type is enough, to make any related string type printable, too.
	 *
	 * ### Support for case-insensitive matching
	 *
	 * As stated in the string-matcher section, case-insensitive-matching requires some sort of case-folding.
	 * For ``ascii``-like strings a common strategy is to simply convert all characters to upper-case before the actual comparison.
	 * For wide-chars and unicode this is definitely more complex, if it shall be done correctly.
	 * Nevertheless, users can provide their own strategy how to perform the case-folding by simply specializing the ``string_case_fold_converter``
	 * trait. For the ``my_char`` from above, this may look like follows:
	 * \snippet CustomString.cpp custom_char case-folding
	 *
	 * This is then enough to make all strings with underlying type of ``my_char`` compatible with case-insensitive string-matchers:
	 * \snippet CustomString.cpp custom_string case-insensitive example
	 */

	/**
	 * \defgroup TYPE_TRAITS_IS_CHARACTER is_character
	 * \ingroup STRING
	 * \ingroup TYPE_TRAITS
	 * \brief Type-trait, which determines, whether the given type is a character-type.
	 * \note User are allowed to add specializations to the trait by themselves, but these types must satisfy the ``std::regular`` concept.
	 * \see https://en.cppreference.com/w/cpp/concepts/regular
	 *
	 * \{
	 */

	/**
	 * \brief Primary template, which always yields ``false``.
	 * \tparam T Type to check.
	 */
	template <typename T>
	struct is_character
		: public std::false_type
	{
	};

	/**
	 * \brief Convenience boolean-constant to the result of ``is_character`` trait.
	 * \tparam T The type to check.
	 */
	template <typename T>
	inline constexpr bool is_character_v{is_character<T>::value};

	/**
	 * \brief Specialization for ``char``.
	 */
	template <>
	struct is_character<char>
		: public std::true_type
	{
	};

	/**
	 * \brief Specialization for ``signed char``.
	 */
	template <>
	struct is_character<signed char>
		: public std::true_type
	{
	};

	/**
	 * \brief Specialization for ``unsigned char``.
	 */
	template <>
	struct is_character<unsigned char>
		: public std::true_type
	{
	};

	/**
	 * \brief Specialization for ``wchar_t``.
	 */
	template <>
	struct is_character<wchar_t>
		: public std::true_type
	{
	};

	/**
	 * \brief Specialization for ``char8_t``.
	 */
	template <>
	struct is_character<char8_t>
		: public std::true_type
	{
	};

	/**
	 * \brief Specialization for ``char16_t``.
	 */
	template <>
	struct is_character<char16_t>
		: public std::true_type
	{
	};

	/**
	 * \brief Specialization for ``char32_t``.
	 */
	template <>
	struct is_character<char32_t>
		: public std::true_type
	{
	};

	/**
	 * \}
	 */

	/**
	 * \defgroup TYPE_TRAITS_STRING_LITERAL_PREFIX string_literal_prefix
	 * \ingroup STRING
	 * \ingroup TYPE_TRAITS
	 * \brief Yields the printable prefix for any char-type.
	 * \note Users may add specializations for their custom types as desired.
	 *
	 * \{
	 */

	/**
	 * \brief Primary template, yielding an empty string.
	 * \tparam Char The char-type.
	 */
	template <satisfies<is_character> Char>
	[[maybe_unused]]
	inline constexpr StringViewT string_literal_prefix{};

	/**
	 * \brief ``char`` specialization.
	 */
	template <>
	[[maybe_unused]]
	inline constexpr StringViewT string_literal_prefix<char>{};

	/**
	 * \brief ``wchar_t`` specialization.
	 */
	template <>
	[[maybe_unused]]
	inline constexpr StringViewT string_literal_prefix<wchar_t>{"L"};

	/**
	 * \brief ``char8_t`` specialization.
	 */
	template <>
	[[maybe_unused]]
	inline constexpr StringViewT string_literal_prefix<char8_t>{"u8"};

	/**
	 * \brief ``char16_t`` specialization.
	 */
	template <>
	[[maybe_unused]]
	inline constexpr StringViewT string_literal_prefix<char16_t>{"u"};

	/**
	 * \brief ``char32_t`` specialization.
	 */
	template <>
	[[maybe_unused]]
	inline constexpr StringViewT string_literal_prefix<char32_t>{"U"};

	/**
	 * \}
	 */

	/**
	 * \defgroup TYPE_TRAITS_STRING_TRAITS string_traits
	 * \ingroup STRING
	 * \ingroup TYPE_TRAITS
	 * \brief Type-trait, which contains properties for the provided string type.
	 *
	 * \{
	 */

	/**
	 * \brief Computes the view type for the given string.
	 * \tparam T Type to check.
	 */
	template <typename T>
	using string_view_t = decltype(string_traits<std::remove_cvref_t<T>>::view(std::declval<T&>()));

	/**
	 * \brief Computes the character type for the given string.
	 * \tparam T Type to check.
	 */
	template <typename T>
	using string_char_t = typename string_traits<std::remove_cvref_t<T>>::char_t;

	/**
	 * \brief Specialization for character pointer types.
	 * \tparam T Type to check.
	 */
	template <typename T>
		requires std::is_pointer_v<T>
				&& is_character_v<std::remove_cv_t<std::remove_pointer_t<T>>>
	struct string_traits<T>
	{
		using char_t = std::remove_const_t<std::remove_pointer_t<T>>;
		using view_t = std::basic_string_view<char_t>;

		[[nodiscard]]
		static constexpr view_t view(const std::remove_pointer_t<T>* str) noexcept
		{
			return view_t{str};
		}
	};

	/**
	 * \brief Specialization for character array types.
	 * \tparam T Type to check.
	 */
	template <typename T>
		requires std::is_array_v<T>
	struct string_traits<T>
		: public string_traits<std::remove_extent_t<T>*>
	{
	};

	/**
	 * \brief Specialization for ``std::basic_string`` types.
	 */
	template <typename Char, typename Traits, typename Allocator>
	struct string_traits<std::basic_string<Char, Traits, Allocator>>
	{
		using char_t = Char;
		using string_t = std::basic_string<Char, Traits, Allocator>;
		using view_t = std::basic_string_view<Char, Traits>;

		[[nodiscard]]
		static constexpr view_t view(const string_t& str) noexcept
		{
			return view_t{str};
		}
	};

	/**
	 * \brief Specialization for ``std::basic_string_view`` types.
	 */
	template <typename Char, typename Traits>
	struct string_traits<std::basic_string_view<Char, Traits>>
	{
		using char_t = Char;
		using view_t = std::basic_string_view<Char, Traits>;

		[[nodiscard]]
		static constexpr view_t view(view_t str) noexcept
		{
			return str;
		}
	};

	/**
	 * \}
	 */

	/**
	 * \brief Determines, whether the given type can be used as a string-type.
	 * \ingroup STRING
	 * \ingroup CONCEPTS
	 */
	template <typename T>
	concept string = requires
	{
		requires is_character_v<string_char_t<T>>;
		requires std::regular<string_char_t<T>>;
		requires std::ranges::contiguous_range<string_view_t<T>>;
		requires std::ranges::sized_range<string_view_t<T>>;
		requires std::ranges::borrowed_range<string_view_t<T>>;
		requires std::same_as<
			string_char_t<T>,
			std::ranges::range_value_t<string_view_t<T>>>;
	};

	/**
	 * \defgroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER string_case_fold_converter
	 * \ingroup STRING
	 * \brief Type-trait, which provides the case-folding algorithm for the char-type, they are specialized for.
	 * \details This implements the case-folding algorithm for the various character-types.
	 * Users should not make any assumptions on what any converter will return, as this is highly
	 * implementation specific. No guarantees are made whether the result will be in upper- or lower-case (or something completely
	 * different). The only guarantee is, that two inputs, which are required to compare equal after the case-folding process,
	 * yield the same result.
	 *
	 * \see https://unicode-org.github.io/icu/userguide/transforms/casemappings.html#case-folding
	 * \see https://www.unicode.org/Public/UNIDATA/CaseFolding.txt
	 *
	 * \note Users are allowed to add specializations as desired.
	 */

	/**
	 * \brief Primary template, purposely undefined.
	 * \tparam Char The character type.
	 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
	 */
	template <satisfies<is_character> Char>
	struct string_case_fold_converter;

	/**
	 * \brief Determines, whether the given type supports string normalization.
	 * \ingroup STRING
	 * \ingroup CONCEPTS
	 */
	template <typename String>
	concept case_foldable_string =
		string<String>
		&& requires(const string_case_fold_converter<string_char_t<String>> converter, string_view_t<String> view)
		{
			{ std::invoke(converter, std::move(view)) } -> std::ranges::forward_range;
		}
		&& requires(std::invoke_result_t<string_case_fold_converter<string_char_t<String>>, string_view_t<String>> normalized)
		{
			requires std::same_as<
				string_char_t<String>,
				std::ranges::range_value_t<decltype(normalized)>>;
		};
}

namespace mimicpp::detail
{
	template <typename View, typename Char>
	concept compatible_string_view_with = is_character_v<Char>
										&& std::ranges::borrowed_range<View>
										&& std::ranges::contiguous_range<View>
										&& std::ranges::sized_range<View>
										&& std::same_as<Char, std::ranges::range_value_t<View>>;
}

#ifndef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER

/**
 * \brief Specialized template for the ``char`` type.
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char>
{
	template <detail::compatible_string_view_with<char> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return std::views::all(std::forward<String>(str))
				| std::views::transform(
					[](const char c) noexcept
					{
						// see notes of https://en.cppreference.com/w/cpp/string/byte/toupper
						// This approach will fail, if str actually contains an utf8-encoded string.
						return static_cast<char>(
							static_cast<unsigned char>(std::toupper(c)));
					});
	}
};

#else

#if __has_include(<uni_algo/case.h>) \
	&& __has_include(<uni_algo/conv.h>)
#include <uni_algo/case.h>
#include <uni_algo/conv.h>
#else
	#error "Unable to find uni_algo includes."
#endif

/**
 * \brief Specialized template for the ``char`` type (with uni_algo backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char>
{
	template <detail::compatible_string_view_with<char> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return una::cases::to_casefold_utf8(
			std::string_view{
				std::ranges::data(str),
				std::ranges::size(str)
			});
	}
};

/**
 * \brief Specialized template for the ``wchar_t`` type (with uni_algo backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<wchar_t>
{
	template <detail::compatible_string_view_with<wchar_t> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return una::cases::to_casefold_utf16(
			std::wstring_view{
				std::ranges::data(str),
				std::ranges::size(str)
			});
	}
};

/**
 * \brief Specialized template for the ``char8_t`` type (with uni_algo backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char8_t>
{
	template <detail::compatible_string_view_with<char8_t> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return una::cases::to_casefold_utf8(
			std::u8string_view{
				std::ranges::data(str),
				std::ranges::size(str)
			});
	}
};

/**
 * \brief Specialized template for the ``char16_t`` type (with uni_algo backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char16_t>
{
	template <detail::compatible_string_view_with<char16_t> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return una::cases::to_casefold_utf16(
			std::u16string_view{
				std::ranges::data(str),
				std::ranges::size(str)
			});
	}
};

/**
 * \brief Specialized template for the ``char32_t`` type (with uni_algo backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char32_t>
{
	template <detail::compatible_string_view_with<char32_t> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return una::utf8to32<char8_t, char32_t>(
			una::cases::to_casefold_utf8(
			una::utf32to8u(
				std::u32string_view{
					std::ranges::data(str),
					std::ranges::size(str)
				})));
	}
};

#endif

#endif
