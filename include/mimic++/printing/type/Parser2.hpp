//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PARSER2_HPP
#define MIMICPP_PARSER2_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/printing/type/NameLexer.hpp"
#include "mimic++/printing/type/Parser2State.hpp"
#include "mimic++/utilities/Algorithm.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/PassKey.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
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
        explicit constexpr TokenStream(lexing::NameLexer& lexer) noexcept
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
            return m_Index == m_Tokens.size() - 1;
        }

        [[nodiscard]]
        constexpr lexing::token const& peek() const
        {
            MIMICPP_ASSERT(m_Index < m_Tokens.size(), "Stream is at end.");
            return m_Tokens[m_Index];
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

    class Context
    {
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
        return [mappings](Key const& key) -> std::optional<Value> {
            if (auto const iter = std::ranges::find(mappings, key, &std::pair<Key, Value>::first);
                iter != std::ranges::end(mappings))
            {
                return iter->second;
            }

            return std::nullopt;
        };
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
    constexpr std::optional<state::TypeId> parse_type_id(TokenStream& stream);

    // see: https://eel.is/c++draft/class.pre#nt:class-key
    [[nodiscard]]
    constexpr std::optional<state::ClassKey> parse_class_key(TokenStream& stream)
    {
        if (auto const* const keyword = peek_if<lexing::keyword>(stream))
        {
            constexpr auto map = make_map<lexing::keyword, state::ClassKey>({
                { lexing::keyword{"class"},  state::ClassKey::id_class},
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

    // see: https://eel.is/c++draft/temp.names#nt:template-argument
    [[nodiscard]]
    constexpr std::optional<state::TemplateArgument> parse_template_argument(TokenStream& stream)
    {
        if (std::optional type = parse_type_id(stream))
        {
            return {state::RecursiveTypeId{*std::move(type)}};
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
    constexpr std::optional<state::TemplateArgumentList> parse_template_argument_list(TokenStream& stream)
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

    // `unqualified-id ::= template-name < template-argument-list? >`, where `template-name==identifier`
    // see: https://eel.is/c++draft/temp.names#nt:simple-template-id
    //
    [[nodiscard]]
    constexpr std::optional<state::SimpleTemplateId> parse_simple_template_id(TokenStream& stream)
    {
        if (auto const* const id = peek_if<lexing::identifier>(stream))
        {
            StateGuard<state::SimpleTemplateId> templateId{stream};
            templateId->name = *id;
            stream.consume();

            if (!expect(stream, lexing::operator_or_punctuator{"<"}))
            {
                return std::nullopt;
            }

            if (std::optional argList = parse_template_argument_list(stream))
            {
                templateId->args = *std::move(argList);
            }

            if (!expect(stream, lexing::operator_or_punctuator{">"}))
            {
                return std::nullopt;
            }

            return std::move(templateId).take();
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/dcl.decl.general#nt:cv-qualifier
    [[nodiscard]]
    constexpr std::optional<state::CVQualifier> parse_cv_qualifier(TokenStream& stream)
    {
        if (auto const* const keyword = peek_if<lexing::keyword>(stream))
        {
            constexpr auto map = make_map<lexing::keyword, state::CVQualifier>({
                {   lexing::keyword{"const"},    state::CVQualifier::id_const},
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
    constexpr std::optional<state::CVQualifierSeq> parse_cv_qualifier_seq(TokenStream& stream)
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

    // see: https://eel.is/c++draft/dcl.decl.general#nt:ptr-operator
    // note: fully ignores `attribute-specifier-seq`
    [[nodiscard]]
    constexpr std::optional<state::PtrOperator> parse_ptr_operator(TokenStream& stream)
    {
        // `ptr-operator ::= &&`
        if (expect(stream, lexing::operator_or_punctuator{"&&"}))
        {
            return {state::ReferenceDeclarator{.qualifier = state::id_refref}};
        }

        // `ptr-operator ::= &`
        if (expect(stream, lexing::operator_or_punctuator{"&"}))
        {
            return {state::ReferenceDeclarator{.qualifier = state::id_ref}};
        }

        // `ptr-operator ::= nested-name-specifier? * cv-qualifier-seq?`
        StateGuard<state::PointerDeclarator> ptr{stream};

        // Todo:
        // ptr->scopes = parse_nested_name_specifier(stream);
        if (!expect(stream, lexing::operator_or_punctuator{"*"}))
        {
            return std::nullopt;
        }

        ptr->qualifiers = parse_cv_qualifier_seq(stream);

        return {std::move(ptr).take()};
    }

    [[nodiscard]]
    constexpr std::optional<state::AbstractDeclarator> parse_abstract_declarator(TokenStream& stream)
    {
        StateGuard<state::AbstractDeclarator> declarator{stream};
        auto& [decorations, core] = declarator->root;

        while (std::optional op = parse_ptr_operator(stream))
        {
            decorations.emplace_back(*std::move(op));
        }

        if (expect(stream, lexing::operator_or_punctuator{"("}))
        {
            std::optional layer = parse_abstract_declarator(stream);
            if (!layer
                || expect(stream, lexing::operator_or_punctuator{")"}))
            {
                return std::nullopt;
            }

            core = std::make_unique<state::AbstractDeclarator::Layer>(std::move(layer->root));
        }
        else if (expect(stream, lexing::operator_or_punctuator{"["}))
        {
            // Todo: parse optional constant-expression
            if (!expect(stream, lexing::operator_or_punctuator{"]"}))
            {
                return std::nullopt;
            }

            core.emplace<state::ArrayDeclarator>();
        }
        /*else if (std::optional params = parse_params_and_qualifiers(stream))
        {

        }*/

        if (decorations.empty()
            && std::holds_alternative<std::monostate>(declarator->root.core))
        {
            return std::nullopt;
        }

        return {std::move(declarator).take()};
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
    }

    // unqualified-id ::= `operator` op
    // see:: https://eel.is/c++draft/over.oper.general#nt:operator
    [[nodiscard]]
    constexpr std::optional<state::OperatorFunctionId> parse_operator_function_id(TokenStream& stream)
    {
        Transaction transaction{stream};

        if (expect(stream, lexing::keyword{"operator"}))
        {
            stream.consume();

            // simple operators are the ones that consist of just a single operator-token.
            if (auto const* const op = peek_if<lexing::operator_or_punctuator>(stream);
                op
                && std::ranges::binary_search(detail::simpleOpCandidates, op->index(), {}, &lexing::operator_or_punctuator::index))
            {
                transaction.commit();
                return state::OperatorFunctionId{.op = *op};
            }

            // Todo: add rest
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/expr.prim.id.unqual#nt:unqualified-id
    [[nodiscard]]
    constexpr std::optional<state::UnqualifiedId> parse_unqualified_id(TokenStream& stream)
    {
        // `unqualified-id ::= operator-function-id`
        if (std::optional const op = parse_operator_function_id(stream))
        {
            return op;
        }

        StateGuard<state::UnqualifiedId> unqalified{stream};

        // `unqualified-id ::= identifier`
        // `unqualified-id ::= template-name < template-argument-list? >`, where `template-name==identifier`
        // see: https://eel.is/c++draft/temp.names#nt:simple-template-id
        if (auto const* const id = peek_if<lexing::identifier>(stream))
        {
            if (std::optional templateId = parse_simple_template_id(stream))
            {
                *unqalified = *std::move(templateId);
            }
            else
            {
                *unqalified = *id;
                stream.consume();
            }

            return std::move(unqalified).take();
        }

        // Todo: add missing
        // `unqualified-id ::= conversion-function-id`
        // `unqualified-id ::= ~ type-name`

        // These rules are unnecessary:
        // `unqualified-id ::= literal-operator-id`
        // `unqualified-id ::= ~ computed-type-specifier`

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/expr.prim.id.qual#nt:qualified-id
    // `qualified-id ::= nested-name-specifier unqualified-id` is rewritten to
    // `qualified-id ::= ::? (unqualified-id ::)* unqualified-id`
    [[nodiscard]]
    constexpr std::optional<state::QualifiedId> parse_qualified_id(TokenStream& stream)
    {
        constexpr lexing::operator_or_punctuator separator{"::"};

        StateGuard<state::QualifiedId> id{stream};
        id->scopes.explicitRoot = expect(stream, separator).has_value();

        std::optional curId = parse_unqualified_id(stream);
        if (!curId)
        {
            return std::nullopt;
        }

        id->identifier = *std::move(curId);
        while (expect(stream, separator))
        {
            curId = parse_unqualified_id(stream);
            if (!curId)
            {
                return std::nullopt;
            }

            id->scopes.scopes.emplace_back(
                std::exchange(id->identifier, *curId));
        }

        return std::move(id).take();
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
    constexpr std::optional<state::TypeId> parse_type_specifier_seq(TokenStream& stream)
    {
        std::optional<state::QualifiedId> qualifiedType{};
        std::optional<state::BuiltinType> builtinType{};

        StateGuard<state::TypeId> typeId{stream};
        while (!stream.is_eof())
        {
            if (auto const* const keyword = peek_if<lexing::keyword>(stream))
            {
                if (std::optional const cv = parse_cv_qualifier(stream);
                    cv
                    && typeId->leadingQualifications.apply(*cv))
                {
                    continue;
                }

                if (std::ranges::binary_search(detail::builtinTypeCandidates, keyword->index(), {}, &lexing::keyword::index)
                    && (builtinType ? *builtinType : builtinType.emplace()).try_apply(*keyword))
                {
                    stream.consume();
                    continue;
                }
            }
            else if (std::optional qualifiedId = parse_qualified_id(stream);
                     qualifiedId
                     && !qualifiedType)
            {
                qualifiedType = std::move(qualifiedId);
                continue;
            }

            break;
        }

        if (qualifiedType
            && !builtinType)
        {
            typeId->base = *std::move(qualifiedType);
            return std::move(typeId).take();
        }

        if (builtinType
            && !qualifiedType)
        {
            typeId->base = *std::move(builtinType);
            return std::move(typeId).take();
        }

        return std::nullopt;
    }

    // see: https://eel.is/c++draft/dcl.name#nt:type-id
    [[nodiscard]]
    constexpr std::optional<state::TypeId> parse_type_id(TokenStream& stream)
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
}

namespace mimicpp::printing::type
{
    constexpr std::optional<parsing::v2::state::TypeId> parse_type(std::string_view const text)
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
}

#endif
