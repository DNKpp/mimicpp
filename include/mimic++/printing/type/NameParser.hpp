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
#include "mimic++/utilities/C++23Backports.hpp"

#include <concepts>
#include <deque>
#include <functional>
#include <stack>
#include <type_traits>
#include <utility>
#include <variant>
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

            // A scope identifier can never be cv prefixed, so any collected prefix must be from higher layer.
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

    class LexerTokenLinearizer
    {
    public:
        [[nodiscard]]
        explicit constexpr LexerTokenLinearizer(StringViewT text) noexcept
            : m_Lexer{std::move(text)}
        {
            m_Next = find_next();
        }

        [[nodiscard]]
        lexing::token next()
        {
            return std::exchange(m_Next, find_next());
        }

        [[nodiscard]]
        constexpr lexing::token const& peek() const noexcept
        {
            return m_Next;
        }

    private:
        static constexpr lexing::operator_or_punctuator backtick{"`"};
        static constexpr lexing::operator_or_punctuator singleQuote{"'"};
        static constexpr lexing::operator_or_punctuator scopeResolution{"::"};

        lexing::NameLexer m_Lexer;
        lexing::token m_Next;

        std::stack<std::deque<lexing::token>> m_BufferedTokens{};

        [[nodiscard]]
        lexing::token find_next()
        {
            if (!m_BufferedTokens.empty())
            {
                auto& layer = m_BufferedTokens.top();
                MIMICPP_ASSERT(!layer.empty(), "Empty buffer layer.");
                auto next = std::move(layer.front());
                if (1u == layer.size())
                {
                    m_BufferedTokens.pop();

                    // When there is still a layer present, we need to search for the next single-quote token
                    // and thus finalize that layer (or start another nested one).
                    if (!m_BufferedTokens.empty())
                    {
                        buffer_tokens();
                    }
                }
                else
                {
                    layer.pop_front();
                }

                return next;
            }

            auto next = m_Lexer.next();

            // If a backtick is detected, we need to push that token into the buffer first, so `buffer_tokens` runs correctly.
            if (auto const* nextClass = std::get_if<lexing::operator_or_punctuator>(&next.classification);
                nextClass
                && backtick == *nextClass)
            {
                m_BufferedTokens.emplace().emplace_back(std::move(next));
                buffer_tokens();

                return find_next();
            }

            return next;
        }

        void buffer_tokens()
        {
            MIMICPP_ASSERT(!m_BufferedTokens.empty(), "Empty token-buffer.");

            while (std::visit(
                [&](auto const& next) { return shall_continue(next); },
                m_Lexer.peek().classification))
            {
                m_BufferedTokens.top().emplace_back(m_Lexer.next());
            }

            m_BufferedTokens.top().emplace_back(m_Lexer.next());

            // We need to make a decision here. The newly gathered layer denotes either a function (with or without return type)
            // or a placeholder like. A placeholder can only be a simple combination of identifier and space.
            auto& layer = m_BufferedTokens.top();
            bool const isPlaceholder = std::ranges::all_of(
                layer.cbegin() + 1,
                layer.cend() - 1,
                [](lexing::token_class const& classification) noexcept {
                    return std::holds_alternative<lexing::space>(classification)
                        || std::holds_alternative<lexing::identifier>(classification);
                },
                &lexing::token::classification);

            // If it's not a placeholder just omit the opening and closing token, so the parser does not get confused.
            if (!isPlaceholder)
            {
                layer.pop_front();
                layer.pop_back();
            }

            // The trailing `::` must be the suffix of the current scope, so that the linearizing process is correct.
            if (auto const* nextOp = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                nextOp
                && scopeResolution == *nextOp)
            {
                layer.emplace_back(m_Lexer.next());
            }
        }

        [[nodiscard]]
        static constexpr bool shall_continue([[maybe_unused]] lexing::end const& token) noexcept
        {
            return false;
        }

        [[nodiscard]]
        bool shall_continue(lexing::operator_or_punctuator const& token)
        {
            if (singleQuote == token)
            {
                return false;
            }

            if (backtick == token)
            {
                m_BufferedTokens.emplace();
            }

            return true;
        }

        [[nodiscard]]
        static constexpr bool shall_continue([[maybe_unused]] auto const& token) noexcept
        {
            return true;
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

        SpecNormalizerVisitor<Visitor> m_Visitor;
        LexerTokenLinearizer m_Lexer;
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
                if (auto const* nextToken = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                    nextToken
                    && openingParens == *nextToken)
                {
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

        [[nodiscard]]
        bool keep_reserved_identifier() const noexcept
        {
            if (!m_TokenStack.empty()
                && token::scopeResolution != m_TokenStack.back())
            {
                return false;
            }

            auto const& next = m_Lexer.peek().classification;
            if (std::holds_alternative<lexing::end>(next))
            {
                return true;
            }

            if (auto const* op = std::get_if<lexing::operator_or_punctuator>(&next))
            {
                return openingAngle == *op
                    || openingParens == *op
                    || scopeResolution == *op;
            }

            return false;
        }

        void handle_lexer_token(lexing::identifier const& token)
        {
            // Some environments add many reserved symbols (e.g. `__cdecl`). We want to filter out most of these,
            // but keep those, which are actual function-names.
            if (!token.content.starts_with("__")
                || keep_reserved_identifier())
            {
                reduce_as_name();
                visitor().add_identifier(token.content);
            }
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

        void handle_placeholder(lexing::operator_or_punctuator const& token, lexing::operator_or_punctuator const& expectedEnd)
        {
            StringViewT content{
                m_Lexer.peek().content.data() - token.text().size(),
                token.text().size()};

            // It should never ever reach the end token, but leave it here to prevent an infinite loop in production.
            for (lexing::token next = m_Lexer.next();
                 !std::holds_alternative<lexing::end>(next.classification);
                 next = m_Lexer.next())
            {
                content = StringViewT{content.data(), next.content.data() + next.content.size()};

                if (auto const* op = std::get_if<lexing::operator_or_punctuator>(&next.classification);
                    op
                    && expectedEnd == *op)
                {
                    break;
                }
            }

            reduce_as_name();
            visitor().add_identifier(content);
        }

        constexpr void handle_lexer_token(lexing::operator_or_punctuator const& token)
        {
            if (scopeResolution == token)
            {
                visitor().add_scope();
                m_TokenStack.emplace_back(token::scopeResolution);
            }
            else if (openingAngle == token)
            {
                // To be a valid template, the `<` token must be prefixed with an actual name.
                if (!m_TokenStack.empty()
                    && token::name == m_TokenStack.back())
                {
                    m_TokenStack.emplace_back(token::open);
                    visitor().begin_template();
                }
                else
                {
                    handle_placeholder(token, closingAngle);
                }
            }
            else if (closingAngle == token)
            {
                pop_until_open_token();
                visitor().end_template();
            }
            else if (openingParens == token)
            {
                // To be a valid function, the `(` token must be prefixed with an actual name or a return type
                // (both are marked as `name` on the token-stack).
                // Function pointers are always prefixed with the return type.
                if (!m_TokenStack.empty()
                    && token::name == m_TokenStack.back())
                {
                    m_TokenStack.emplace_back(token::open);
                    visitor().open_parenthesis();
                }
                else
                {
                    handle_placeholder(token, closingParens);
                }
            }
            else if (closingParens == token)
            {
                // `)(` looks like a function pointer.
                if (auto const* nextToken = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                    nextToken
                    && openingParens == *nextToken)
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
            else if (openingCurly == token)
            {
                handle_placeholder(token, closingCurly);
            }
            else if (backtick == token)
            {
                handle_placeholder(token, singleQuote);
            }
            else if (commaSeparator == token)
            {
                m_TokenStack.emplace_back(token::argSeparator);
                visitor().add_argument();
            }

            else if (pointer == token)
            {
                visitor().add_ptr();
            }
            else if (lvalueRef == token)
            {
                visitor().add_lvalue_ref();
            }
            else if (rvalueRef == token)
            {
                visitor().add_rvalue_ref();
            }
        }

        constexpr void handle_lexer_token(lexing::keyword const& token)
        {
            if (operatorKeyword == token)
            {
                handle_operator_token();
            }
            else if (constKeyword == token)
            {
                visitor().add_const();
            }
            else if (volatileKeyword == token)
            {
                visitor().add_volatile();
            }
            else if (noexceptKeyword == token)
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

                if (openingParens == *operatorToken)
                {
                    finishMultiOpOperator(closingParens);
                }
                else if (openingSquare == *operatorToken)
                {
                    finishMultiOpOperator(closingSquare);
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
