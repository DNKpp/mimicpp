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

#include <functional>
#include <utility>
#include <variant>

namespace mimicpp::printing::type::parsing
{
    enum class token : std::uint8_t
    {
        scope,
        scopeResolution,
        arg,
        open
    };

    template <typename T>
    concept parser_visitor = std::movable<T>
                          && requires(std::unwrap_reference_t<T> visitor, StringViewT content) {
                                 visitor.begin();
                                 visitor.end();

                                 visitor.add_identifier(content);
                                 visitor.add_scope();
                                 visitor.add_argument();

                                 visitor.add_const();
                                 visitor.add_volatile();
                                 visitor.add_noexcept();
                                 visitor.add_ptr();
                                 visitor.add_lvalue_ref();
                                 visitor.add_rvalue_ref();

                                 visitor.begin_template();
                                 visitor.end_template();

                                 visitor.end_return_type();
                                 visitor.open_parenthesis();
                                 visitor.end_function();
                                 visitor.end_function_ptr();

                                 visitor.begin_operator_identifier();
                                 visitor.end_operator_identifier();
                             };

    template <parser_visitor Visitor>
    class SpecNormalizerVisitor
    {
    public:
        [[nodiscard]]
        explicit SpecNormalizerVisitor(Visitor visitor) noexcept(std::is_nothrow_move_constructible_v<Visitor>)
            : m_Visitor{std::move(visitor)}
        {
        }

        constexpr void begin()
        {
            m_Specs.emplace();
            visitor().begin();
        }

        constexpr void end()
        {
            MIMICPP_ASSERT(1u == m_Specs.size(), "Out of sync.");

            finalized_current_specs();
            visitor().end();
        }

        constexpr void add_identifier(StringViewT const content)
        {
            visitor().add_identifier(content);
        }

        constexpr void add_const()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            m_Specs.top().isConst = true;
        }

        constexpr void add_volatile()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            m_Specs.top().isVolatile = true;
        }

        constexpr void add_noexcept()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            m_Specs.top().isNoexcept = true;
        }

        constexpr void add_ptr()
        {
            finalized_current_specs();
            m_Specs.emplace();
            visitor().add_ptr();
        }

        constexpr void add_lvalue_ref()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            m_Specs.top().isLValueRef = true;
        }

        constexpr void add_rvalue_ref()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            m_Specs.top().isRValueRef = true;
        }

        constexpr void add_scope()
        {
            visitor().add_scope();
        }

        constexpr void add_argument()
        {
            finalized_current_specs();
            m_Specs.emplace();
            visitor().add_argument();
        }

        constexpr void begin_template()
        {
            m_Specs.emplace();
            visitor().begin_template();
        }

        constexpr void end_template()
        {
            finalized_current_specs();
            visitor().end_template();
        }

        constexpr void end_return_type()
        {
            finalized_current_specs();
            m_Specs.emplace();
            visitor().end_return_type();
        }

        constexpr void open_parenthesis()
        {
            m_Specs.emplace();
            visitor().open_parenthesis();
        }

        constexpr void end_function()
        {
            finalized_current_specs();
            visitor().end_function();
        }

        constexpr void end_function_ptr()
        {
            finalized_current_specs();
            visitor().end_function_ptr();
        }

        constexpr void begin_operator_identifier()
        {
            visitor().begin_operator_identifier();
        }

        constexpr void end_operator_identifier()
        {
            visitor().end_operator_identifier();
        }

    private:
        Visitor m_Visitor;

        struct specs
        {
            bool isConst{false};
            bool isVolatile{false};
            bool isNoexcept{false};
            bool isLValueRef{false};
            bool isRValueRef{false};
        };

        std::stack<specs> m_Specs{};

        [[nodiscard]]
        constexpr auto& visitor() noexcept
        {
            return static_cast<std::unwrap_reference_t<Visitor>&>(m_Visitor);
        }

        void finalized_current_specs()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            auto const specs = m_Specs.top();
            m_Specs.pop();

            MIMICPP_ASSERT(!(specs.isLValueRef && specs.isRValueRef), "Both reference types detected.");

            if (specs.isConst)
            {
                visitor().add_const();
            }

            if (specs.isVolatile)
            {
                visitor().add_volatile();
            }

            if (specs.isLValueRef)
            {
                visitor().add_lvalue_ref();
            }
            else if (specs.isRValueRef)
            {
                visitor().add_rvalue_ref();
            }

            if (specs.isNoexcept)
            {
                visitor().add_noexcept();
            }
        }
    };

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

            visitor().end();
        }

    private:
        SpecNormalizerVisitor<Visitor> m_Visitor;
        lexing::NameLexer m_Lexer;
        std::deque<token> m_TokenStack{};

        [[nodiscard]]
        constexpr auto& visitor() noexcept
        {
            return m_Visitor;
        }

        static constexpr void handle_lexer_token([[maybe_unused]] lexing::end const& token) noexcept
        {
            util::unreachable();
        }

        constexpr void handle_lexer_token([[maybe_unused]] lexing::space const& token) noexcept
        {
            // Function types are given in form `ret ()` (note the whitespace).
            // That's the only assumption we have when distinguishing from `foo()`.
            if (!m_TokenStack.empty()
                && token::scope == m_TokenStack.back())
            {
                constexpr lexing::operator_or_punctuator openParensToken{"("};
                if (auto const* nextToken = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                    nextToken
                    && openParensToken == *nextToken)
                {
                    m_TokenStack.pop_back();
                    visitor().end_return_type();
                }
            }
        }

        void reduce_as_scope()
        {
            if (!m_TokenStack.empty())
            {
                if (token::scopeResolution == m_TokenStack.back())
                {
                    m_TokenStack.pop_back();

                    if (!m_TokenStack.empty()
                        && token::scope == m_TokenStack.back())
                    {
                        m_TokenStack.pop_back();
                    }
                }
                // The only reason, why there may be two consecutive scopes is, that it's a function with return type.
                else if (token::scope == m_TokenStack.back())
                {
                    m_TokenStack.pop_back();
                    visitor().end_return_type();
                }
            }

            m_TokenStack.emplace_back(token::scope);
        }

        void handle_lexer_token(lexing::identifier const& token)
        {
            reduce_as_scope();
            visitor().add_identifier(token.content);
        }

        constexpr void pop_until_open_token()
        {
            bool finished = false;
            while (!finished)
            {
                MIMICPP_ASSERT(!m_TokenStack.empty(), "Stack already depleted.");

                finished = token::open == m_TokenStack.back();
                m_TokenStack.pop_back();
            }
        }

        constexpr void reduce_as_arg()
        {
            MIMICPP_ASSERT(!m_TokenStack.empty(), "Stack already depleted.");
            MIMICPP_ASSERT(token::scope == m_TokenStack.back(), "Unexpected token.");

            m_TokenStack.pop_back();
            m_TokenStack.emplace_back(token::arg);
        }

        constexpr void handle_lexer_token(lexing::operator_or_punctuator const& token)
        {
            if (constexpr lexing::operator_or_punctuator scopeResolution{"::"};
                scopeResolution == token)
            {
                visitor().add_scope();
                m_TokenStack.emplace_back(token::scopeResolution);
            }
            else if (constexpr lexing::operator_or_punctuator templateBegin{"<"};
                     templateBegin == token)
            {
                m_TokenStack.emplace_back(token::open);
                visitor().begin_template();
            }
            else if (constexpr lexing::operator_or_punctuator templateEnd{">"};
                     templateEnd == token)
            {
                pop_until_open_token();
                visitor().end_template();
            }
            else if (constexpr lexing::operator_or_punctuator functionBegin{"("};
                     functionBegin == token)
            {
                m_TokenStack.emplace_back(token::open);
                visitor().open_parenthesis();
            }
            else if (constexpr lexing::operator_or_punctuator functionEnd{")"};
                     functionEnd == token)
            {
                // `)(` looks like a function pointer.
                if (auto const* nextToken = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                    nextToken
                    && functionBegin == *nextToken)
                {
                    pop_until_open_token();
                    m_TokenStack.emplace_back(token::scope);
                    visitor().end_function_ptr();
                }
                else
                {
                    pop_until_open_token();
                    visitor().end_function();
                }
            }
            else if (constexpr lexing::operator_or_punctuator commaSeparator{","};
                     commaSeparator == token)
            {
                reduce_as_arg();
                visitor().add_argument();
            }
            else if (constexpr lexing::operator_or_punctuator pointer{"*"};
                     pointer == token)
            {
                visitor().add_ptr();
            }
            else if (constexpr lexing::operator_or_punctuator lvalueRef{"&"};
                     lvalueRef == token)
            {
                visitor().add_lvalue_ref();
            }
            else if (constexpr lexing::operator_or_punctuator rvalueRef{"&&"};
                     rvalueRef == token)
            {
                visitor().add_rvalue_ref();
            }
        }

        constexpr void handle_lexer_token(lexing::keyword const& token)
        {
            if (constexpr lexing::keyword operatorKeyword{"operator"};
                operatorKeyword == token)
            {
                handle_operator_token();
            }
            else if (constexpr lexing::keyword constKeyword{"const"};
                     constKeyword == token)
            {
                visitor().add_const();
            }
            else if (constexpr lexing::keyword volatileKeyword{"volatile"};
                     volatileKeyword == token)
            {
                visitor().add_volatile();
            }
            else if (constexpr lexing::keyword noexceptKeyword{"noexcept"};
                     noexceptKeyword == token)
            {
                visitor().add_noexcept();
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
                    visitor().add_identifier(content);
                };

                if (constexpr lexing::operator_or_punctuator openingParens{"("};
                    openingParens == *operatorToken)
                {
                    constexpr lexing::operator_or_punctuator closingOp{")"};
                    finishMultiOpOperator(closingOp);
                }
                else if (constexpr lexing::operator_or_punctuator openingSquareParens{"["};
                         openingSquareParens == *operatorToken)
                {
                    constexpr lexing::operator_or_punctuator closingOp{"]"};
                    finishMultiOpOperator(closingOp);
                }
                else
                {
                    visitor().add_identifier(next.content);
                }

                visitor().end_operator_identifier();
                reduce_as_scope();
            }
        }
    };
}

#endif
