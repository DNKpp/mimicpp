//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_NAME_PARSER_HPP
#define MIMICPP_PRINTING_TYPE_NAME_PARSER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/type/NameLexer.hpp"
#include "mimic++/utilities/Concepts.hpp"
#include "mimic++/utilities/TypeList.hpp"

#include <concepts>
#include <functional>
#include <vector>

namespace mimicpp::printing::type::parsing
{
    template <typename T>
    concept parser_visitor = std::movable<T>
                          && requires(std::unwrap_reference_t<T> visitor, StringViewT content) {
                                 visitor.begin();
                                 visitor.end();

                                 visitor.add_identifier(content);
                                 visitor.add_scope();

                                 visitor.begin_name();
                                 visitor.end_name();

                                 visitor.begin_type();
                                 visitor.end_type();
                             };

    template <parser_visitor Visitor>
    [[nodiscard]]
    constexpr auto& unwrap_visitor(Visitor& visitor) noexcept
    {
        return static_cast<
            std::add_lvalue_reference_t<
                std::unwrap_reference_t<Visitor>>>(visitor);
    }

    namespace token
    {
        class Identifier
        {
        public:
            StringViewT content{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(!content.empty(), "Empty identifier is not allowed.");

                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.add_identifier(content);
            }
        };

        class ScopeSequence
        {
        public:
            std::vector<Identifier> scopes{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(!scopes.empty(), "Empty scope-sequence is not allowed.");

                auto& unwrapped = unwrap_visitor(visitor);

                for (auto const& id : scopes)
                {
                    std::invoke(id, unwrapped);
                    unwrapped.add_scope();
                }
            }
        };

        class Type
        {
        public:
            std::optional<ScopeSequence> scopes{};
            Identifier identifier;

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.begin_type();
                unwrapped.begin_name();

                if (scopes)
                {
                    std::invoke(*scopes, unwrapped);
                }

                std::invoke(identifier, unwrapped);

                unwrapped.end_name();
                unwrapped.end_type();
            }
        };
    }

    using Token = std::variant<
        token::Identifier,
        token::ScopeSequence,
        token::Type>;
    using TokenStack = std::vector<Token>;

    template <typename T>
    concept token_type = requires(Token const& token) {
        { std::holds_alternative<Token>(token) } -> util::boolean_testable;
    };

    namespace detail
    {
        template <typename Last, typename... Others>
        [[nodiscard]]
        constexpr bool is_suffix_of(
            [[maybe_unused]] util::type_list<Last, Others...> const types,
            std::span<Token const> const tokenStack)
        {
            if (tokenStack.empty()
                || !std::holds_alternative<Last>(tokenStack.back()))
            {
                return false;
            }

            if constexpr (0u < sizeof...(Others))
            {
                return is_suffix_of(
                    util::type_list<Others...>{},
                    tokenStack.first(tokenStack.size() - 1));
            }

            return true;
        }
    }

    template <token_type First, token_type... Others>
    constexpr bool is_suffix_of(std::span<Token const> const tokenStack)
    {
        using types = util::type_list<First, Others...>;

        return 1u + sizeof...(Others) <= tokenStack.size()
            && detail::is_suffix_of(util::type_list_reverse_t<types>{}, tokenStack);
    }

    template <token_type First, token_type... Others>
    [[nodiscard]]
    constexpr std::optional<std::tuple<First&, Others&...>> match_suffix(std::span<Token> const tokenStack)
    {
        if (is_suffix_of<First, Others...>(tokenStack))
        {
            auto const suffix = tokenStack.last(1u + sizeof...(Others));

            std::size_t i{0u};
            return std::tie(
                std::get<First>(suffix[0u]),
                std::get<Others>(suffix[++i])...);
        }

        return std::nullopt;
    }

    namespace token
    {
        inline bool try_reduce_as_scope_sequence(TokenStack& tokenStack)
        {
            if (std::optional suffix = match_suffix<ScopeSequence, Identifier>(tokenStack))
            {
                auto& [scopeSeq, id] = *suffix;

                scopeSeq.scopes.emplace_back(std::move(id));
                tokenStack.pop_back();

                return true;
            }

            if (std::optional suffix = match_suffix<Identifier>(tokenStack))
            {
                ScopeSequence seq{
                    .scopes = {std::move(std::get<0>(*suffix))}};
                tokenStack.pop_back();
                tokenStack.emplace_back(std::move(seq));

                return true;
            }

            return false;
        }

        inline bool try_reduce_as_type(TokenStack& tokenStack)
        {
            if (std::optional suffix = match_suffix<ScopeSequence, Identifier>(tokenStack))
            {
                auto& [scopeSeq, id] = *suffix;

                Type newType{
                    .scopes = std::move(scopeSeq),
                    .identifier = std::move(id)};
                tokenStack.resize(tokenStack.size() - 2u);
                tokenStack.emplace_back(std::move(newType));

                return true;
            }

            if (std::optional suffix = match_suffix<Identifier>(tokenStack))
            {
                Type newType{
                    .identifier = {std::move(std::get<0>(*suffix))}};
                tokenStack.pop_back();
                tokenStack.emplace_back(std::move(newType));

                return true;
            }

            return false;
        }
    }

    template <parser_visitor Visitor>
    class NameParser
    {
    public:
        [[nodiscard]]
        explicit NameParser(Visitor visitor, StringViewT content) noexcept(std::is_nothrow_move_constructible_v<Visitor>)
            : m_Visitor{std::move(visitor)},
              m_Lexer{std::move(content)}
        {
        }

        void operator()()
        {
            visitor().begin();

            for (lexing::token next = m_Lexer.next();
                 !std::holds_alternative<lexing::end>(next.classification);
                 next = m_Lexer.next())
            {
                std::visit(
                    [&](auto const& tokenClass) { handle_lexer_token(tokenClass); },
                    next.classification);
            }

            try_reduce_as_type(m_TokenStack);

            MIMICPP_ASSERT(1u == m_TokenStack.size(), "A single end-state is required.");
            MIMICPP_ASSERT(std::holds_alternative<token::Type>(m_TokenStack.back()), "Only token::Type is allowed as end-state.");
            std::invoke(
                std::get<token::Type>(m_TokenStack.back()),
                visitor());

            visitor().end();
        }

    private:
        static constexpr lexing::operator_or_punctuator openingParens{"("};
        static constexpr lexing::operator_or_punctuator closingParens{")"};
        static constexpr lexing::operator_or_punctuator openingAngle{"<"};
        static constexpr lexing::operator_or_punctuator closingAngle{">"};
        static constexpr lexing::operator_or_punctuator openingCurly{"{"};
        static constexpr lexing::operator_or_punctuator closingCurly{"}"};
        static constexpr lexing::operator_or_punctuator openingSquare{"["};
        static constexpr lexing::operator_or_punctuator closingSquare{"]"};
        static constexpr lexing::operator_or_punctuator backtick{"`"};
        static constexpr lexing::operator_or_punctuator singleQuote{"'"};
        static constexpr lexing::operator_or_punctuator scopeResolution{"::"};
        static constexpr lexing::operator_or_punctuator commaSeparator{","};
        static constexpr lexing::operator_or_punctuator pointer{"*"};
        static constexpr lexing::operator_or_punctuator lvalueRef{"&"};
        static constexpr lexing::operator_or_punctuator rvalueRef{"&&"};
        static constexpr lexing::keyword operatorKeyword{"operator"};
        static constexpr lexing::keyword constKeyword{"const"};
        static constexpr lexing::keyword volatileKeyword{"volatile"};
        static constexpr lexing::keyword noexceptKeyword{"noexcept"};

        Visitor m_Visitor;
        lexing::NameLexer m_Lexer;

        std::vector<Token> m_TokenStack{};

        [[nodiscard]]
        constexpr auto& visitor() noexcept
        {
            return unwrap_visitor(m_Visitor);
        }

        constexpr void handle_lexer_token([[maybe_unused]] lexing::end const& end)
        {
            util::unreachable();
        }

        constexpr void handle_lexer_token([[maybe_unused]] lexing::space const& space)
        {
        }

        constexpr void handle_lexer_token(lexing::identifier const& identifier)
        {
            m_TokenStack.emplace_back(
                token::Identifier{.content = identifier.content});
        }

        constexpr void handle_lexer_token([[maybe_unused]] lexing::keyword const& keyword)
        {
        }

        constexpr void handle_lexer_token(lexing::operator_or_punctuator const& token)
        {
            if (scopeResolution == token)
            {
                token::try_reduce_as_scope_sequence(m_TokenStack);
            }
        }
    };
}

#endif
