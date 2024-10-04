// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_STRING_HPP
#define MIMICPP_STRING_HPP

#pragma once

#include "mimic++/Fwd.hpp"

#include <type_traits>

namespace mimicpp
{
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
	 * \brief Specialization for character pointer types.
	 * \tparam T Type to check.
	 */
	template <typename T>
		requires std::is_pointer_v<T>
				&& is_character_v<std::remove_cv_t<std::remove_pointer_t<T>>>
	struct string_traits<T>
	{
		using char_t = std::remove_const_t<std::remove_pointer_t<T>>;
		using string_t = std::basic_string_view<char_t>;
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
	};

	/**
	 * \brief Specialization for ``std::basic_string_view`` types.
	 * \tparam T Type to check.
	 */
	template <typename Char, typename Traits>
	struct string_traits<std::basic_string_view<Char, Traits>>
	{
		using char_t = Char;
		using string_t = std::basic_string_view<Char, Traits>;
	};

	/**
	 * \}
	 */
}

#endif
