//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_TYPE_LIST_HPP
#define MIMICPP_UTILITIES_TYPE_LIST_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/Concepts.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <cstddef>
    #include <tuple>
    #include <utility>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \brief A very basic type-list template.
     * \tparam Args The types.
     */
    template <typename... Args>
    struct type_list
    {
        static constexpr std::size_t size = sizeof...(Args);
    };

    namespace detail
    {
        template <typename ProcessedList, typename PendingList>
        struct type_list_reverse;

        template <typename ProcessedList>
        struct type_list_reverse<ProcessedList, type_list<>>
        {
            using type = ProcessedList;
        };

        template <typename... ProcessedArgs, typename First, typename... Args>
        struct type_list_reverse<type_list<ProcessedArgs...>, type_list<First, Args...>>
            : public type_list_reverse<type_list<First, ProcessedArgs...>, type_list<Args...>>
        {
        };
    }

    template <typename TypeList>
    struct type_list_reverse
    {
        using type = typename detail::type_list_reverse<type_list<>, TypeList>::type;
    };

    template <typename TypeList>
    using type_list_reverse_t = typename type_list_reverse<TypeList>::type;

    namespace detail
    {
        template <typename ProcessedList, typename PendingList>
        struct type_list_pop_back;

        template <typename... ProcessedArgs, typename Last>
        struct type_list_pop_back<type_list<ProcessedArgs...>, type_list<Last>>
        {
            using type = type_list<ProcessedArgs...>;
            using popped = Last;
        };

        template <typename... ProcessedArgs, typename First, typename... Args>
        struct type_list_pop_back<type_list<ProcessedArgs...>, type_list<First, Args...>>
            : public type_list_pop_back<type_list<ProcessedArgs..., First>, type_list<Args...>>
        {
        };
    }

    template <typename TypeList>
    struct type_list_pop_back
        : public detail::type_list_pop_back<type_list<>, TypeList>
    {
    };

    template <typename TypeList>
    using type_list_pop_back_t = typename type_list_pop_back<TypeList>::type;

    template <template <typename...> typename Template, typename TypeList>
    struct type_list_populate;

    template <template <typename...> typename Template, typename... Args>
    struct type_list_populate<Template, type_list<Args...>>
    {
        using type = Template<Args...>;
    };

    template <template <typename...> typename Template, typename TypeList>
    using type_list_populate_t = typename type_list_populate<Template, TypeList>::type;
}

template <typename... Args>
struct std::tuple_size<mimicpp::util::type_list<Args...>> // NOLINT(*-dcl58-cpp)
    : std::integral_constant<std::size_t, mimicpp::util::type_list<Args...>::size>
{
};

namespace mimicpp::util::detail
{
    template <typename Parsed, typename... Rest>
    struct unique;

    template <typename... Uniques, typename First, typename... Others>
    struct unique<
        type_list<Uniques...>,
        First,
        Others...>
    {
        using current_t = std::conditional_t<
            same_as_any<First, Uniques...>,
            type_list<Uniques...>,
            type_list<Uniques..., First>>;

        using type_t = typename unique<
            current_t,
            Others...>::type_t;
    };

    template <typename... Uniques>
    struct unique<type_list<Uniques...>>
    {
        using type_t = type_list<Uniques...>;
    };

    template <typename... Types>
    using unique_list_t = typename unique<type_list<>, Types...>::type_t;

    template <std::default_initializable FillElement, std::size_t n, typename... Elements>
        requires(sizeof...(Elements) <= n)
    [[nodiscard]]
    constexpr auto expand_tuple(std::tuple<Elements...>&& tuple)
    {
        // prior to c++23, tuple_cat does not officially support tuple-like types,
        // thus we transform the generated array manually
        return std::tuple_cat(
            std::move(tuple),
            std::apply(
                [](auto&&... elements) { return std::make_tuple(std::move(elements)...); },
                std::array<FillElement, n - sizeof...(Elements)>{}));
    }
}

#endif
