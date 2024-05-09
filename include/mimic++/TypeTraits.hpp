// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TYPE_TRAITS_HPP
#define MIMICPP_TYPE_TRAITS_HPP

#pragma once

#include <tuple>

namespace mimicpp
{
	template <typename Signature>
	struct signature_add_noexcept;

	template <typename Signature>
	using signature_add_noexcept_t = typename signature_add_noexcept<Signature>::type;

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

	template <typename Signature>
	struct signature_remove_noexcept;

	template <typename Signature>
	using signature_remove_noexcept_t = typename signature_remove_noexcept<Signature>::type;

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

	template <typename Signature>
	struct signature_decay;

	template <typename Signature>
	using signature_decay_t = typename signature_decay<Signature>::type;

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
	struct signature_return_type;

	template <typename Signature>
		requires std::is_function_v<Signature>
	struct signature_return_type<Signature>
		: public signature_return_type<signature_decay_t<Signature>>
	{
	};

	template <typename Signature>
	using signature_return_type_t = typename signature_return_type<Signature>::type;

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
	struct signature_param_type;

	template <std::size_t index, typename Signature>
		requires std::is_function_v<Signature>
	struct signature_param_type<index, Signature>
		: public signature_param_type<
			index,
			signature_decay_t<Signature>>
	{
	};

	template <std::size_t index, typename Signature>
	using signature_param_type_t = typename signature_param_type<index, Signature>::type;

	template <std::size_t index, typename Return, typename... Params>
	struct signature_param_type<index, Return(Params...)>
		: public std::tuple_element<index, std::tuple<Params...>>
	{
	};

	template <typename Signature>
	struct signature_param_list;

	template <typename Signature>
		requires std::is_function_v<Signature>
	struct signature_param_list<Signature>
		: public signature_param_list<
			signature_decay_t<Signature>>
	{
	};

	template <typename Signature>
	using signature_param_list_t = typename signature_param_list<Signature>::type;

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

		template <typename Return, typename... Params, bool reversed>
		struct is_overloadable_with<Return(Params...), Return(Params...) const, reversed>
			: public std::true_type
		{
		};

		template <typename Return, typename... Params, bool reversed>
		struct is_overloadable_with<Return(Params...) &, Return(Params...) const &, reversed>
			: public std::true_type
		{
		};

		template <typename Return, typename... Params, bool reversed>
		struct is_overloadable_with<Return(Params...) &, Return(Params...) &&, reversed>
			: public std::true_type
		{
		};

		template <typename Return, typename... Params, bool reversed>
		struct is_overloadable_with<Return(Params...) &, Return(Params...) const &&, reversed>
			: public std::true_type
		{
		};

		template <typename Return, typename... Params, bool reversed>
		struct is_overloadable_with<Return(Params...) const &, Return(Params...) &&, reversed>
			: public std::true_type
		{
		};

		template <typename Return, typename... Params, bool reversed>
		struct is_overloadable_with<Return(Params...) const &, Return(Params...) const &&, reversed>
			: public std::true_type
		{
		};

		template <typename Return, typename... Params, bool reversed>
		struct is_overloadable_with<Return(Params...) &&, Return(Params...) const &&, reversed>
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

	template <typename First, typename Second>
	inline constexpr bool is_overloadable_with_v = is_overloadable_with<First, Second>::value;
}

#endif
