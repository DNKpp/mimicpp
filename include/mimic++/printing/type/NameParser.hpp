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
#include "mimic++/printing/type/NameParserReductions.hpp"
#include "mimic++/printing/type/NameParserTokens.hpp"
#include "mimic++/utilities/C++23Backports.hpp"

#include <array>
#include <functional>
#include <iterator>
#include <type_traits>
#include <variant>

namespace mimicpp::printing::type::parsing
{
    template <parser_visitor Visitor>
    class NameParser
    {
    public:
        [[nodiscard]]
        explicit constexpr NameParser(Visitor visitor, StringViewT const& content) noexcept(std::is_nothrow_move_constructible_v<Visitor>)
            : m_Visitor{std::move(visitor)},
              m_Content{content},
              m_Lexer{content}
        {
        }

        constexpr void parse_type()
        {
            parse();
            token::try_reduce_as_type(m_TokenStack);
            if (!finalize<token::Type>())
            {
                emit_unrecognized();
            }
        }

        constexpr void parse_function()
        {
            parse();

            if (m_HasConversionOperator)
            {
                token::reduce_as_conversion_operator_function_identifier(m_TokenStack);
            }
            else
            {
                is_suffix_of<token::FunctionIdentifier>(m_TokenStack)
                    || token::try_reduce_as_function_identifier(m_TokenStack);
            }

            token::try_reduce_as_function(m_TokenStack);
            if (!finalize<token::Function>())
            {
                // Well, this is a workaround to circumvent issues with lambdas on some environments.
                // gcc produces lambdas in form `<lambda()>` which are not recognized as actual functions.
                token::try_reduce_as_type(m_TokenStack);
                if (!finalize<token::Type>())
                {
                    emit_unrecognized();
                }
            }
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
        static constexpr lexing::operator_or_punctuator colon{":"};
        static constexpr lexing::operator_or_punctuator leftShift{"<<"};
        static constexpr lexing::operator_or_punctuator rightShift{">>"};
        static constexpr lexing::keyword operatorKeyword{"operator"};
        static constexpr lexing::keyword constKeyword{"const"};
        static constexpr lexing::keyword volatileKeyword{"volatile"};
        static constexpr lexing::keyword noexceptKeyword{"noexcept"};
        static constexpr lexing::keyword coAwaitKeyword{"co_await"};
        static constexpr lexing::keyword newKeyword{"new"};
        static constexpr lexing::keyword deleteKeyword{"delete"};
        static constexpr lexing::keyword classKeyword{"class"};
        static constexpr lexing::keyword structKeyword{"struct"};
        static constexpr lexing::keyword enumKeyword{"enum"};

        Visitor m_Visitor;
        StringViewT m_Content;
        lexing::NameLexer m_Lexer;
        bool m_HasConversionOperator{false};

        std::vector<Token> m_TokenStack{};

        constexpr void parse()
        {
            for (lexing::token next = m_Lexer.next();
                 !std::holds_alternative<lexing::end>(next.classification);
                 next = m_Lexer.next())
            {
                std::visit(
                    [&](auto const& tokenClass) { handle_lexer_token(next.content, tokenClass); },
                    next.classification);
            }
        }

        template <token_type EndToken>
        constexpr bool finalize()
        {
            auto& unwrapped = unwrap_visitor(m_Visitor);

            if (1u == m_TokenStack.size())
            {
                if (auto const* const end = std::get_if<EndToken>(&m_TokenStack.back()))
                {
                    unwrapped.begin();
                    std::invoke(
                        std::get<EndToken>(m_TokenStack.back()),
                        m_Visitor);
                    unwrapped.end();

                    return true;
                }
            }

            return false;
        }

        constexpr void emit_unrecognized()
        {
            auto& unwrapped = unwrap_visitor(m_Visitor);
            unwrapped.unrecognized(m_Content);
        }

        static constexpr void handle_lexer_token([[maybe_unused]] StringViewT const content, [[maybe_unused]] lexing::end const& end)
        {
            util::unreachable();
        }

        constexpr void handle_lexer_token([[maybe_unused]] StringViewT const content, [[maybe_unused]] lexing::space const& space)
        {
            // In certain cases, a space after an identifier has semantic significance.
            // For example, consider the type names `void ()` and `foo()`:
            // - `void ()` represents a function type returning `void`.
            // - `foo()` represents a function named `foo`.
            if (is_suffix_of<token::Identifier>(m_TokenStack))
            {
                token::try_reduce_as_type(m_TokenStack);
            }
        }

        constexpr void handle_lexer_token([[maybe_unused]] StringViewT const content, lexing::identifier const& identifier)
        {
            m_TokenStack.emplace_back(
                token::Identifier{.content = identifier.content});
        }

        constexpr void handle_lexer_token([[maybe_unused]] StringViewT const content, lexing::keyword const& keyword)
        {
            if (constKeyword == keyword)
            {
                auto& specs = token::get_or_emplace_specs(m_TokenStack);
                MIMICPP_ASSERT(!specs.layers.empty(), "Zero spec layers detected.");
                auto& top = specs.layers.back();
                MIMICPP_ASSERT(!top.isConst, "Specs is already const.");
                top.isConst = true;
            }
            else if (volatileKeyword == keyword)
            {
                auto& specs = token::get_or_emplace_specs(m_TokenStack);
                MIMICPP_ASSERT(!specs.layers.empty(), "Zero spec layers detected.");
                auto& top = specs.layers.back();
                MIMICPP_ASSERT(!top.isConst, "Specs is already volatile.");
                top.isVolatile = true;
            }
            else if (noexceptKeyword == keyword)
            {
                auto& specs = token::get_or_emplace_specs(m_TokenStack);
                MIMICPP_ASSERT(!specs.isNoexcept, "Specs already is a noexcept.");
                specs.isNoexcept = true;
            }
            else if (operatorKeyword == keyword && !process_simple_operator())
            {
                // Conversion operators can not be part of a scope, thus they can not appear multiple times in a single type-name.
                MIMICPP_ASSERT(!m_HasConversionOperator, "Multiple conversion operators detected.");

                m_TokenStack.emplace_back(token::OperatorKeyword{});
                m_HasConversionOperator = true;
            }
            else if (constexpr std::array collection{classKeyword, structKeyword, enumKeyword};
                     util::contains(collection, keyword))
            {
                // This token is needed, so we do not accidentally treat e.g. `(anonymous class)` as function args,
                // because otherwise there would just be the `anonymous` identifier left.
                m_TokenStack.emplace_back(token::TypeContext{.content = content});
            }
        }

        constexpr bool process_simple_operator()
        {
            auto dropSpaceInput = [this] {
                if (std::holds_alternative<lexing::space>(m_Lexer.peek().classification))
                {
                    std::ignore = m_Lexer.next();
                }
            };

            dropSpaceInput();

            // As we assume valid input, we do not have to check for the actual symbol.
            if (auto const next = m_Lexer.peek();
                auto const* operatorToken = std::get_if<lexing::operator_or_punctuator>(&next.classification))
            {
                std::ignore = m_Lexer.next();

                auto const finishMultiOpOperator = [&, this](lexing::operator_or_punctuator const& expectedClosingOp) {
                    auto const [closingContent, classification] = m_Lexer.next();
                    MIMICPP_ASSERT(lexing::token_class{expectedClosingOp} == classification, "Invalid input.");

                    StringViewT const content{
                        next.content.data(),
                        next.content.size() + closingContent.size()};
                    m_TokenStack.emplace_back(
                        token::Identifier{
                            .content = token::Identifier::OperatorInfo{.symbol = content}});
                };

                if (openingParens == *operatorToken)
                {
                    finishMultiOpOperator(closingParens);
                }
                else if (openingSquare == *operatorToken)
                {
                    finishMultiOpOperator(closingSquare);
                }
                // `operator <` and `operator <<` needs to be handled carefully, as it may come in as a template:
                // `operator<<>` is actually `operator< <>`.
                // Note: No tested c++ compiler actually allows `operator<<>`, but some environments still procude this.
                else if (leftShift == *operatorToken)
                {
                    dropSpaceInput();

                    if (auto const* nextOp = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                        nextOp
                        // When next token starts a function or template, we know it's actually `operator <<`.
                        && (openingParens == *nextOp || openingAngle == *nextOp))
                    {
                        m_TokenStack.emplace_back(
                            token::Identifier{
                                .content = token::Identifier::OperatorInfo{.symbol = next.content}});
                    }
                    // looks like an `operator< <>`, so just treat both `<` separately.
                    else
                    {
                        m_TokenStack.emplace_back(
                            token::Identifier{
                                .content = token::Identifier::OperatorInfo{.symbol = next.content.substr(0u, 1u)}});
                        handle_lexer_token(next.content.substr(1u, 1u), openingAngle);
                    }
                }
                else
                {
                    m_TokenStack.emplace_back(
                        token::Identifier{
                            .content = token::Identifier::OperatorInfo{.symbol = next.content}});
                }

                dropSpaceInput();

                return true;
            }
            else if (auto const* keywordToken = std::get_if<lexing::keyword>(&next.classification);
                     keywordToken
                     && util::contains(std::array{newKeyword, deleteKeyword, coAwaitKeyword}, *keywordToken))
            {
                std::ignore = m_Lexer.next();

                StringViewT content = next.content;

                if (newKeyword == *keywordToken || deleteKeyword == *keywordToken)
                {
                    dropSpaceInput();

                    if (auto* opAfter = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                        opAfter
                        && openingSquare == *opAfter)
                    {
                        // Strip `[]` or `[ ]` from the input.
                        std::ignore = m_Lexer.next();
                        dropSpaceInput();
                        auto const closing = m_Lexer.next();
                        MIMICPP_ASSERT(closingSquare == std::get<lexing::operator_or_punctuator>(closing.classification), "Invalid input.");

                        content = StringViewT{
                            next.content.data(),
                            closing.content.data() + closing.content.size()};
                    }
                }

                m_TokenStack.emplace_back(
                    token::Identifier{
                        .content = token::Identifier::OperatorInfo{.symbol = content}});

                dropSpaceInput();

                return true;
            }

            return false;
        }

        constexpr void handle_lexer_token(StringViewT const content, lexing::operator_or_punctuator const& token)
        {
            if (scopeResolution == token)
            {
                token::try_reduce_as_function_identifier(m_TokenStack);

                m_TokenStack.emplace_back(
                    std::in_place_type<token::ScopeResolution>,
                    content);
                token::try_reduce_as_scope_sequence(m_TokenStack);
            }
            else if (commaSeparator == token)
            {
                if (is_suffix_of<token::Type>(m_TokenStack)
                    || token::try_reduce_as_type(m_TokenStack))
                {
                    token::try_reduce_as_arg_sequence(m_TokenStack);
                }

                m_TokenStack.emplace_back(
                    std::in_place_type<token::ArgSeparator>,
                    content);
            }
            else if (lvalueRef == token)
            {
                auto& specs = token::get_or_emplace_specs(m_TokenStack);
                MIMICPP_ASSERT(token::Specs::Refness::none == specs.refness, "Specs already is a reference.");
                specs.refness = token::Specs::Refness::lvalue;
            }
            else if (rvalueRef == token)
            {
                auto& specs = token::get_or_emplace_specs(m_TokenStack);
                MIMICPP_ASSERT(token::Specs::Refness::none == specs.refness, "Specs already is a reference.");
                specs.refness = token::Specs::Refness::rvalue;
            }
            else if (pointer == token)
            {
                auto& specs = token::get_or_emplace_specs(m_TokenStack);
                specs.layers.emplace_back();
            }
            else if (openingAngle == token)
            {
                m_TokenStack.emplace_back(
                    std::in_place_type<token::OpeningAngle>,
                    content);
            }
            else if (closingAngle == token)
            {
                if (is_suffix_of<token::Type>(m_TokenStack)
                    || token::try_reduce_as_type(m_TokenStack))
                {
                    token::try_reduce_as_arg_sequence(m_TokenStack);
                }

                m_TokenStack.emplace_back(
                    std::in_place_type<token::ClosingAngle>,
                    content);
                token::try_reduce_as_template_identifier(m_TokenStack)
                    || token::try_reduce_as_placeholder_identifier_wrapped<token::OpeningAngle, token::ClosingAngle>(m_TokenStack);
            }
            else if (openingParens == token)
            {
                m_TokenStack.emplace_back(
                    std::in_place_type<token::OpeningParens>,
                    content);
            }
            else if (closingParens == token)
            {
                if (is_suffix_of<token::Type>(m_TokenStack)
                    || token::try_reduce_as_type(m_TokenStack))
                {
                    token::try_reduce_as_arg_sequence(m_TokenStack);
                }

                m_TokenStack.emplace_back(
                    std::in_place_type<token::ClosingParens>,
                    content);

                token::try_reduce_as_function_context(m_TokenStack)
                    || token::try_reduce_as_function_ptr(m_TokenStack)
                    || token::try_reduce_as_placeholder_identifier_wrapped<token::OpeningParens, token::ClosingParens>(m_TokenStack);
            }
            else if (openingCurly == token)
            {
                m_TokenStack.emplace_back(
                    std::in_place_type<token::OpeningCurly>,
                    content);
            }
            else if (closingCurly == token)
            {
                m_TokenStack.emplace_back(
                    std::in_place_type<token::ClosingCurly>,
                    content);
                token::try_reduce_as_placeholder_identifier_wrapped<token::OpeningCurly, token::ClosingCurly>(m_TokenStack);
            }
            else if (backtick == token)
            {
                m_TokenStack.emplace_back(
                    std::in_place_type<token::OpeningBacktick>,
                    content);
            }
            else if (singleQuote == token)
            {
                if (token::try_reduce_as_function_identifier(m_TokenStack))
                {
                    unwrap_msvc_like_function();
                }
                // Something like `id1::id2 should become id1::id2, so just remove the leading backtick.
                else if (is_suffix_of<token::OpeningBacktick, token::ScopeSequence, token::Identifier>(m_TokenStack))
                {
                    m_TokenStack.erase(m_TokenStack.cend() - 3u);
                }
                else
                {
                    m_TokenStack.emplace_back(
                        std::in_place_type<token::ClosingSingleQuote>,
                        content);
                    // Well, some environments wrap in `' (like msvc) and some wrap in '' (libc++).
                    token::try_reduce_as_placeholder_identifier_wrapped<token::OpeningBacktick, token::ClosingSingleQuote>(m_TokenStack)
                        || token::try_reduce_as_placeholder_identifier_wrapped<token::ClosingSingleQuote, token::ClosingSingleQuote>(m_TokenStack);
                }
            }
            // The current parsing process will never receive an `<<` or `>>` without a preceding `operator` keyword.
            // As the current `operator` parsing currently consumes the next op-symbol, we will never reach this point
            // with an actual left or right-shift. So, to make that easier, just split them.
            else if (leftShift == token)
            {
                handle_lexer_token(content.substr(0, 1u), openingAngle);
                handle_lexer_token(content.substr(1u, 1u), openingAngle);
            }
            else if (rightShift == token)
            {
                handle_lexer_token(content.substr(0, 1u), closingAngle);
                handle_lexer_token(content.substr(1u, 1u), closingAngle);
            }
        }

        void unwrap_msvc_like_function()
        {
            MIMICPP_ASSERT(is_suffix_of<token::FunctionIdentifier>(m_TokenStack), "Invalid state.");

            auto funIdentifier = std::get<token::FunctionIdentifier>(m_TokenStack.back());
            m_TokenStack.pop_back();

            token::ScopeSequence scopes{};
            if (auto* scopeSeq = match_suffix<token::ScopeSequence>(m_TokenStack))
            {
                scopes = std::move(*scopeSeq);
                m_TokenStack.pop_back();
            }

            // Ignore return-types.
            if (is_suffix_of<token::Type>(m_TokenStack)
                || token::try_reduce_as_type(m_TokenStack))
            {
                m_TokenStack.pop_back();
            }

            MIMICPP_ASSERT(match_suffix<token::OpeningBacktick>(m_TokenStack), "Invalid state.");
            m_TokenStack.pop_back();

            if (auto* targetScopes = match_suffix<token::ScopeSequence>(m_TokenStack))
            {
                auto& target = targetScopes->scopes;
                target.insert(
                    target.cend(),
                    std::make_move_iterator(scopes.scopes.begin()),
                    std::make_move_iterator(scopes.scopes.end()));
            }
            else
            {
                m_TokenStack.emplace_back(std::move(scopes));
            }

            m_TokenStack.emplace_back(std::move(funIdentifier));
        }
    };
}

#endif
