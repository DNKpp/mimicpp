//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_TEMPLATED_HPP
#define MIMICPP_PRINTING_TYPE_TEMPLATED_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Utility.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/type/PrintType.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <memory_resource>
#include <type_traits>
#include <utility>

namespace mimicpp::printing::detail::type
{
#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

    template <typename T>
    [[nodiscard]]
    StringT pretty_template_name()
    {
        StringT name = detail::prettify_type_name(detail::type_name<T>());
        name.erase(std::ranges::find(name, '<'), name.cend());

        return name;
    }

    template <typename NameGenerator>
    struct basic_template_type_printer
    {
        [[nodiscard]]
        static StringViewT name()
        {
            static const StringT name = std::invoke(NameGenerator{});
            return name;
        }
    };

    template <typename Type, template <typename...> typename Template, typename LeadingArgList>
    struct is_default_arg_for
        : public std::false_type
    {
    };

    template <typename Type, template <typename...> typename Template, typename... LeadingArgs>
        requires requires { typename Template<LeadingArgs...>; }
    struct is_default_arg_for<Type, Template, type_list<LeadingArgs...>>
        : public std::bool_constant<
              std::same_as<
                  Template<LeadingArgs...>,
                  Template<LeadingArgs..., Type>>>
    {
    };

    template <typename Type, template <typename...> typename Template, typename LeadingArgList>
    concept default_arg_for = is_default_arg_for<Type, Template, LeadingArgList>::value;

    template <typename List>
    struct type_list_pop_back_if_can
        : public type_list_pop_back<List>
    {
    };

    template <>
    struct type_list_pop_back_if_can<type_list<>>
    {
        using type = type_list<>;
    };

    template <
        typename ArgList,
        template <typename...> typename Template,
        typename PopBack = type_list_pop_back_if_can<ArgList>>
    struct drop_default_args_for_impl
    {
        using type = ArgList;
    };

    template <typename ArgList, template <typename...> typename Template, typename PopBack>
        requires requires { typename PopBack::popped; }
              && default_arg_for<typename PopBack::popped, Template, typename PopBack::type>
    struct drop_default_args_for_impl<ArgList, Template, PopBack>
        : public drop_default_args_for_impl<typename PopBack::type, Template>
    {
    };

    template <template <typename...> typename Template, typename ArgList>
    struct drop_default_args_for
        : public drop_default_args_for_impl<ArgList, Template>
    {
    };

    template <template <typename...> typename Template, typename ArgList>
    struct template_type_name_generator_fn
    {
        [[nodiscard]]
        StringT operator()() const
        {
            using MinimalArgList = typename drop_default_args_for<Template, ArgList>::type;
            using Type = type_list_populate_t<Template, MinimalArgList>;
            const StringT templateName = pretty_template_name<Type>();

            StringStreamT out{};
            out << templateName << '<';
            print_separated(std::ostreambuf_iterator{out}, ", ", MinimalArgList{});
            out << '>';

            return std::move(out).str();
        }
    };

    template <template <typename...> typename Template, typename... Ts>
    struct template_type_printer<Template<Ts...>>
        : public basic_template_type_printer<
              template_type_name_generator_fn<Template, type_list<Ts...>>>
    {
    };

    template <typename Allocator>
    struct is_pmr_allocator
        : public std::false_type
    {
    };

    template <typename T>
    struct is_pmr_allocator<std::pmr::polymorphic_allocator<T>>
        : public std::true_type
    {
    };

    template <template <typename...> typename Template, typename... Ts>
    concept std_pmr_container =
        requires {
            typename Template<Ts...>::allocator_type;
            typename Template<Ts...>::value_type;
        }
        && is_pmr_allocator<typename Template<Ts...>::allocator_type>::value
        && default_arg_for<std::allocator<typename Template<Ts...>::value_type>, Template, type_list_pop_back_t<type_list<Ts...>>>;

    static constexpr StringViewT stdPrefix{"std::"};

    template <template <typename...> typename Template, typename ArgList>
    struct potential_pmr_container_type_name_generator_fn
    {
        [[nodiscard]]
        StringT operator()() const
        {
            StringT name = std::invoke(
                template_type_name_generator_fn<
                    Template,
                    type_list_pop_back_t<ArgList>>{});

            // we do not want to accidentally manipulate non-std types, so make sure the `std::`` is actually part of the type
            if (name.starts_with(stdPrefix))
            {
                name.insert(stdPrefix.size(), StringViewT{"pmr::"});
            }
            // It's not an actual `std` type, but we've removed the allocator for the name generation, thus we need
            // generate it again with the actual allocator.
            else
            {
                name = std::invoke(template_type_name_generator_fn<Template, ArgList>{});
            }

            return name;
        }
    };

    template <template <typename...> typename Template, typename... Ts>
        requires std_pmr_container<Template, Ts...>
    struct template_type_printer<Template<Ts...>>
        : public basic_template_type_printer<
              potential_pmr_container_type_name_generator_fn<Template, type_list<Ts...>>>
    {
    };

    // used for array, span and the like
    // should also handle c++26s inplace_vector
    // see: https://en.cppreference.com/w/cpp/container/inplace_vector
    template <template <typename, auto...> typename Template, typename T, auto n>
    struct template_type_printer<Template<T, n>>
        : public basic_template_type_printer<
              decltype([] {
                  return format::format(
                      "{}<{}, {}>",
                      pretty_template_name<Template<T, n>>(),
                      mimicpp::print_type<T>(),
                      n);
              })>
    {
    };

    template <template <typename, auto...> typename Template, typename T, auto n>
        requires std::same_as<Template<T>, Template<T, n>>
    struct template_type_printer<Template<T, n>>
        : public basic_template_type_printer<
              decltype([] {
                  return format::format(
                      "{}<{}>",
                      pretty_template_name<Template<T>>(),
                      mimicpp::print_type<T>());
              })>
    {
    };

#endif

}

#endif
