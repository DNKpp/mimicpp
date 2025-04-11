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

            finalize_current_specs();
            visitor().end();
        }

        constexpr void add_identifier(StringViewT const content)
        {
            m_IsPrefixMode = false;
            visitor().add_identifier(content);
        }

        constexpr void add_const()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            auto& current = m_IsPrefixMode
                              ? m_Specs.top().first
                              : m_Specs.top().second;
            current.isConst = true;
        }

        constexpr void add_volatile()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            auto& current = m_IsPrefixMode
                              ? m_Specs.top().first
                              : m_Specs.top().second;
            current.isVolatile = true;
        }

        constexpr void add_noexcept()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");
            MIMICPP_ASSERT(!m_IsPrefixMode, "Invalid state.");

            m_Specs.top().second.isNoexcept = true;
        }

        constexpr void add_ptr()
        {
            finalize_current_specs();
            m_Specs.emplace();
            m_IsPrefixMode = false;

            visitor().add_ptr();
        }

        constexpr void add_lvalue_ref()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");
            MIMICPP_ASSERT(!m_IsPrefixMode, "Invalid state.");

            m_Specs.top().second.isLValueRef = true;
        }

        constexpr void add_rvalue_ref()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");
            MIMICPP_ASSERT(!m_IsPrefixMode, "Invalid state.");

            m_Specs.top().second.isRValueRef = true;
        }

        constexpr void add_scope()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            // A scoped identifier can never be cv prefixed, so any collected prefix must be from higher scope.
            // But we need to handle suffix-specs, so let's just add a temporary spec-layer.
            m_Specs.emplace(
                specs{},
                std::exchange(m_Specs.top().second, {}));
            finalize_current_specs();
            m_IsPrefixMode = false;

            visitor().add_scope();
        }

        constexpr void add_argument()
        {
            finalize_current_specs();
            m_IsPrefixMode = true;
            m_Specs.emplace();

            visitor().add_argument();
        }

        constexpr void begin_template()
        {
            m_IsPrefixMode = true;
            m_Specs.emplace();

            visitor().begin_template();
        }

        constexpr void end_template()
        {
            finalize_current_specs();
            m_IsPrefixMode = false;

            visitor().end_template();
        }

        constexpr void end_return_type()
        {
            finalize_current_specs();
            m_Specs.emplace();

            visitor().end_return_type();
        }

        constexpr void open_parenthesis()
        {
            m_IsPrefixMode = true;
            m_Specs.emplace();

            visitor().open_parenthesis();
        }

        constexpr void end_function()
        {
            finalize_current_specs();
            m_IsPrefixMode = false;

            visitor().end_function();
        }

        constexpr void end_function_ptr()
        {
            finalize_current_specs();
            m_IsPrefixMode = false;

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

            [[nodiscard]]
            static specs merge(specs const& prefix, specs suffix) noexcept
            {
                MIMICPP_ASSERT(!prefix.isNoexcept, "Invalid prefix.");
                MIMICPP_ASSERT(!prefix.isLValueRef, "Invalid prefix.");
                MIMICPP_ASSERT(!prefix.isRValueRef, "Invalid prefix.");

                suffix.isConst = prefix.isConst || suffix.isConst;
                suffix.isVolatile = prefix.isVolatile || suffix.isVolatile;

                return suffix;
            }
        };

        bool m_IsPrefixMode{true};
        std::stack<std::pair<specs, specs>> m_Specs{};

        [[nodiscard]]
        constexpr auto& visitor() noexcept
        {
            return static_cast<std::unwrap_reference_t<Visitor>&>(m_Visitor);
        }

        void finalize_current_specs()
        {
            MIMICPP_ASSERT(!m_Specs.empty(), "Invalid state.");

            auto const specs = specs::merge(m_Specs.top().first, m_Specs.top().second);
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

    enum class token : std::uint8_t
    {
        name,
        scopeResolution,
        argSeparator,
        open
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
                && token::name == m_TokenStack.back())
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

        void reduce_as_name()
        {
            if (!m_TokenStack.empty())
            {
                if (token::scopeResolution == m_TokenStack.back())
                {
                    m_TokenStack.pop_back();

                    if (!m_TokenStack.empty()
                        && token::name == m_TokenStack.back())
                    {
                        m_TokenStack.pop_back();
                    }
                }
                // The only reason, why there may be two consecutive names is, that it's a function with return type.
                else if (token::name == m_TokenStack.back())
                {
                    m_TokenStack.pop_back();
                    visitor().end_return_type();
                }
            }

            m_TokenStack.emplace_back(token::name);
        }

        void handle_lexer_token(lexing::identifier const& token)
        {
            reduce_as_name();
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
                    m_TokenStack.emplace_back(token::name);
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
                m_TokenStack.emplace_back(token::argSeparator);
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
                reduce_as_name();
            }
        }
    };
}

#endif
