// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TYPE_TRAITS_HPP
#define MIMICPP_TYPE_TRAITS_HPP

#pragma once

#include "Fwd.hpp"

#include <concepts>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace mimicpp
{
	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...)>
	{
		using type = Return(Params...) noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...)>
	{
		using type = Return(Params..., ...) noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) noexcept>
	{
		using type = Return(Params...) noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) noexcept>
	{
		using type = Return(Params..., ...) noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) const>
	{
		using type = Return(Params...) const noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) const>
	{
		using type = Return(Params..., ...) const noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) const noexcept>
	{
		using type = Return(Params...) const noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) const noexcept>
	{
		using type = Return(Params..., ...) const noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) &>
	{
		using type = Return(Params...) & noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) &>
	{
		using type = Return(Params..., ...) & noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) & noexcept>
	{
		using type = Return(Params...) & noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) & noexcept>
	{
		using type = Return(Params..., ...) & noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) const &>
	{
		using type = Return(Params...) const & noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) const &>
	{
		using type = Return(Params..., ...) const & noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) const & noexcept>
	{
		using type = Return(Params...) const & noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) const & noexcept>
	{
		using type = Return(Params..., ...) const & noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) &&>
	{
		using type = Return(Params...) && noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) &&>
	{
		using type = Return(Params..., ...) && noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) && noexcept>
	{
		using type = Return(Params...) && noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) && noexcept>
	{
		using type = Return(Params..., ...) && noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) const &&>
	{
		using type = Return(Params...) const && noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) const &&>
	{
		using type = Return(Params..., ...) const && noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params...) const && noexcept>
	{
		using type = Return(Params...) const && noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_add_noexcept<Return(Params..., ...) const && noexcept>
	{
		using type = Return(Params..., ...) const && noexcept;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...)>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...)>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) const>
	{
		using type = Return(Params...) const;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) const>
	{
		using type = Return(Params..., ...) const;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) const noexcept>
	{
		using type = Return(Params...) const;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) const noexcept>
	{
		using type = Return(Params..., ...) const;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) &>
	{
		using type = Return(Params...) &;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) &>
	{
		using type = Return(Params..., ...) &;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) & noexcept>
	{
		using type = Return(Params...) &;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) & noexcept>
	{
		using type = Return(Params..., ...) &;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) const &>
	{
		using type = Return(Params...) const &;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) const &>
	{
		using type = Return(Params..., ...) const &;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) const & noexcept>
	{
		using type = Return(Params...) const &;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) const & noexcept>
	{
		using type = Return(Params..., ...) const &;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) &&>
	{
		using type = Return(Params...) &&;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) &&>
	{
		using type = Return(Params..., ...) &&;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) && noexcept>
	{
		using type = Return(Params...) &&;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) && noexcept>
	{
		using type = Return(Params..., ...) &&;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) const &&>
	{
		using type = Return(Params...) const &&;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) const &&>
	{
		using type = Return(Params..., ...) const &&;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params...) const && noexcept>
	{
		using type = Return(Params...) const &&;
	};

	template <typename Return, typename... Params>
	struct signature_remove_noexcept<Return(Params..., ...) const && noexcept>
	{
		using type = Return(Params..., ...) const &&;
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...)>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) &>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const &>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) &&>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const &&>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) & noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const & noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) && noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params...) const && noexcept>
	{
		using type = Return(Params...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...)>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) &>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const &>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) &&>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const &&>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) & noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const & noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) && noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Return, typename... Params>
	struct signature_decay<Return(Params..., ...) const && noexcept>
	{
		using type = Return(Params..., ...);
	};

	template <typename Signature>
		requires std::is_function_v<Signature>
	struct signature_return_type<Signature>
		: public signature_return_type<signature_decay_t<Signature>>
	{
	};

	template <typename Return, typename... Params>
	struct signature_return_type<Return(Params...)>
	{
		using type = Return;
	};

	template <typename Return, typename... Params>
	struct signature_return_type<Return(Params..., ...)>
	{
		using type = Return;
	};

	template <std::size_t index, typename Signature>
		requires std::is_function_v<Signature>
	struct signature_param_type<index, Signature>
		: public signature_param_type<
			index,
			signature_decay_t<Signature>>
	{
	};

	template <std::size_t index, typename Return, typename... Params>
	struct signature_param_type<index, Return(Params...)>
		: public std::tuple_element<index, std::tuple<Params...>>
	{
	};

	template <typename Signature>
		requires std::is_function_v<Signature>
	struct signature_param_list<Signature>
		: public signature_param_list<
			signature_decay_t<Signature>>
	{
	};

	template <typename Return, typename... Params>
	struct signature_param_list<Return(Params...)>
	{
		using type = std::tuple<Params...>;
	};

	namespace detail
	{
		template <typename First, typename Second, bool reversed = false>
		struct is_overloadable_with
			: public std::conditional_t<
				reversed,
				std::false_type,
				is_overloadable_with<Second, First, true>>
		{
		};

		template <typename First, typename Second>
			requires (
				!std::same_as<
					signature_param_list_t<signature_decay_t<First>>,
					signature_param_list_t<signature_decay_t<Second>>>)
		struct is_overloadable_with<First, Second, false>
			: public std::true_type
		{
		};

		template <typename Return1, typename Return2, typename... Params, bool reversed>
		struct is_overloadable_with<Return1(Params...), Return2(Params...) const, reversed>
			: public std::true_type
		{
		};

		template <typename Return1, typename Return2, typename... Params, bool reversed>
		struct is_overloadable_with<Return1(Params...) &, Return2(Params...) const &, reversed>
			: public std::true_type
		{
		};

		template <typename Return1, typename Return2, typename... Params, bool reversed>
		struct is_overloadable_with<Return1(Params...) &, Return2(Params...) &&, reversed>
			: public std::true_type
		{
		};

		template <typename Return1, typename Return2, typename... Params, bool reversed>
		struct is_overloadable_with<Return1(Params...) &, Return2(Params...) const &&, reversed>
			: public std::true_type
		{
		};

		template <typename Return1, typename Return2, typename... Params, bool reversed>
		struct is_overloadable_with<Return1(Params...) const &, Return2(Params...) &&, reversed>
			: public std::true_type
		{
		};

		template <typename Return1, typename Return2, typename... Params, bool reversed>
		struct is_overloadable_with<Return1(Params...) const &, Return2(Params...) const &&, reversed>
			: public std::true_type
		{
		};

		template <typename Return1, typename Return2, typename... Params, bool reversed>
		struct is_overloadable_with<Return1(Params...) &&, Return2(Params...) const &&, reversed>
			: public std::true_type
		{
		};
	}

	template <typename First, typename Second>
	struct is_overloadable_with
		: public detail::is_overloadable_with<
			signature_remove_noexcept_t<First>,
			signature_remove_noexcept_t<Second>>
	{
	};

	template <typename First>
	struct is_overload_set<First>
		: public std::true_type
	{
	};

	template <typename First, typename Second, typename... Others>
	struct is_overload_set<First, Second, Others...>
		: public std::conjunction<
			is_overloadable_with<First, Second>,
			is_overload_set<First, Others...>,
			is_overload_set<Second, Others...>>
	{
	};

	/**
	 * \defgroup TYPE_TRAITS_IS_CHARACTER is_character
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

	/**
	 * \brief Determines, whether the given type can be used as a string-type.
	 */
	template <typename T>
	concept string = requires
					{
						typename string_traits<std::remove_cvref_t<T>>::char_t;
						typename string_traits<std::remove_cvref_t<T>>::string_t;
					}
					&& std::equality_comparable<typename string_traits<std::remove_cvref_t<T>>::string_t>
					&& std::ranges::forward_range<typename string_traits<std::remove_cvref_t<T>>::string_t>;
}

#endif
