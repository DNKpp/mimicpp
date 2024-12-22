// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_STACKTRACE_HPP
#define MIMICPP_STACKTRACE_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Printer.hpp"
#include "mimic++/Utility.hpp"

#include <any>
// ReSharper disable once CppUnusedIncludeDirective
#include <functional> // std::invoke
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace mimicpp::custom
{
    template <typename Tag>
    struct find_stacktrace_backend;
}

namespace mimicpp
{
    struct register_tag
    {
    };

    template <typename T>
    concept stacktrace_backend =
        std::copyable<T>
        && requires(stacktrace_traits<std::remove_cvref_t<T>> traits, const std::any& any, const std::size_t value) {
               { decltype(traits)::current(value) } -> std::convertible_to<std::remove_cvref_t<T>>;
               { decltype(traits)::size(any) } -> std::convertible_to<std::size_t>;
               { decltype(traits)::empty(any) } -> std::convertible_to<bool>;
               { decltype(traits)::description(any, value) } -> std::convertible_to<std::string>;
               { decltype(traits)::source_file(any, value) } -> std::convertible_to<std::string>;
               { decltype(traits)::source_line(any, value) } -> std::convertible_to<std::size_t>;
           };

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
                     && stacktrace_backend<Inner>
        [[nodiscard]]
        explicit constexpr Stacktrace(Inner&& inner)
            : m_Inner{std::forward<Inner>(inner)},
              m_SizeFn{&Traits::size},
              m_EmptyFn{&Traits::empty},
              m_DescriptionFn{&Traits::description},
              m_SourceFileFn{&Traits::source_file},
              m_SourceLineFn{&Traits::source_line}
        {
        }

        [[nodiscard]]
        Stacktrace(const Stacktrace&) = default;
        Stacktrace& operator=(const Stacktrace&) = default;

        [[nodiscard]]
        Stacktrace(Stacktrace&&) = default;
        Stacktrace& operator=(Stacktrace&&) = default;

        [[nodiscard]]
        constexpr std::size_t size() const
        {
            return std::invoke(m_SizeFn, m_Inner);
        }

        [[nodiscard]]
        constexpr bool empty() const
        {
            return std::invoke(m_EmptyFn, m_Inner);
        }

        [[nodiscard]]
        constexpr std::string description(const std::size_t at) const
        {
            return std::invoke(m_DescriptionFn, m_Inner, at);
        }

        [[nodiscard]]
        constexpr std::string source_file(const std::size_t at) const
        {
            return std::invoke(m_SourceFileFn, m_Inner, at);
        }

        [[nodiscard]]
        constexpr std::size_t source_line(const std::size_t at) const
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
        template <typename Tag, typename FindBackendT = custom::find_stacktrace_backend<Tag>>
            requires requires {
                {
                    stacktrace_traits<
                        typename FindBackendT::type>::current(std::size_t{})
                } -> stacktrace_backend;
            }
        [[nodiscard]]
        constexpr auto current([[maybe_unused]] const priority_tag<2>, const std::size_t skip)
        {
            return stacktrace_traits<
                typename FindBackendT::type>::current(skip + 1);
        }

        template <typename Tag, typename FindBackendT = find_stacktrace_backend<Tag>>
            requires requires {
                {
                    stacktrace_traits<
                        typename FindBackendT::type>::current(std::size_t{})
                } -> stacktrace_backend;
            }
        [[nodiscard]]
        constexpr auto current([[maybe_unused]] const priority_tag<1>, const std::size_t skip)
        {
            return stacktrace_traits<
                typename FindBackendT::type>::current(skip + 1);
        }

        template <typename Tag>
        constexpr auto current([[maybe_unused]] const priority_tag<0>, [[maybe_unused]] const std::size_t skip)
        {
            static_assert(
                always_false<Tag>{},
                "mimic++ does not have a registered stacktrace-backend.");
        }

        constexpr priority_tag<2> maxTag;

        struct current_fn
        {
            template <typename Tag = register_tag>
            [[nodiscard]]
            Stacktrace operator()(const std::size_t skip) const
            {
                return Stacktrace{
                    stacktrace_current_hook::current<Tag>(maxTag, skip + 1)};
            }

            template <typename Tag = register_tag>
            [[nodiscard]]
            Stacktrace operator()() const
            {
                return Stacktrace{
                    stacktrace_current_hook::current<Tag>(maxTag, 1u)};
            }
        };
    }

    [[maybe_unused]]
    constexpr detail::stacktrace_current_hook::current_fn current_stacktrace{};
}

template <>
class mimicpp::detail::Printer<mimicpp::Stacktrace>
{
public:
    template <print_iterator OutIter>
    static OutIter print(OutIter out, const Stacktrace& stacktrace)
    {
        if (stacktrace.empty())
        {
            return format::format_to(
                std::move(out),
                "empty");
        }

        for (const std::size_t i : std::views::iota(0u, stacktrace.size()))
        {
            out = format::format_to(
                std::move(out),
                "{} [{}], {}\n",
                stacktrace.source_file(i),
                stacktrace.source_line(i),
                stacktrace.description(i));
        }

        return out;
    }
};

namespace mimicpp
{
    /**
     * \brief The fallback stacktrace-backend.
     * \details In fact, it's only use is to reduce the "defined" branching in the production code.
     */
    class EmptyStacktraceBackend
    {
    };
}

template <>
struct mimicpp::stacktrace_traits<mimicpp::EmptyStacktraceBackend>
{
    using BackendT = EmptyStacktraceBackend;

    [[nodiscard]]
    static BackendT current([[maybe_unused]] const std::size_t skip) noexcept
    {
        return BackendT{};
    }

    [[nodiscard]]
    static constexpr std::size_t size([[maybe_unused]] const std::any& storage) noexcept
    {
        return 0u;
    }

    [[nodiscard]]
    static constexpr bool empty([[maybe_unused]] const std::any& storage) noexcept
    {
        return true;
    }

    static std::string description([[maybe_unused]] const std::any& storage, [[maybe_unused]] const std::size_t at)
    {
        raise_unsupported_operation();
    }

    static std::string source_file([[maybe_unused]] const std::any& storage, [[maybe_unused]] const std::size_t at)
    {
        raise_unsupported_operation();
    }

    [[nodiscard]]
    static std::size_t source_line([[maybe_unused]] const std::any& storage, [[maybe_unused]] const std::size_t at)
    {
        raise_unsupported_operation();
    }

private:
    [[noreturn]]
    static void raise_unsupported_operation()
    {
        throw std::runtime_error{"EmptyStacktraceBackend doesn't support this operation."};
    }
};

static_assert(
    mimicpp::stacktrace_backend<mimicpp::EmptyStacktraceBackend>,
    "mimicpp::EmptyStacktraceBackend does not satisfy the stacktrace_backend concept");

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE

    #ifdef MIMICPP_CONFIG_USE_CPPTRACE

        #if __has_include(<cpptrace/basic.hpp>)
            #include <cpptrace/basic.hpp>
        #elif __has_include(<cpptrace/cpptrace.hpp>)
            // this is necessary for old cpptrace versions.
            // see: https://github.com/jeremy-rifkin/libassert/issues/110
            #include <cpptrace/cpptrace.hpp>
        #else
            #error "The cpptrace stacktrace backend is explicitly enabled, but the the required include-file can not be found."
        #endif

namespace mimicpp::cpptrace
{
    class Backend
    {
    public:
        ~Backend() = default;

        [[nodiscard]]
        explicit Backend(::cpptrace::raw_trace&& trace) noexcept
            : m_Trace{std::move(trace)}
        {
        }

        Backend(const Backend&) = default;
        Backend& operator=(const Backend&) = default;
        Backend(Backend&&) = default;
        Backend& operator=(Backend&&) = default;

        [[nodiscard]]
        const ::cpptrace::stacktrace& data() const
        {
            if (const auto* raw = std::get_if<::cpptrace::raw_trace>(&m_Trace))
            {
                m_Trace = raw->resolve();
            }

            return std::get<::cpptrace::stacktrace>(m_Trace);
        }

    private:
        using TraceT = std::variant<::cpptrace::raw_trace, ::cpptrace::stacktrace>;
        mutable TraceT m_Trace;
    };
}

template <>
struct mimicpp::find_stacktrace_backend<mimicpp::register_tag>
{
    using type = cpptrace::Backend;
};

template <>
struct mimicpp::stacktrace_traits<mimicpp::cpptrace::Backend>
{
    using BackendT = cpptrace::Backend;

    [[nodiscard]]
    static BackendT current(const std::size_t skip)
    {
        return BackendT{::cpptrace::generate_raw_trace(skip + 1)};
    }

    [[nodiscard]]
    static std::size_t size(const std::any& storage)
    {
        return get(storage).frames.size();
    }

    [[nodiscard]]
    static bool empty(const std::any& storage)
    {
        return get(storage).empty();
    }

    [[nodiscard]]
    static std::string description(const std::any& storage, const std::size_t at)
    {
        return get_frame(storage, at).symbol;
    }

    [[nodiscard]]
    static std::string source_file(const std::any& storage, const std::size_t at)
    {
        return get_frame(storage, at).filename;
    }

    [[nodiscard]]
    static std::size_t source_line(const std::any& storage, const std::size_t at)
    {
        return get_frame(storage, at).line.value_or(0u);
    }

    [[nodiscard]]
    static const ::cpptrace::stacktrace& get(const std::any& storage)
    {
        return std::any_cast<const BackendT&>(storage).data();
    }

    [[nodiscard]]
    static const ::cpptrace::stacktrace_frame& get_frame(const std::any& storage, const std::size_t at)
    {
        return get(storage).frames.at(at);
    }
};

static_assert(
    mimicpp::stacktrace_backend<mimicpp::cpptrace::Backend>,
    "mimicpp::cpptrace::Backend does not satisfy the stacktrace_backend concept");

        #define MIMICPP_DETAIL_WORKING_STACKTRACE_BACKEND

    #elif defined __cpp_lib_stacktrace

        #include <stacktrace>

template <>
struct mimicpp::find_stacktrace_backend<mimicpp::register_tag>
{
    using type = std::stacktrace;
};

template <typename Allocator>
struct mimicpp::stacktrace_traits<std::basic_stacktrace<Allocator>>
{
    using BackendT = std::basic_stacktrace<Allocator>;

    [[nodiscard]]
    static BackendT current(const std::size_t skip)
    {
        return BackendT::current(skip + 1);
    }

    [[nodiscard]]
    static std::size_t size(const std::any& storage)
    {
        return get(storage).size();
    }

    [[nodiscard]]
    static bool empty(const std::any& storage)
    {
        return get(storage).empty();
    }

    [[nodiscard]]
    static std::string description(const std::any& storage, const std::size_t at)
    {
        return get_entry(storage, at).description();
    }

    [[nodiscard]]
    static std::string source_file(const std::any& storage, const std::size_t at)
    {
        return get_entry(storage, at).source_file();
    }

    [[nodiscard]]
    static std::size_t source_line(const std::any& storage, const std::size_t at)
    {
        return get_entry(storage, at).source_line();
    }

    [[nodiscard]]
    static const BackendT& get(const std::any& storage)
    {
        return std::any_cast<const BackendT&>(storage);
    }

    [[nodiscard]]
    static const std::stacktrace_entry& get_entry(const std::any& storage, const std::size_t at)
    {
        return get(storage).at(at);
    }
};

static_assert(
    mimicpp::stacktrace_backend<std::stacktrace>,
    "std::stacktrace does not satisfy the stacktrace_backend concept");

        #define MIMICPP_DETAIL_WORKING_STACKTRACE_BACKEND

    #else

        // neither backend is available, but maybe a custom type?
        // so, we should not emit an error here.

    #endif

#endif

// This is enabled as fallback solution, when neither std::stacktrace nor cpptrace is available,
// or MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE simply not defined.
#ifndef MIMICPP_DETAIL_WORKING_STACKTRACE_BACKEND

template <>
struct mimicpp::find_stacktrace_backend<mimicpp::register_tag>
{
    using type = EmptyStacktraceBackend;
};

#endif

#endif
