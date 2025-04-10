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
#include "mimic++/utilities/Algorithm.hpp"

#include <functional>
#include <utility>
#include <variant>

namespace mimicpp::printing::type::parsing
{
    enum class token : std::uint8_t
    {
        scope,
        scopeResolution,
        prefixSpec,
        arg,
        open
    };

    template <typename T>
    concept parser_visitor = std::movable<T>
                          && requires(std::unwrap_reference_t<T> visitor, StringViewT content) {
                                 visitor.begin();
                                 visitor.end();

                                 visitor.begin_template();
                                 visitor.end_template();

                                 visitor.open_parenthesis();
                                 visitor.end_function();

                                 visitor.push_identifier(content);
                                 visitor.push_scope();
                                 visitor.push_argument();
                                 visitor.push_spec(content);

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

            consume_prefix_spec_if_can();
            visitor().end();
        }

    private:
        Visitor m_Visitor;
        lexing::NameLexer m_Lexer;
        std::vector<lexing::keyword> m_PrefixSpecs{};
        std::deque<token> m_TokenStack{};

        [[nodiscard]]
        constexpr auto& visitor() noexcept
        {
            return static_cast<std::unwrap_reference_t<Visitor>&>(m_Visitor);
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
                    consume_prefix_spec_if_can();
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
                    consume_prefix_spec_if_can();
                    m_TokenStack.pop_back();
                    visitor().end_return_type();
                }
            }

            m_TokenStack.emplace_back(token::scope);
        }

        void handle_lexer_token(lexing::identifier const& token)
        {
            reduce_as_scope();
            visitor().push_identifier(token.content);
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

        [[nodiscard]]
        constexpr bool is_prefix_spec() const noexcept
        {
            return m_TokenStack.empty()
                || token::scope != m_TokenStack.back();
        }

        void push_prefix_spec(lexing::keyword const& spec)
        {
            m_TokenStack.emplace_back(token::prefixSpec);
            m_PrefixSpecs.emplace_back(spec);
        }

        void consume_prefix_spec_if_can()
        {
            if (m_TokenStack.size() < 2u
                || token::scope != m_TokenStack.back())
            {
                return;
            }

            m_TokenStack.pop_back();

            // Find all prefix specs and apply in correct order.
            if (auto const iter = std::ranges::find_if_not(
                    m_TokenStack | std::views::reverse,
                    std::bind_front(std::equal_to{}, token::prefixSpec));
                iter != m_TokenStack.crbegin())
            {
                auto const count = std::ranges::distance(m_TokenStack.crbegin(), iter);
                MIMICPP_ASSERT(0 <= count && std::cmp_less_equal(count, m_PrefixSpecs.size()), "Out of sync.");

                for (auto const& spec : m_PrefixSpecs
                                            | std::views::reverse
                                            | std::views::take(count)
                                            | std::views::reverse)
                {
                    visitor().push_spec(spec.text());
                }
                m_PrefixSpecs.erase(m_PrefixSpecs.cend() - count, m_PrefixSpecs.cend());
                m_TokenStack.erase(iter.base(), m_TokenStack.cend());
            }

            m_TokenStack.emplace_back(token::scope);
        }

        constexpr void reduce_as_arg()
        {
            consume_prefix_spec_if_can();

            MIMICPP_ASSERT(!m_TokenStack.empty(), "Stack already depleted.");
            MIMICPP_ASSERT(token::scope == m_TokenStack.back(), "Unexpected token.");

            m_TokenStack.pop_back();
            m_TokenStack.emplace_back(token::arg);
        }

        constexpr void handle_lexer_token(lexing::operator_or_punctuator const& token)
        {
            constexpr lexing::operator_or_punctuator pointer{"*"};
            constexpr lexing::operator_or_punctuator lvalueRef{"&"};
            constexpr lexing::operator_or_punctuator rvalueRef{"&&"};

            if (constexpr lexing::operator_or_punctuator scopeResolution{"::"};
                scopeResolution == token)
            {
                visitor().push_scope();
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
                consume_prefix_spec_if_can();
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
                consume_prefix_spec_if_can();
                pop_until_open_token();
                visitor().end_function();
            }
            else if (constexpr lexing::operator_or_punctuator commaSeparator{","};
                     commaSeparator == token)
            {
                reduce_as_arg();
                visitor().push_argument();
            }
            else if (std::ranges::contains(std::array{pointer, lvalueRef, rvalueRef}, token))
            {
                consume_prefix_spec_if_can();
                visitor().push_spec(token.text());
            }
        }

        constexpr void handle_lexer_token(lexing::keyword const& token)
        {
            constexpr lexing::keyword constKeyword{"const"};
            constexpr lexing::keyword volatileKeyword{"volatile"};

            if (constexpr lexing::keyword operatorKeyword{"operator"};
                operatorKeyword == token)
            {
                handle_operator_token();
            }
            else if (constexpr lexing::keyword noexceptKeyword{"noexcept"};
                     noexceptKeyword == token)
            {
                visitor().push_spec("noexcept");
            }
            else if (constKeyword == token || volatileKeyword == token)
            {
                if (is_prefix_spec())
                {
                    push_prefix_spec(token);
                }
                else
                {
                    consume_prefix_spec_if_can();
                    visitor().push_spec(token.text());
                }
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
                    visitor().push_identifier(next.content);
                }

                visitor().end_operator_identifier();
                reduce_as_scope();
            }
        }
    };
}

#endif
