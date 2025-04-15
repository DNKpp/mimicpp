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

        class Space
        {
        };

        class ArgSeparator
        {
        public:
            StringViewT content;
        };

        class OpeningAngle
        {
        public:
            StringViewT content;
        };

        class ClosingAngle
        {
        public:
            StringViewT content;
        };

        class OpeningParens
        {
        public:
            StringViewT content;
        };

        class ClosingParens
        {
        public:
            StringViewT content;
        };

        class OpeningCurly
        {
        public:
            StringViewT content;
        };

        class ClosingCurly
        {
        public:
            StringViewT content;
        };

        class OpeningBacktick
        {
        public:
            StringViewT content;
        };

        class ClosingSingleQuote
        {
        public:
            StringViewT content;
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

            struct FunctionInfo
            {
                ArgSequence args;
                Specs specs{};
            };

            std::optional<FunctionInfo> functionInfo{};

            [[nodiscard]]
            constexpr bool is_template() const noexcept
            {
                return templateArgs.has_value();
            }

            [[nodiscard]]
            constexpr bool is_function() const noexcept
            {
                return functionInfo.has_value();
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

                if (functionInfo)
                {
                    functionInfo->args.handle_as_function_args(unwrapped);
                    std::invoke(functionInfo->specs, unwrapped);
                }
            }
        };

        class ScopeSequence
        {
        public:
            std::vector<Identifier> scopes{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(!scopes.empty(), "Empty scope-sequence is not allowed.");

                auto& unwrapped = unwrap_visitor(visitor);

                for (auto const& scope : scopes)
                {
                    unwrapped.begin_scope();
                    const bool isFunction = scope.is_function();
                    if (isFunction)
                    {
                        unwrapped.begin_function();
                    }

                    std::invoke(scope, unwrapped);

                    if (isFunction)
                    {
                        unwrapped.end_function();
                    }
                    unwrapped.end_scope();
                }
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
            Identifier identifier{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(identifier.is_function(), "Identifier must denote a function.");

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

                // Handle identifier manually, as it requires that the top-level name is not empty, which may be
                // violated when an actual function-type is provided.

                if (auto const& name = identifier.content;
                    !name.empty())
                {
                    unwrapped.add_identifier(name);
                }

                if (auto const& templateArgs = identifier.templateArgs)
                {
                    templateArgs->handle_as_template_args(unwrapped);
                }

                auto const& functionInfo = *identifier.functionInfo;
                functionInfo.args.handle_as_function_args(unwrapped);
                std::invoke(functionInfo.specs, unwrapped);

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
        token::Space,
        token::ArgSeparator,
        token::OpeningAngle,
        token::ClosingAngle,
        token::OpeningParens,
        token::ClosingParens,
        token::OpeningCurly,
        token::ClosingCurly,
        token::OpeningBacktick,
        token::ClosingSingleQuote,

        token::Identifier,
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

        constexpr void ignore_space(std::span<Token>& tokenStack) noexcept
        {
            if (is_suffix_of<Space>(tokenStack))
            {
                remove_suffix(tokenStack, 1u);
            }
        }

        constexpr bool try_reduce_as_scope_sequence(TokenStack& tokenStack)
        {
            if (!is_suffix_of<Identifier>(tokenStack))
            {
                return false;
            }

            auto identifier = std::get<Identifier>(std::move(tokenStack.back()));
            tokenStack.pop_back();

            if (auto* sequence = match_suffix<ScopeSequence>(tokenStack))
            {
                sequence->scopes.emplace_back(std::move(identifier));
            }
            else
            {
                tokenStack.emplace_back(
                    ScopeSequence{
                        .scopes = {std::move(identifier)}});
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

        [[nodiscard]]
        constexpr bool is_identifier_prefix(std::span<Token const> const tokenStack) noexcept
        {
            return tokenStack.empty()
                || is_suffix_of<Space>(tokenStack)
                || is_suffix_of<ScopeSequence>(tokenStack)
                || is_suffix_of<Specs>(tokenStack)
                || is_suffix_of<OpeningAngle>(tokenStack)
                || is_suffix_of<OpeningParens>(tokenStack);
        }

        template <token_type Opening, token_type Closing>
        constexpr bool try_reduce_as_placeholder_identifier_wrapped(TokenStack& tokenStack)
        {
            MIMICPP_ASSERT(is_suffix_of<Closing>(tokenStack), "Token-stack does not have the closing token as top.");
            std::span pendingTokens{tokenStack.begin(), tokenStack.end() - 1};

            auto const openingIter = std::ranges::find_if(
                pendingTokens | std::views::reverse,
                [](Token const& token) noexcept { return std::holds_alternative<Opening>(token); });
            if (openingIter == pendingTokens.rend()
                || !is_identifier_prefix({pendingTokens.begin(), openingIter.base() - 1}))
            {
                return false;
            }

            // Just treat everything between the opening and closing as placeholder identifier.
            auto const& opening = std::get<Opening>(*std::ranges::prev(openingIter.base(), 1));
            auto const& closing = std::get<Closing>(tokenStack.back());
            auto const contentLength = (closing.content.data() - opening.content.data()) + closing.content.size();
            StringViewT const content{opening.content.data(), contentLength};

            pendingTokens = std::span{pendingTokens.begin(), openingIter.base() - 1};
            tokenStack.resize(pendingTokens.size());
            tokenStack.emplace_back(Identifier{.content = content});

            return true;
        }

        constexpr bool try_reduce_as_placeholder_identifier(TokenStack& tokenStack)
        {
            if (is_suffix_of<ClosingParens>(tokenStack))
            {
                return try_reduce_as_placeholder_identifier_wrapped<OpeningParens, ClosingParens>(tokenStack);
            }

            if (is_suffix_of<ClosingAngle>(tokenStack))
            {
                return try_reduce_as_placeholder_identifier_wrapped<OpeningAngle, ClosingAngle>(tokenStack);
            }

            if (is_suffix_of<ClosingSingleQuote>(tokenStack))
            {
                return try_reduce_as_placeholder_identifier_wrapped<OpeningBacktick, ClosingSingleQuote>(tokenStack);
            }

            if (is_suffix_of<ClosingCurly>(tokenStack))
            {
                return try_reduce_as_placeholder_identifier_wrapped<OpeningCurly, ClosingCurly>(tokenStack);
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

            // ignore_space(pendingTokens);
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

            // When ScopeSequence or Identifier starts with a placeholder-like, there will be an additional space-token.
            ignore_space(pendingTokens);
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

        inline bool try_reduce_as_named_function(TokenStack& tokenStack)
        {
            std::span pendingTokens{tokenStack};
            if (auto* funIdentifier = match_suffix<Identifier>(pendingTokens);
                funIdentifier
                && funIdentifier->is_function())
            {
                FunctionType funType{
                    .identifier = std::move(*funIdentifier)};
                remove_suffix(pendingTokens, 1u);

                if (auto* scopes = match_suffix<ScopeSequence>(pendingTokens))
                {
                    funType.scopes = std::move(*scopes);
                    remove_suffix(pendingTokens, 1u);
                }

                // When ScopeSequence or Identifier starts with a placeholder-like, there will be an additional space-token.
                ignore_space(pendingTokens);
                if (auto* returnType = match_suffix<Type>(pendingTokens))
                {
                    funType.returnType = std::make_shared<Type>(std::move(*returnType));
                    remove_suffix(pendingTokens, 1u);
                }

                tokenStack.resize(
                    std::exchange(pendingTokens, {}).size());

                // There may be something similar to a return type in front, which hasn't been reduced yet.
                if (!funType.returnType
                    && try_reduce_as_type(tokenStack))
                {
                    funType.returnType = std::make_shared<Type>(
                        std::get<Type>(std::move(tokenStack.back())));
                    tokenStack.pop_back();
                }

                tokenStack.emplace_back(
                    std::in_place_type<Type>,
                    std::move(funType));

                return true;
            }

            return false;
        }

        inline bool try_reduce_as_unnamed_function(TokenStack& tokenStack)
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

            // The space is required, because the return type will always be spaced away from the parens.
            if (!is_suffix_of<Type, Space, OpeningParens>(pendingStack))
            {
                return false;
            }
            // Remove just the space and opening-parens, as we need to consume the type next.
            remove_suffix(pendingStack, 2u);

            FunctionType funType{
                .returnType = std::make_shared<Type>(
                    std::get<Type>(
                        std::move(pendingStack.back())))};
            remove_suffix(pendingStack, 1u);

            auto& funIdentifier = funType.identifier.functionInfo.emplace();
            if (funSpecs)
            {
                funIdentifier.specs = std::move(*funSpecs);
            }

            if (funArgs)
            {
                funIdentifier.args = std::move(*funArgs);
            }

            tokenStack.resize(pendingStack.size());
            tokenStack.emplace_back(
                std::in_place_type<Type>,
                std::move(funType));

            return true;
        }

        inline bool try_reduce_as_function_type(TokenStack& tokenStack)
        {
            return try_reduce_as_named_function(tokenStack)
                || try_reduce_as_unnamed_function(tokenStack);
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

            auto* funIdentifier = match_suffix<Identifier>(pendingStack);
            if (!funIdentifier
                || funIdentifier->is_function())
            {
                return false;
            }

            auto& funInfo = funIdentifier->functionInfo.emplace();

            if (funSpecs)
            {
                funInfo.specs = std::move(*funSpecs);
            }

            if (funArgs)
            {
                funInfo.args = std::move(*funArgs);
            }

            tokenStack.resize(pendingStack.size());

            return true;
        }

        inline bool try_reduce_as_type(TokenStack& tokenStack)
        {
            try_reduce_as_function_identifier(tokenStack);

            return try_reduce_as_function_type(tokenStack)
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
                    [&](auto const& tokenClass) { handle_lexer_token(next.content, tokenClass); },
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

        constexpr void handle_lexer_token([[maybe_unused]] StringViewT const content, [[maybe_unused]] lexing::end const& end)
        {
            util::unreachable();
        }

        constexpr void handle_lexer_token([[maybe_unused]] StringViewT const content, [[maybe_unused]] lexing::space const& space)
        {
            // In some cases, a space after an identifier carries semantic meaning.
            // I.e. consider these two type-names: `void ()` and `foo()`,
            // where the former is a type returning `void` and the latter is a function named `foo`.
            // In fact keep all spaces directly before all opening tokens.
            if (auto const* nextOp = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                nextOp
                && util::contains(std::array{openingParens, openingAngle, openingCurly, openingSquare, backtick}, *nextOp))
            {
                token::try_reduce_as_type(m_TokenStack);
                m_TokenStack.emplace_back(token::Space{});
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

        constexpr void handle_lexer_token(StringViewT const content, lexing::operator_or_punctuator const& token)
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

                m_TokenStack.emplace_back(
                    std::in_place_type<token::ArgSeparator>,
                    content);
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
                // Handle something like `{...}<Args>`.
                //                             ^
                token::try_reduce_as_placeholder_identifier(m_TokenStack);

                m_TokenStack.emplace_back(
                    std::in_place_type<token::OpeningAngle>,
                    content);
            }
            else if (closingAngle == token)
            {
                token::try_reduce_as_type(m_TokenStack)
                    && token::try_reduce_as_arg_sequence(m_TokenStack);

                m_TokenStack.emplace_back(
                    std::in_place_type<token::ClosingAngle>,
                    content);
                token::try_reduce_as_template_identifier(m_TokenStack)
                    || token::try_reduce_as_placeholder_identifier_wrapped<token::OpeningAngle, token::ClosingAngle>(m_TokenStack);
            }
            else if (openingParens == token)
            {
                // Handle something like `void {...}(Args)`.
                //                                  ^
                token::try_reduce_as_placeholder_identifier(m_TokenStack);

                m_TokenStack.emplace_back(
                    std::in_place_type<token::OpeningParens>,
                    content);
            }
            else if (closingParens == token)
            {
                token::try_reduce_as_type(m_TokenStack)
                    && token::try_reduce_as_arg_sequence(m_TokenStack);

                m_TokenStack.emplace_back(
                    std::in_place_type<token::ClosingParens>,
                    content);
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
                m_TokenStack.emplace_back(
                    std::in_place_type<token::ClosingSingleQuote>,
                    content);
                token::try_reduce_as_placeholder_identifier_wrapped<token::OpeningBacktick, token::ClosingSingleQuote>(m_TokenStack);
            }
        }
    };
}

#endif
