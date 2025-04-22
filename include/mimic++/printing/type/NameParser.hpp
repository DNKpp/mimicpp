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
                                 visitor.unrecognized(content);

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

                                 visitor.begin_function_ptr();
                                 visitor.end_function_ptr();
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
        };

        class FunctionArgs
        {
        public:
            ArgSequence args{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.begin_function_args();
                std::invoke(args, unwrapped);
                unwrapped.end_function_args();
            }
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

        class FunctionContext
        {
        public:
            FunctionArgs args{};
            Specs specs{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                std::invoke(args, unwrapped);
                std::invoke(specs, unwrapped);
            }
        };

        class FunctionIdentifier
        {
        public:
            Identifier identifier;
            FunctionContext context{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                std::invoke(identifier, unwrapped);
                std::invoke(context, unwrapped);
            }
        };

        class ScopeSequence
        {
        public:
            using Scope = std::variant<Identifier, FunctionIdentifier>;
            std::vector<Scope> scopes{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(!scopes.empty(), "Empty scope-sequence is not allowed.");

                auto& unwrapped = unwrap_visitor(visitor);

                for (auto const& scope : scopes)
                {
                    unwrapped.begin_scope();
                    std::visit(
                        [&](auto const& id) { handle_scope(unwrapped, id); },
                        scope);
                    unwrapped.end_scope();
                }
            }

        private:
            template <parser_visitor Visitor>
            constexpr void handle_scope(Visitor& visitor, Identifier const& scope) const
            {
                std::invoke(scope, visitor);
            }

            template <parser_visitor Visitor>
            constexpr void handle_scope(Visitor& visitor, FunctionIdentifier const& scope) const
            {
                visitor.begin_function();
                std::invoke(scope, visitor);
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

        class FunctionPtr
        {
        public:
            std::optional<ScopeSequence> scopes{};
            Specs specs{};

            struct NestedInfo
            {
                std::shared_ptr<FunctionPtr> ptr{};
                FunctionContext ctx{};
            };

            std::optional<NestedInfo> nested{};
        };

        class FunctionPtrType
        {
        public:
            std::shared_ptr<Type> returnType{};
            std::optional<ScopeSequence> scopes{};
            Specs specs{};
            FunctionContext context{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(returnType, "Return type is mandatory for function-ptrs.");

                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.begin_return_type();
                std::invoke(*returnType, visitor);
                unwrapped.end_return_type();

                unwrapped.begin_function_ptr();
                if (scopes)
                {
                    std::invoke(*scopes, unwrapped);
                }

                std::invoke(specs, unwrapped);
                unwrapped.end_function_ptr();

                std::invoke(context, unwrapped);
            }
        };

        class Type
        {
        public:
            using State = std::variant<RegularType, FunctionPtrType>;
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

        class FunctionType
        {
        public:
            Type returnType{};
            FunctionContext context{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.begin_type();
                unwrapped.begin_function();

                unwrapped.begin_return_type();
                std::invoke(returnType, visitor);
                unwrapped.end_return_type();

                std::invoke(context, unwrapped);

                unwrapped.end_function();
                unwrapped.end_type();
            }
        };

        class Function
        {
        public:
            std::shared_ptr<Type> returnType{};
            std::optional<ScopeSequence> scopes{};
            FunctionIdentifier identifier{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.begin_type();
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
                unwrapped.end_type();
            }
        };

        class End
        {
        public:
            using State = std::variant<Type, FunctionType, Function>;
            State state{};

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                auto& unwrapped = unwrap_visitor(visitor);

                unwrapped.begin();
                std::visit(
                    [&](auto const& s) { std::invoke(s, unwrapped); },
                    state);
                unwrapped.end();
            }
        };
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
        token::FunctionIdentifier,
        token::ScopeSequence,
        token::ArgSequence,
        token::FunctionArgs,
        token::FunctionContext,
        token::FunctionPtr,
        token::Specs,
        token::Type,
        token::End>;
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
            else
            {
                return true;
            }
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

        inline bool try_reduce_as_scope_sequence(TokenStack& tokenStack)
        {
            ScopeSequence::Scope scope{};
            if (auto* identifier = match_suffix<Identifier>(tokenStack))
            {
                scope = std::move(*identifier);
            }
            else if (auto* funIdentifier = match_suffix<FunctionIdentifier>(tokenStack))
            {
                scope = std::move(*funIdentifier);
            }
            else
            {
                return false;
            }

            tokenStack.pop_back();

            if (auto* sequence = match_suffix<ScopeSequence>(tokenStack))
            {
                sequence->scopes.emplace_back(std::move(scope));
            }
            else
            {
                tokenStack.emplace_back(
                    ScopeSequence{
                        .scopes = {std::move(scope)}});
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

        constexpr bool try_reduce_as_function_args(TokenStack& tokenStack)
        {
            std::span pendingTokens{tokenStack};
            if (!is_suffix_of<ClosingParens>(pendingTokens))
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            auto* args = match_suffix<ArgSequence>(pendingTokens);
            if (args)
            {
                remove_suffix(pendingTokens, 1u);
            }

            if (!is_suffix_of<OpeningParens>(pendingTokens))
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            FunctionArgs funArgs{};
            if (args)
            {
                funArgs.args = std::move(*args);
            }

            tokenStack.resize(pendingTokens.size());
            tokenStack.emplace_back(std::move(funArgs));

            return true;
        }

        constexpr bool try_reduce_as_function_context(TokenStack& tokenStack)
        {
            std::span pendingStack{tokenStack};

            Specs* funSpecs{};
            if (auto* specs = match_suffix<Specs>(pendingStack))
            {
                funSpecs = specs;
                remove_suffix(pendingStack, 1u);
            }

            if (auto* funArgs = match_suffix<FunctionArgs>(pendingStack))
            {
                remove_suffix(pendingStack, 1u);

                FunctionContext funContext{
                    .args = std::move(*funArgs)};

                if (funSpecs)
                {
                    funContext.specs = std::move(*funSpecs);
                }

                tokenStack.resize(pendingStack.size());
                tokenStack.emplace_back(std::move(funContext));

                return true;
            }

            return false;
        }

        constexpr bool try_reduce_as_function_identifier(TokenStack& tokenStack)
        {
            if (std::optional suffix = match_suffix<Identifier, FunctionContext>(tokenStack))
            {
                auto& [identifier, funCtx] = *suffix;
                FunctionIdentifier funIdentifier{
                    .identifier = std::move(identifier),
                    .context = std::move(funCtx)};

                tokenStack.pop_back();
                tokenStack.back() = std::move(funIdentifier);

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

        inline bool try_reduce_as_function_ptr(TokenStack& tokenStack)
        {
            std::span pendingTokens{tokenStack};
            if (!is_suffix_of<ClosingParens>(pendingTokens))
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            auto* nestedFunCtx = match_suffix<FunctionContext>(pendingTokens);
            FunctionPtr* nestedFunPtr{};
            if (nestedFunCtx)
            {
                remove_suffix(pendingTokens, 1u);

                if (auto* ptr = match_suffix<FunctionPtr>(pendingTokens))
                {
                    nestedFunPtr = ptr;
                    remove_suffix(pendingTokens, 1u);
                }
            }

            auto* specs = match_suffix<Specs>(pendingTokens);
            if (!specs
                || !specs->has_ptr())
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            auto* scopeSeq = match_suffix<ScopeSequence>(pendingTokens);
            if (match_suffix<ScopeSequence>(pendingTokens))
            {
                remove_suffix(pendingTokens, 1u);
            }

            if (!is_suffix_of<OpeningParens>(pendingTokens))
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            FunctionPtr funPtr{.specs = std::move(*specs)};
            if (scopeSeq)
            {
                funPtr.scopes = std::move(*scopeSeq);
            }

            if (nestedFunCtx)
            {
                FunctionPtr::NestedInfo nested{
                    .ctx = std::move(*nestedFunCtx)};

                if (nestedFunPtr)
                {
                    nested.ptr = std::make_shared<FunctionPtr>(std::move(*nestedFunPtr));
                }

                funPtr.nested = std::move(nested);
            }

            tokenStack.resize(pendingTokens.size());
            tokenStack.emplace_back(std::move(funPtr));

            return true;
        }

        namespace detail
        {
            void handled_nested_function_ptr(TokenStack& tokenStack, FunctionPtr::NestedInfo info);
        }

        inline bool try_reduce_as_function_ptr_type(TokenStack& tokenStack)
        {
            if (std::optional suffix = match_suffix<Type, Space, FunctionPtr, FunctionContext>(tokenStack))
            {
                auto& [returnType, space, ptr, ctx] = *suffix;

                std::optional nestedInfo = std::move(ptr.nested);
                FunctionPtrType ptrType{
                    .returnType = std::make_shared<Type>(std::move(returnType)),
                    .scopes = std::move(ptr.scopes),
                    .specs = std::move(ptr.specs),
                    .context = std::move(ctx)};

                tokenStack.resize(tokenStack.size() - 3);
                tokenStack.back().emplace<Type>(std::move(ptrType));

                // We got something like `ret (*(outer-args))(args)` or `ret (*(*)(outer-args))(args)`, where the currently
                // processed function-ptr is actually the return-type of the inner function(-ptr).
                // This may nested in an arbitrary depth!
                if (nestedInfo)
                {
                    detail::handled_nested_function_ptr(tokenStack, *std::move(nestedInfo));
                }

                return true;
            }

            return false;
        }

        namespace detail
        {
            inline void handled_nested_function_ptr(TokenStack& tokenStack, FunctionPtr::NestedInfo info)
            {
                auto& [ptr, ctx] = info;

                // We need to insert an extra space, to follow the general syntax constraints.
                tokenStack.emplace_back(Space{});

                bool const isFunPtr{ptr};
                if (ptr)
                {
                    tokenStack.emplace_back(std::move(*ptr));
                }

                tokenStack.emplace_back(std::move(ctx));

                if (isFunPtr)
                {
                    try_reduce_as_function_ptr_type(tokenStack);
                }
            }
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

        inline bool try_reduce_as_type(TokenStack& tokenStack)
        {
            token::try_reduce_as_function_context(tokenStack);

            return try_reduce_as_function_ptr_type(tokenStack)
                || try_reduce_as_regular_type(tokenStack);
        }

        inline bool try_reduce_as_function(TokenStack& tokenStack)
        {
            std::span pendingTokens{tokenStack};
            if (auto* funIdentifier = match_suffix<FunctionIdentifier>(pendingTokens))
            {
                Function function{
                    .identifier = std::move(*funIdentifier)};
                remove_suffix(pendingTokens, 1u);

                if (auto* scopes = match_suffix<ScopeSequence>(pendingTokens))
                {
                    function.scopes = std::move(*scopes);
                    remove_suffix(pendingTokens, 1u);
                }

                // When ScopeSequence or Identifier starts with a placeholder-like, there will be an additional space-token.
                ignore_space(pendingTokens);
                if (auto* returnType = match_suffix<Type>(pendingTokens))
                {
                    function.returnType = std::make_shared<Type>(std::move(*returnType));
                    remove_suffix(pendingTokens, 1u);
                }

                tokenStack.resize(
                    std::exchange(pendingTokens, {}).size());

                // There may be something similar to a return type in front, which hasn't been reduced yet.
                if (!function.returnType
                    && try_reduce_as_type(tokenStack))
                {
                    function.returnType = std::make_shared<Type>(
                        std::get<Type>(std::move(tokenStack.back())));
                    tokenStack.pop_back();
                }

                tokenStack.emplace_back(
                    std::in_place_type<End>,
                    std::move(function));

                return true;
            }

            return false;
        }

        constexpr bool try_reduce_as_function_type(TokenStack& tokenStack)
        {
            // The space is required, because the return type will always be spaced away from the parens.
            if (std::optional suffix = match_suffix<Type, Space, FunctionContext>(tokenStack))
            {
                auto& [returnType, space, funCtx] = *suffix;
                FunctionType funType{
                    .returnType = std::move(returnType),
                    .context = std::move(funCtx)};

                tokenStack.resize(tokenStack.size() - 2);
                tokenStack.back().emplace<End>(std::move(funType));

                return true;
            }

            return false;
        }

        inline bool try_reduce_as_end(TokenStack& tokenStack)
        {
            try_reduce_as_function_context(tokenStack);

            if (is_suffix_of<FunctionIdentifier>(tokenStack)
                || try_reduce_as_function_identifier(tokenStack))
            {
                return try_reduce_as_function(tokenStack);
            }

            if (is_suffix_of<Type>(tokenStack)
                || try_reduce_as_type(tokenStack))
            {
                // Do to some function-ptr reductions, there may be no actual `type`-token present.
                // If not, it's probably a function-type.
                if (auto* type = std::get_if<Type>(&tokenStack.back()))
                {
                    tokenStack.back().emplace<End>(
                        std::exchange(*type, {}));

                    return true;
                }
            }

            return try_reduce_as_function_type(tokenStack);
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
        explicit constexpr NameParser(Visitor visitor, StringViewT const& content) noexcept(std::is_nothrow_move_constructible_v<Visitor>)
            : m_Visitor{std::move(visitor)},
              m_Content{content},
              m_Lexer{content}
        {
        }

        constexpr void operator()()
        {
            for (lexing::token next = m_Lexer.next();
                 !std::holds_alternative<lexing::end>(next.classification);
                 next = m_Lexer.next())
            {
                std::visit(
                    [&](auto const& tokenClass) { handle_lexer_token(next.content, tokenClass); },
                    next.classification);
            }

            try_reduce_as_end(m_TokenStack);

            if (1u == m_TokenStack.size()
                && std::holds_alternative<token::End>(m_TokenStack.back()))
            {
                std::invoke(
                    std::get<token::End>(m_TokenStack.back()),
                    m_Visitor);
            }
            else
            {
                unwrap_visitor(m_Visitor).unrecognized(m_Content);
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
        static constexpr lexing::keyword operatorKeyword{"operator"};
        static constexpr lexing::keyword constKeyword{"const"};
        static constexpr lexing::keyword volatileKeyword{"volatile"};
        static constexpr lexing::keyword noexceptKeyword{"noexcept"};

        Visitor m_Visitor;
        StringViewT m_Content;
        lexing::NameLexer m_Lexer;

        std::vector<Token> m_TokenStack{};

        static constexpr void handle_lexer_token([[maybe_unused]] StringViewT const content, [[maybe_unused]] lexing::end const& end)
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
                token::try_reduce_as_function_context(m_TokenStack)
                    && token::try_reduce_as_function_identifier(m_TokenStack);
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

                token::try_reduce_as_function_args(m_TokenStack)
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
                m_TokenStack.emplace_back(
                    std::in_place_type<token::ClosingSingleQuote>,
                    content);
                token::try_reduce_as_placeholder_identifier_wrapped<token::OpeningBacktick, token::ClosingSingleQuote>(m_TokenStack);
            }
        }
    };
}

#endif
