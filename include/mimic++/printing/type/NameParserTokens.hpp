//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_NAME_PARSER_TOKENS_HPP
#define MIMICPP_PRINTING_TYPE_NAME_PARSER_TOKENS_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/Concepts.hpp"

#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <variant>
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

                                 visitor.begin_operator_identifier();
                                 visitor.end_operator_identifier();
                             };

    template <parser_visitor Visitor>
    [[nodiscard]]
    constexpr auto& unwrap_visitor(Visitor& visitor) noexcept
    {
        return static_cast<
            std::add_lvalue_reference_t<
                std::unwrap_reference_t<Visitor>>>(visitor);
    }
}

namespace mimicpp::printing::type::parsing::token
{
    class Type;

    class Space
    {
    };

    class OperatorKeyword
    {
    };

    class ScopeResolution
    {
    public:
        StringViewT content;
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

    class TypeContext
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

            constexpr void merge(Layer const& others) noexcept
            {
                MIMICPP_ASSERT(!(isConst && others.isConst), "Merging same specs.");
                MIMICPP_ASSERT(!(isVolatile && others.isVolatile), "Merging same specs.");

                isConst = isConst || others.isConst;
                isVolatile = isVolatile || others.isVolatile;
            }

            template <parser_visitor Visitor>
            constexpr void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                if (isConst)
                {
                    inner.add_const();
                }

                if (isVolatile)
                {
                    inner.add_volatile();
                }
            }
        };

        std::vector<Layer> layers{1u};

        enum Refness : std::uint8_t
        {
            none,
            lvalue,
            rvalue
        };

        Refness refness{none};
        bool isNoexcept{false};

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

            switch (refness)
            {
            case lvalue:
                unwrapped.add_lvalue_ref();
                break;

            case rvalue:
                unwrapped.add_rvalue_ref();
                break;

            case none: [[fallthrough]];
            default:   break;
            }

            if (isNoexcept)
            {
                unwrapped.add_noexcept();
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
        struct OperatorInfo
        {
            using Symbol = std::variant<StringViewT, std::shared_ptr<Type>>;
            Symbol symbol{};
        };

        using Content = std::variant<StringViewT, OperatorInfo>;
        Content content{};
        std::optional<ArgSequence> templateArgs{};

        [[nodiscard]]
        constexpr bool is_template() const noexcept
        {
            return templateArgs.has_value();
        }

        [[nodiscard]]
        constexpr bool is_void() const noexcept
        {
            auto const* id = std::get_if<StringViewT>(&content);

            return id
                && "void" == *id;
        }

        template <parser_visitor Visitor>
        constexpr void operator()(Visitor& visitor) const
        {
            auto& unwrapped = unwrap_visitor(visitor);

            std::visit(
                [&](auto const& inner) { handle_content(unwrapped, inner); },
                content);

            if (templateArgs)
            {
                templateArgs->handle_as_template_args(unwrapped);
            }
        }

    public:
        template <parser_visitor Visitor>
        static constexpr void handle_content(Visitor& visitor, StringViewT const& content)
        {
            MIMICPP_ASSERT(!content.empty(), "Empty identifier is not allowed.");

            visitor.add_identifier(content);
        }

        template <parser_visitor Visitor>
        static constexpr void handle_content(Visitor& visitor, OperatorInfo const& content)
        {
            visitor.begin_operator_identifier();
            std::visit(
                [&](auto const& symbol) { handle_op_symbol(visitor, symbol); },
                content.symbol);
            visitor.end_operator_identifier();
        }

        template <parser_visitor Visitor>
        static constexpr void handle_op_symbol(Visitor& visitor, StringViewT const& symbol)
        {
            MIMICPP_ASSERT(!symbol.empty(), "Empty symbol is not allowed.");

            visitor.add_identifier(symbol);
        }

        template <parser_visitor Visitor>
        static constexpr void handle_op_symbol(Visitor& visitor, std::shared_ptr<Type> const& type)
        {
            MIMICPP_ASSERT(type, "Empty type-symbol is not allowed.");

            std::invoke(*type, visitor);
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

            unwrapped.begin_type();

            if (scopes)
            {
                std::invoke(*scopes, unwrapped);
            }

            std::invoke(identifier, unwrapped);
            std::invoke(specs, unwrapped);

            unwrapped.end_type();
        }
    };

    class FunctionType
    {
    public:
        std::shared_ptr<Type> returnType{};
        FunctionContext context{};

        template <parser_visitor Visitor>
        void operator()(Visitor& visitor) const
        {
            MIMICPP_ASSERT(returnType, "Return type is mandatory for function-types.");

            auto& unwrapped = unwrap_visitor(visitor);

            unwrapped.begin_function();

            unwrapped.begin_return_type();
            std::invoke(*returnType, visitor);
            unwrapped.end_return_type();

            std::invoke(context, unwrapped);

            unwrapped.end_function();
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

            unwrapped.begin_type();

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

            unwrapped.end_type();
        }
    };

    class Type
    {
    public:
        using State = std::variant<RegularType, FunctionType, FunctionPtrType>;
        State state;

        [[nodiscard]]
        constexpr bool is_void() const noexcept
        {
            auto const* const regularType = std::get_if<RegularType>(&state);

            return regularType
                && regularType->identifier.is_void();
        }

        template <parser_visitor Visitor>
        void operator()(Visitor& visitor) const
        {
            auto& unwrapped = unwrap_visitor(visitor);

            std::visit(
                [&](auto const& inner) { std::invoke(inner, unwrapped); },
                state);
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
    };

    class End
    {
    public:
        using State = std::variant<Type, Function>;
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

namespace mimicpp::printing::type::parsing
{
    using Token = std::variant<
        token::Space,
        token::OperatorKeyword,
        token::ScopeResolution,
        token::ArgSeparator,
        token::OpeningAngle,
        token::ClosingAngle,
        token::OpeningParens,
        token::ClosingParens,
        token::OpeningCurly,
        token::ClosingCurly,
        token::OpeningBacktick,
        token::ClosingSingleQuote,
        token::TypeContext,

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
}

#endif
