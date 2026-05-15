//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_PRINT_VISITOR_HPP
#define MIMICPP_PRINTING_TYPE_PRINT_VISITOR_HPP

#pragma once

#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/type/ParserState.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/Overloaded.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <variant>
#endif

namespace mimicpp::printing::type::parsing::v2
{

    template <print_iterator OutIter>
    class PrintVisitor
    {
    public:
        [[nodiscard]]
        explicit constexpr PrintVisitor(OutIter out)
            : m_OutIter{std::move(out)}
        {
        }

        [[nodiscard]]
        constexpr OutIter out() const
        {
            return m_OutIter;
        }

        constexpr void visit(state::TypeId const& typeId)
        {
            std::visit(
                [&](auto& base) { visit(base); },
                typeId.base);
            visit(typeId.qualifications);
            visit(typeId.declarator.root);
        }

        constexpr void visit(state::FunctionId const& functionId)
        {
            if (functionId.returnType)
            {
                visit(*functionId.returnType);
                m_OutIter = format::format_to(std::move(m_OutIter), " ");
            }

            visit(functionId.identifier);
        }

    private:
        OutIter m_OutIter;
        int m_NestedDepth{0};

        template <typename State>
        constexpr void visit(state::Recursive<State> const& state)
        {
            visit(*state);
        }

        constexpr void visit(state::BuiltinType::SignedSpec const spec)
        {
            switch (spec)
            {
                using Spec = state::BuiltinType::SignedSpec;
            case Spec::id_signed:
                m_OutIter = format::format_to(std::move(m_OutIter), "signed");
                break;
            case Spec::id_unsigned:
                m_OutIter = format::format_to(std::move(m_OutIter), "unsigned");
                break;

            default:
                util::unreachable();
            }
        }

        constexpr void visit(state::BuiltinType::SizeSpec const spec)
        {
            switch (spec)
            {
                using Spec = state::BuiltinType::SizeSpec;
            case Spec::id_short:
                m_OutIter = format::format_to(std::move(m_OutIter), "short");
                break;
            case Spec::id_long:
                m_OutIter = format::format_to(std::move(m_OutIter), "long");
                break;
            case Spec::id_longlong:
                m_OutIter = format::format_to(std::move(m_OutIter), "long long");
                break;

            default:
                util::unreachable();
            }
        }

        constexpr void visit(state::BuiltinType const& type)
        {
            bool first = true;

            if (type.signedSpec)
            {
                visit(*type.signedSpec);
                first = false;
            }

            if (type.sizeSpec)
            {
                if (!std::exchange(first, false))
                {
                    m_OutIter = format::format_to(std::move(m_OutIter), " ");
                }

                visit(*type.sizeSpec);
            }

            if (type.base
                // Do not print redundant `int` when a size-spec is applied.
                && (!type.sizeSpec || lexing::keyword{"int"} != *type.base))
            {
                if (!std::exchange(first, false))
                {
                    m_OutIter = format::format_to(std::move(m_OutIter), " ");
                }

                m_OutIter = format::format_to(std::move(m_OutIter), "{}", type.base->text());
            }
        }

        constexpr void visit(state::RefQualifier const qualifier)
        {
            switch (qualifier)
            {
                using Qualifier = state::RefQualifier;
            case Qualifier::id_ref:
                m_OutIter = format::format_to(std::move(m_OutIter), "&");
                break;
            case Qualifier::id_refref:
                m_OutIter = format::format_to(std::move(m_OutIter), "&&");
                break;

            default:
                util::unreachable();
            }
        }

        constexpr void visit(state::CVQualifierSeq const qualifiers)
        {
            if (qualifiers.isConst)
            {
                m_OutIter = format::format_to(m_OutIter, " const");
            }

            if (qualifiers.isVolatile)
            {
                m_OutIter = format::format_to(m_OutIter, " volatile");
            }
        }

        constexpr void visit(state::ConstantExpression const& expression)
        {
            m_OutIter = format::format_to(m_OutIter, "{}", expression.content);
        }

        static constexpr std::array syntheticAliases = std::to_array<std::pair<std::string_view, std::string_view>>({
            {"{anonymous}",           "{anon-ns}"},
            {"(anonymous namespace)", "{anon-ns}"},

            {"`anonymous-namespace'", "{anon-ns}"}, // msvc
            {"`anonymous namespace'", "{anon-ns}"}, // msvc
        });

        constexpr void visit(state::Identifier const& id)
        {
            if (id.isSynthetic)
            {
                if (auto const iter = std::ranges::find(syntheticAliases, id.content, [](auto const& e) { return e.first; });
                    iter != syntheticAliases.cend())
                {
                    m_OutIter = format::format_to(std::move(m_OutIter), "{}", iter->second);
                    return;
                }

                // These synthetic lambda-ids will be generated on gcc:
                // `<lambda(<args>)`, where `<args>` will be replaced by `...` if not empty.
                if (constexpr std::string_view prefix{"<lambda("}, suffix{")>"};
                    id.content.starts_with(prefix)
                    && id.content.ends_with(suffix))
                {
                    std::string_view args = id.content;
                    args.remove_prefix(prefix.size());
                    args.remove_suffix(suffix.size());

                    if (!args.empty())
                    {
                        args = "...";
                    }

                    m_OutIter = format::format_to(std::move(m_OutIter), "{}{}{}", prefix, args, suffix);
                    return;
                }

                // These synthetic lambda-ids will be generated on clang:
                if (constexpr std::string_view prefix{"(lambda at "}, suffix{")"};
                    id.content.starts_with(prefix)
                    && id.content.ends_with(suffix))
                {
                    m_OutIter = format::format_to(std::move(m_OutIter), "<lambda()>");
                    return;
                }

                // These synthetic ids will be generated on clang:
                if (constexpr std::string_view prefix{"(unnamed "}, suffix{")"};
                    id.content.starts_with(prefix)
                    && id.content.ends_with(suffix))
                {
                    std::string_view classType = id.content;
                    classType.remove_prefix(prefix.size());
                    classType.remove_suffix(suffix.size());
                    classType = classType.substr(0, classType.find(" at "));

                    m_OutIter = format::format_to(std::move(m_OutIter), "<unnamed {}>", classType);
                    return;
                }

                // These synthetic ids will be generated on clang as well.
                if (constexpr std::string_view prefix{"(anonymous "}, suffix{")"};
                    id.content.starts_with(prefix)
                    && id.content.ends_with(suffix))
                {
                    std::string_view classType = id.content;
                    classType.remove_prefix(prefix.size());
                    classType.remove_suffix(suffix.size());
                    classType = classType.substr(0, classType.find(" at "));

                    m_OutIter = format::format_to(std::move(m_OutIter), "<unnamed {}>", classType);
                    return;
                }
            }

            m_OutIter = format::format_to(std::move(m_OutIter), "{}", id.content);
        }

        constexpr void visit(state::PointerDeclarator const& declarator)
        {
            if (declarator.scopes)
            {
                visit(*declarator.scopes);
            }

            m_OutIter = format::format_to(std::move(m_OutIter), "*");

            if (declarator.qualifiers)
            {
                visit(*declarator.qualifiers);
            }
        }

        constexpr void visit(state::ReferenceDeclarator const& declarator)
        {
            visit(declarator.qualifier);
        }

        constexpr void visit(state::ArrayDeclarator const& declarator)
        {
            m_OutIter = format::format_to(std::move(m_OutIter), "[");

            if (declarator.size)
            {
                visit(*declarator.size);
            }

            m_OutIter = format::format_to(std::move(m_OutIter), "]");
        }

        MIMICPP_DETAIL_CONSTEXPR_PRETTY_TYPES void visit(state::FunctionDeclarator const& declarator)
        {
            if (0 < m_NestedDepth)
            {
                return;
            }

            m_OutIter = format::format_to(std::move(m_OutIter), "(");

            // Sometimes params just contain a single (`ret (void)`), which will be suppressed.
            state::TypeId const voidTypeId{.base = state::BuiltinType{.base = lexing::keyword{"void"}}};
            if (1u != declarator.params.size()
                || voidTypeId != declarator.params.front())
            {
                ++m_NestedDepth;
                join(
                    declarator.params,
                    ", ",
                    "",
                    [&](auto const& arg) { visit(arg); });
                --m_NestedDepth;
            }

            m_OutIter = format::format_to(std::move(m_OutIter), ")");

            visit(declarator.qualifiers);
            if (declarator.refQualifier)
            {
                visit(*declarator.refQualifier);
            }

            if (declarator.isNoexcept)
            {
                m_OutIter = format::format_to(std::move(m_OutIter), " noexcept");
            }
        }

        constexpr void visit(state::TemplateArgumentList const& args)
        {
            if (0 < m_NestedDepth)
            {
                return;
            }

            m_OutIter = format::format_to(std::move(m_OutIter), "<");

            ++m_NestedDepth;
            join(
                args,
                ", ",
                "",
                [this](auto const& arg) {
                    std::visit(
                        [&](auto const& inner) { visit(inner); },
                        arg);
                });
            --m_NestedDepth;

            m_OutIter = format::format_to(std::move(m_OutIter), ">");
        }

        constexpr void visit(state::OperatorFunctionId const& id)
        {
            m_OutIter = format::format_to(std::move(m_OutIter), "operator");
            std::visit(
                util::Overloaded{
                    [this](lexing::operator_or_punctuator const& op) {
                        m_OutIter = format::format_to(std::move(m_OutIter), "{}", op.text());
                    },
                    [this](std::pair<lexing::keyword const, bool> const& op) {
                        m_OutIter = format::format_to(std::move(m_OutIter), " {}", op.first.text());
                        if (op.second)
                        {
                            m_OutIter = format::format_to(std::move(m_OutIter), "[]");
                        }
                    },
                    [this](std::span<lexing::operator_or_punctuator const, 2u> const op) {
                        m_OutIter = format::format_to(
                            std::move(m_OutIter),
                            "{}{}",
                            op.front().text(),
                            op.back().text());
                    }},
                id.symbol);
        }

        constexpr void visit(state::ConversionFunctionId const& id)
        {
            m_OutIter = format::format_to(std::move(m_OutIter), "operator ");
            visit(id.target);
        }

        constexpr void visit(state::DestructorFunctionId const& id)
        {
            m_OutIter = format::format_to(std::move(m_OutIter), "~{}", id.name.content);
        }

        constexpr void visit(state::UnqualifiedId const& nested)
        {
            std::visit(
                [&](auto const& inner) { visit(inner); },
                nested.name);

            if (nested.templateArgs)
            {
                visit(*nested.templateArgs);
            }

            if (nested.functionDeclarator)
            {
                visit(*nested.functionDeclarator);
            }
        }

        constexpr void visit(state::ScopeSequence const& sequence)
        {
            if (sequence.explicitRoot)
            {
                m_OutIter = format::format_to(std::move(m_OutIter), "::");
            }

            // Do not print that additional noise like e.g. `__cxx11` from gcc and clang.
            auto printableScopes =
                sequence.scopes
                | std::views::filter([](state::UnqualifiedId const& scope) {
                      if (scope.templateArgs
                          || scope.functionDeclarator)
                      {
                          return true;
                      }

                      if (auto const* const id = std::get_if<state::Identifier>(&scope.name))
                      {
                          return id->content != "__cxx11";
                      }

                      return true;
                  });

            ++m_NestedDepth;
            join(
                printableScopes,
                "::",
                "::",
                [&](auto const& scope) { visit(scope); });
            --m_NestedDepth;
        }

        constexpr void visit(state::QualifiedId const& id)
        {
            visit(id.scopes);
            visit(id.identifier);
        }

        constexpr void visit(state::AbstractDeclarator::Layer const& declarator)
        {
            for (auto const& decoration : declarator.decorations)
            {
                std::visit(
                    [&](auto const& inner) { visit(inner); },
                    decoration);
            }

            if (declarator.nested)
            {
                m_OutIter = format::format_to(std::move(m_OutIter), "(");
                visit(*declarator.nested);
                m_OutIter = format::format_to(std::move(m_OutIter), ")");
            }

            if (declarator.function)
            {
                visit(*declarator.function);
            }

            std::ranges::for_each(
                declarator.arrays,
                [&](auto const& arr) { visit(arr); });
        }

        template <std::ranges::forward_range Range>
        constexpr void join(
            Range&& range,
            std::string_view const separator,
            std::string_view const terminator,
            std::invocable<std::ranges::range_reference_t<Range>> auto action)
        {
            if (std::ranges::subrange elements{range})
            {
                std::invoke(action, elements.front());
                for (auto const& element : elements.advance(1u))
                {
                    m_OutIter = format::format_to(std::move(m_OutIter), "{}", separator);
                    std::invoke(action, element);
                }

                m_OutIter = format::format_to(std::move(m_OutIter), "{}", terminator);
            }
        }
    };
}

#endif
