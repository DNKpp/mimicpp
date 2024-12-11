// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TYPE_TRAITS_HPP
#define MIMICPP_TYPE_TRAITS_HPP

#pragma once

#include "Fwd.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace mimicpp
{
    /**
     * \defgroup TYPE_TRAITS type-traits
     * \brief Contains various type-traits
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_ADD_NOEXCEPT signature_add_noexcept
     * \ingroup TYPE_TRAITS
     * \brief Adds the ``noexcept`` specification to a signature.
     *
     *\{
     */

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
    struct signature_add_noexcept<Return(Params...)&>
    {
        using type = Return(Params...) & noexcept;
    };

    template <typename Return, typename... Params>
    struct signature_add_noexcept<Return(Params..., ...)&>
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
    struct signature_add_noexcept<Return(Params...) const&>
    {
        using type = Return(Params...) const& noexcept;
    };

    template <typename Return, typename... Params>
    struct signature_add_noexcept<Return(Params..., ...) const&>
    {
        using type = Return(Params..., ...) const& noexcept;
    };

    template <typename Return, typename... Params>
    struct signature_add_noexcept<Return(Params...) const & noexcept>
    {
        using type = Return(Params...) const& noexcept;
    };

    template <typename Return, typename... Params>
    struct signature_add_noexcept<Return(Params..., ...) const & noexcept>
    {
        using type = Return(Params..., ...) const& noexcept;
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
    struct signature_add_noexcept<Return(Params...) const&&>
    {
        using type = Return(Params...) const&& noexcept;
    };

    template <typename Return, typename... Params>
    struct signature_add_noexcept<Return(Params..., ...) const&&>
    {
        using type = Return(Params..., ...) const&& noexcept;
    };

    template <typename Return, typename... Params>
    struct signature_add_noexcept<Return(Params...) const && noexcept>
    {
        using type = Return(Params...) const&& noexcept;
    };

    template <typename Return, typename... Params>
    struct signature_add_noexcept<Return(Params..., ...) const && noexcept>
    {
        using type = Return(Params..., ...) const&& noexcept;
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_REMOVE_NOEXCEPT signature_remove_noexcept
     * \ingroup TYPE_TRAITS
     * \brief Removes the ``noexcept`` specification to a signature (if present).
     *
     *\{
     */

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
    struct signature_remove_noexcept<Return(Params...)&>
    {
        using type = Return(Params...) &;
    };

    template <typename Return, typename... Params>
    struct signature_remove_noexcept<Return(Params..., ...)&>
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
    struct signature_remove_noexcept<Return(Params...) const&>
    {
        using type = Return(Params...) const&;
    };

    template <typename Return, typename... Params>
    struct signature_remove_noexcept<Return(Params..., ...) const&>
    {
        using type = Return(Params..., ...) const&;
    };

    template <typename Return, typename... Params>
    struct signature_remove_noexcept<Return(Params...) const & noexcept>
    {
        using type = Return(Params...) const&;
    };

    template <typename Return, typename... Params>
    struct signature_remove_noexcept<Return(Params..., ...) const & noexcept>
    {
        using type = Return(Params..., ...) const&;
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
    struct signature_remove_noexcept<Return(Params...) const&&>
    {
        using type = Return(Params...) const&&;
    };

    template <typename Return, typename... Params>
    struct signature_remove_noexcept<Return(Params..., ...) const&&>
    {
        using type = Return(Params..., ...) const&&;
    };

    template <typename Return, typename... Params>
    struct signature_remove_noexcept<Return(Params...) const && noexcept>
    {
        using type = Return(Params...) const&&;
    };

    template <typename Return, typename... Params>
    struct signature_remove_noexcept<Return(Params..., ...) const && noexcept>
    {
        using type = Return(Params..., ...) const&&;
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_DECAY signature_decay
     * \ingroup TYPE_TRAITS
     * \brief Removes all specifications from the given signature.
     *
     *\{
     */

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
    struct signature_decay<Return(Params...)&>
    {
        using type = Return(Params...);
    };

    template <typename Return, typename... Params>
    struct signature_decay<Return(Params...) const&>
    {
        using type = Return(Params...);
    };

    template <typename Return, typename... Params>
    struct signature_decay<Return(Params...) &&>
    {
        using type = Return(Params...);
    };

    template <typename Return, typename... Params>
    struct signature_decay<Return(Params...) const&&>
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
    struct signature_decay<Return(Params..., ...)&>
    {
        using type = Return(Params..., ...);
    };

    template <typename Return, typename... Params>
    struct signature_decay<Return(Params..., ...) const&>
    {
        using type = Return(Params..., ...);
    };

    template <typename Return, typename... Params>
    struct signature_decay<Return(Params..., ...) &&>
    {
        using type = Return(Params..., ...);
    };

    template <typename Return, typename... Params>
    struct signature_decay<Return(Params..., ...) const&&>
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

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_RETURN_TYPE signature_return_type
     * \ingroup TYPE_TRAITS
     * \brief Extracts the return type from a given signature.
     *
     *\{
     */

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

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_PARAM_TYPE signature_param_type
     * \ingroup TYPE_TRAITS
     * \brief Extracts the ``i``th param type from a given signature.
     *
     *\{
     */

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

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_PARAM_LIST signature_param_list
     * \ingroup TYPE_TRAITS
     * \brief Extracts all param types from a given signature (packed into a ``std::tuple``).
     *
     *\{
     */

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

    /**
     * \}
     */

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
            requires(
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
        struct is_overloadable_with<Return1(Params...)&, Return2(Params...) const&, reversed>
            : public std::true_type
        {
        };

        template <typename Return1, typename Return2, typename... Params, bool reversed>
        struct is_overloadable_with<Return1(Params...)&, Return2(Params...)&&, reversed>
            : public std::true_type
        {
        };

        template <typename Return1, typename Return2, typename... Params, bool reversed>
        struct is_overloadable_with<Return1(Params...)&, Return2(Params...) const&&, reversed>
            : public std::true_type
        {
        };

        template <typename Return1, typename Return2, typename... Params, bool reversed>
        struct is_overloadable_with<Return1(Params...) const&, Return2(Params...)&&, reversed>
            : public std::true_type
        {
        };

        template <typename Return1, typename Return2, typename... Params, bool reversed>
        struct is_overloadable_with<Return1(Params...) const&, Return2(Params...) const&&, reversed>
            : public std::true_type
        {
        };

        template <typename Return1, typename Return2, typename... Params, bool reversed>
        struct is_overloadable_with<Return1(Params...)&&, Return2(Params...) const&&, reversed>
            : public std::true_type
        {
        };
    }

    /**
     * \defgroup TYPE_TRAITS_IS_OVERLOADABLE_WITH is_overloadable_with
     * \ingroup TYPE_TRAITS
     * \brief Determines, whether two signatures are valid overloads.
     *
     *\{
     */

    template <typename First, typename Second>
    struct is_overloadable_with
        : public detail::is_overloadable_with<
              signature_remove_noexcept_t<First>,
              signature_remove_noexcept_t<Second>>
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_IS_OVERLOAD_SET is_overload_set
     * \ingroup TYPE_TRAITS
     * \brief Determines, whether a list of signatures form a valid overloads-set.
     *
     *\{
     */

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
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_UINT_WITH_SIZE unit_with_size
     * \ingroup TYPE_TRAITS
     * \brief Provides the member alias ``type`` with the expected uint-type.
     * \details This trait always yields a type with the exact size. If no such type exists, the member ``type`` is undefined.
     *
     *\{
     */

    /**
     * \brief 1-byte specialization.
     */
    template <>
    struct uint_with_size<1u>
    {
        using type = std::uint8_t;
    };

    /**
     * \brief 2-byte specialization.
     */
    template <>
    struct uint_with_size<2u>
    {
        using type = std::uint16_t;
    };

    /**
     * \brief 4-byte specialization.
     */
    template <>
    struct uint_with_size<4u>
    {
        using type = std::uint32_t;
    };

    /**
     * \brief 8-byte specialization.
     */
    template <>
    struct uint_with_size<8u>
    {
        using type = std::uint64_t;
    };

    /**
     * \}
     */
}

#endif
