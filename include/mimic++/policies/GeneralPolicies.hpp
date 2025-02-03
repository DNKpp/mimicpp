//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_POLICIES_GENERAL_POLICIES_HPP
#define MIMICPP_POLICIES_GENERAL_POLICIES_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/printing/StatePrinter.hpp"

#include <cassert>
#include <iterator>

namespace mimicpp::detail
{
    [[nodiscard]]
    constexpr bool is_matching(const Constness lhs, const Constness rhs) noexcept
    {
        return std::cmp_not_equal(0, util::to_underlying(lhs) & util::to_underlying(rhs));
    }

    [[nodiscard]]
    constexpr bool is_matching(const ValueCategory lhs, const ValueCategory rhs) noexcept
    {
        return std::cmp_not_equal(0, util::to_underlying(lhs) & util::to_underlying(rhs));
    }
}

namespace mimicpp::expectation_policies
{
    class InitFinalize
    {
    public:
        template <typename Return, typename... Args>
        static constexpr void finalize_call(const call::Info<Return, Args...>&) noexcept
        {
        }
    };

    template <ValueCategory expected>
    class Category
    {
    public:
        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        template <typename Return, typename... Args>
        static constexpr bool matches(const call::Info<Return, Args...>& info) noexcept
        {
            return mimicpp::detail::is_matching(info.fromCategory, expected);
        }

        template <typename Return, typename... Args>
        static constexpr void consume([[maybe_unused]] const call::Info<Return, Args...>& info) noexcept
        {
            assert(mimicpp::detail::is_matching(info.fromCategory, expected) && "Call does not match.");
        }

        [[nodiscard]]
        static StringT describe()
        {
            StringStreamT stream{};
            stream << "expect: from ";
            mimicpp::print(std::ostreambuf_iterator{stream}, expected);
            stream << " category overload";

            return std::move(stream).str();
        }
    };

    template <Constness constness>
    class Constness
    {
    public:
        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        template <typename Return, typename... Args>
        static constexpr bool matches(const call::Info<Return, Args...>& info) noexcept
        {
            return mimicpp::detail::is_matching(info.fromConstness, constness);
        }

        template <typename Return, typename... Args>
        static constexpr void consume([[maybe_unused]] const call::Info<Return, Args...>& info) noexcept
        {
            assert(mimicpp::detail::is_matching(info.fromConstness, constness) && "Call does not match.");
        }

        [[nodiscard]]
        static StringT describe()
        {
            StringStreamT stream{};
            stream << "expect: from ";
            mimicpp::print(std::ostreambuf_iterator{stream}, constness);
            stream << " qualified overload";

            return std::move(stream).str();
        }
    };
}

#endif
