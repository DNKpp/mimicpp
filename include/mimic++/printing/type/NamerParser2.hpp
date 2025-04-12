//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_NAME_PARSER2_HPP
#define MIMICPP_PRINTING_TYPE_NAME_PARSER2_HPP

#pragma once

namespace mimicpp::printing::type::parsing2
{
    template <typename T>
    concept parser_visitor = std::movable<T>
                          && requires(std::unwrap_reference_t<T> visitor, StringViewT content) {
                                 visitor.begin();
                                 visitor.end();

                                 visitor.add_identifier(content);
                                 visitor.add_scope();
                                 visitor.add_argument();

                                 visitor.add_const();
                                 visitor.add_volatile();
                                 visitor.add_noexcept();
                                 visitor.add_ptr();
                                 visitor.add_lvalue_ref();
                                 visitor.add_rvalue_ref();

                                 visitor.begin_name();
                                 visitor.end_name();

                                 visitor.begin_type();
                                 visitor.end_type();

                                 visitor.begin_template();
                                 visitor.end_template();

                                 visitor.open_parenthesis();
                                 visitor.close_parenthesis();

                                 visitor.begin_function();
                                 visitor.end_function();
                                 visitor.begin_return_type();
                                 visitor.end_return_type();
                                 visitor.begin_args();
                                 visitor.end_args();

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

    namespace token
    {
        using action_fn = std::function<void()>;

        class ScopeResolution
        {
        };

        class ArgSeparator
        {
        };

        using Scope = action_fn;

        class Name
        {
        public:
            std::vector<Scope> scopes{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_name();

                bool isFirst{true};
                for (auto const& scope : scopes)
                {
                    if (!std::exchange(isFirst, false))
                    {
                        inner.add_scope();
                    }

                    std::invoke(scope);
                }

                inner.end_name();
            }
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

                void merge(Layer const& others) noexcept
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
            };

            std::vector<Layer> layers{1u};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                MIMICPP_ASSERT(!layers.empty(), "Invalid state.");

                auto& inner = unwrap_visitor(visitor);

                bool isFirst{true};
                for (auto const [isConst, isVolatile, isNoexcept, isLValueRef, isRValueRef] : layers)
                {
                    if (!std::exchange(isFirst, false))
                    {
                        inner.add_ptr();
                    }

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
            }
        };

        class Type
        {
        public:
            Name name{};
            Specs specs{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_type();

                std::invoke(name, visitor);
                std::invoke(specs, visitor);

                inner.end_type();
            }
        };

        class ArgList
        {
        public:
            std::vector<Type> types{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_args();

                for (auto const& type : types)
                {
                    std::invoke(type, visitor);
                }

                inner.end_args();
            }
        };

        class Function
        {
        public:
            Type returnType{};
            Name name{};
            ArgList argList{};
            Specs specs{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_function();

                inner.begin_return_type();
                std::invoke(returnType, visitor);
                inner.end_return_type();

                std::invoke(name, visitor);

                inner.open_parenthesis();
                std::invoke(argList, visitor);
                inner.close_parenthesis();

                std::invoke(specs, visitor);

                inner.end_function();
            }
        };
    }

    using Token = std::variant<
        token::ArgSeparator,
        token::ScopeResolution,
        token::Name,
        token::ArgList,
        token::Specs,
        token::Type,
        token::Function>;

    template <typename Last, typename... Others>
    [[nodiscard]]
    constexpr std::size_t determine_longest_suffix(std::span<Token const> const tokenStack)
    {
        if (tokenStack.empty()
            || !std::holds_alternative<Last>(tokenStack.back()))
        {
            return 0u;
        }

        if constexpr (0u < sizeof...(Others))
        {
            return 1u + determine_longest_suffix<Others...>(tokenStack.first(tokenStack.size() - 1));
        }
        else
        {
            return 1u;
        }
    }

    template <parser_visitor Visitor>
    class NameParser
    {
    public:
        [[nodiscard]]
        explicit NameParser(Visitor visitor, StringViewT content) noexcept(std::is_nothrow_move_constructible_v<Visitor>)
            : m_Visitor{std::move(visitor)},
              m_Lexer{std::move(content)}
        {
        }

        void operator()()
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

            reduce_as_end_state();

            MIMICPP_ASSERT(1u == m_TokenStack.size(), "A single end-state is required.");
            if (auto const* type = std::get_if<token::Type>(&m_TokenStack.back()))
            {
                std::invoke(*type, visitor());
            }
            else if (auto const* function = std::get_if<token::Function>(&m_TokenStack.back()))
            {
                std::invoke(*function, visitor());
            }
            else
            {
                util::unreachable();
            }

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

        static constexpr void handle_lexer_token([[maybe_unused]] lexing::end const& token) noexcept
        {
            util::unreachable();
        }

        constexpr void handle_lexer_token([[maybe_unused]] lexing::space const& token) noexcept
        {
        }

        bool try_reduce_as_type()
        {
            token::Specs specs{};
            if (2u == determine_longest_suffix<token::Specs, token::Name>(m_TokenStack))
            {
                specs = std::get<token::Specs>(std::move(m_TokenStack.back()));
                m_TokenStack.pop_back();
            }
            else if (0u == determine_longest_suffix<token::Name>(m_TokenStack))
            {
                return false;
            }

            auto name = std::get<token::Name>(std::move(m_TokenStack.back()));
            m_TokenStack.pop_back();

            if (1u == determine_longest_suffix<token::Specs>(m_TokenStack))
            {
                auto const& prefixSpecs = std::get<token::Specs>(m_TokenStack.back());
                MIMICPP_ASSERT(1u == prefixSpecs.layers.size(), "Invalid state.");
                specs.layers.front().merge(prefixSpecs.layers.front());
                m_TokenStack.pop_back();
            }

            m_TokenStack.emplace_back(
                token::Type{
                    .name = std::move(name),
                    .specs = std::move(specs)});

            return true;
        }

        void reduce_as_end_state()
        {
            try_reduce_as_type();
        }

        void reduce_as_name(token::Scope scope)
        {
            switch (determine_longest_suffix<token::ScopeResolution, token::Name>(m_TokenStack))
            {
            case 2:
                m_TokenStack.pop_back();
                std::get<token::Name>(m_TokenStack.back())
                    .scopes
                    .emplace_back(std::move(scope));
                break;

            case 1u:
                // It's an absolute scope, ignore that.
                m_TokenStack.pop_back();
                [[fallthrough]];

            default:
                m_TokenStack.emplace_back(token::Name{{std::move(scope)}});
                break;
            }
        }

        void handle_lexer_token(lexing::identifier const& token)
        {
            reduce_as_name(
                [&visitor = visitor(), content = token.content] {
                    visitor.add_identifier(content);
                });
        }

        void reduce_as_specs(token::Specs::Layer specs)
        {
            if (1u == determine_longest_suffix<token::Specs>(m_TokenStack))
            {
                auto& layers = std::get<token::Specs>(m_TokenStack.back()).layers;
                MIMICPP_ASSERT(!layers.empty(), "Invalid specs state.");
                layers.back().merge(specs);
            }
            else
            {
                m_TokenStack.emplace_back(token::Specs{{std::move(specs)}});
            }
        }

        void reduce_as_specs_layer()
        {
            if (1u == determine_longest_suffix<token::Specs>(m_TokenStack))
            {
                std::get<token::Specs>(m_TokenStack.back())
                    .layers.emplace_back();
            }
            else
            {
                m_TokenStack.emplace_back(token::Specs{
                    .layers = {2u, token::Specs::Layer{}}
                });
            }
        }

        constexpr void handle_lexer_token(lexing::operator_or_punctuator const& token)
        {
            if (scopeResolution == token)
            {
                m_TokenStack.emplace_back(token::ScopeResolution{});
            }
            else if (lvalueRef == token)
            {
                reduce_as_specs({.isLValueRef = true});
            }
            else if (rvalueRef == token)
            {
                reduce_as_specs({.isRValueRef = true});
            }
            else if (pointer == token)
            {
                reduce_as_specs_layer();
            }
        }

        constexpr void handle_lexer_token(lexing::keyword const& token)
        {
            if (constKeyword == token)
            {
                reduce_as_specs({.isConst = true});
            }
            else if (volatileKeyword == token)
            {
                reduce_as_specs({.isVolatile = true});
            }
            else if (noexceptKeyword == token)
            {
                reduce_as_specs({.isNoexcept = true});
            }
        }

        constexpr void handle_operator_token()
        {
        }
    };
}

#endif
