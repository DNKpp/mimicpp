// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_STACKTRACE_HPP
#define MIMICPP_STACKTRACE_HPP

#pragma once

#include "Stacktrace.hpp"
#include "mimic++/Utility.hpp"

#include <any>
#include <type_traits>
#include <utility>
#include <variant>

namespace mimicpp::custom
{
    template <typename Tag>
    struct stacktrace_backend;
}

namespace mimicpp
{
    struct register_tag
    {
    };

    template <typename Tag>
    struct stacktrace_backend;

    template <typename Backend>
    struct stacktrace_traits;

    class Stacktrace
    {
    public:
        using size_fn = std::size_t (*)(const std::any&);
        using empty_fn = bool (*)(const std::any&);
        using description_fn = std::string (*)(const std::any&, std::size_t);
        using source_file_fn = std::string (*)(const std::any&, std::size_t);
        using source_line_fn = std::size_t (*)(const std::any&, std::size_t);

        ~Stacktrace() = default;

        template <typename Inner, typename Traits = stacktrace_traits<std::remove_cvref_t<Inner>>>
            requires(!std::same_as<Stacktrace, std::remove_cvref_t<Inner>>)
        [[nodiscard]]
        explicit Stacktrace(Inner&& inner)
            : m_Inner{std::move(inner)},
              m_SizeFn{&Traits::size},
              m_EmptyFn{&Traits::empty},
              m_DescriptionFn{&Traits::description},
              m_SourceFileFn{&Traits::source_file},
              m_SourceLineFn{&Traits::source_line}
        {
        }

        [[nodiscard]]
        Stacktrace(Stacktrace&&) = default;
        Stacktrace& operator=(Stacktrace&&) = default;

        Stacktrace(const Stacktrace&) = delete;
        Stacktrace& operator=(const Stacktrace&) = delete;

        [[nodiscard]]
        std::size_t size() const
        {
            return std::invoke(m_SizeFn, m_Inner);
        }

        [[nodiscard]]
        bool empty() const
        {
            return std::invoke(m_EmptyFn, m_EmptyFn);
        }

        [[nodiscard]]
        std::string description(const std::size_t at) const
        {
            return std::invoke(m_DescriptionFn, m_Inner, at);
        }

        [[nodiscard]]
        std::string source_file(const std::size_t at) const
        {
            return std::invoke(m_SourceFileFn, m_Inner, at);
        }

        [[nodiscard]]
        std::size_t source_line(const std::size_t at) const
        {
            return std::invoke(m_SourceLineFn, m_Inner, at);
        }

    private:
        std::any m_Inner;
        size_fn m_SizeFn;
        empty_fn m_EmptyFn;
        description_fn m_DescriptionFn;
        source_file_fn m_SourceFileFn;
        source_line_fn m_SourceLineFn;
    };

    namespace detail::stacktrace_current_hook
    {
        template <typename Tag, typename BackendSelectorT = custom::stacktrace_backend<Tag>>
            requires requires {
                {
                    stacktrace_traits<
                        typename BackendSelectorT::type>::current(std::size_t{})
                } -> std::convertible_to<typename BackendSelectorT::type>;
                requires std::constructible_from<Stacktrace, typename BackendSelectorT::type>;
            }
        [[nodiscard]]
        constexpr auto current([[maybe_unused]] const priority_tag<2>, const std::size_t skip)
        {
            return stacktrace_traits<
                typename BackendSelectorT::type>::current(skip + 1);
        }

        template <typename Tag, typename BackendSelectorT = stacktrace_backend<Tag>>
            requires requires {
                {
                    stacktrace_traits<
                        typename BackendSelectorT::type>::current(std::size_t{})
                } -> std::convertible_to<typename BackendSelectorT::type>;
                requires std::constructible_from<Stacktrace, typename BackendSelectorT::type>;
            }
        [[nodiscard]]
        constexpr auto current([[maybe_unused]] const priority_tag<1>, const std::size_t skip)
        {
            return stacktrace_traits<
                typename BackendSelectorT::type>::current(skip + 1);
        }

        template <typename Tag>
        [[nodiscard]]
        constexpr auto current([[maybe_unused]] const priority_tag<0>, [[maybe_unused]] const std::size_t skip)
        {
        }

        constexpr priority_tag<2> maxTag;

        struct current_fn
        {
            template <typename Tag = register_tag>
            [[nodiscard]]
            constexpr Stacktrace operator()(const std::size_t skip) const
            {
                return Stacktrace{
                    stacktrace_current_hook::current<Tag>(maxTag, skip + 1)};
            }

            template <typename Tag = register_tag>
            [[nodiscard]]
            constexpr Stacktrace operator()() const
            {
                return Stacktrace{
                    stacktrace_current_hook::current<Tag>(maxTag, 1u)};
            }
        };
    }

    [[maybe_unused]]
    constexpr detail::stacktrace_current_hook::current_fn current_stacktrace{};
}

#endif
