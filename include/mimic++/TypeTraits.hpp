//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_TYPE_TRAITS_HPP
#define MIMICPP_TYPE_TRAITS_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/utilities/TypeList.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <cstddef>
    #include <cstdint>
    #include <tuple>
    #include <type_traits>
#endif

namespace mimicpp::detail
{
    template <typename Signature>
    struct has_default_call_convention
        : public std::false_type
    {
    };

    template <typename Signature>
    inline constexpr bool has_default_call_convention_v = has_default_call_convention<Signature>::value;

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...)>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...)>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) const>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) const>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) const noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) const noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...)&>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...)&>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) & noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) & noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) const&>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) const&>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) const & noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) const & noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) &&>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) &&>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) && noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) && noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) const&&>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) const&&>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params...) const && noexcept>
        : public std::true_type
    {
    };

    template <typename Return, typename... Params>
    struct has_default_call_convention<Return(Params..., ...) const && noexcept>
        : public std::true_type
    {
    };

    struct default_call_convention
    {
    };
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp
{
    /**
     * \defgroup TYPE_TRAITS type-traits
     * \brief Contains various type-traits
     *
     */

    /**
     * \brief Determines, whether the given signature has default call-convention.
     * \tparam Signature The signature to check.
     */
    template <typename Signature>
    concept has_default_call_convention = detail::has_default_call_convention_v<Signature>;

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_CALL_CONVENTION signature_call_convention
     * \ingroup TYPE_TRAITS
     * \brief Determines the call-convention-tag of the given signature.
     * \note Users may add specializations for their custom call-convention-tags.
     *
     *\{
     */

    /**
     * \brief Template specialization for the default call-convention.
     * \tparam Signature The signature.
     */
    template <has_default_call_convention Signature>
    struct signature_call_convention<Signature>
    {
        using type = detail::default_call_convention;
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_CALL_CONVENTION_TRAITS call_convention_traits
     * \ingroup TYPE_TRAITS
     * \brief Used for selecting the correct behaviour depending on the call-convention.
     * \note Users may add specializations for their custom call-convention-tags.
     * \details All specializations are required to define at least the following members:
     * ```cpp
     * using tag_t = // Tag, which has been used for the specialization.
     *
     * template <typename Signature>
     * using remove_call_convention_t = // Trait, which removes the call-convention from the signature (if present).

     * template <typename Signature>
     * using add_call_convention_t = // Trait, which adds the call-convention to the signature (if necessary).

     * template <typename Derived, typename Signature>
     * using call_interface_t = // Interface, which defines ``operator ()`` with the call-convention.
     * ```
     *
     *\{
     */

    /**
     * \brief Template specialization for the default call-convention.
     */
    template <>
    struct call_convention_traits<detail::default_call_convention>
    {
        using tag_t = detail::default_call_convention;

        template <typename Signature>
        using remove_call_convention_t = std::type_identity_t<Signature>;

        template <typename Signature>
        using add_call_convention_t = std::type_identity_t<Signature>;

        template <typename Derived, typename Signature>
        using call_interface_t = detail::DefaultCallInterface<Derived, Signature>;
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_REMOVE_CALL_CONVENTION signature_remove_call_convention
     * \ingroup TYPE_TRAITS
     * \brief Removes the call-convention from a signature (if present).
     *
     *\{
     */

    template <typename Signature>
    struct signature_remove_call_convention
    {
        using type = typename call_convention_traits<
            signature_call_convention_t<Signature>>::template remove_call_convention_t<Signature>;
    };

    /**
     * \}
     */

    namespace detail
    {

        /**
         * \brief Helper type, which removes any existing call-convention, applies the given trait and re-applies the removed call-convention afterwards.
         */
        template <
            typename Signature,
            template <typename> typename Trait,
            typename CallConvTrait = call_convention_traits<signature_call_convention_t<Signature>>,
            typename Applied = Trait<typename CallConvTrait::template remove_call_convention_t<Signature>>>
        struct signature_apply_trait
            : public std::bool_constant<Applied::value>
        {
            using type = typename CallConvTrait::template add_call_convention_t<typename Applied::type>;
        };
    }

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_REMOVE_NOEXCEPT signature_remove_noexcept
     * \ingroup TYPE_TRAITS
     * \brief Removes the ``noexcept`` specification from a signature (if present).
     *
     *\{
     */

    namespace detail
    {
        template <typename Signature>
        struct signature_remove_noexcept;

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...)>
            : public std::false_type
        {
            using type = Return(Params...);
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...)>
            : public std::false_type
        {
            using type = Return(Params..., ...);
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) noexcept>
            : public std::true_type
        {
            using type = Return(Params...);
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) noexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...);
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) const>
            : public std::false_type
        {
            using type = Return(Params...) const;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) const>
            : public std::false_type
        {
            using type = Return(Params..., ...) const;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) const noexcept>
            : public std::true_type
        {
            using type = Return(Params...) const;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) const noexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) const;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...)&>
            : public std::false_type
        {
            using type = Return(Params...) &;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...)&>
            : public std::false_type
        {
            using type = Return(Params..., ...) &;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) & noexcept>
            : public std::true_type
        {
            using type = Return(Params...) &;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) & noexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) &;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) const&>
            : public std::false_type
        {
            using type = Return(Params...) const&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) const&>
            : public std::false_type
        {
            using type = Return(Params..., ...) const&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) const & noexcept>
            : public std::true_type
        {
            using type = Return(Params...) const&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) const & noexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) const&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) &&>
            : public std::false_type
        {
            using type = Return(Params...) &&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) &&>
            : public std::false_type
        {
            using type = Return(Params..., ...) &&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) && noexcept>
            : public std::true_type
        {
            using type = Return(Params...) &&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) && noexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) &&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) const&&>
            : public std::false_type
        {
            using type = Return(Params...) const&&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) const&&>
            : public std::false_type
        {
            using type = Return(Params..., ...) const&&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params...) const && noexcept>
            : public std::true_type
        {
            using type = Return(Params...) const&&;
        };

        template <typename Return, typename... Params>
        struct signature_remove_noexcept<Return(Params..., ...) const && noexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) const&&;
        };
    }

    template <typename Signature>
    struct signature_remove_noexcept
        : public detail::signature_apply_trait<Signature, detail::signature_remove_noexcept>
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_ADD_NOEXCEPT signature_add_noexcept
     * \ingroup TYPE_TRAITS
     * \brief Adds the ``noexcept`` specification to a signature.
     *
     *\{
     */

    namespace detail
    {
        template <typename Signature>
        struct signature_add_noexcept;

        template <typename Signature>
            requires signature_remove_noexcept<Signature>::value
        struct signature_add_noexcept<Signature>
            : public std::false_type
        {
            using type = Signature;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params...)>
            : public std::true_type
        {
            using type = Return(Params...) noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params..., ...)>
            : public std::true_type
        {
            using type = Return(Params..., ...) noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params...) const>
            : public std::true_type
        {
            using type = Return(Params...) const noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params..., ...) const>
            : public std::true_type
        {
            using type = Return(Params..., ...) const noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params...)&>
            : public std::true_type
        {
            using type = Return(Params...) & noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params..., ...)&>
            : public std::true_type
        {
            using type = Return(Params..., ...) & noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params...) const&>
            : public std::true_type
        {
            using type = Return(Params...) const& noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params..., ...) const&>
            : public std::true_type
        {
            using type = Return(Params..., ...) const& noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params...) &&>
            : public std::true_type
        {
            using type = Return(Params...) && noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params..., ...) &&>
            : public std::true_type
        {
            using type = Return(Params..., ...) && noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params...) const&&>
            : public std::true_type
        {
            using type = Return(Params...) const&& noexcept;
        };

        template <typename Return, typename... Params>
        struct signature_add_noexcept<Return(Params..., ...) const&&>
            : public std::true_type
        {
            using type = Return(Params..., ...) const&& noexcept;
        };
    }

    template <typename Signature>
    struct signature_add_noexcept
        : public detail::signature_apply_trait<Signature, detail::signature_add_noexcept>
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_IS_NOEXCEPT signature_is_noexcept
     * \ingroup TYPE_TRAITS
     * \brief Determines whether the given signature has a ``noexcept`` specification.
     *
     *\{
     */

    template <typename Signature>
    struct signature_is_noexcept
        : public std::bool_constant<signature_remove_noexcept<Signature>::value>
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_REMOVE_REF_QUALIFIER signature_remove_ref_qualifier
     * \ingroup TYPE_TRAITS
     * \brief Removes the ref-qualifier of a signature (if present).
     *
     *\{
     */

    namespace detail
    {
        template <typename Signature, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl;

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params...), isNoexcept>
            : public std::false_type
        {
            using type = Return(Params...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params..., ...), isNoexcept>
            : public std::false_type
        {
            using type = Return(Params..., ...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params...) const, isNoexcept>
            : public std::false_type
        {
            using type = Return(Params...) const noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params..., ...) const, isNoexcept>
            : public std::false_type
        {
            using type = Return(Params..., ...) const noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params...)&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params..., ...)&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params...) const&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) const noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params..., ...) const&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) const noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params...)&&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params..., ...)&&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params...) const&&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) const noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_ref_qualifier_impl<Return(Params..., ...) const&&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) const noexcept(isNoexcept);
        };

        // Todo: When msvc some day supports deducing noexcept, we can further simplify this.
        template <typename Signature, typename Trait = signature_remove_noexcept<Signature>>
        struct signature_remove_ref_qualifier_
            : public signature_remove_ref_qualifier_impl<typename Trait::type, Trait::value>
        {
        };

        template <typename Signature>
        using signature_remove_ref_qualifier = signature_remove_ref_qualifier_<Signature>;
    }

    template <typename Signature>
    struct signature_remove_ref_qualifier
        : public detail::signature_apply_trait<Signature, detail::signature_remove_ref_qualifier>
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_REMOVE_CONST_QUALIFIER signature_remove_const_qualifier
     * \ingroup TYPE_TRAITS
     * \brief Removes the const-qualifier of a signature (if present).
     *
     *\{
     */

    namespace detail
    {
        template <typename Signature, bool isNoexcept>
        struct signature_remove_const_qualifier_impl;

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params...), isNoexcept>
            : public std::false_type
        {
            using type = Return(Params...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params...) const, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params..., ...), isNoexcept>
            : public std::false_type
        {
            using type = Return(Params..., ...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params..., ...) const, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params...)&, isNoexcept>
            : public std::false_type
        {
            using type = Return(Params...) & noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params...) const&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) & noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params..., ...)&, isNoexcept>
            : public std::false_type
        {
            using type = Return(Params..., ...) & noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params..., ...) const&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) & noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params...)&&, isNoexcept>
            : public std::false_type
        {
            using type = Return(Params...) && noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params...) const&&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) && noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params..., ...)&&, isNoexcept>
            : public std::false_type
        {
            using type = Return(Params..., ...) && noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_remove_const_qualifier_impl<Return(Params..., ...) const&&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) && noexcept(isNoexcept);
        };

        // Todo: When msvc some day supports deducing noexcept, we can further simplify this.
        template <typename Signature, typename Trait = signature_remove_noexcept<Signature>>
        struct signature_remove_const_qualifier_
            : public signature_remove_const_qualifier_impl<typename Trait::type, Trait::value>
        {
        };

        template <typename Signature>
        using signature_remove_const_qualifier = signature_remove_const_qualifier_<Signature>;
    }

    template <typename Signature>
    struct signature_remove_const_qualifier
        : public detail::signature_apply_trait<Signature, detail::signature_remove_const_qualifier>
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_ADD_CONST_QUALIFIER signature_add_const_qualifier
     * \ingroup TYPE_TRAITS
     * \brief Adds the const-qualifier of a signature (if not already present).
     *
     *\{
     */

    namespace detail
    {
        template <typename Signature, bool isNoexcept>
        struct signature_add_const_qualifier_impl;

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_add_const_qualifier_impl<Return(Params...), isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) const noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_add_const_qualifier_impl<Return(Params..., ...), isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) const noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_add_const_qualifier_impl<Return(Params...)&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) const& noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_add_const_qualifier_impl<Return(Params..., ...)&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) const& noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_add_const_qualifier_impl<Return(Params...)&&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params...) const&& noexcept(isNoexcept);
        };

        template <typename Return, typename... Params, bool isNoexcept>
        struct signature_add_const_qualifier_impl<Return(Params..., ...)&&, isNoexcept>
            : public std::true_type
        {
            using type = Return(Params..., ...) const&& noexcept(isNoexcept);
        };

        // Todo: When msvc some day supports deducing noexcept, we can further simplify this.
        template <typename Signature, typename Trait = signature_remove_noexcept<Signature>>
        struct signature_add_const_qualifier_
            : public signature_add_const_qualifier_impl<typename Trait::type, Trait::value>
        {
        };

        template <typename Signature>
            requires signature_remove_const_qualifier<Signature>::value
        struct signature_add_const_qualifier_<Signature>
            : public std::false_type
        {
            using type = Signature;
        };

        template <typename Signature>
        using signature_add_const_qualifier = signature_add_const_qualifier_<Signature>;
    }

    template <typename Signature>
    struct signature_add_const_qualifier
        : public detail::signature_apply_trait<Signature, detail::signature_add_const_qualifier>
    {
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

    template <typename Signature>
    struct signature_decay
    {
        using type =
            typename detail::signature_remove_ref_qualifier_impl<
                typename detail::signature_remove_const_qualifier_impl<
                    typename detail::signature_remove_noexcept<
                        signature_remove_call_convention_t<Signature>>::type,
                    false>::type,
                false>::type;
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
    struct signature_return_type
        /** \cond Help doxygen with recursion.*/
        : public signature_return_type<signature_decay_t<Signature>>
    /** \endcond */
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
    struct signature_param_type
        /** \cond Help doxygen with recursion.*/
        : public signature_param_type<
              index,
              signature_decay_t<Signature>>
    /** \endcond */
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
     * \defgroup TYPE_TRAITS_SIGNATURE_CONST_QUALIFICATION signature_const_qualification
     * \ingroup TYPE_TRAITS
     * \brief Determines the const-qualification of the given signature.
     *
     *\{
     */

    template <typename Signature>
    struct signature_const_qualification
        : public std::integral_constant<
              Constness,
              signature_remove_const_qualifier<Signature>::value
                  ? Constness::as_const
                  : Constness::non_const>
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_REF_QUALIFICATION signature_ref_qualification
     * \ingroup TYPE_TRAITS
     * \brief Determines the ref-qualification of the given signature.
     *
     *\{
     */

    template <typename Signature>
    struct signature_ref_qualification
        /** \cond Help doxygen with recursion.*/
        : public signature_ref_qualification<
              signature_remove_noexcept_t<
                  signature_remove_const_qualifier_t<
                      signature_remove_call_convention_t<Signature>>>>
    /** \endcond */
    {
    };

    template <typename Return, typename... Params>
    struct signature_ref_qualification<Return(Params...)>
        : public std::integral_constant<
              ValueCategory,
              ValueCategory::any>
    {
    };

    template <typename Return, typename... Params>
    struct signature_ref_qualification<Return(Params..., ...)>
        : public std::integral_constant<
              ValueCategory,
              ValueCategory::any>
    {
    };

    template <typename Return, typename... Params>
    struct signature_ref_qualification<Return(Params...)&>
        : public std::integral_constant<
              ValueCategory,
              ValueCategory::lvalue>
    {
    };

    template <typename Return, typename... Params>
    struct signature_ref_qualification<Return(Params..., ...)&>
        : public std::integral_constant<
              ValueCategory,
              ValueCategory::lvalue>
    {
    };

    template <typename Return, typename... Params>
    struct signature_ref_qualification<Return(Params...) &&>
        : public std::integral_constant<
              ValueCategory,
              ValueCategory::rvalue>
    {
    };

    template <typename Return, typename... Params>
    struct signature_ref_qualification<Return(Params..., ...) &&>
        : public std::integral_constant<
              ValueCategory,
              ValueCategory::rvalue>
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_SIGNATURE_PARAM_LIST signature_param_list
     * \ingroup TYPE_TRAITS
     * \brief Extracts all param types from a given signature (packed into a ``mimicpp::type_list``).
     *
     *\{
     */

    template <typename Signature>
    struct signature_param_list
        /** \cond Help doxygen with recursion.*/
        : public signature_param_list<
              signature_decay_t<Signature>>
    /** \endcond */
    {
    };

    template <typename Return, typename... Params>
    struct signature_param_list<Return(Params...)>
    {
        using type = util::type_list<Params...>;
    };

    /**
     * \}
     */
}

namespace mimicpp::detail
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

    template <typename Signature>
    using normalize_overload_t = signature_remove_noexcept_t<
        signature_remove_call_convention_t<Signature>>;
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp
{
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
              detail::normalize_overload_t<First>,
              detail::normalize_overload_t<Second>>
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

    template <typename First, typename... Others>
    struct is_overload_set
        : public std::conjunction<
              is_overloadable_with<First, Others>...,
              is_overload_set<Others...>>
    {
    };

    template <typename First>
    struct is_overload_set<First>
        : public std::true_type
    {
    };

    /**
     * \}
     */

    /**
     * \defgroup TYPE_TRAITS_UINT_WITH_SIZE uint_with_size
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
