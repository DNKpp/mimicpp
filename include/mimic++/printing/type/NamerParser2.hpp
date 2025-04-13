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
                                 visitor.begin_function_ptr();
                                 visitor.end_function_ptr();
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

        class OpenParens
        {
        };

        class CloseParens
        {
        };

        class OpenAngle
        {
        };

        class CloseAngle
        {
        };

        class OpenCurly
        {
        };

        class CloseCurly
        {
        };

        class OpenSquare
        {
        };

        class CloseSquare
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

            [[nodiscard]]
            constexpr bool has_ptr() const noexcept
            {
                return 1u < layers.size();
            }

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

        class Type;

        class ArgList
        {
        public:
            std::vector<Type> types;

            ~ArgList() noexcept;
            ArgList();
            ArgList(ArgList const&);
            ArgList& operator=(ArgList const&);
            ArgList(ArgList&&) noexcept;
            ArgList& operator=(ArgList&&) noexcept;

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const;
        };

        class Template
        {
        public:
            ArgList argList{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_template();
                std::invoke(argList, visitor);
                inner.end_template();
            }
        };

        class FunctionArgs
        {
        public:
            ArgList argList{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.open_parenthesis();
                std::invoke(argList, visitor);
                inner.close_parenthesis();
            }
        };

        class RegularType
        {
        public:
            Name name;
            std::optional<Template> templateInfo{};
            Specs specs{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_type();

                std::invoke(name, visitor);
                if (templateInfo)
                {
                    std::invoke(*templateInfo, visitor);
                }
                std::invoke(specs, visitor);

                inner.end_type();
            }
        };

        class FunctionPtr
        {
        public:
            std::vector<Scope> scopes{};
            Specs specs;

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_function_ptr();

                for (auto const& scope : scopes)
                {
                    std::invoke(scope);
                    visitor.add_scope();
                }

                MIMICPP_ASSERT(specs.has_ptr(), "Invalid specs.");
                std::invoke(specs, visitor);

                inner.end_function_ptr();
            }
        };

        class Function
        {
        public:
            std::shared_ptr<Type> returnType{};
            std::variant<std::monostate, Name, FunctionPtr> description{};
            std::optional<Template> templateInfo{};
            FunctionArgs args{};
            Specs specs{};

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_function();

                if (returnType)
                {
                    inner.begin_return_type();
                    std::invoke(*returnType, visitor);
                    inner.end_return_type();
                }

                std::visit(
                    [&](auto const& desc) { handle_description(visitor, desc); },
                    description);

                if (templateInfo)
                {
                    std::invoke(*templateInfo, visitor);
                }

                std::invoke(args, visitor);
                std::invoke(specs, visitor);

                inner.end_function();
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
            using State = std::variant<RegularType, Function>;
            State m_State;

            template <parser_visitor Visitor>
            void operator()(Visitor& visitor) const
            {
                std::visit(
                    [&](auto const& inner) { std::invoke(inner, visitor); },
                    m_State);
            }
        };

        inline ArgList::~ArgList() noexcept = default;
        inline ArgList::ArgList() = default;
        inline ArgList::ArgList(ArgList const&) = default;
        inline ArgList& ArgList::operator=(ArgList const&) = default;
        inline ArgList::ArgList(ArgList&&) noexcept = default;
        inline ArgList& ArgList::operator=(ArgList&&) noexcept = default;

        template <parser_visitor Visitor>
        void ArgList::operator()(Visitor& visitor) const
        {
            if (!types.empty())
            {
                auto& inner = unwrap_visitor(visitor);

                inner.begin_args();

                bool isFirst{true};
                for (auto const& type : types)
                {
                    if (!std::exchange(isFirst, false))
                    {
                        inner.add_argument();
                    }

                    std::invoke(type, visitor);
                }

                inner.end_args();
            }
        }
    }

    using Token = std::variant<
        token::ArgSeparator,
        token::ScopeResolution,

        token::OpenParens,
        token::CloseParens,
        token::OpenAngle,
        token::CloseAngle,
        token::OpenCurly,
        token::CloseCurly,
        token::OpenSquare,
        token::CloseSquare,

        token::Name,
        token::ArgList,
        token::Template,
        token::FunctionArgs,
        token::FunctionPtr,
        token::Specs,
        token::Type>;

    namespace detail
    {
        template <typename Last, typename... Others>
        [[nodiscard]]
        constexpr std::size_t determine_longest_suffix(
            [[maybe_unused]] util::type_list<Last, Others...> const types,
            std::span<Token const> const tokenStack)
        {
            if (tokenStack.empty()
                || !std::holds_alternative<Last>(tokenStack.back()))
            {
                return 0u;
            }

            int recursiveCount{};
            if constexpr (0u < sizeof...(Others))
            {
                recursiveCount = determine_longest_suffix(
                    util::type_list<Others...>{},
                    tokenStack.first(tokenStack.size() - 1));
            }

            return 1u + recursiveCount;
        }
    }

    template <typename First, typename... Others>
    [[nodiscard]]
    constexpr std::size_t determine_longest_suffix(std::span<Token const> const tokenStack)
    {
        using types = util::type_list<First, Others...>;

        return detail::determine_longest_suffix(util::type_list_reverse_t<types>{}, tokenStack);
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

            try_reduce_as_type();

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

        template <typename T>
        [[nodiscard]]
        constexpr T extract_top_as() noexcept(std::is_nothrow_move_constructible_v<T>)
        {
            MIMICPP_ASSERT(!m_TokenStack.empty(), "Stack is depleted.");
            MIMICPP_ASSERT(std::holds_alternative<T>(m_TokenStack.back()), "Back does not match T.");

            T token = std::get<T>(std::move(m_TokenStack.back()));
            m_TokenStack.pop_back();

            return token;
        }

        static constexpr void handle_lexer_token([[maybe_unused]] lexing::end const& token) noexcept
        {
            util::unreachable();
        }

        bool try_reduce_as_arg_list()
        {
            if (0u < determine_longest_suffix<token::ArgList, token::ArgSeparator, token::Type>(m_TokenStack))
            {
                auto type = extract_top_as<token::Type>();

                if (2u == determine_longest_suffix<token::ArgList, token::ArgSeparator>(m_TokenStack))
                {
                    m_TokenStack.pop_back();
                    std::get<token::ArgList>(m_TokenStack.back())
                        .types
                        .emplace_back(std::move(type));
                }
                else
                {
                    token::ArgList argList{};
                    argList.types.emplace_back(std::move(type));
                    m_TokenStack.emplace_back(std::move(argList));
                }

                return true;
            }

            return false;
        }

        bool try_reduce_as_template()
        {
            if (4u == determine_longest_suffix<token::Name, token::OpenAngle, token::ArgList, token::CloseAngle>(m_TokenStack))
            {
                m_TokenStack.pop_back();

                token::Template newTemplate{
                    .argList = extract_top_as<token::ArgList>()};
                MIMICPP_ASSERT(!newTemplate.argList.types.empty(), "Empty arg-list must be omitted.");
                m_TokenStack.pop_back();

                m_TokenStack.emplace_back(std::move(newTemplate));

                return true;
            }

            if (3u == determine_longest_suffix<token::Name, token::OpenAngle, token::CloseAngle>(m_TokenStack))
            {
                m_TokenStack.pop_back();
                m_TokenStack.pop_back();

                m_TokenStack.emplace_back(token::Template{});

                return true;
            }

            return false;
        }

        bool try_reduce_as_function_args()
        {
            if (3u == determine_longest_suffix<token::OpenParens, token::ArgList, token::CloseParens>(m_TokenStack))
            {
                m_TokenStack.pop_back();

                token::FunctionArgs newFunctionArgs{
                    .argList = extract_top_as<token::ArgList>()};
                MIMICPP_ASSERT(!newFunctionArgs.argList.types.empty(), "Empty arg-list must be omitted.");
                m_TokenStack.pop_back();

                m_TokenStack.emplace_back(std::move(newFunctionArgs));

                return true;
            }

            if (2u == determine_longest_suffix<token::OpenParens, token::CloseParens>(m_TokenStack))
            {
                m_TokenStack.pop_back();
                m_TokenStack.pop_back();

                m_TokenStack.emplace_back(token::FunctionArgs{});

                return true;
            }

            return false;
        }

        bool try_reduce_as_function()
        {
            std::span pendingStack{m_TokenStack};

            token::Specs* specs{};
            if (1u == determine_longest_suffix<token::Specs>(pendingStack))
            {
                specs = &std::get<token::Specs>(pendingStack.back());
                pendingStack = pendingStack.first(pendingStack.size() - 1u);
            }

            if (0u == determine_longest_suffix<token::FunctionArgs>(pendingStack))
            {
                return false;
            }

            token::Function functionInfo{
                .args = std::get<token::FunctionArgs>(std::move(pendingStack.back()))};
            pendingStack = pendingStack.first(pendingStack.size() - 1u);

            if (specs)
            {
                functionInfo.specs = std::move(*specs);
            }

            if (1u == determine_longest_suffix<token::Template>(pendingStack))
            {
                functionInfo.templateInfo = std::get<token::Template>(std::move(pendingStack.back()));
                pendingStack = pendingStack.first(pendingStack.size() - 1u);
            }

            if (1u == determine_longest_suffix<token::Name>(pendingStack))
            {
                functionInfo.description = std::get<token::Name>(std::move(pendingStack.back()));
                pendingStack = pendingStack.first(pendingStack.size() - 1u);
            }
            else if (1u == determine_longest_suffix<token::FunctionPtr>(pendingStack))
            {
                functionInfo.description = std::get<token::FunctionPtr>(std::move(pendingStack.back()));
                pendingStack = pendingStack.first(pendingStack.size() - 1u);
            }

            if (1u == determine_longest_suffix<token::Type>(pendingStack))
            {
                functionInfo.returnType = std::make_shared<token::Type>(
                    std::get<token::Type>(std::move(pendingStack.back())));
                pendingStack = pendingStack.first(pendingStack.size() - 1u);
            }

            m_TokenStack.resize(pendingStack.size());
            m_TokenStack.emplace_back(
                std::in_place_type<token::Type>,
                std::move(functionInfo));

            return true;
        }

        bool try_reduce_as_regular_type()
        {
            std::span pendingStack{m_TokenStack};

            token::Specs* specs{};
            if (1u == determine_longest_suffix<token::Specs>(pendingStack))
            {
                specs = &std::get<token::Specs>(pendingStack.back());
                pendingStack = pendingStack.first(pendingStack.size() - 1u);
            }

            token::Template* templateInfo{};
            if (1u == determine_longest_suffix<token::Template>(pendingStack))
            {
                templateInfo = &std::get<token::Template>(pendingStack.back());
                pendingStack = pendingStack.first(pendingStack.size() - 1u);
            }

            if (0u == determine_longest_suffix<token::Name>(pendingStack))
            {
                return false;
            }

            token::RegularType type{
                .name = std::get<token::Name>(std::move(pendingStack.back()))};
            pendingStack = pendingStack.first(pendingStack.size() - 1u);

            if (specs)
            {
                type.specs = std::move(*specs);
            }

            if (templateInfo)
            {
                type.templateInfo = std::move(*templateInfo);
            }

            if (1u == determine_longest_suffix<token::Specs>(pendingStack))
            {
                auto const& prefixSpecs = std::get<token::Specs>(pendingStack.back());
                MIMICPP_ASSERT(1u == prefixSpecs.layers.size(), "Invalid state.");
                type.specs.layers.front().merge(prefixSpecs.layers.front());
                pendingStack = pendingStack.first(pendingStack.size() - 1u);
            }

            m_TokenStack.resize(pendingStack.size());
            m_TokenStack.emplace_back(
                std::in_place_type<token::Type>,
                std::move(type));

            return true;
        }

        bool try_reduce_as_type()
        {
            return try_reduce_as_function()
                || try_reduce_as_regular_type();
        }

        void reduce_as_name(token::Scope scope)
        {
            // Finalize already gathered type info, when possible.
            // This will be required, when we already parsed the return-type of a function and now start processing
            // it's name.
            try_reduce_as_type();

            switch (determine_longest_suffix<token::Name, token::ScopeResolution>(m_TokenStack))
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

        constexpr void handle_lexer_token([[maybe_unused]] lexing::space const& token) noexcept
        {
            // When we receive a space before a `(` token, it may mean, that we already processed a return-type of a
            // function type.
            // => `ret ()`
            if (auto const* op = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                op
                && openingParens == *op)
            {
                try_reduce_as_type();
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

        bool try_reduce_as_member_function_ptr()
        {
            if (5u == determine_longest_suffix<token::OpenParens, token::Name, token::ScopeResolution, token::Specs, token::CloseParens>(m_TokenStack))
            {
                m_TokenStack.pop_back();
                auto specs = extract_top_as<token::Specs>();
                MIMICPP_ASSERT(specs.has_ptr(), "Invalid specs.");
                m_TokenStack.pop_back();
                auto name = extract_top_as<token::Name>();
                m_TokenStack.pop_back();

                m_TokenStack.emplace_back(
                    token::FunctionPtr{
                        .scopes = std::move(name).scopes,
                        .specs = std::move(specs)});

                return true;
            }

            return false;
        }

        bool try_reduce_as_free_function_ptr()
        {
            if (3u == determine_longest_suffix<token::OpenParens, token::Specs, token::CloseParens>(m_TokenStack))
            {
                m_TokenStack.pop_back();
                auto specs = extract_top_as<token::Specs>();
                MIMICPP_ASSERT(specs.has_ptr(), "Invalid specs.");
                m_TokenStack.pop_back();

                m_TokenStack.emplace_back(
                    token::FunctionPtr{.specs = std::move(specs)});

                return true;
            }

            return false;
        }

        bool try_reduce_as_function_ptr()
        {
            return try_reduce_as_free_function_ptr()
                || try_reduce_as_member_function_ptr();
        }

        constexpr void handle_lexer_token(lexing::operator_or_punctuator const& token)
        {
            if (scopeResolution == token)
            {
                m_TokenStack.emplace_back(token::ScopeResolution{});
            }
            else if (commaSeparator == token)
            {
                try_reduce_as_type()
                    && try_reduce_as_arg_list();

                m_TokenStack.emplace_back(token::ArgSeparator{});
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
            else if (openingParens == token)
            {
                m_TokenStack.emplace_back(token::OpenParens{});
            }
            else if (closingParens == token)
            {
                try_reduce_as_type()
                    && try_reduce_as_arg_list();

                m_TokenStack.emplace_back(token::CloseParens{});

                if (auto const* op = std::get_if<lexing::operator_or_punctuator>(&m_Lexer.peek().classification);
                    !op
                    || openingParens != *op
                    || !try_reduce_as_function_ptr())
                {
                    try_reduce_as_function_args()
                        && try_reduce_as_function_args();
                }
            }
            else if (openingAngle == token)
            {
                m_TokenStack.emplace_back(token::OpenAngle{});
            }
            else if (closingAngle == token)
            {
                try_reduce_as_type()
                    && try_reduce_as_arg_list();

                m_TokenStack.emplace_back(token::CloseAngle{});
                try_reduce_as_template();
            }
            else if (openingCurly == token)
            {
                m_TokenStack.emplace_back(token::OpenCurly{});
            }
            else if (closingCurly == token)
            {
                m_TokenStack.emplace_back(token::CloseCurly{});
            }
            else if (openingSquare == token)
            {
                m_TokenStack.emplace_back(token::OpenSquare{});
            }
            else if (closingSquare == token)
            {
                m_TokenStack.emplace_back(token::CloseSquare{});
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
    };
}

#endif
