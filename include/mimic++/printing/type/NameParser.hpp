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

                                 visitor.begin_type();
                                 visitor.end_type();

                                 visitor.begin_scope();
                                 visitor.end_scope();

                                 visitor.add_identifier(content);
                                 visitor.add_arg();

                                 visitor.begin_template_args();
                                 visitor.end_template_args();

                                 visitor.add_const();
                                 visitor.add_volatile();
                                 visitor.add_noexcept();
                                 visitor.add_ptr();
                                 visitor.add_lvalue_ref();
                                 visitor.add_rvalue_ref();

                                 visitor.begin_function();
                                 visitor.end_function();
                                 visitor.begin_return_type();
                                 visitor.end_return_type();
                                 visitor.begin_function_args();
                                 visitor.end_function_args();
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
        class Type;

        class ArgSeparator
        {
        };

        class OpeningAngle
        {
        };

        class ClosingAngle
        {
        };

        class OpeningParens
        {
        };

        class ClosingParens
        {
        };

        class Specs
        {
        public:
            struct Layer
            {
                bool isConst{false};
                bool isVolatile{false};
                bool isNoexcept{false};
                bool isLValueRef{false};
                bool isRValueRef{false};

                constexpr void merge(Layer const& others) noexcept
                {
                    MIMICPP_ASSERT(!(isConst && others.isConst), "Merging same specs.");
                    MIMICPP_ASSERT(!(isVolatile && others.isVolatile), "Merging same specs.");
                    MIMICPP_ASSERT(!(isNoexcept && others.isNoexcept), "Merging same specs.");
                    MIMICPP_ASSERT(!(isLValueRef && others.isLValueRef), "Merging same specs.");
                    MIMICPP_ASSERT(!(isRValueRef && others.isRValueRef), "Merging same specs.");

                    MIMICPP_ASSERT(!(isLValueRef && others.isRValueRef), "Both reference types detected.");
                    MIMICPP_ASSERT(!(isRValueRef && others.isLValueRef), "Both reference types detected.");

                    isConst = isConst || others.isConst;
                    isVolatile = isVolatile || others.isVolatile;
                    isNoexcept = isNoexcept || others.isNoexcept;
                    isLValueRef = isLValueRef || others.isLValueRef;
                    isRValueRef = isRValueRef || others.isRValueRef;
                }

                template <parser_visitor Visitor>
                constexpr void operator()(Visitor& visitor) const
                {
                    auto& inner = unwrap_visitor(visitor);

                    MIMICPP_ASSERT(!(isLValueRef && isRValueRef), "Both reference types detected.");

                    if (isConst)
                    {
                        inner.add_const();
                    }

                    if (isVolatile)
                    {
                        inner.add_volatile();
                    }

                    if (isLValueRef)
                    {
                        inner.add_lvalue_ref();
                    }
                    else if (isRValueRef)
                    {
                        inner.add_rvalue_ref();
                    }

                    if (isNoexcept)
                    {
                        inner.add_noexcept();
                    }
                }
            };

            std::vector<Layer> layers{1u};

            [[nodiscard]]
            constexpr bool has_ptr() const noexcept
            {
                return 1u < layers.size();
            }

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(!layers.empty(), "Invalid state.");

                auto& unwrapped = unwrap_visitor(visitor);

                std::invoke(layers.front(), unwrapped);

                for (auto const& layer : layers | std::views::drop(1u))
                {
                    unwrapped.add_ptr();
                    std::invoke(layer, unwrapped);
                }
            }
        };

        class ArgSequence
        {
        public:
            std::vector<Type> types;

            constexpr ~ArgSequence() noexcept;
            constexpr ArgSequence();
            constexpr ArgSequence(ArgSequence const&);
            constexpr ArgSequence& operator=(ArgSequence const&);
            constexpr ArgSequence(ArgSequence&&) noexcept;
            constexpr ArgSequence& operator=(ArgSequence&&) noexcept;

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const;

            template <parser_visitor Visitor>
            constexpr void handle_as_template_args(Visitor& visitor) const;

            template <parser_visitor Visitor>
            constexpr void handle_as_function_args(Visitor& visitor) const;
        };

        class Identifier
        {
        public:
            StringViewT content{};
            std::optional<ArgSequence> templateArgs{};

            [[nodiscard]]
            constexpr bool is_template() const noexcept
            {
                return templateArgs.has_value();
            }

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(!content.empty(), "Empty identifier is not allowed.");

                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.add_identifier(content);

                if (templateArgs)
                {
                    templateArgs->handle_as_template_args(unwrapped);
                }
            }
        };

        class FunctionIdentifier
        {
        public:
            std::optional<Identifier> identifier{};
            ArgSequence args{};
            Specs specs{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                if (identifier)
                {
                    std::invoke(*identifier, unwrapped);
                }

                args.handle_as_function_args(unwrapped);
                std::invoke(specs, unwrapped);
            }
        };

        class ScopeSequence
        {
        public:
            using Id = std::variant<Identifier, FunctionIdentifier>;
            std::vector<Id> scopes{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(!scopes.empty(), "Empty scope-sequence is not allowed.");

                auto& unwrapped = unwrap_visitor(visitor);

                for (auto const& scope : scopes)
                {
                    unwrapped.begin_scope();
                    std::visit(
                        [&](auto const& id) { handle_identifier(id, unwrapped); },
                        scope);
                    unwrapped.end_scope();
                }
            }

        private:
            template <parser_visitor Visitor>
            constexpr void handle_identifier(Identifier const& id, Visitor& visitor) const
            {
                std::invoke(id, visitor);
            }

            template <parser_visitor Visitor>
            constexpr void handle_identifier(FunctionIdentifier const& id, Visitor& visitor) const
            {
                visitor.begin_function();
                std::invoke(id, visitor);
                visitor.end_function();
            }
        };

        class RegularType
        {
        public:
            std::optional<ScopeSequence> scopes{};
            Identifier identifier;
            Specs specs{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                if (scopes)
                {
                    std::invoke(*scopes, unwrapped);
                }

                std::invoke(identifier, unwrapped);
                std::invoke(specs, unwrapped);
            }
        };

        class FunctionType
        {
        public:
            std::shared_ptr<Type> returnType{};
            std::optional<ScopeSequence> scopes{};
            FunctionIdentifier identifier{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.begin_function();

                if (returnType)
                {
                    unwrapped.begin_return_type();
                    std::invoke(*returnType, visitor);
                    unwrapped.end_return_type();
                }

                if (scopes)
                {
                    std::invoke(*scopes, unwrapped);
                }

                std::invoke(identifier, unwrapped);

                unwrapped.end_function();
            }

        private:
            template <parser_visitor Visitor>
            static constexpr void handle_description([[maybe_unused]] Visitor& visitor, [[maybe_unused]] std::monostate const state)
            {
            }

            template <parser_visitor Visitor, typename Description>
            static constexpr void handle_description(Visitor& visitor, Description const& description)
            {
                std::invoke(description, visitor);
            }
        };

        class Type
        {
        public:
            using State = std::variant<RegularType, FunctionType>;
            State m_State;

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.begin_type();

                std::visit(
                    [&](auto const& inner) { std::invoke(inner, unwrapped); },
                    m_State);

                unwrapped.end_type();
            }
        };

        constexpr ArgSequence::~ArgSequence() noexcept = default;
        constexpr ArgSequence::ArgSequence() = default;
        constexpr ArgSequence::ArgSequence(ArgSequence const&) = default;
        constexpr ArgSequence& ArgSequence::operator=(ArgSequence const&) = default;
        constexpr ArgSequence::ArgSequence(ArgSequence&&) noexcept = default;
        constexpr ArgSequence& ArgSequence::operator=(ArgSequence&&) noexcept = default;

        template <parser_visitor Visitor>
        constexpr void ArgSequence::operator()(Visitor& visitor) const
        {
            if (!types.empty())
            {
                auto& unwrapped = unwrap_visitor(visitor);

                std::invoke(types.front(), unwrapped);

                for (auto const& type : types | std::views::drop(1))
                {
                    unwrapped.add_arg();
                    std::invoke(type, unwrapped);
                }
            }
        }

        template <parser_visitor Visitor>
        constexpr void ArgSequence::handle_as_template_args(Visitor& visitor) const
        {
            auto& unwrapped = unwrap_visitor(visitor);

            unwrapped.begin_template_args();
            std::invoke(*this, unwrapped);
            unwrapped.end_template_args();
        }

        template <parser_visitor Visitor>
        constexpr void ArgSequence::handle_as_function_args(Visitor& visitor) const
        {
            auto& unwrapped = unwrap_visitor(visitor);

            unwrapped.begin_function_args();
            std::invoke(*this, unwrapped);
            unwrapped.end_function_args();
        }
    }

    using Token = std::variant<
        token::ArgSeparator,
        token::OpeningAngle,
        token::ClosingAngle,
        token::OpeningParens,
        token::ClosingParens,

        token::Identifier,
        token::FunctionIdentifier,
        token::ScopeSequence,
        token::ArgSequence,
        token::Specs,
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
            std::span<Token const> const tokenStack) noexcept
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
    constexpr bool is_suffix_of(std::span<Token const> const tokenStack) noexcept
    {
        using types = util::type_list<First, Others...>;

        return 1u + sizeof...(Others) <= tokenStack.size()
            && detail::is_suffix_of(util::type_list_reverse_t<types>{}, tokenStack);
    }

    template <token_type Leading, token_type... Others>
    [[nodiscard]]
    constexpr auto match_suffix(std::span<Token> const tokenStack) noexcept
    {
        if constexpr (0u == sizeof...(Others))
        {
            Leading* result{};
            if (is_suffix_of<Leading>(tokenStack))
            {
                result = &std::get<Leading>(tokenStack.back());
            }

            return result;
        }
        else
        {
            std::optional<std::tuple<Leading&, Others&...>> result{};
            if (is_suffix_of<Leading, Others...>(tokenStack))
            {
                auto const suffix = tokenStack.last(1u + sizeof...(Others));

                result = std::invoke(
                    [&]<std::size_t... indices>([[maybe_unused]] std::index_sequence<indices...> const) noexcept {
                        return std::tie(
                            std::get<Leading>(suffix[0u]),
                            std::get<Others>(suffix[1u + indices])...);
                    },
                    std::index_sequence_for<Others...>{});
            }

            return result;
        }
    }

    constexpr void remove_suffix(std::span<Token>& tokenStack, std::size_t const count) noexcept
    {
        MIMICPP_ASSERT(count <= tokenStack.size(), "Count exceeds stack size.");
        tokenStack = tokenStack.first(tokenStack.size() - count);
    }

    namespace token
    {
        bool try_reduce_as_type(TokenStack& tokenStack);

        constexpr bool try_reduce_as_scope_sequence(TokenStack& tokenStack)
        {
            ScopeSequence::Id id{};
            if (auto const* identifier = match_suffix<Identifier>(tokenStack))
            {
                id = std::move(*identifier);
            }
            else if (auto const* funIdentifier = match_suffix<FunctionIdentifier>(tokenStack))
            {
                id = std::move(*funIdentifier);
            }
            else
            {
                return false;
            }

            tokenStack.pop_back();

            if (auto* sequence = match_suffix<ScopeSequence>(tokenStack))
            {
                sequence->scopes.emplace_back(std::move(id));
            }
            else
            {
                tokenStack.emplace_back(
                    ScopeSequence{
                        .scopes = {std::move(id)}});
            }

            return true;
        }

        constexpr bool try_reduce_as_arg_sequence(TokenStack& tokenStack)
        {
            if (std::optional suffix = match_suffix<ArgSequence, ArgSeparator, Type>(tokenStack))
            {
                auto& [seq, sep, type] = *suffix;

                seq.types.emplace_back(std::move(type));
                tokenStack.resize(tokenStack.size() - 2u);

                return true;
            }

            if (auto* type = match_suffix<Type>(tokenStack))
            {
                ArgSequence seq{};
                seq.types.emplace_back(std::move(*type));
                tokenStack.pop_back();
                tokenStack.emplace_back(std::move(seq));

                return true;
            }

            return false;
        }

        constexpr bool try_reduce_as_template_identifier(TokenStack& tokenStack)
        {
            if (std::optional suffix = match_suffix<Identifier, OpeningAngle, ArgSequence, ClosingAngle>(tokenStack))
            {
                auto& [id, opening, args, closing] = *suffix;
                if (id.is_template())
                {
                    return false;
                }

                id.templateArgs = std::move(args);
                tokenStack.resize(tokenStack.size() - 3u);

                return true;
            }

            if (std::optional suffix = match_suffix<Identifier, OpeningAngle, ClosingAngle>(tokenStack))
            {
                auto& [id, opening, closing] = *suffix;
                if (id.is_template())
                {
                    return false;
                }

                id.templateArgs.emplace();
                tokenStack.resize(tokenStack.size() - 2u);

                return true;
            }

            return false;
        }

        constexpr bool try_reduce_as_regular_type(TokenStack& tokenStack)
        {
            std::span pendingTokens{tokenStack};

            Specs* suffixSpecs{};
            if (auto* specs = match_suffix<Specs>(pendingTokens))
            {
                suffixSpecs = specs;
                remove_suffix(pendingTokens, 1u);
            }

            if (!is_suffix_of<Identifier>(pendingTokens))
            {
                return false;
            }

            RegularType newType{
                .identifier = std::move(std::get<Identifier>(pendingTokens.back()))};
            remove_suffix(pendingTokens, 1u);

            if (suffixSpecs)
            {
                newType.specs = std::move(*suffixSpecs);
            }

            if (auto* seq = match_suffix<ScopeSequence>(pendingTokens))
            {
                newType.scopes = std::move(*seq);
                remove_suffix(pendingTokens, 1u);
            }

            if (auto* prefixSpecs = match_suffix<Specs>(pendingTokens))
            {
                auto& layers = prefixSpecs->layers;
                MIMICPP_ASSERT(1u == layers.size(), "Prefix specs can not have more than one layer.");
                auto& specs = layers.front();
                MIMICPP_ASSERT(!specs.isLValueRef && !specs.isRValueRef && !specs.isNoexcept, "Invalid prefix specs.");
                newType.specs.layers.front().merge(specs);
                remove_suffix(pendingTokens, 1u);
            }

            tokenStack.resize(pendingTokens.size());
            tokenStack.emplace_back(
                std::in_place_type<Type>,
                std::move(newType));

            return true;
        }

        inline bool try_reduce_as_function_type(TokenStack& tokenStack)
        {
            if (auto* funIdentifier = match_suffix<FunctionIdentifier>(tokenStack))
            {
                FunctionType funType{
                    .identifier = std::move(*funIdentifier)};
                tokenStack.pop_back();

                if (auto* scopes = match_suffix<ScopeSequence>(tokenStack))
                {
                    funType.scopes = std::move(*scopes);
                    tokenStack.pop_back();
                }

                if (auto* returnType = match_suffix<Type>(tokenStack))
                {
                    funType.returnType = std::make_shared<Type>(std::move(*returnType));
                    tokenStack.pop_back();
                }

                tokenStack.emplace_back(
                    std::in_place_type<Type>,
                    std::move(funType));

                return true;
            }

            return false;
        }

        constexpr bool try_reduce_as_function_identifier(TokenStack& tokenStack)
        {
            std::span pendingStack{tokenStack};

            Specs* funSpecs{};
            if (auto* specs = match_suffix<Specs>(pendingStack))
            {
                funSpecs = specs;
                remove_suffix(pendingStack, 1u);
            }

            if (!is_suffix_of<ClosingParens>(pendingStack))
            {
                return false;
            }
            remove_suffix(pendingStack, 1u);

            ArgSequence* funArgs{};
            if (auto* args = match_suffix<ArgSequence>(pendingStack))
            {
                funArgs = args;
                remove_suffix(pendingStack, 1u);
            }

            if (!is_suffix_of<OpeningParens>(pendingStack))
            {
                return false;
            }
            remove_suffix(pendingStack, 1u);

            FunctionIdentifier funIdentifier{};
            if (auto* identifier = match_suffix<Identifier>(pendingStack))
            {
                funIdentifier.identifier = std::move(*identifier);
                remove_suffix(pendingStack, 1u);
            }

            if (funSpecs)
            {
                funIdentifier.specs = std::move(*funSpecs);
            }

            if (funArgs)
            {
                funIdentifier.args = std::move(*funArgs);
            }

            tokenStack.resize(pendingStack.size());

            // There may be something similar to a return type in front, which hasn't been reduced yet.
            try_reduce_as_type(tokenStack);

            tokenStack.emplace_back(std::move(funIdentifier));

            return true;
        }

        inline bool try_reduce_as_type(TokenStack& tokenStack)
        {
            bool const isFunction = token::try_reduce_as_function_identifier(tokenStack)
                                 && token::try_reduce_as_function_type(tokenStack);
            return isFunction
                || try_reduce_as_regular_type(tokenStack);
        }

        constexpr void add_specs(Specs::Layer newSpecs, TokenStack& tokenStack)
        {
            if (auto* specs = match_suffix<Specs>(tokenStack))
            {
                auto& layers = specs->layers;
                MIMICPP_ASSERT(!layers.empty(), "Invalid specs state.");
                layers.back().merge(newSpecs);
            }
            else
            {
                tokenStack.emplace_back(
                    Specs{.layers = {std::move(newSpecs)}});
            }
        }

        constexpr void add_specs_layer(TokenStack& tokenStack)
        {
            if (auto* specs = match_suffix<Specs>(tokenStack))
            {
                auto& layers = specs->layers;
                MIMICPP_ASSERT(!layers.empty(), "Invalid specs state.");
                layers.emplace_back();
            }
            else
            {
                tokenStack.emplace_back(
                    Specs{
                        .layers = {2u, Specs::Layer{}}
                });
            }
        }
    }

    template <parser_visitor Visitor>
    class NameParser
    {
    public:
        [[nodiscard]]
        explicit constexpr NameParser(Visitor visitor, StringViewT content) noexcept(std::is_nothrow_move_constructible_v<Visitor>)
            : m_Visitor{std::move(visitor)},
              m_Lexer{std::move(content)}
        {
        }

        constexpr void operator()()
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

        constexpr void handle_lexer_token(lexing::keyword const& keyword)
        {
            if (constKeyword == keyword)
            {
                token::add_specs({.isConst = true}, m_TokenStack);
            }
            else if (volatileKeyword == keyword)
            {
                token::add_specs({.isVolatile = true}, m_TokenStack);
            }
            else if (noexceptKeyword == keyword)
            {
                token::add_specs({.isNoexcept = true}, m_TokenStack);
            }
        }

        constexpr void handle_lexer_token(lexing::operator_or_punctuator const& token)
        {
            if (scopeResolution == token)
            {
                token::try_reduce_as_function_identifier(m_TokenStack);
                token::try_reduce_as_scope_sequence(m_TokenStack);
            }
            else if (commaSeparator == token)
            {
                token::try_reduce_as_type(m_TokenStack)
                    && token::try_reduce_as_arg_sequence(m_TokenStack);

                m_TokenStack.emplace_back(token::ArgSeparator{});
            }
            else if (lvalueRef == token)
            {
                token::add_specs({.isLValueRef = true}, m_TokenStack);
            }
            else if (rvalueRef == token)
            {
                token::add_specs({.isRValueRef = true}, m_TokenStack);
            }
            else if (pointer == token)
            {
                token::add_specs_layer(m_TokenStack);
            }
            else if (openingAngle == token)
            {
                m_TokenStack.emplace_back(token::OpeningAngle{});
            }
            else if (closingAngle == token)
            {
                token::try_reduce_as_type(m_TokenStack)
                    && token::try_reduce_as_arg_sequence(m_TokenStack);

                m_TokenStack.emplace_back(token::ClosingAngle{});
                token::try_reduce_as_template_identifier(m_TokenStack);
            }
            else if (openingParens == token)
            {
                m_TokenStack.emplace_back(token::OpeningParens{});
            }
            else if (closingParens == token)
            {
                token::try_reduce_as_type(m_TokenStack)
                    && token::try_reduce_as_arg_sequence(m_TokenStack);

                m_TokenStack.emplace_back(token::ClosingParens{});
            }
        }
    };
}

#endif
