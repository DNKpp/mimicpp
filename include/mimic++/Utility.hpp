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
#include <optional>
#include <source_location>
#include <string_view>
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

    [[nodiscard]]
    constexpr bool is_same_source_location(
        const std::source_location& lhs,
        const std::source_location& rhs) noexcept
    {
        return std::string_view{lhs.file_name()} == std::string_view{rhs.file_name()}
            && std::string_view{lhs.function_name()} == std::string_view{rhs.function_name()}
            && lhs.line() == rhs.line()
            && lhs.column() == rhs.column();
    }

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
}

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

    struct expectation_info
    {
        std::source_location sourceLocation = std::source_location::current();
        StringT mockName{};

        [[nodiscard]]
        friend bool operator==(const expectation_info& lhs, const expectation_info& rhs)
        {
            return lhs.mockName == rhs.mockName
                && is_same_source_location(lhs.sourceLocation, rhs.sourceLocation);
        }
    };
}

#endif
