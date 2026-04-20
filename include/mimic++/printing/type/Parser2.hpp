//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PARSER2_HPP
#define MIMICPP_PARSER2_HPP

#pragma once

#include "NameLexer.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/type/NameLexer.hpp"
#include "mimic++/printing/type/NameParserTokens.hpp"
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

    namespace state
    {
        enum ClassKey
        {
            id_class = 0,
            id_struct,
            id_union
        };
    }

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

    namespace state
    {
        struct TemplateName
        {
            lexing::identifier identifier;
        };
    }

    // see: https://eel.is/c++draft/temp.names#nt:template-name
    [[nodiscard]]
    constexpr std::optional<state::TemplateName> parse_template_name(TokenStream& stream)
    {
        if (auto const* const identifier = peek_if<lexing::identifier>(stream))
        {
            stream.consume();
            return state::TemplateName{*identifier};
        }

        return std::nullopt;
    }

    namespace state
    {
        struct TemplateArgument
        {
        };
    }

    // see: https://eel.is/c++draft/temp.names#nt:template-argument
    [[nodiscard]]
    constexpr std::optional<state::TemplateArgument> parse_template_argument(TokenStream& /*stream*/)
    {
        /*
         * template-argument:
                template-argument-name
                constant-expression
                type-id
                braced-init-list
        */
        return std::nullopt;
    }

    namespace state
    {
        using TemplateArgumentList = std::vector<TemplateArgument>;
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

    namespace state
    {
        struct SimpleTemplateId
        {
            TemplateName name{};
            TemplateArgumentList args{};
        };
    }

    // see: https://eel.is/c++draft/temp.names#nt:simple-template-id
    [[nodiscard]]
    constexpr std::optional<state::SimpleTemplateId> parse_simple_template_id(TokenStream& stream)
    {
        Transaction transaction{stream};
        if (std::optional name = parse_template_name(stream);
            name
            && expect(stream, lexing::operator_or_punctuator{"<"}))
        {
            std::optional argList = parse_template_argument_list(stream);
            if (expect(stream, lexing::operator_or_punctuator{">"}))
            {
                transaction.commit();
                state::SimpleTemplateId id{.name = *std::move(name)};
                if (argList)
                {
                    id.args = *std::move(argList);
                }

                return id;
            }
        }

        return std::nullopt;
    }

    namespace state
    {
        // Todo: handle
        // operator-function-id < template-argument-list >
        // literal-operator-id < template-argument-list >
        using TemplateId = std::variant<
            SimpleTemplateId>;
    }

    // see: https://eel.is/c++draft/temp.names#nt:template-id
    [[nodiscard]]
    constexpr std::optional<state::TemplateId> parse_template_id(TokenStream& stream)
    {
        if (std::optional simpleId = parse_simple_template_id(stream))
        {
            return {*std::move(simpleId)};
        }

        return std::nullopt;
    }

    namespace state
    {
        enum class CVQualifier
        {
            id_const = 0,
            id_volatile
        };
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

    namespace state
    {
        struct CVQualifierSeq
        {
            bool isConst{false};
            bool isVolatile{false};

            [[nodiscard]]
            friend bool operator==(CVQualifierSeq const&, CVQualifierSeq const&) = default;

            [[nodiscard]]
            constexpr bool apply(CVQualifier const qualifier) noexcept
            {
                switch (qualifier)
                {
                case CVQualifier::id_const:
                    return !std::exchange(isConst, true);

                case CVQualifier::id_volatile:
                    return !std::exchange(isVolatile, true);

                default:
                    util::unreachable();
                }
            }
        };
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

    namespace state
    {
        enum RefQualifier
        {
            id_ref = 0,
            id_refref
        };
    }

    namespace state
    {
        struct PtrOperator
        {
            // Todo: nested-name-specifier
            // ignore attribute-specifier-seq

            enum class Type
            {
                ptr = 0,
                ref,
                refref
            };

            Type type{};
            std::optional<CVQualifierSeq> qualifiers{};
        };
    }

    // see: https://eel.is/c++draft/dcl.decl.general#nt:ptr-operator
    [[nodiscard]]
    constexpr std::optional<state::PtrOperator> parse_ptr_operator(TokenStream& stream)
    {
        // Todo: handle
        // nested-name-specifier * attribute-specifier-seq? cv-qualifier-seq?
        using Type = state::PtrOperator::Type;
        if (expect(stream, lexing::operator_or_punctuator{"*"}))
        {
            return state::PtrOperator{
                .type = Type::ptr,
                .qualifiers = parse_cv_qualifier_seq(stream)};
        }

        if (expect(stream, lexing::operator_or_punctuator{"&&"}))
        {
            return {state::PtrOperator{.type = Type::refref}};
        }

        if (expect(stream, lexing::operator_or_punctuator{"&"}))
        {
            return {state::PtrOperator{.type = Type::ref}};
        }

        return std::nullopt;
    }

    /*
    (* The entry point for a type-id *)
    type-id
        ::= type-specifier-seq abstract-declarator?

    abstract-declarator
        ::= ptr-abstract-declarator
        |   noptr-abstract-declarator

    ptr-abstract-declarator
        ::= ptr-operator abstract-declarator  (* Pointers/Refs pile up on the left *)
        |   noptr-abstract-declarator

    noptr-abstract-declarator
        ::= "(" abstract-declarator ")" suffix-list
        |   suffix-list

    suffix-list
        ::= ( parameters-and-qualifiers | array-suffix )*
    */

    namespace state
    {
        struct ParametersAndQualifiers
        {
        };

        struct ConstantExpression
        {
        };

        struct ArrayDeclarator
        {
            std::optional<ConstantExpression> size{};
        };

        struct FunctionDeclarator
        {
            // params-and-qualifiers
        };

        struct AbstractDeclarator
        {
            struct Layer
            {
                std::vector<PtrOperator> decorations{};

                using Core = std::variant<
                    std::monostate,
                    ArrayDeclarator,
                    FunctionDeclarator,
                    std::unique_ptr<Layer>>;
                Core core{};
            };

            Layer root{};
        };
    }

    [[nodiscard]]
    constexpr std::optional<state::AbstractDeclarator> parse_abstract_declarator(TokenStream& stream)
    {
        Transaction transaction{stream};
        state::AbstractDeclarator declarator{};

        while (std::optional op = parse_ptr_operator(stream))
        {
            declarator.root.decorations.emplace_back(*std::move(op));
        }

        if (expect(stream, lexing::operator_or_punctuator{"("}))
        {
            std::optional layer = parse_abstract_declarator(stream);
            if (!layer
                || expect(stream, lexing::operator_or_punctuator{")"}))
            {
                return std::nullopt;
            }

            declarator.root.core = std::make_unique<state::AbstractDeclarator::Layer>(std::move(layer->root));
        }
        else if (expect(stream, lexing::operator_or_punctuator{"["}))
        {
            // Todo: parse optional constant-expression
            if (!expect(stream, lexing::operator_or_punctuator{"]"}))
            {
                return std::nullopt;
            }

            declarator.root.core.emplace<state::ArrayDeclarator>();
        }
        /*else if (std::optional params = parse_params_and_qualifiers(stream))
        {

        }*/

        if (declarator.root.decorations.empty()
            && std::holds_alternative<std::monostate>(declarator.root.core))
        {
            return std::nullopt;
        }

        return {std::move(declarator)};
    }

    namespace state
    {
        struct OperatorFunctionId
        {
            lexing::operator_or_punctuator op;

            [[nodiscard]]
            friend bool operator==(OperatorFunctionId const&, OperatorFunctionId const&) = default;
        };

        using UnqualifiedId = std::variant<
            lexing::identifier,
            OperatorFunctionId>;

        // This models more or less: https://eel.is/c++draft/expr.prim.id.qual#nt:nested-name-specifier
        struct ScopeSequence
        {
            bool explicitRoot{};
            std::vector<UnqualifiedId> scopes{};
        };

        // see: https://eel.is/c++draft/expr.prim.id.qual#nt:qualified-id
        struct QualifiedId
        {
            ScopeSequence scopes{};
            UnqualifiedId identifier{};
        };
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
        // `unqualified-id ::= identifier`
        if (auto const* const id = peek_if<lexing::identifier>(stream))
        {
            stream.consume();
            return {*id};
        }

        // `unqualified-id ::= operator-function-id`
        if (std::optional const op = parse_operator_function_id(stream))
        {
            return op;
        }

        // Todo: add missing
        // `unqualified-id ::= conversion-function-id`
        // `unqualified-id ::= ~ type-name`
        // `unqualified-id ::= template-id`

        // These rules are unnecessary:
        // `unqualified-id ::= literal-operator-id`
        // `unqualified-id ::= ~ computed-type-specifier`

        return std::nullopt;
    }

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
        constexpr explicit StateGuard(TokenStream& stream, Args&&... args)
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

    namespace state
    {
        struct BuiltinType
        {
            std::optional<lexing::keyword> base{};

            enum class SizeSpec : std::int8_t
            {
                id_short = 0,
                id_long,
                id_longlong
            };

            std::optional<SizeSpec> sizeSpec{};

            enum class SignedSpec : std::int8_t
            {
                id_signed = 0,
                id_unsigned
            };

            std::optional<SignedSpec> signedSpec{};

            [[nodiscard]]
            constexpr bool try_apply(lexing::keyword const& keyword)
            {
                return try_apply_size_spec(keyword)
                    || try_apply_signed_spec(keyword)
                    || try_apply_base(keyword);
            }

            [[nodiscard]]
            friend bool operator==(BuiltinType const&, BuiltinType const&) = default;

        private:
            [[nodiscard]]
            constexpr bool try_apply_base(lexing::keyword const& keyword) noexcept
            {
                // Todo: verify correct type keywords
                if (!base)
                {
                    base = keyword;
                    return true;
                }

                return false;
            }

            [[nodiscard]]
            constexpr bool try_apply_signed_spec(lexing::keyword const& keyword) noexcept
            {
                std::optional const spec = std::invoke([&]() -> std::optional<SignedSpec> {
                    if (lexing::keyword{"unsigned"} == keyword)
                    {
                        return SignedSpec::id_unsigned;
                    }

                    if (lexing::keyword{"signed"} == keyword)
                    {
                        return SignedSpec::id_signed;
                    }

                    return std::nullopt;
                });

                if (spec
                    && !signedSpec)
                {
                    signedSpec = spec;
                    return true;
                }

                return false;
            }

            [[nodiscard]]
            constexpr bool try_apply_size_spec(lexing::keyword const& keyword) noexcept
            {
                std::optional const spec = std::invoke([&]() -> std::optional<SizeSpec> {
                    if (lexing::keyword{"long"} == keyword)
                    {
                        return SizeSpec::id_long;
                    }

                    if (lexing::keyword{"short"} == keyword)
                    {
                        return SizeSpec::id_short;
                    }

                    return std::nullopt;
                });

                if (spec)
                {
                    if (!sizeSpec)
                    {
                        sizeSpec = spec;
                        return true;
                    }

                    if (*sizeSpec == SizeSpec::id_long
                        && *spec == SizeSpec::id_long)
                    {
                        sizeSpec = SizeSpec::id_longlong;
                        return true;
                    }
                }

                return false;
            }
        };

        using BaseType = std::variant<
            QualifiedId,
            BuiltinType>;

        struct TypeId
        {
            CVQualifierSeq leadingQualifications{};
            BaseType base;
            AbstractDeclarator declarator{};
        };
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

                if (std::ranges::binary_search(detail::builtinTypeCandidates, keyword->index(), {}, &lexing::keyword::index))
                {
                    if ((builtinType ? *builtinType : builtinType.emplace()).try_apply(*keyword))
                    {
                        stream.consume();
                        continue;
                    }
                }
            }
            else if (std::optional qualifiedId = parse_qualified_id(stream);
                     qualifiedId
                     && !qualifiedType)
            {
                qualifiedType = std::move(qualifiedId);
                continue;
            }

            return std::nullopt;
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
