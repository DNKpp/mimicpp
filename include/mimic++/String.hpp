// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_STRING_HPP
#define MIMICPP_STRING_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Utility.hpp"

#include <bit>
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
	 */

	/**
	 * \defgroup TYPE_TRAITS_IS_CHARACTER is_character
	 * \ingroup STRING
	 * \ingroup TYPE_TRAITS
	 * \brief Type-trait, which determines, whether the given type is a character-type.
	 * \attention User are not allowed to add specializations to the trait by themselves.
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
	 * \tparam T Type to check.
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
	 * \tparam T Type to check.
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
	 * \brief Type-trait, which contains properties for the provided string type.
	 * \details This implements the case-folding algorithm for the various character-types.
	 * Users should not make any assumptions on what any converter will return, as this is highly
	 * implementation specific. No guarantees are made whether the result will be in upper- or lower-case (or something completely
	 * different). The only guarantee is, that two inputs, which are required to compare equal after the case-folding process,
	 * yield the same result.
	 *
	 * \see https://unicode-org.github.io/icu/userguide/transforms/casemappings.html#case-folding
	 * \see https://www.unicode.org/Public/UNIDATA/CaseFolding.txt
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
						// This approach will fail, str actually contains an utf8-encoded string.
						return static_cast<char>(
							static_cast<unsigned char>(std::toupper(c)));
					});
	}
};

#else

#include <unicodelib.h>
#include <unicodelib_encodings.h>

/**
 * \brief Specialized template for the ``char`` type (with unicodelib backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char>
{
	template <detail::compatible_string_view_with<char> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return unicode::utf8::encode(
			unicode::to_case_fold(
				unicode::utf8::decode(
					std::string_view{
						std::ranges::data(str),
						std::ranges::size(str)
					})));
	}
};

/**
 * \brief Specialized template for the ``wchar_t`` type (with unicodelib backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<wchar_t>
{
	template <detail::compatible_string_view_with<wchar_t> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return unicode::to_wstring(
			unicode::to_case_fold(
				unicode::to_utf32(
					std::wstring_view{
						std::ranges::data(str),
						std::ranges::size(str)
					})));
	}
};

/**
 * \brief Specialized template for the ``char8_t`` type (with unicodelib backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char8_t>
{
	template <detail::compatible_string_view_with<char8_t> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		const std::string caseFolded = std::invoke(
			mimicpp::string_case_fold_converter<char>{},
			std::string_view{
				std::bit_cast<const char*>(std::ranges::data(str)),
				std::ranges::size(str)
			});

		return std::u8string{
			caseFolded.cbegin(),
			caseFolded.cend()
		};
	}
};

/**
 * \brief Specialized template for the ``char16_t`` type (with unicodelib backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char16_t>
{
	template <detail::compatible_string_view_with<char16_t> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return unicode::utf16::encode(
			unicode::to_case_fold(
				unicode::utf16::decode(
					std::u16string_view{
						std::ranges::data(str),
						std::ranges::size(str)
					})));
	}
};

/**
 * \brief Specialized template for the ``char32_t`` type (with unicodelib backend).
 * \ingroup TYPE_TRAITS_STRING_CASE_FOLD_CONVERTER
 */
template <>
struct mimicpp::string_case_fold_converter<char32_t>
{
	template <detail::compatible_string_view_with<char32_t> String>
	[[nodiscard]]
	constexpr auto operator ()(String&& str) const
	{
		return unicode::to_case_fold(
			std::u32string_view{
				std::ranges::data(str),
				std::ranges::size(str)
			});
	}
};

#endif

#endif
