//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_NAME_PRINT_VISITOR_HPP
#define MIMICPP_PRINTING_TYPE_NAME_PRINT_VISITOR_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/type/NameParser.hpp"

#include <algorithm>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace mimicpp::printing::type
{
    [[nodiscard]]
    inline auto const& alias_map()
    {
        static std::unordered_map<StringViewT, StringViewT> const aliases{
            {"(anonymous namespace)", "{anon-ns}"},
            {          "{anonymous}", "{anon-ns}"},
            {"`anonymous namespace'", "{anon-ns}"},
            {           "<lambda()>",    "lambda"}
        };

        return aliases;
    }

    [[nodiscard]]
    inline auto const& ignored_identifiers()
    {
        static std::unordered_set<StringViewT> const collection{
            "__cxx11",
            "__1"};

        return collection;
    }

    template <print_iterator OutIter>
    class PrintVisitor
    {
    public:
        [[nodiscard]]
        explicit PrintVisitor(OutIter out) noexcept(std::is_nothrow_move_constructible_v<OutIter>)
            : m_Out{std::move(out)}
        {
        }

        [[nodiscard]]
        constexpr OutIter out() const noexcept
        {
            return m_Out;
        }

        constexpr void unrecognized(StringViewT const content)
        {
            print(content);
        }

        static constexpr void begin()
        {
        }

        static constexpr void end()
        {
        }

        static constexpr void begin_type()
        {
        }

        static constexpr void end_type()
        {
        }

        constexpr void begin_scope()
        {
            m_Context.push_scope();
        }

        constexpr void end_scope()
        {
            if (!std::exchange(m_IgnoreNextScopeResolution, false))
            {
                print("::");
            }

            m_Context.pop_scope();
        }

        constexpr void add_identifier(StringViewT content)
        {
            if (content.starts_with("{lambda(")
                && content.ends_with('}'))
            {
                auto const closingIter = std::ranges::find(content | std::views::reverse, ')');
                print("lambda");
                print(StringViewT{closingIter.base(), content.cend() - 1});

                return;
            }

            // Lambdas can have the for `'lambda\\d*'`. Just print everything between ''.
            if (constexpr StringViewT lambdaPrefix{"'lambda"};
                content.starts_with(lambdaPrefix)
                && content.ends_with('\''))
            {
                print(content.substr(1u, content.size() - 2u));

                return;
            }

            if (ignored_identifiers().contains(content))
            {
                m_IgnoreNextScopeResolution = true;

                return;
            }

            auto const& aliases = alias_map();
            if (auto const iter = aliases.find(content);
                iter != aliases.cend())
            {
                content = iter->second;
            }
            print(content);
        }

        constexpr void begin_template_args()
        {
            m_Context.push_arg_sequence();

            print("<");
        }

        constexpr void end_template_args()
        {
            print(">");

            m_Context.pop_arg_sequence();
        }

        constexpr void add_arg()
        {
            print(", ");
        }

        static constexpr void begin_function()
        {
        }

        static constexpr void end_function()
        {
        }

        static constexpr void begin_return_type()
        {
        }

        constexpr void end_return_type()
        {
            print(" ");
        }

        constexpr void begin_function_args()
        {
            m_Context.push_arg_sequence();

            print("(");
        }

        constexpr void end_function_args()
        {
            print(")");

            m_Context.pop_arg_sequence();
        }

        constexpr void begin_function_ptr()
        {
            print("(");
        }

        constexpr void end_function_ptr()
        {
            print(")");
        }

        constexpr void begin_operator_identifier()
        {
            print("operator");
        }

        static constexpr void end_operator_identifier()
        {
        }

        constexpr void add_const()
        {
            if (m_Context.is_spec_printable())
            {
                print(" const");
            }
        }

        constexpr void add_volatile()
        {
            if (m_Context.is_spec_printable())
            {
                print(" volatile");
            }
        }

        constexpr void add_noexcept()
        {
            if (m_Context.is_spec_printable())
            {
                print(" noexcept");
            }
        }

        constexpr void add_ptr()
        {
            if (m_Context.is_spec_printable())
            {
                print("*");
            }
        }

        constexpr void add_lvalue_ref()
        {
            if (m_Context.is_spec_printable())
            {
                print("&");
            }
        }

        constexpr void add_rvalue_ref()
        {
            if (m_Context.is_spec_printable())
            {
                print("&&");
            }
        }

    private:
        OutIter m_Out;
        bool m_IgnoreNextScopeResolution{false};

        class Context
        {
        public:
            [[nodiscard]]
            constexpr bool is_printable() const noexcept
            {
                if (auto const size = m_Stack.size();
                    size <= 1u)
                {
                    return true;
                }
                else if (2u == size)
                {
                    return Type::argSequence == m_Stack.front()
                        && Type::scope == m_Stack.back();
                }

                return false;
            }

            [[nodiscard]]
            constexpr bool is_spec_printable() const noexcept
            {
                return 0 == m_ScopeDepth;
            }

            void push_scope()
            {
                m_Stack.emplace_back(Type::scope);
                ++m_ScopeDepth;
            }

            void pop_scope()
            {
                MIMICPP_ASSERT(0 < m_ScopeDepth, "Unbalanced depth.");
                --m_ScopeDepth;
                MIMICPP_ASSERT(!m_Stack.empty() && Type::scope == m_Stack.back(), "Context-stack out of sync.");
                m_Stack.pop_back();
            }

            void push_arg_sequence()
            {
                m_Stack.emplace_back(Type::argSequence);
                ++m_ArgSeqDepth;
            }

            void pop_arg_sequence()
            {
                MIMICPP_ASSERT(0 < m_ArgSeqDepth, "Unbalanced depth.");
                --m_ArgSeqDepth;
                MIMICPP_ASSERT(!m_Stack.empty() && Type::argSequence == m_Stack.back(), "Context-stack out of sync.");
                m_Stack.pop_back();
            }

        private:
            enum class Type : std::uint8_t
            {
                scope,
                argSequence
            };

            std::vector<Type> m_Stack{};
            int m_ScopeDepth{};
            int m_ArgSeqDepth{};
        };

        Context m_Context{};

        constexpr void print(StringViewT const text)
        {
            if (m_Context.is_printable())
            {
                m_Out = std::ranges::copy(text, std::move(m_Out)).out;
            }
        }
    };
}

#endif
