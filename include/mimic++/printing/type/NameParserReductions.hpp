//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_NAME_PARSER_REDUCTIONS_HPP
#define MIMICPP_PRINTING_TYPE_NAME_PARSER_REDUCTIONS_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/type/NameParserTokens.hpp"
#include "mimic++/utilities/TypeList.hpp"

#include <optional>
#include <span>
#include <utility>
#include <variant>

namespace mimicpp::printing::type::parsing
{
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
            std::span pendingTokens{tokenStack};
            if (!is_suffix_of<ScopeResolution>(pendingTokens))
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            ScopeSequence::Scope scope{};
            if (auto* identifier = match_suffix<Identifier>(pendingTokens))
            {
                scope = std::move(*identifier);
            }
            else if (auto* funIdentifier = match_suffix<FunctionIdentifier>(pendingTokens))
            {
                scope = std::move(*funIdentifier);
            }
            else
            {
                return false;
            }

            remove_suffix(pendingTokens, 1u);
            tokenStack.resize(pendingTokens.size());

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
            std::span pendingTokens{tokenStack};
            if (!is_suffix_of<ClosingAngle>(pendingTokens))
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            auto* args = match_suffix<ArgSequence>(pendingTokens);
            if (args)
            {
                remove_suffix(pendingTokens, 1u);
            }

            if (!is_suffix_of<OpeningAngle>(pendingTokens))
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            auto* id = match_suffix<Identifier>(pendingTokens);
            if (!id
                || id->is_template())
            {
                return false;
            }

            if (args)
            {
                id->templateArgs = std::move(*args);
            }
            else
            {
                id->templateArgs.emplace();
            }
            tokenStack.resize(pendingTokens.size());

            return true;
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

            // There can never be valid function-args in form of `::()`, thus reject it.
            if (is_suffix_of<ScopeResolution>(pendingTokens)
                || is_suffix_of<ScopeSequence>(pendingTokens))
            {
                return false;
            }

            FunctionArgs funArgs{};
            if (args)
            {
                // We omit function args with only `void`.
                if (1u != args->types.size()
                    || !args->types.front().is_void())
                {
                    funArgs.args = std::move(*args);
                }
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

        inline bool try_reduce_as_function_identifier(TokenStack& tokenStack)
        {
            std::span pendingStack{tokenStack};
            ignore_space(pendingStack);

            if (std::optional suffix = match_suffix<Identifier, FunctionContext>(pendingStack))
            {
                remove_suffix(pendingStack, 2u);

                auto& [identifier, funCtx] = *suffix;
                FunctionIdentifier funIdentifier{
                    .identifier = std::move(identifier),
                    .context = std::move(funCtx)};

                tokenStack.resize(pendingStack.size() + 1u);
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
                || is_suffix_of<OpeningParens>(tokenStack)
                || is_suffix_of<OpeningBacktick>(tokenStack);
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

            ignore_space(pendingTokens);

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

        inline bool try_reduce_as_regular_type(TokenStack& tokenStack)
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
                MIMICPP_ASSERT(token::Specs::Refness::none == prefixSpecs->refness && !prefixSpecs->isNoexcept, "Invalid prefix specs.");
                MIMICPP_ASSERT(1u == layers.size(), "Prefix specs can not have more than one layer.");

                newType.specs.layers.front().merge(layers.front());
                remove_suffix(pendingTokens, 1u);
            }

            // Ignore something like `class` or `struct` directly in front of a type.
            if (is_suffix_of<TypeContext>(pendingTokens))
            {
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

        inline void reduce_as_conversion_operator_function_identifier(TokenStack& tokenStack)
        {
            MIMICPP_ASSERT(is_suffix_of<FunctionContext>(tokenStack), "Invalid state");
            auto funCtx = std::get<FunctionContext>(std::move(tokenStack.back()));
            tokenStack.pop_back();

            try_reduce_as_type(tokenStack);
            MIMICPP_ASSERT(is_suffix_of<Type>(tokenStack), "Invalid state");
            auto targetType = std::make_shared<Type>(
                std::get<Type>(std::move(tokenStack.back())));
            tokenStack.pop_back();

            MIMICPP_ASSERT(is_suffix_of<OperatorKeyword>(tokenStack), "Invalid state");
            tokenStack.back().emplace<Identifier>(
                Identifier::OperatorInfo{.symbol = std::move(targetType)});
            tokenStack.emplace_back(std::move(funCtx));
            try_reduce_as_function_identifier(tokenStack);
        }

        inline bool try_reduce_as_function_type(TokenStack& tokenStack)
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

        inline bool try_reduce_as_end(TokenStack& tokenStack, bool const expectsConversionOperator)
        {
            try_reduce_as_function_context(tokenStack);

            if (expectsConversionOperator)
            {
                reduce_as_conversion_operator_function_identifier(tokenStack);

                return try_reduce_as_function(tokenStack);
            }

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

        [[nodiscard]]
        constexpr Specs& get_or_emplace_specs(TokenStack& tokenStack)
        {
            if (auto* specs = match_suffix<Specs>(tokenStack))
            {
                return *specs;
            }

            return std::get<Specs>(tokenStack.emplace_back(Specs{}));
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
}

#endif
