//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PARSER_HPP
#define MIMICPP_PRINTING_TYPE_PARSER_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/printing/type/Lexer.hpp"
#include "mimic++/printing/type/ParserState.hpp"
#include "mimic++/utilities/Algorithm.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/PassKey.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <optional>
    #include <span>
    #include <utility>
    #include <variant>
#endif

namespace mimicpp::printing::type::parsing::v2
{
    class Transaction;

    class TokenStream
    {
    public:
        [[nodiscard]]
        explicit MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES TokenStream(lexing::NameLexer& lexer) noexcept
        {
            while (!std::holds_alternative<lexing::end>(lexer.peek().classification))
            {
                if (auto token = lexer.next();
                    !std::holds_alternative<lexing::space>(token.classification))
                {
                    m_Tokens.emplace_back(std::move(token));
                }
            }

            m_Tokens.emplace_back(lexer.next());
        }

        [[nodiscard]]
        constexpr bool is_eof() const noexcept
        {
            return m_Index == std::ranges::size(m_Tokens) - 1u;
        }

        [[nodiscard]]
        constexpr lexing::token const& peek() const
        {
            MIMICPP_ASSERT(m_Index < std::ranges::size(m_Tokens), "Stream is at end.");
            return std::span{m_Tokens}[m_Index];
        }

        constexpr void consume()
        {
            MIMICPP_ASSERT(!is_eof(), "EOF cannot be consumed.");
            ++m_Index;
        }

        [[nodiscard]]
        constexpr std::size_t pos() const noexcept
        {
            return m_Index;
        }

        constexpr void seek(util::pass_key<Transaction> const /*key*/, std::size_t const pos)
        {
            m_Index = pos;
        }

    private:
        std::vector<lexing::token> m_Tokens;
        std::size_t m_Index{};
    };

    class Transaction
    {
    public:
        Transaction(Transaction const&) = delete;
        Transaction& operator=(Transaction const&) = delete;
        Transaction(Transaction&&) = delete;
        Transaction& operator=(Transaction&&) = delete;

        constexpr ~Transaction() noexcept
        {
            if (is_active())
            {
                m_Stream->seek(util::pass_key<Transaction>{}, m_Checkpoint);
            }
        }

        [[nodiscard]]
        explicit constexpr Transaction(TokenStream& stream) noexcept
            : m_Stream{std::addressof(stream)},
              m_Checkpoint{stream.pos()}
        {
        }

        constexpr void commit() noexcept
        {
            m_Stream = nullptr;
        }

        [[nodiscard]]
        constexpr bool is_active() const noexcept
        {
            return m_Stream != nullptr;
        }

    private:
        TokenStream* m_Stream;
        std::size_t m_Checkpoint;
    };

    template <typename State>
    class StateGuard
    {
    public:
        StateGuard(StateGuard const&) = delete;
        StateGuard& operator=(StateGuard const&) = delete;
        StateGuard(StateGuard&&) = delete;
        StateGuard& operator=(StateGuard&&) = delete;

        ~StateGuard() = default;

        template <typename... Args>
            requires std::constructible_from<State, Args...>
        [[nodiscard]]
        explicit constexpr StateGuard(TokenStream& stream, Args&&... args)
            : m_Transaction{stream},
              m_State{std::in_place, std::forward<Args>(args)...}
        {
        }

        [[nodiscard]]
        constexpr State& operator*() noexcept
        {
            MIMICPP_ASSERT(m_State, "State was already consumed.");
            return *m_State;
        }

        [[nodiscard]]
        constexpr State* operator->() noexcept
        {
            MIMICPP_ASSERT(m_State, "State was already consumed.");
            return &*m_State;
        }

        [[nodiscard]]
        constexpr std::optional<State> take() &&
        {
            MIMICPP_ASSERT(m_State, "State was already consumed.");
            m_Transaction.commit();
            return std::exchange(m_State, std::nullopt);
        }

    private:
        Transaction m_Transaction;
        std::optional<State> m_State;
    };

    template <std::equality_comparable Key, typename Value, std::size_t length>
    [[nodiscard]]
    constexpr auto make_map(std::pair<Key, Value> const (&mappings)[length])
    {
        class Map
        {
        public:
            [[nodiscard]]
            explicit constexpr Map(std::pair<Key, Value> const (&candidates)[length])
                : m_Candidates{std::to_array(candidates)}
            {
            }

            [[nodiscard]]
            constexpr std::optional<Value> operator()(Key const& key) const
            {
                if (auto const iter = std::ranges::find(m_Candidates, key, &std::pair<Key, Value>::first);
                iter != std::ranges::end(m_Candidates))
                {
                    return iter->second;
                }

                return std::nullopt;
            }

        private:
            std::array<std::pair<Key, Value>, length> m_Candidates;
        };

        return Map{mappings};
    }

    template <typename LexerTokenClass>
    [[nodiscard]]
    constexpr LexerTokenClass const* peek_if(TokenStream const& stream) noexcept
    {
        return std::get_if<LexerTokenClass>(&stream.peek().classification);
    }

    template <std::equality_comparable LexerTokenClass>
    [[nodiscard]]
    constexpr std::optional<LexerTokenClass> expect(TokenStream& stream, LexerTokenClass const& expected)
    {
        if (auto const* const token = peek_if<LexerTokenClass>(stream);
            token
            && *token == expected)
        {
            std::optional result = *token;
            stream.consume();

            return result;
        }

        return std::nullopt;
    }

    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::FunctionDeclarator> parse_parameters_and_qualifiers(TokenStream& stream);

    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::TypeId> parse_type_specifier_seq(TokenStream& stream);

    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::PtrOperator> parse_ptr_operator(TokenStream& stream);

    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::TypeId> parse_type_id(TokenStream& stream);

    // see: https://eel.is/c++draft/class.pre#nt:class-key
    [[nodiscard]]
    constexpr std::optional<state::ClassKey> parse_class_key(TokenStream& stream)
    {
        if (auto const* const keyword = peek_if<lexing::keyword>(stream))
        {
            constexpr auto map = make_map<lexing::keyword, state::ClassKey>({
                {lexing::keyword{"class"},  state::ClassKey::id_class },
                {lexing::keyword{"struct"}, state::ClassKey::id_struct}
                // Todo: {lexing::keyword{"union"}, tokens::class_key::id_union}
            });

            if (std::optional const classKey = map(*keyword))
            {
                stream.consume();
                return classKey;
            }
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/dcl.enum#nt:enum-key
    [[nodiscard]]
    constexpr std::optional<state::EnumKey> parse_enum_key(TokenStream& stream)
    {
        if (expect(stream, lexing::keyword{"enum"}))
        {
            if (expect(stream, lexing::keyword{"class"}))
            {
                return state::EnumKey::id_enum_class;
            }

            if (expect(stream, lexing::keyword{"struct"}))
            {
                return state::EnumKey::id_enum_struct;
            }

            return state::EnumKey::id_enum;
        }

        return std::nullopt;
    }

    [[nodiscard]]
    constexpr std::optional<state::TypeKey> parse_type_key(TokenStream& stream)
    {
        if (auto const key = parse_class_key(stream))
        {
            return {*key};
        }

        if (auto const key = parse_enum_key(stream))
        {
            return {*key};
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/dcl.decl.general#nt:ref-qualifier
    [[nodiscard]]
    constexpr std::optional<state::RefQualifier> parse_ref_qualifier(TokenStream& stream)
    {
        if (auto const* const op = peek_if<lexing::operator_or_punctuator>(stream))
        {
            constexpr auto map = make_map<lexing::operator_or_punctuator, state::RefQualifier>({
                {lexing::operator_or_punctuator{"&"},  state::RefQualifier::id_ref   },
                {lexing::operator_or_punctuator{"&&"}, state::RefQualifier::id_refref}
            });

            if (std::optional const qualifier = map(*op))
            {
                stream.consume();
                return qualifier;
            }
        }

        return std::nullopt;
    }

    // Constant-expressions are actually a very deeply nested set of rules,
    // which more or less boils down to a `primary-expression` for this kind of task.
    // see: https://eel.is/c++draft/expr.const.general#nt:constant-expression
    // see: https://eel.is/c++draft/expr.prim.grammar#nt:primary-expression
    // > constant-expression ::= primary-expression
    // > primary-expression ::= literal
    [[nodiscard]]
    constexpr std::optional<state::ConstantExpression> parse_constant_expression(TokenStream& stream)
    {
        if (auto const* const literal = peek_if<lexing::literal>(stream))
        {
            stream.consume();
            return {*literal};
        }

        return std::nullopt;
    }

    namespace detail
    {
        struct PlaceholderWrapCandidate
        {
            lexing::operator_or_punctuator open;
            lexing::operator_or_punctuator close;
        };

        [[nodiscard]]
        consteval auto make_placeholder_wrap_candidates() noexcept
        {
            using op = lexing::operator_or_punctuator;
            std::array raw = {
                PlaceholderWrapCandidate{.open = op{"{"}, .close = op{"}"}},
                PlaceholderWrapCandidate{.open = op{"<"}, .close = op{">"}},
                PlaceholderWrapCandidate{.open = op{"("}, .close = op{")"}},
                PlaceholderWrapCandidate{.open = op{"`"}, .close = op{"'"}},
            };

            constexpr auto projection = [](auto const& e) { return e.open.index(); };
            std::ranges::sort(raw, {}, projection);
            MIMICPP_ASSERT(raw.cend() == std::ranges::unique(raw, {}, projection).begin(), "Fix your input!");

            return raw;
        }

        inline constexpr std::array placeholderWrapCandidates = make_placeholder_wrap_candidates();

        [[nodiscard]]
        constexpr char const* find_end_token(TokenStream& stream, lexing::operator_or_punctuator const open, lexing::operator_or_punctuator const close)
        {
            while (!stream.is_eof())
            {
                if (auto const* const cur = std::get_if<lexing::operator_or_punctuator>(&stream.peek().classification))
                {
                    if (open == *cur)
                    {
                        stream.consume();
                        std::ignore = find_end_token(stream, open, close);
                        continue;
                    }
                    else if (close == *cur)
                    {
                        auto const* const end = stream.peek().content.data() + stream.peek().content.size();
                        stream.consume();

                        return end;
                    }
                }

                // silently skip anything between open and close token.
                stream.consume();
            }

            return nullptr;
        }
    }

    // This is not directly reflected in the standard, but each ecosystem has their own specific kind of representing
    // e.g. anonymous types and namespaces, or lambdas.
    [[nodiscard]]
    constexpr std::optional<state::Identifier> parse_synthetic_id(TokenStream& stream)
    {
        StateGuard<state::Identifier> id{stream};
        id->isSynthetic = true;

        if (!std::holds_alternative<lexing::operator_or_punctuator>(stream.peek().classification))
        {
            return std::nullopt;
        }

        auto const begin = stream.peek();
        auto const iter = util::binary_find(
            detail::placeholderWrapCandidates,
            std::get<lexing::operator_or_punctuator>(begin.classification).index(),
            {},
            [](auto const& e) { return e.open.index(); });
        if (iter == detail::placeholderWrapCandidates.cend())
        {
            return std::nullopt;
        }

        stream.consume();

        // This recursively searches for a matching `close` token, while handling nested `open close` token-ranges.
        if (auto const* const end = detail::find_end_token(stream, iter->open, iter->close))
        {
            id->content = {begin.content.data(), end};
            return std::move(id).take();
        }

        return std::nullopt;
    }

    // > identifier ::= lexing-id
    // > identifier ::= synthetic-id
    [[nodiscard]]
    constexpr std::optional<state::Identifier> parse_identifier(TokenStream& stream)
    {
        if (auto const* const id = peek_if<lexing::identifier>(stream))
        {
            state::Identifier result{.content = id->content};
            stream.consume();
            return result;
        }

        return parse_synthetic_id(stream);
    }

    // see: https://eel.is/c++draft/temp.names#nt:template-argument
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::TemplateArgument> parse_template_argument(TokenStream& stream)
    {
        if (std::optional type = parse_type_id(stream))
        {
            return {state::Recursive{*std::move(type)}};
        }
        /*
         * template-argument:
                template-argument-name
                constant-expression
                type-id
                braced-init-list
        */
        return std::nullopt;
    }

    // see: https://eel.is/c++draft/temp.names#nt:template-argument-list
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::TemplateArgumentList> parse_template_argument_list(TokenStream& stream)
    {
        std::optional first = parse_template_argument(stream);
        if (!first)
        {
            return std::nullopt;
        }

        state::TemplateArgumentList args{*std::move(first)};
        for (auto const* delimiter = peek_if<lexing::operator_or_punctuator>(stream);
             delimiter && lexing::operator_or_punctuator{","} == *delimiter;
             delimiter = peek_if<lexing::operator_or_punctuator>(stream))
        {
            Transaction transaction{stream};
            stream.consume();

            std::optional arg = parse_template_argument(stream);
            if (!arg)
            {
                break;
            }

            args.emplace_back(*std::move(arg));
            transaction.commit();
        }

        return args;
    }

    // This is a custom rule, which parses enclosed template-args, but without a preceding identifier.
    // > template-arg-list ::= `<` `>`
    // > template-arg-list ::= `<` template-argument (`,` template-argument)* `>`
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::TemplateArgumentList> parse_template_clause(TokenStream& stream)
    {
        StateGuard<state::TemplateArgumentList> args{stream};

        if (!expect(stream, lexing::operator_or_punctuator{"<"}))
        {
            return std::nullopt;
        }

        if (std::optional argList = parse_template_argument_list(stream))
        {
            *args = *std::move(argList);
        }

        if (!expect(stream, lexing::operator_or_punctuator{">"}))
        {
            return std::nullopt;
        }

        return std::move(args).take();
    }

    // see: https://eel.is/c++draft/dcl.decl.general#nt:cv-qualifier
    [[nodiscard]]
    constexpr std::optional<state::CVQualifier> parse_cv_qualifier(TokenStream& stream)
    {
        if (auto const* const keyword = peek_if<lexing::keyword>(stream))
        {
            constexpr auto map = make_map<lexing::keyword, state::CVQualifier>({
                {lexing::keyword{"const"},    state::CVQualifier::id_const   },
                {lexing::keyword{"volatile"}, state::CVQualifier::id_volatile}
            });

            if (std::optional const qualifier = map(*keyword))
            {
                stream.consume();
                return qualifier;
            }
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/dcl.decl.general#nt:cv-qualifier-seq
    // This is a simplified version of the general grammar because there are in fact just two possible qualifications.
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::CVQualifierSeq> parse_cv_qualifier_seq(TokenStream& stream)
    {
        bool isSet{false};
        state::CVQualifierSeq qualifiers{};
        while (std::optional const qualifier = parse_cv_qualifier(stream))
        {
            isSet = true;
            switch (*qualifier)
            {
            case state::CVQualifier::id_const:
                MIMICPP_ASSERT(!qualifiers.isConst, "`const` is already applied.");
                qualifiers.isConst = true;
                break;

            case state::CVQualifier::id_volatile:
                MIMICPP_ASSERT(!qualifiers.isVolatile, "`volatile` is already applied.");
                qualifiers.isVolatile = true;
                break;

            default:
                util::unreachable();
            }
        }

        if (isSet)
        {
            return {qualifiers};
        }

        return std::nullopt;
    }

    namespace detail
    {
        [[nodiscard]]
        consteval auto make_simple_operator_candidates() noexcept
        {
            std::array texts = util::concat_arrays(
                lexing::texts::comparison,
                lexing::texts::assignment,
                lexing::texts::incOrDec,
                lexing::texts::arithmetic,
                lexing::texts::bitArithmetic,
                lexing::texts::logical,
                std::to_array<std::string_view>({"->", "->*", ","}));

            std::array ops = std::apply(
                [](auto&... opTexts) { return std::array{lexing::operator_or_punctuator{opTexts}...}; },
                texts);
            std::ranges::sort(ops, {}, &lexing::operator_or_punctuator::index);
            MIMICPP_ASSERT(ops.cend() == std::ranges::unique(ops).begin(), "Fix your input!");

            return ops;
        }

        inline constexpr std::array simpleOpCandidates = make_simple_operator_candidates();

        inline constexpr std::array doubleOpCandidates = {
            std::tuple{lexing::operator_or_punctuator{"("}, lexing::operator_or_punctuator{")"}},
            std::tuple{lexing::operator_or_punctuator{"["}, lexing::operator_or_punctuator{"]"}}
        };
    }

    // unqualified-id ::= `operator` op
    // see:: https://eel.is/c++draft/over.oper.general#nt:operator
    template <bool requireOperatorKeyword = true>
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::OperatorFunctionId> parse_operator_function_id(TokenStream& stream)
    {
        Transaction transaction{stream};

        if (!expect(stream, lexing::keyword{"operator"})
            && requireOperatorKeyword)
        {
            return std::nullopt;
        }

        if (auto const* const op = peek_if<lexing::operator_or_punctuator>(stream))
        {
            if (std::ranges::binary_search(detail::simpleOpCandidates, op->index(), {}, &lexing::operator_or_punctuator::index))
            {
                state::OperatorFunctionId id{.symbol = *op};
                stream.consume();
                transaction.commit();
                return id;
            }

            for (auto&& [first, second] : detail::doubleOpCandidates)
            {
                if (expect(stream, first)
                    && expect(stream, second))
                {
                    state::OperatorFunctionId id{
                        .symbol = std::array{first, second}
                    };
                    transaction.commit();
                    return id;
                }
            }

            return std::nullopt;
        }

        if (auto const* const op = peek_if<lexing::keyword>(stream))
        {
            if (lexing::keyword{"new"} == *op
                || lexing::keyword{"delete"} == *op)
            {
                std::pair symbol{*op, false};
                stream.consume();
                transaction.commit();

                if (Transaction inner{stream};
                    expect(stream, lexing::operator_or_punctuator{"["})
                    && expect(stream, lexing::operator_or_punctuator{"]"}))
                {
                    inner.commit();
                    symbol.second = true;
                }

                return state::OperatorFunctionId{.symbol = symbol};
            }

            if (lexing::keyword{"co_await"} == *op)
            {
                state::OperatorFunctionId id{
                    .symbol = std::pair{*op, false}
                };
                stream.consume();
                transaction.commit();
                return id;
            }

            return std::nullopt;
        }

        return std::nullopt;
    }

    // > conversion-function-id ::= `operator` conversion-type-id
    // > conversion-type-id ::= type-specifier-seq ptr-operator*
    // see: https://eel.is/c++draft/class.conv.fct#nt:conversion-function-id
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::ConversionFunctionId> parse_conversion_function_id(TokenStream& stream)
    {
        StateGuard<state::TypeId> target{stream};

        if (!expect(stream, lexing::keyword{"operator"}))
        {
            return std::nullopt;
        }

        std::optional base = parse_type_specifier_seq(stream);
        if (!base)
        {
            return std::nullopt;
        }

        *target = *std::move(base);
        while (std::optional op = parse_ptr_operator(stream))
        {
            target->declarator.root.decorations.emplace_back(*std::move(op));
        }

        return state::ConversionFunctionId{
            .target = state::Recursive{*std::move(target).take()}};
    }

    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::LambdaFunctionId> parse_lambda_function_id(TokenStream& stream)
    {
        StateGuard<state::LambdaFunctionId> identifier{stream};

        if (!std::holds_alternative<lexing::operator_or_punctuator>(stream.peek().classification))
        {
            return std::nullopt;
        }

        auto const closeOp = std::invoke([&]() -> std::optional<lexing::operator_or_punctuator> {
            if (lexing::operator_or_punctuator{"<"} == std::get<lexing::operator_or_punctuator>(stream.peek().classification))
            {
                return lexing::operator_or_punctuator{">"};
            }

            if (lexing::operator_or_punctuator{"("} == std::get<lexing::operator_or_punctuator>(stream.peek().classification))
            {
                return lexing::operator_or_punctuator{")"};
            }

            return std::nullopt;
        });
        if (!closeOp)
        {
            return std::nullopt;
        }

        auto const openToken = stream.peek();
        stream.consume();

        if (auto const* const id = peek_if<lexing::identifier>(stream);
            id
            && id->content.starts_with("lambda"))
        {
            // This recursively searches for a matching `close` token, while handling nested `open close` token-ranges.
            if (auto const* const end = detail::find_end_token(
                stream, std::get<lexing::operator_or_punctuator>(openToken.classification),
                *closeOp))
            {
                identifier->content = {openToken.content.data(), end};
                return std::move(identifier).take();
            }
        }

        return std::nullopt;
    }

    // This rule is part of the more general unqualified-id rule.
    // see: https://eel.is/c++draft/expr.prim.id.unqual#nt:unqualified-id
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::DestructorFunctionId> parse_destructor_function_id(TokenStream& stream)
    {
        StateGuard<state::DestructorFunctionId> identifier{stream};

        if (expect(stream, lexing::operator_or_punctuator{"~"}))
        {
            if (std::optional id = parse_identifier(stream))
            {
                identifier->name = *std::move(id);
                return std::move(identifier).take();
            }
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/expr.prim.id.unqual#nt:unqualified-id
    // handles
    // > unqualified-id ::= identifier
    // > unqualified-id ::= template-id
    // > unqualified-id ::= operator-function-id
    // > unqualified-id ::= conversion-function-id
    // > unqualified-id ::= lambda-function-id
    // > unqualified-id ::= `~`type-name
    // but slightly rewritten
    //
    // These rules are unnecessary:
    // > unqualified-id ::= literal-operator-id
    // > unqualified-id ::= `~`computed-type-specifier
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::UnqualifiedId> parse_unqualified_id(TokenStream& stream)
    {
        StateGuard<state::UnqualifiedId> nestedId{stream};

        // A destructor cannot be templated
        if (std::optional dtor = parse_destructor_function_id(stream))
        {
            nestedId->name = *std::move(dtor);
            return std::move(nestedId).take();
        }

        // A synthetic lambda-id cannot be templated
        if (std::optional lambda = parse_lambda_function_id(stream))
        {
            nestedId->name = *std::move(lambda);
            return std::move(nestedId).take();
        }

        if (std::optional op = parse_operator_function_id(stream))
        {
            nestedId->name = *std::move(op);
        }
        // A conversion-function cannot be templated
        else if (std::optional conv = parse_conversion_function_id(stream))
        {
            nestedId->name = *std::move(conv);
            return std::move(nestedId).take();
        }
        else if (std::optional id = parse_identifier(stream))
        {
            nestedId->name = *std::move(id);
        }
        else
        {
            return std::nullopt;
        }

        if (std::optional templateArgs = parse_template_clause(stream))
        {
            nestedId->templateArgs = *std::move(templateArgs);
        }

        return std::move(nestedId).take();
    }

    // This is more or less a custom rule, which handles all nested identifier scopes;
    // i.e., all parts that are terminated by a `::` token.
    // > unqualified-id ::= identifier template-clause? parameters-and-qualifiers? `::`
    // > unqualified-id ::= operator-function-id template-clause? parameters-and-qualifiers `::`
    // > unqualified-id ::= conversion-function-id parameters-and-qualifiers `::`
    // > unqualified-id ::= destructor-function-id parameters-and-qualifiers `::`
    // > unqualified-id ::= lambda-id `::`
    //
    // Note that the additional optional `parameters-and-qualifiers` token is not reflected in the standard,
    // but rather an extension due to real-life requirements.
    template <bool isFirst>
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::UnqualifiedId> parse_nested_id(TokenStream& stream)
    {
        StateGuard<state::UnqualifiedId> nestedId{stream};

        // A synthetic lambda-id can neither be templated nor be suffixed by an argument list
        if (std::optional lambda = parse_lambda_function_id(stream))
        {
            nestedId->name = *std::move(lambda);
        }
        else
        {
            bool requiresFunction{false};
            bool acceptTemplate{false};

            // A destructor cannot be templated
            if (std::optional dtor = parse_destructor_function_id(stream))
            {
                requiresFunction = true;
                nestedId->name = *std::move(dtor);
            }
            else if (std::optional opId = std::invoke([&]() -> std::optional<state::OperatorFunctionId> {
                         Transaction innerTransaction{stream};
                         if (std::optional op = parse_operator_function_id<true>(stream))
                         {
                             requiresFunction = true;
                             acceptTemplate = true;
                             innerTransaction.commit();
                             return op;
                         }

                         if constexpr (!isFirst)
                         {
                             // In general, the `operator` keyword is mandatory, but sometimes it is omitted (e.g., on msvc).
                             // In this case, the whole scope is just the plain operator symbol; so no template- and function-details.
                             if (std::optional op = parse_operator_function_id<false>(stream);
                                 op
                                 // Peek here (and thus ensure that it is immediately followed by a `::` token),
                                 // because we need to prevent false-positives. For example `<tag>::` must not be treated as `operator<`.
                                 && peek_if<lexing::operator_or_punctuator>(stream)
                                 && lexing::operator_or_punctuator{"::"} == *peek_if<lexing::operator_or_punctuator>(stream))
                             {
                                 innerTransaction.commit();
                                 return op;
                             }
                         }

                         return std::nullopt;
                     }))
            {
                nestedId->name = *std::move(opId);
            }
            // A conversion-function cannot be templated
            else if (std::optional conv = parse_conversion_function_id(stream))
            {
                requiresFunction = true;
                nestedId->name = *std::move(conv);
            }
            else if (std::optional id = parse_identifier(stream))
            {
                acceptTemplate = true;
                nestedId->name = *std::move(id);
            }
            else
            {
                return std::nullopt;
            }

            if (acceptTemplate)
            {
                nestedId->templateArgs = parse_template_clause(stream);
            }

            if (std::optional functionDecl = parse_parameters_and_qualifiers(stream))
            {
                nestedId->functionDeclarator = *std::move(functionDecl);
            }
            else if (requiresFunction)
            {
                return std::nullopt;
            }
        }

        if (expect(stream, lexing::operator_or_punctuator{"::"}))
        {
            return std::move(nestedId).take();
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/expr.prim.id.qual#nt:nested-name-specifier
    // > nested-name-specifier ::= `::`
    //
    // These rules are not distinguishable in this task.
    // > nested-name-specifier ::= type-name `::`
    // > nested-name-specifier ::= namespace-name `::`
    // => nested-name-specifier ::= nested-id
    //
    // These rules form a left-recursion:
    // > nested-name-specifier ::= nested-name-specifier identifier `::`
    // > nested-name-specifier ::= nested-name-specifier `template`? simple-template-id `::`
    // => nested-name-specifier ::= `::` nested-id*
    // => nested-name-specifier ::= nested-id+
    //
    // Note that these rules are fully ignored:
    // > computed-type-specifier `::`
    // > splice-scope-specifier `::`
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::ScopeSequence> parse_nested_name_specifier(TokenStream& stream)
    {
        StateGuard<state::ScopeSequence> scopes{stream};
        scopes->explicitRoot = expect(stream, lexing::operator_or_punctuator{"::"}).has_value();

        std::optional initId = parse_nested_id<true>(stream);
        if (!initId)
        {
            if (scopes->explicitRoot)
            {
                return std::move(scopes).take();
            }

            return std::nullopt;
        }

        scopes->scopes.emplace_back(*std::move(initId));

        while (!stream.is_eof())
        {
            Transaction transaction{stream};

            std::optional curId = parse_nested_id<false>(stream);
            if (!curId)
            {
                break;
            }

            scopes->scopes.emplace_back(*std::move(curId));
            transaction.commit();
        }

        return std::move(scopes).take();
    }

    // see: https://eel.is/c++draft/expr.prim.id.qual#nt:qualified-id
    // > qualified-id ::= nested-name-specifier? unqualified-id
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::QualifiedId> parse_qualified_id(TokenStream& stream)
    {
        StateGuard<state::QualifiedId> id{stream};

        if (std::optional scopes = parse_nested_name_specifier(stream))
        {
            id->scopes = *std::move(scopes);
        }

        if (std::optional topLevelId = parse_unqualified_id(stream))
        {
            id->identifier = *std::move(topLevelId);
            return std::move(id).take();
        }

        return std::nullopt;
    }

    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::CallConvention> parse_call_convention(TokenStream& stream)
    {
        // Accept symbols like `__cdecl`.
        if (auto const* const id = peek_if<lexing::identifier>(stream);
            id
            && id->content.starts_with("__"))
        {
            state::CallConvention convention{.name = *id};
            stream.consume();
            return convention;
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/dcl.decl.general#nt:ptr-operator
    // note: fully ignores `attribute-specifier-seq`
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::PtrOperator> parse_ptr_operator(TokenStream& stream)
    {
        // > ptr-operator ::= `&`
        // > ptr-operator ::= `&&`
        if (std::optional const ref = parse_ref_qualifier(stream))
        {
            return {state::ReferenceDeclarator{.qualifier = *ref}};
        }

        // > ptr-operator ::= call-convention? nested-name-specifier? `*` cv-qualifier-seq?
        // Note that `call-convention` will only be generated by msvc.
        StateGuard<state::PointerDeclarator> ptr{stream};
        ptr->callConvention = parse_call_convention(stream);
        ptr->scopes = parse_nested_name_specifier(stream);
        if (!expect(stream, lexing::operator_or_punctuator{"*"}))
        {
            return std::nullopt;
        }

        ptr->qualifiers = parse_cv_qualifier_seq(stream);

        return {std::move(ptr).take()};
    }

    // This rule is not directly reflected by the standard, but mirrors the reality more closely than the original
    // `noptr-abstract-declarator` non-terminal, which allows that `params-and-qualifiers` non-terminals can appear between
    // multiple `[ constant-expression ]` sequences.
    // see: https://eel.is/c++draft/dcl.name#nt:noptr-abstract-declarator
    [[nodiscard]]
    constexpr std::optional<state::ArrayDeclarator> parse_array_declarator(TokenStream& stream)
    {
        StateGuard<state::ArrayDeclarator> declarator{stream};
        if (expect(stream, lexing::operator_or_punctuator{"["}))
        {
            declarator->size = parse_constant_expression(stream);
            if (expect(stream, lexing::operator_or_punctuator{"]"}))
            {
                return std::move(declarator).take();
            }
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/dcl.fct#nt:parameter-declaration
    // `attribute-specifier-seq`, `decl-specifier-seq` and `this` are ignored.
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::TypeId> parse_parameter_declaration(TokenStream& stream)
    {
        // This is just a very naive approach; does this hold?
        return parse_type_id(stream);
    }

    // see: https://eel.is/c++draft/dcl.fct#nt:parameter-declaration-clause
    // and https://eel.is/c++draft/dcl.fct#nt:parameter-declaration-list
    // The `...` is fully ignored.
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<std::vector<state::TypeId>> parse_parameter_declaration_clause(TokenStream& stream)
    {
        StateGuard<std::vector<state::TypeId>> params{stream};

        do
        {
            std::optional param = parse_parameter_declaration(stream);
            if (!param)
            {
                // zero params are valid!
                if (params->empty())
                {
                    break;
                }

                return std::nullopt;
            }

            params->emplace_back(*std::move(param));
        }
        while (expect(stream, lexing::operator_or_punctuator{","}));

        return std::move(params).take();
    }

    // see: https://eel.is/c++draft/dcl.decl.general#nt:parameters-and-qualifiers
    // `attribute-specifier-seq` is ignored
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::FunctionDeclarator> parse_parameters_and_qualifiers(TokenStream& stream)
    {
        StateGuard<state::FunctionDeclarator> declarator{stream};

        if (!expect(stream, lexing::operator_or_punctuator{"("}))
        {
            return std::nullopt;
        }

        std::optional clause = parse_parameter_declaration_clause(stream);
        if (!clause)
        {
            return std::nullopt;
        }

        if (!expect(stream, lexing::operator_or_punctuator{")"}))
        {
            return std::nullopt;
        }

        if (std::optional cv = parse_cv_qualifier_seq(stream))
        {
            declarator->qualifiers = *std::move(cv);
        }

        declarator->refQualifier = parse_ref_qualifier(stream);
        declarator->isNoexcept = expect(stream, lexing::keyword{"noexcept"}).has_value();

        declarator->params.reserve(clause->size());
        std::ranges::transform(
            *clause,
            std::back_inserter(declarator->params),
            [](state::TypeId& id) { return state::Recursive{std::move(id)}; });

        return {std::move(declarator).take()};
    }

    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::AbstractDeclarator> parse_abstract_declarator(TokenStream& stream)
    {
        StateGuard<state::AbstractDeclarator> declarator{stream};
        auto& root = declarator->root;

        while (std::optional op = parse_ptr_operator(stream))
        {
            root.decorations.emplace_back(*std::move(op));
        }

        if (Transaction nestedTransaction{stream};
            expect(stream, lexing::operator_or_punctuator{"("}))
        {
            if (std::optional layer = parse_abstract_declarator(stream);
                layer
                && expect(stream, lexing::operator_or_punctuator{")"}))
            {
                root.nested.emplace(std::move(layer->root));
                nestedTransaction.commit();
            }
        }

        if (std::optional params = parse_parameters_and_qualifiers(stream))
        {
            root.function.emplace(*std::move(params));
        }

        while (std::optional arrayDecl = parse_array_declarator(stream))
        {
            root.arrays.emplace_back(*std::move(arrayDecl));
        }

        if (root.decorations.empty()
            && !root.nested
            && !root.function
            && root.arrays.empty())
        {
            return std::nullopt;
        }

        return {std::move(declarator).take()};
    }

    namespace detail
    {
        [[nodiscard]]
        consteval auto make_builtin_type_candidates() noexcept
        {
            std::array types = std::apply(
                [](auto&... keyword) { return std::array{lexing::keyword{keyword}...}; },
                lexing::texts::typeKeywords);
            std::ranges::sort(types, {}, &lexing::keyword::index);
            MIMICPP_ASSERT(types.cend() == std::ranges::unique(types).begin(), "Fix your input!");

            return types;
        }

        inline constexpr std::array builtinTypeCandidates = make_builtin_type_candidates();
    }

    // see: https://eel.is/c++draft/dcl.type.general#nt:type-specifier-seq
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::TypeId> parse_type_specifier_seq(TokenStream& stream)
    {
        std::optional<state::QualifiedId> qualifiedType{};
        std::optional<state::BuiltinType> builtinType{};
        std::optional<state::TypeKey> typeKey{};

        StateGuard<state::TypeId> typeId{stream};
        while (!stream.is_eof())
        {
            Transaction transaction{stream};

            if (auto const* const keyword = peek_if<lexing::keyword>(stream))
            {
                // just silently ignore any linkage keyword.
                if (lexing::keyword{"static"} == *keyword
                    || lexing::keyword{"constexpr"} == *keyword)
                {
                    stream.consume();
                    transaction.commit();
                    continue;
                }

                if (std::optional const cv = parse_cv_qualifier(stream))
                {
                    if (!typeId->qualifications.apply(*cv))
                    {
                        break;
                    }

                    transaction.commit();
                    continue;
                }

                if (!qualifiedType
                    && std::ranges::binary_search(detail::builtinTypeCandidates, keyword->index(), {}, &lexing::keyword::index))
                {
                    if (!(builtinType ? *builtinType : builtinType.emplace()).try_apply(*keyword))
                    {
                        break;
                    }

                    stream.consume();
                    transaction.commit();
                    continue;
                }

                if (std::optional key = parse_type_key(stream);
                    key
                    && !typeKey)
                {
                    typeKey = std::move(key);
                    transaction.commit();
                    continue;
                }
            }
            else if (!qualifiedType && !builtinType)
            {
                if (std::optional qualifiedId = parse_qualified_id(stream))
                {
                    qualifiedType = std::move(qualifiedId);
                    transaction.commit();
                    continue;
                }
            }

            break;
        }

        if (qualifiedType
            && !builtinType)
        {
            qualifiedType->typeKey = std::move(typeKey);
            typeId->base = *std::move(qualifiedType);
            return std::move(typeId).take();
        }

        if (builtinType
            && !qualifiedType
            && !typeKey)
        {
            typeId->base = *std::move(builtinType);
            return std::move(typeId).take();
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/dcl.name#nt:type-id
    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::TypeId> parse_type_id(TokenStream& stream)
    {
        StateGuard<state::TypeId> id{stream};
        std::optional base = parse_type_specifier_seq(stream);
        if (!base)
        {
            return std::nullopt;
        }

        *id = *std::move(base);

        if (std::optional declarator = parse_abstract_declarator(stream))
        {
            id->declarator = *std::move(declarator);
        }

        return std::move(id).take();
    }

    [[nodiscard]]
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<state::FunctionId> parse_function(TokenStream& stream)
    {
        StateGuard<state::FunctionId> id{stream};
        if (std::optional returnType = parse_type_id(stream))
        {
            id->callConvention = parse_call_convention(stream);

            if (std::optional identifier = parse_qualified_id(stream))
            {
                if (std::optional declarator = parse_parameters_and_qualifiers(stream))
                {
                    id->returnType = std::move(returnType);
                    id->identifier = *std::move(identifier);
                    id->identifier.identifier.functionDeclarator = *std::move(declarator);

                    return std::move(id).take();
                }
            }
            // Well, sometimes there is no return-type given, but the parsed identifier actually is the function-id.
            // E.g., `<lambda()>`
            // This is a workaround!
            else if (auto* const qualifiedId = std::get_if<state::QualifiedId>(&returnType->base))
            {
                id->identifier = std::move(*qualifiedId);
                id->identifier.identifier.functionDeclarator = std::move(returnType->declarator.root.function);

                return std::move(id).take();
            }
        }

        return std::nullopt;
    }
}

namespace mimicpp::printing::type
{
    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<parsing::v2::state::TypeId> parse_type(std::string_view const text)
    {
        lexing::NameLexer lexer{text};
        parsing::v2::TokenStream stream{lexer};
        if (std::optional typeId = parsing::v2::parse_type_id(stream);
            typeId
            && stream.is_eof())
        {
            return typeId;
        }

        return std::nullopt;
    }

    MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES std::optional<parsing::v2::state::FunctionId> parse_function(std::string_view const text)
    {
        lexing::NameLexer lexer{text};
        parsing::v2::TokenStream stream{lexer};
        if (std::optional functionId = parsing::v2::parse_function(stream))
        {
            if (stream.is_eof()
                // Sometimes certain template details are added in separate square-brackets
                || (text.ends_with(']')
                    && parsing::v2::expect(stream, lexing::operator_or_punctuator{"["})))
            {
                return functionId;
            }
        }

        return std::nullopt;
    }
}

#endif
