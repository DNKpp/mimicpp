//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_TEMPLATED_HPP
#define MIMICPP_PRINTING_TYPE_TEMPLATED_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/Fwd.hpp"
#include "mimic++/printing/type/PrintType.hpp"
#include "mimic++/utilities/TypeList.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <concepts>
    #include <functional>
    #include <memory>
    #include <memory_resource>
    #include <type_traits>
    #include <utility>
#endif

namespace mimicpp::printing::type::detail
{
#ifdef MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES

    template <typename T, print_iterator OutIter>
    constexpr OutIter pretty_template_name(OutIter out)
    {
        StringT name = type_name<T>();
        auto const iter = std::ranges::find(name, '<');
        MIMICPP_ASSERT(iter != name.cend(), "Given name is not a template.");
        name.erase(iter, name.end());

        return type::prettify_type(std::move(out), std::move(name));
    }

    template <typename NameGenerator>
    struct basic_template_type_printer
    {
        template <print_iterator OutIter>
        static constexpr OutIter print(OutIter out)
        {
            return std::invoke(NameGenerator{}, std::move(out));
        }
    };

    template <typename Type, template <typename...> typename Template, typename LeadingArgList>
    struct is_default_arg_for
        : public std::false_type
    {
    };

    template <typename Type, template <typename...> typename Template, typename... LeadingArgs>
        requires requires { typename Template<LeadingArgs...>; }
    struct is_default_arg_for<Type, Template, util::type_list<LeadingArgs...>>
        : public std::is_same<
              Template<LeadingArgs...>,
              Template<LeadingArgs..., Type>>
    {
    };

    template <typename Type, template <typename...> typename Template, typename LeadingArgList>
    concept default_arg_for = is_default_arg_for<Type, Template, LeadingArgList>::value;

    template <typename List>
    struct type_list_pop_back_if_can
        : public util::type_list_pop_back<List>
    {
    };

    template <>
    struct type_list_pop_back_if_can<util::type_list<>>
    {
        using type = util::type_list<>;
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
        template <print_iterator OutIter>
        constexpr OutIter operator()(OutIter out) const
        {
            using MinimalArgList = typename drop_default_args_for<Template, ArgList>::type;
            using Type = util::type_list_populate_t<Template, MinimalArgList>;

            out = pretty_template_name<Type>(std::move(out));
            out = format::format_to(std::move(out), "<");
            out = print_separated(std::move(out), ", ", MinimalArgList{});
            out = format::format_to(std::move(out), ">");

            return out;
        }
    };

    template <template <typename...> typename Template, typename... Ts>
    struct template_type_printer<Template<Ts...>>
        : public basic_template_type_printer<
              template_type_name_generator_fn<Template, util::type_list<Ts...>>>
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
        && default_arg_for<std::allocator<typename Template<Ts...>::value_type>, Template, util::type_list_pop_back_t<util::type_list<Ts...>>>;

    static constexpr StringViewT stdPrefix{"std::"};

    template <template <typename...> typename Template, typename ArgList>
    struct potential_pmr_container_type_name_generator_fn
    {
        template <print_iterator OutIter>
        OutIter operator()(OutIter out) const
        {
            StringT const name = std::invoke(
                [] {
                    StringStreamT stream{};
                    std::invoke(
                        template_type_name_generator_fn<
                            Template,
                            util::type_list_pop_back_t<ArgList>>{},
                        std::ostreambuf_iterator{stream});
                    return std::move(stream).str();
                });

            // we do not want to accidentally manipulate non-std types, so make sure the `std::`` is actually part of the type
            if (name.starts_with(stdPrefix))
            {
                out = format::format_to(std::move(out), "std::pmr::");
                out = std::ranges::copy(
                          name.cbegin() + stdPrefix.size(),
                          name.cend(),
                          std::move(out))
                          .out;
            }
            // It's not an actual `std` type, but we've removed the allocator for the name generation, thus we need
            // generate it again with the actual allocator.
            else
            {
                out = std::invoke(
                    template_type_name_generator_fn<Template, ArgList>{},
                    std::move(out));
            }

            return out;
        }
    };

    template <template <typename...> typename Template, typename... Ts>
        requires std_pmr_container<Template, Ts...>
    struct template_type_printer<Template<Ts...>>
        : public basic_template_type_printer<
              potential_pmr_container_type_name_generator_fn<Template, util::type_list<Ts...>>>
    {
    };

    // used for array, span and the like
    // should also handle c++26s inplace_vector
    // see: https://en.cppreference.com/w/cpp/container/inplace_vector
    template <template <typename, auto...> typename Template, typename T, auto n>
    struct template_type_printer<Template<T, n>>
        : public basic_template_type_printer<
              decltype([]<print_iterator OutIter>(OutIter out) {
                  out = pretty_template_name<Template<T, n>>(std::move(out));
                  out = format::format_to(std::move(out), "<");
                  out = mimicpp::print_type<T>(std::move(out));
                  out = format::format_to(std::move(out), ", {}>", n);

                  return out;
              })>
    {
    };

    template <template <typename, auto...> typename Template, typename T, auto n>
        requires std::same_as<Template<T>, Template<T, n>>
    struct template_type_printer<Template<T, n>>
        : public basic_template_type_printer<
              decltype([]<print_iterator OutIter>(OutIter out) {
                  out = pretty_template_name<Template<T, n>>(std::move(out));
                  out = format::format_to(std::move(out), "<");
                  out = mimicpp::print_type<T>(std::move(out));
                  out = format::format_to(std::move(out), ">");

                  return out;
              })>
    {
    };

#endif

}

#endif
