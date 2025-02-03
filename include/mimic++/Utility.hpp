//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITY_HPP
#define MIMICPP_UTILITY_HPP

#pragma once

#include "mimic++/Fwd.hpp"

#include <array>
#include <cassert>
#include <ranges>
#include <tuple>
#include <utility>

namespace mimicpp
{
    template <typename...>
    struct always_false
        : public std::bool_constant<false>
    {
    };

    template <std::size_t priority>
    struct priority_tag
        /** \cond Help doxygen with recursion.*/
        : public priority_tag<priority - 1>
    /** \endcond */
    {
    };

    template <>
    struct priority_tag<0>
    {
    };

    template <typename From, typename To>
    concept explicitly_convertible_to =
        requires {
            static_cast<To>(std::declval<From>());
        };

    template <typename From, typename To>
    concept nothrow_explicitly_convertible_to =
        explicitly_convertible_to<From, To>
        && requires {
               { static_cast<To>(std::declval<From>()) } noexcept;
           };

    template <typename T, typename... Others>
    concept same_as_any = (... || std::same_as<T, Others>);

    template <typename T>
        requires std::is_enum_v<T>
    [[nodiscard]]
    constexpr std::underlying_type_t<T> to_underlying(const T value) noexcept
    {
        return static_cast<std::underlying_type_t<T>>(value);
    }

    template <typename T, template <typename> typename Trait>
    concept satisfies = Trait<T>::value;

    // GCOVR_EXCL_START

#ifdef __cpp_lib_unreachable
    using std::unreachable;
#else

    /**
     * \brief Invokes undefined behavior
     * \see https://en.cppreference.com/w/cpp/utility/unreachable
     * \note Implementation directly taken from https://en.cppreference.com/w/cpp/utility/unreachable
     */
    [[noreturn]]
    inline void unreachable()
    {
        assert(false);

            // Uses compiler specific extensions if possible.
            // Even if no extension is used, undefined behavior is still raised by
            // an empty function body and the noreturn attribute.
    #if defined(_MSC_VER) && !defined(__clang__) // MSVC
        __assume(false);
    #else                                        // GCC, Clang
        __builtin_unreachable();
    #endif
    }
#endif

    // GCOVR_EXCL_STOP

    [[nodiscard]]
    constexpr bool is_matching(const Constness lhs, const Constness rhs) noexcept
    {
        return std::cmp_not_equal(0, to_underlying(lhs) & to_underlying(rhs));
    }

    [[nodiscard]]
    constexpr bool is_matching(const ValueCategory lhs, const ValueCategory rhs) noexcept
    {
        return std::cmp_not_equal(0, to_underlying(lhs) & to_underlying(rhs));
    }

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
struct std::tuple_size<mimicpp::type_list<Args...>> // NOLINT(*-dcl58-cpp)
    : std::integral_constant<std::size_t, mimicpp::type_list<Args...>::size>
{
};

namespace mimicpp::detail
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
