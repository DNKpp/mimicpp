//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_SCOPE_NAVIGATOR_HPP
#define MIMICPP_PRINTING_TYPE_SCOPE_NAVIGATOR_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/Algorithm.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <concepts>
#include <functional>
#include <optional>

namespace mimicpp::printing::type
{
    // see: https://en.cppreference.com/w/cpp/string/byte/isspace
    constexpr auto is_space = [](char const c) noexcept {
        return static_cast<bool>(std::isspace(static_cast<unsigned char>(c)));
    };

    // see: https://en.cppreference.com/w/cpp/string/byte/isdigit
    constexpr auto is_digit = [](char const c) noexcept {
        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(c)));
    };

    // see: https://en.cppreference.com/w/cpp/string/byte/isxdigit
    constexpr auto is_hex_digit = [](char const c) noexcept {
        return static_cast<bool>(std::isxdigit(static_cast<unsigned char>(c)));
    };

    template <std::random_access_iterator Iter>
        requires std::constructible_from<StringViewT, Iter, Iter>
    [[nodiscard]]
    constexpr StringViewT trimmed(Iter const begin, Iter const end)
    {
        auto const trimmedBegin = std::ranges::find_if_not(begin, end, is_space);
        auto const trimmedEnd = std::ranges::find_if_not(
            std::make_reverse_iterator(end),
            std::make_reverse_iterator(trimmedBegin),
            is_space);
        return StringViewT{trimmedBegin, trimmedEnd.base()};
    }

    [[nodiscard]]
    constexpr StringViewT trimmed(StringViewT const str)
    {
        return trimmed(str.cbegin(), str.cend());
    }

    constexpr StringViewT anonymousNamespaceTargetScopeText{"{anon-ns}"};
    constexpr StringViewT scopeDelimiter{"::"};
    constexpr StringViewT argumentDelimiter{","};
    constexpr std::array openingBrackets{'<', '(', '[', '{', '`'};
    constexpr std::array closingBrackets{'>', ')', ']', '}', '\''};

    struct scope_info
    {
        StringViewT identifier{};
        std::optional<StringViewT> templateArgs{};
        std::optional<StringViewT> functionArgs{};
        StringViewT specs{};
    };

    [[nodiscard]]
    constexpr std::ranges::borrowed_subrange_t<StringViewT> detect_last_enclosed(
        StringViewT const& name,
        StringViewT const opening,
        StringViewT const closing)
    {
        MIMICPP_ASSERT(!name.starts_with(' ') && !name.ends_with(' '), "Name is not trimmed.");
        MIMICPP_ASSERT(1u == opening.size(), "Opening token must have a size of one.");
        MIMICPP_ASSERT(1u == closing.size(), "Closing token must have a size of one.");

        auto reversedName = name | std::views::reverse;

        auto const delimiterMatch = util::find_next_unwrapped_token(
            reversedName,
            scopeDelimiter,
            closingBrackets,
            openingBrackets);
        auto const closingMatch = util::find_next_unwrapped_token(
            std::ranges::subrange{reversedName.begin(), delimiterMatch.begin()},
            closing,
            closingBrackets,
            openingBrackets);
        auto const openingIter = util::find_closing_token(
            std::ranges::subrange{closingMatch.end(), delimiterMatch.begin()},
            closing.front(),
            opening.front());
        // If no `opening closing` range could be found or the identifier between `scopeDelimiter opening` is empty,
        // it's not what we are looking for.
        if (openingIter == delimiterMatch.begin()
            || std::ranges::empty(trimmed(delimiterMatch.begin().base(), openingIter.base() - 1)))
        {
            return {name.cend(), name.cend()};
        }

        return {openingIter.base() - 1, closingMatch.begin().base()};
    }

    [[nodiscard]]
    constexpr std::optional<std::tuple<StringViewT, StringViewT>> detect_function_scope_info(StringViewT& name)
    {
        MIMICPP_ASSERT(!name.starts_with(' ') && !name.ends_with(' '), "Name is not trimmed.");

        auto const enclosedMatch = detect_last_enclosed(name, "(", ")");
        if (enclosedMatch.empty())
        {
            return std::nullopt;
        }
        MIMICPP_ASSERT(enclosedMatch.front() == '(' && enclosedMatch.back() == ')', "Unexpected result.");

        StringViewT argList = trimmed(enclosedMatch.begin() + 1, enclosedMatch.end() - 1);
        if (argList == "void")
        {
            argList = {};
        }

        StringViewT const specs = trimmed(enclosedMatch.end(), name.end());
        name = {name.cbegin(), enclosedMatch.begin()};

        return std::tuple{argList, specs};
    }

    [[nodiscard]]
    constexpr std::optional<std::tuple<StringViewT, StringViewT>> detect_template_scope_info(StringViewT& name)
    {
        MIMICPP_ASSERT(!name.starts_with(' ') && !name.ends_with(' '), "Name is not trimmed.");

        auto const enclosedMatch = detect_last_enclosed(name, "<", ">");
        if (enclosedMatch.empty())
        {
            return std::nullopt;
        }
        MIMICPP_ASSERT(enclosedMatch.front() == '<' && enclosedMatch.back() == '>', "Unexpected result.");

        StringViewT const specs = trimmed(enclosedMatch.end(), name.cend());
        StringViewT const args = trimmed(enclosedMatch.begin() + 1, enclosedMatch.end() - 1);
        name = trimmed(name.cbegin(), enclosedMatch.begin());

        return std::tuple{args, specs};
    }

    [[nodiscard]]
    constexpr scope_info gather_scope_info(StringViewT scope)
    {
        scope_info info{};
        if (std::optional const fnInfo = detect_function_scope_info(scope))
        {
            auto const& [args, specs] = *fnInfo;
            info.functionArgs = args;
            info.specs = specs;
        }

        if (std::optional const fnInfo = detect_template_scope_info(scope))
        {
            auto const& [args, specs] = *fnInfo;
            info.functionArgs = args;

            if (!specs.empty())
            {
                MIMICPP_ASSERT(!info.functionArgs, "Template can not have specs, when it's a function.");
                info.specs = specs;
            }
        }

        info.identifier = scope;

        return info;
    }

    template <typename Visitor>
        requires std::invocable<Visitor const, scope_info>
    class ScopeNavigator
    {
    public:
        [[nodiscard]]
        explicit constexpr ScopeNavigator(Visitor visitor) noexcept(std::is_nothrow_move_constructible_v<Visitor>)
            : m_Visitor{std::move(visitor)}
        {
        }

        constexpr void operator()(StringViewT content) const
        {
            while (!content.empty())
            {
                auto const delimiter = util::find_next_unwrapped_token(
                    content,
                    scopeDelimiter,
                    openingBrackets,
                    closingBrackets);
                StringViewT const scope = trimmed(content.cbegin(), delimiter.begin());
                content = StringViewT{delimiter.end(), content.end()};
                handle_scope(scope);
            }
        }

    private:
        Visitor m_Visitor;

        constexpr void handle_scope(StringViewT const scope) const
        {
            MIMICPP_ASSERT(!scope.starts_with(' ') && !scope.ends_with(' '), "Scope is not trimmed.");

            if (scope.starts_with('`') && scope.ends_with('\''))
            {
                StringViewT const content = trimmed(scope.substr(1u, scope.size() - 2u));
                std::invoke(*this, content);
            }
            else
            {
                scope_info const info = gather_scope_info(scope);
                std::invoke(m_Visitor, info);
            }
        }
    };
}

#endif
