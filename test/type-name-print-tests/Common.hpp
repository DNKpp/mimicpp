//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "mimic++/utilities/StaticString.hpp"
#include "mimic++/utilities/TypeList.hpp"

#include <string>
#include <string_view>

namespace mimicpp::testing
{
    [[nodiscard, maybe_unused]]
    inline std::string maybe_pattern(std::string const& pattern)
    {
        return "(:?" + pattern + ")?";
    }

    template <typename T, util::StaticString suffixText>
    struct modifier
    {
        using type = T;
        static constexpr std::string_view suffix = suffixText.view();
    };

    namespace mods
    {
        template <typename T>
        using decayed = modifier<T, "">;

        template <typename T>
        using add_lvalue_ref = modifier<T&, "&">;
        template <typename T>
        using add_const_lvalue_ref = modifier<T const&, " const&">;
        template <typename T>
        using add_volatile_lvalue_ref = modifier<T volatile&, " volatile&">;
        template <typename T>
        using add_cv_lvalue_ref = modifier<T const volatile&, " const volatile&">;

        template <typename T>
        using add_rvalue_ref = modifier<T&&, "&&">;
        template <typename T>
        using add_const_rvalue_ref = modifier<T const&&, " const&&">;
        template <typename T>
        using add_volatile_rvalue_ref = modifier<T volatile&&, " volatile&&">;
        template <typename T>
        using add_cv_rvalue_ref = modifier<T const volatile&&, " const volatile&&">;

        template <typename T>
        using add_pointer = modifier<T*, "\\*">;
        template <typename T>
        using add_const_pointer = modifier<T const*, " const\\*">;
    }

    template <template <typename> typename Mod>
    struct modifier_maker
    {
        template <typename T>
        using type = Mod<T>::type;

        static constexpr std::string_view suffix = Mod<int>::suffix;
    };

    template <typename Mod, typename T>
    using mod_type_t = Mod::template type<T>;

    using common_mod_list = util::type_list<
        modifier_maker<mods::decayed>,

        modifier_maker<mods::add_lvalue_ref>,
        modifier_maker<mods::add_const_lvalue_ref>,
        modifier_maker<mods::add_volatile_lvalue_ref>,
        modifier_maker<mods::add_cv_lvalue_ref>,

        modifier_maker<mods::add_rvalue_ref>,
        modifier_maker<mods::add_const_rvalue_ref>,
        modifier_maker<mods::add_volatile_rvalue_ref>,
        modifier_maker<mods::add_cv_rvalue_ref>,

        modifier_maker<mods::add_pointer>,
        modifier_maker<mods::add_const_pointer>>;
}

namespace mimicpp
{
#define TESTING_COMMON_MODS (               \
    testing::mods::decayed,                 \
                                            \
    testing::mods::add_lvalue_ref,          \
    testing::mods::add_const_lvalue_ref,    \
    testing::mods::add_volatile_lvalue_ref, \
    testing::mods::add_cv_lvalue_ref,       \
                                            \
    testing::mods::add_rvalue_ref,          \
    testing::mods::add_const_rvalue_ref,    \
    testing::mods::add_volatile_rvalue_ref, \
    testing::mods::add_cv_rvalue_ref,       \
                                            \
    testing::mods::add_pointer,             \
    testing::mods::add_const_pointer)
}

