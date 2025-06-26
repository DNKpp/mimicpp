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

    constexpr void ignore_space(std::span<Token>& tokenStack) noexcept
    {
        if (is_suffix_of<token::Space>(tokenStack))
        {
            remove_suffix(tokenStack, 1u);
        }
    }

    constexpr void ignore_reserved_identifier(std::span<Token>& tokenStack) noexcept
    {
        if (auto const* const id = match_suffix<token::Identifier>(tokenStack);
            id
            && id->is_reserved())
        {
            remove_suffix(tokenStack, 1u);
        }
    }

    namespace token
    {
        bool try_reduce_as_type(TokenStack& tokenStack);

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

        MIMICPP_DETAIL_CONSTEXPR_VECTOR bool try_reduce_as_arg_sequence(TokenStack& tokenStack)
        {
            std::span pendingTokens{tokenStack};
            if (std::optional suffix = match_suffix<ArgSequence, ArgSeparator, Type>(pendingTokens))
            {
                // Keep ArgSequence
                remove_suffix(pendingTokens, 2u);
                auto& [seq, sep, type] = *suffix;

                seq.types.emplace_back(std::move(type));
                tokenStack.resize(pendingTokens.size());

                return true;
            }

            if (auto* type = match_suffix<Type>(pendingTokens))
            {
                remove_suffix(pendingTokens, 1u);

                ArgSequence seq{};
                seq.types.emplace_back(std::move(*type));
                tokenStack.resize(pendingTokens.size());
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

        MIMICPP_DETAIL_CONSTEXPR_VECTOR bool try_reduce_as_function_context(TokenStack& tokenStack)
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

            FunctionContext funCtx{};
            if (args)
            {
                // We omit function args with only `void`.
                if (1u != args->types.size()
                    || !args->types.front().is_void())
                {
                    funCtx.args = std::move(*args);
                }
            }

            tokenStack.resize(pendingTokens.size());
            tokenStack.emplace_back(std::move(funCtx));

            return true;
        }

        inline bool try_reduce_as_function_identifier(TokenStack& tokenStack)
        {
            std::span pendingStack{tokenStack};

            // There may be a space, when the function is wrapped inside single-quotes.
            ignore_space(pendingStack);

            // Ignore something like `__ptr64` on msvc.
            ignore_reserved_identifier(pendingStack);

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
                || is_suffix_of<Type>(tokenStack)
                || is_suffix_of<TypeContext>(tokenStack)
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
                pendingTokens.rbegin(),
                pendingTokens.rend(),
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

            // There may be a space in front of the placeholder, which isn't necessary.
            ignore_space(pendingTokens);

            tokenStack.resize(pendingTokens.size() + 1u);
            tokenStack.back() = Identifier{.content = content};

            return true;
        }

        inline bool try_reduce_as_function_type(TokenStack& tokenStack)
        {
            std::span pendingTokens{tokenStack};

            auto* const ctx = match_suffix<FunctionContext>(pendingTokens);
            if (!ctx)
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            // The return type is always delimited by space from the arg-list.
            if (!is_suffix_of<Space>(pendingTokens))
            {
                // Well, of course there is an exception to the "always".
                // There is that case on msvc, where it does not add that space between the function-args and the call-convention.
                // E.g. `void __cdecl()`.
                // But, as we can be pretty sure from the context, that the identifier can never be the function name, accept it
                // as valid delimiter.
                if (auto const* const id = match_suffix<Identifier>(pendingTokens);
                    !id
                    || !id->is_reserved())
                {
                    return false;
                }
            }
            remove_suffix(pendingTokens, 1u);

            // Ignore call-convention.
            ignore_reserved_identifier(pendingTokens);

            auto* const returnType = match_suffix<Type>(pendingTokens);
            if (!returnType)
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            FunctionType funType{
                .returnType = std::make_shared<Type>(std::move(*returnType)),
                .context = std::move(*ctx)};

            tokenStack.resize(
                std::exchange(pendingTokens, {}).size() + 1u);
            tokenStack.back().emplace<Type>(std::move(funType));

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

            ignore_space(pendingTokens);
            // Ignore call-convention.
            ignore_reserved_identifier(pendingTokens);

            auto* specs = match_suffix<Specs>(pendingTokens);
            ScopeSequence* scopeSeq{};
            if (specs && specs->has_ptr())
            {
                remove_suffix(pendingTokens, 1u);

                if (auto* const seq = match_suffix<ScopeSequence>(pendingTokens))
                {
                    scopeSeq = seq;
                    remove_suffix(pendingTokens, 1u);
                }

                // Ignore call-convention, which may have already been reduced to a type.
                if (auto const* const type = match_suffix<Type>(pendingTokens))
                {
                    if (auto const* const regular = std::get_if<RegularType>(&type->state);
                        regular
                        && regular->identifier.is_reserved())
                    {
                        remove_suffix(pendingTokens, 1u);
                    }
                }
            }
            else
            {
                RegularType* regular{};
                if (auto* const type = match_suffix<Type>(pendingTokens))
                {
                    regular = std::get_if<RegularType>(&type->state);
                }

                // Unfortunately msvc produces something like `(__cdecl*)` for the function-ptr part.
                // There is no way to reliably detect whether denotes a function-ptr or argument-list.
                // So we have to make sure, that the reduction is only called in the right places.
                // Then we can extract the info from that type.
                if (!regular
                    || !regular->identifier.is_reserved()
                    || !regular->specs.has_ptr())
                {
                    return false;
                }

                specs = &regular->specs;
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
            std::span pendingTokens{tokenStack};

            // Ignore something like `__ptr64`.
            ignore_reserved_identifier(pendingTokens);

            // The return type is always delimited by space from the spec part.
            if (std::optional suffix = match_suffix<Type, Space, FunctionPtr, FunctionContext>(pendingTokens))
            {
                remove_suffix(pendingTokens, 4u);
                auto& [returnType, space, ptr, ctx] = *suffix;

                std::optional nestedInfo = std::move(ptr.nested);
                FunctionPtrType ptrType{
                    .returnType = std::make_shared<Type>(std::move(returnType)),
                    .scopes = std::move(ptr.scopes),
                    .specs = std::move(ptr.specs),
                    .context = std::move(ctx)};

                tokenStack.resize(
                    std::exchange(pendingTokens, {}).size() + 1u);
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
                else
                {
                    try_reduce_as_function_type(tokenStack);
                }
            }
        }

        inline bool try_reduce_as_regular_type(TokenStack& tokenStack)
        {
            std::span pendingTokens{tokenStack};
            auto* const identifier = match_suffix<Identifier>(pendingTokens);
            if (!identifier)
            {
                return false;
            }
            remove_suffix(pendingTokens, 1u);

            // There may be the case, where we already reduced a Type but additionally got something like `__ptr64`.
            // E.g. `int& __ptr64`. Remove that trailing identifier and treat it as a successful reduction.
            if (identifier->is_reserved()
                && is_suffix_of<Type>(pendingTokens))
            {
                tokenStack.pop_back();

                return true;
            }

            auto* const scopes = match_suffix<ScopeSequence>(pendingTokens);
            if (scopes)
            {
                remove_suffix(pendingTokens, 1u);
            }

            auto* const prefixSpecs = match_suffix<Specs>(pendingTokens);
            if (prefixSpecs)
            {
                // Prefix-specs can only have `const` and/or `volatile`.
                if (auto const& layers = prefixSpecs->layers;
                    token::Specs::Refness::none != prefixSpecs->refness
                    || prefixSpecs->isNoexcept
                    || 1u != layers.size())
                {
                    return false;
                }

                remove_suffix(pendingTokens, 1u);
            }

            // We do never allow two or more adjacent `Type` tokens, as there is literally no case where this would make sense.
            if (is_suffix_of<Type>(pendingTokens)
                || is_suffix_of<FunctionContext>(pendingTokens))
            {
                return false;
            }

            RegularType newType{.identifier = std::move(*identifier)};
            if (prefixSpecs)
            {
                newType.specs = std::move(*prefixSpecs);
            }

            if (scopes)
            {
                newType.scopes = std::move(*scopes);
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
            return try_reduce_as_function_ptr_type(tokenStack)
                || try_reduce_as_function_type(tokenStack)
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

                // Ignore call-convention.
                ignore_reserved_identifier(pendingTokens);

                if (auto* returnType = match_suffix<Type>(pendingTokens))
                {
                    function.returnType = std::make_shared<Type>(std::move(*returnType));
                    remove_suffix(pendingTokens, 1u);
                }

                tokenStack.resize(
                    std::exchange(pendingTokens, {}).size());
                tokenStack.emplace_back(std::move(function));

                return true;
            }

            return false;
        }

        inline void reduce_as_conversion_operator_function_identifier(TokenStack& tokenStack)
        {
            // Functions reported by stacktrace are sometimes not in actual function form,
            // so we need to be more permissive here.
            std::optional<FunctionContext> funCtx{};
            if (auto* const ctx = match_suffix<FunctionContext>(tokenStack))
            {
                funCtx = std::move(*ctx);
                tokenStack.pop_back();
            }

            try_reduce_as_type(tokenStack);
            MIMICPP_ASSERT(is_suffix_of<Type>(tokenStack), "Invalid state");
            auto targetType = std::make_shared<Type>(
                std::get<Type>(std::move(tokenStack.back())));
            tokenStack.pop_back();

            MIMICPP_ASSERT(is_suffix_of<OperatorKeyword>(tokenStack), "Invalid state");
            tokenStack.back() = Identifier{
                .content = Identifier::OperatorInfo{.symbol = std::move(targetType)}};

            if (funCtx)
            {
                tokenStack.emplace_back(*std::move(funCtx));
                try_reduce_as_function_identifier(tokenStack);
            }
        }

        [[nodiscard]]
        constexpr Specs& get_or_emplace_specs(TokenStack& tokenStack)
        {
            // Maybe wo got something like `type&` and need to reduce that identifier to an actual `Type` token.
            if (is_suffix_of<Identifier>(tokenStack))
            {
                if (try_reduce_as_type(tokenStack))
                {
                    return std::get<Type>(tokenStack.back()).specs();
                }

                // The reduction failed, so it's something like `__ptr64` and should be ignored.
                // This may happen, when specs are attached to functions, like `ret T::foo() & __ptr64`.
                MIMICPP_ASSERT(std::get<Identifier>(tokenStack.back()).is_reserved(), "Unexpected token.");
                tokenStack.pop_back();
            }

            if (auto* const type = match_suffix<Type>(tokenStack))
            {
                return type->specs();
            }

            if (auto* const ctx = match_suffix<FunctionContext>(tokenStack))
            {
                return ctx->specs;
            }

            if (auto* specs = match_suffix<Specs>(tokenStack))
            {
                return *specs;
            }

            // No specs found yet? Assume prefix specs.
            return std::get<Specs>(tokenStack.emplace_back(Specs{}));
        }
    }
}

#endif
