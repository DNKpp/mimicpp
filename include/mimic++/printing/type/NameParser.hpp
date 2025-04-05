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
#include "mimic++/utilities/Overloaded.hpp"

#include <functional>
#include <utility>
#include <variant>

namespace mimicpp::printing::type::parsing
{
    struct scope_resolution
    {
        [[nodiscard]]
        bool operator==(scope_resolution const&) const = default;
    };

    struct arg_seperator
    {
        [[nodiscard]]
        bool operator==(arg_seperator const&) const = default;
    };

    struct identifier
    {
        bool isOperator{false};
        StringViewT content{};

        [[nodiscard]]
        bool operator==(identifier const&) const = default;
    };

    using token = std::variant<
        identifier,
        arg_seperator,
        scope_resolution>;

    template <typename T>
    concept parser_visitor = std::movable<T>
                          && requires(std::unwrap_reference_t<T> visitor, StringViewT content) {
                                 visitor.begin();
                                 visitor.end();

                                 visitor.push_identifier(content);
                                 visitor.push_scope();

                                 visitor.begin_operator_identifier();
                                 visitor.end_operator_identifier();
                             };

    template <parser_visitor Visitor>
    class NameParser
    {
    public:
        [[nodiscard]]
        explicit NameParser(Visitor visitor, StringViewT content) noexcept
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

            visitor().end();
        }

    private:
        Visitor m_Visitor;
        lexing::NameLexer m_Lexer;
        std::stack<token> m_TokenStack{};

        [[nodiscard]]
        constexpr auto& visitor() noexcept
        {
            return static_cast<std::unwrap_reference_t<Visitor>&>(m_Visitor);
        }

        static constexpr void handle_lexer_token([[maybe_unused]] lexing::end const& token) noexcept
        {
            util::unreachable();
        }

        constexpr void handle_lexer_token(lexing::keyword const& token)
        {
            if (constexpr lexing::keyword operatorToken{"operator"};
                operatorToken == token)
            {
                handle_operator_token();
            }
        }

        static constexpr void handle_lexer_token([[maybe_unused]] lexing::space const& token) noexcept
        {
        }

        constexpr void handle_lexer_token([[maybe_unused]] lexing::identifier const& token)
        {
            visitor().push_identifier(token.content);
        }

        constexpr void handle_lexer_token([[maybe_unused]] lexing::operator_or_punctuator const& token)
        {
            if (static constexpr lexing::operator_or_punctuator scopeResolution{"::"};
                scopeResolution == token)
            {
                visitor().push_scope();
            }
        }

        constexpr void handle_operator_token()
        {
            visitor().begin_operator_identifier();

            if (std::holds_alternative<lexing::space>(m_Lexer.peek().classification))
            {
                std::ignore = m_Lexer.next();
            }

            auto const next = m_Lexer.next();
            if (auto const* operatorToken = std::get_if<lexing::operator_or_punctuator>(&next.classification))
            {
                auto const finishMultiOpOperator = [&, this](lexing::operator_or_punctuator const& expectedClosingOp) {
                    auto const [closingContent, classification] = m_Lexer.next();
                    MIMICPP_ASSERT(lexing::token_class{expectedClosingOp} == classification, "Invalid input.");

                    StringViewT const content{
                        next.content.data(),
                        next.content.size() + closingContent.size()};
                    visitor().push_identifier(content);
                };

                if (static constexpr lexing::operator_or_punctuator openingParens{"("};
                    openingParens == *operatorToken)
                {
                    static constexpr lexing::operator_or_punctuator closingOp{")"};
                    finishMultiOpOperator(closingOp);
                }
                else if (static constexpr lexing::operator_or_punctuator openingSquareParens{"["};
                         openingSquareParens == *operatorToken)
                {
                    static constexpr lexing::operator_or_punctuator closingOp{"]"};
                    finishMultiOpOperator(closingOp);
                }
                else
                {
                    visitor().push_identifier(next.content);
                }

                visitor().end_operator_identifier();
            }
        }
    };
}

#endif
