//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_STACKTRACE_HPP
#define MIMICPP_STACKTRACE_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/PathPrinter.hpp"
#include "mimic++/printing/StatePrinter.hpp"
#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/AlwaysFalse.hpp"
#include "mimic++/utilities/PriorityTag.hpp"

#include <algorithm>
#include <any>
// ReSharper disable once CppUnusedIncludeDirective
#include <functional> // std::invoke
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace mimicpp::custom
{
    /**
     * \brief Users may define this type to enable their own stacktrace-backend
     * \note See \ref STACKTRACE "stacktrace" documentation for an example.
     * \ingroup STACKTRACE
     */
    struct find_stacktrace_backend;
}

namespace mimicpp::stacktrace
{
    /**
     * \defgroup STACKTRACE stacktrace
     * \brief Contains stacktrace related functionalities.
     * \details As ``mimic++`` is officially a C++20 framework, it can not rely on built-in stack trace support from the STL.
     * However, stack traces are particularly valuable for users in the context of mocking, especially when expectations are violated.
     * To address this, ``mimic++`` introduces a simple stacktrace abstraction that can be used to integrate existing stacktrace
     * implementations.
     *
     * \note The \ref MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE macro must be defined to fully enable the support.
     * - If the \ref MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE is defined, the ``cpptrace::stacktrace`` is selected as the default stacktrace-backend.
     * - If ``std::stacktrace`` is available (i.e. c++23 is available), it's selected as the default stacktrace-backend.
     * - Otherwise, the ``stacktrace::NullBackend`` is selected, which does not provide any valuable information.
     *
     * \attention The built-in backend-priority is skipped if a non-debug build-mode is detected (i.e. when ``NDEBUG`` is defined),
     * because neither ``std::stacktrace`` nor ``cpptrace`` do provide correct information.
     * In this case the ``stacktrace::NullBackend`` is chosen as the active stacktrace-backend.
     *
     * \details
     * ### Custom Stacktrace Backends
     *
     * In any case, users can define ``mimicpp::custom::find_stacktrace_backend`` to enable their own stacktrace-backend,
     * which will then be preferred over any other stacktrace-backend (even for non-debug builds).
     * That type must contain at least a ``type`` member-alias, which denotes the desired stacktrace-backend implementation.
     *
     * ```cpp
     * struct mimicpp::custom::find_stacktrace_backend
     * {
     *      using type = MyStacktraceBackend;
     * };
     * ```
     * Additionally, a specialization for the ``stacktrace::backend_traits`` template must be added, which defines at least the
     * following functions:
     * ```cpp
     * template <>
     * struct mimicpp::stacktrace::backend_traits<MyStacktraceBackend>
     * {
     *    static MyStacktraceBackend current(const std::size_t skip);
     *    static std::size_t size(const MyStacktraceBackend& backend);
     *    static bool empty(const MyStacktraceBackend& backend);
     *    static std::string description(const MyStacktraceBackend& backend, const std::size_t index);
     *    static std::string source_file(const MyStacktraceBackend& backend, const std::size_t index);
     *    static std::size_t source_line(const MyStacktraceBackend& backend, const std::size_t index);
     * };
     * ```
     * \note The ``index`` param denotes the index of the selected stacktrace-entry.
     *
     * \{
     */

    /**
     * \brief Helper type for getting the default stacktrace-backend.
     * \note See \ref STACKTRACE "stacktrace" documentation for an example.
     */
    struct find_backend;

    /**
     * \brief Trait type for stacktrace backends.
     * \note See \ref STACKTRACE "stacktrace" documentation for an example.
     */
    template <typename Backend>
    struct backend_traits;

    /**
     * \brief Checks whether the given type satisfies the requirements of a stacktrace backend.
     * \tparam T Type to check.
     */
    template <typename T>
    concept backend =
        std::copyable<T>
        && requires(backend_traits<std::remove_cvref_t<T>> traits, const std::remove_cvref_t<T>& backend, const std::size_t value) {
               { decltype(traits)::current(value) } -> std::convertible_to<std::remove_cvref_t<T>>;
               { decltype(traits)::size(backend) } -> std::convertible_to<std::size_t>;
               { decltype(traits)::empty(backend) } -> std::convertible_to<bool>;
               { decltype(traits)::description(backend, value) } -> std::convertible_to<std::string>;
               { decltype(traits)::source_file(backend, value) } -> std::convertible_to<std::string>;
               { decltype(traits)::source_line(backend, value) } -> std::convertible_to<std::size_t>;
           };

    /**
     * \}
     */
}

namespace mimicpp::stacktrace::detail
{
    template <typename Backend>
    [[nodiscard]]
    std::size_t size(const std::any& backend)
    {
        return backend_traits<Backend>::size(
            std::any_cast<const Backend&>(backend));
    }

    template <typename Backend>
    [[nodiscard]]
    bool empty(const std::any& backend)
    {
        return backend_traits<Backend>::empty(
            std::any_cast<const Backend&>(backend));
    }

    template <typename Backend>
    [[nodiscard]]
    std::string description(const std::any& backend, const std::size_t index)
    {
        return backend_traits<Backend>::description(
            std::any_cast<const Backend&>(backend),
            index);
    }

    template <typename Backend>
    [[nodiscard]]
    std::string source_file(const std::any& backend, const std::size_t index)
    {
        return backend_traits<Backend>::source_file(
            std::any_cast<const Backend&>(backend),
            index);
    }

    template <typename Backend>
    [[nodiscard]]
    std::size_t source_line(const std::any& backend, const std::size_t index)
    {
        return backend_traits<Backend>::source_line(
            std::any_cast<const Backend&>(backend),
            index);
    }
}

namespace mimicpp
{
    /**
     * \brief A simple type-erase stacktrace abstraction.
     * \ingroup STACKTRACE
     */
    class Stacktrace
    {
    public:
        using size_fn = std::size_t (*)(const std::any&);
        using empty_fn = bool (*)(const std::any&);
        using description_fn = std::string (*)(const std::any&, std::size_t);
        using source_file_fn = std::string (*)(const std::any&, std::size_t);
        using source_line_fn = std::size_t (*)(const std::any&, std::size_t);

        /**
         * \brief Defaulted destructor.
         */
        ~Stacktrace() = default;

        /**
         * \brief Constructor storing the given stacktrace-backend type-erased.
         * \tparam Inner The actual stacktrace-backend type.
         * \tparam Traits The auto-detected trait type.
         * \param inner The actual stacktrace-backend object.
         */
        template <typename Inner>
            requires(!std::same_as<Stacktrace, std::remove_cvref_t<Inner>>)
                     && stacktrace::backend<Inner>
        [[nodiscard]]
        explicit constexpr Stacktrace(Inner&& inner)
            : m_Inner{std::forward<Inner>(inner)},
              m_SizeFn{&stacktrace::detail::size<std::remove_cvref_t<Inner>>},
              m_EmptyFn{&stacktrace::detail::empty<std::remove_cvref_t<Inner>>},
              m_DescriptionFn{&stacktrace::detail::description<std::remove_cvref_t<Inner>>},
              m_SourceFileFn{&stacktrace::detail::source_file<std::remove_cvref_t<Inner>>},
              m_SourceLineFn{&stacktrace::detail::source_line<std::remove_cvref_t<Inner>>}
        {
        }

        /**
         * \brief Defaulted move-constructor.
         */
        [[nodiscard]]
        Stacktrace(const Stacktrace&) = default;

        /**
         * \brief Defaulted move-assignment-operator.
         */
        Stacktrace& operator=(const Stacktrace&) = default;

        /**
         * \brief Defaulted copy-constructor.
         */
        [[nodiscard]]
        Stacktrace(Stacktrace&&) = default;

        /**
         * \brief Defaulted copy-assignment-operator.
         */
        Stacktrace& operator=(Stacktrace&&) = default;

        /**
         * \brief Queries the underlying stacktrace-backend for its size.
         * \return The stacktrace-entry size.
         */
        [[nodiscard]]
        constexpr std::size_t size() const
        {
            return std::invoke(m_SizeFn, m_Inner);
        }

        /**
         * \brief Queries the underlying stacktrace-backend whether its empty.
         * \return ``True`` if no stacktrace-entries exist.
         */
        [[nodiscard]]
        constexpr bool empty() const
        {
            return std::invoke(m_EmptyFn, m_Inner);
        }

        /**
         * \brief Queries the underlying stacktrace-backend for the description of the selected stacktrace-entry.
         * \param at The stacktrace-entry index.
         * \return The description of the selected stacktrace-entry.
         */
        [[nodiscard]]
        constexpr std::string description(const std::size_t at) const
        {
            return std::invoke(m_DescriptionFn, m_Inner, at);
        }

        /**
         * \brief Queries the underlying stacktrace-backend for the source-file of the selected stacktrace-entry.
         * \param at The stacktrace-entry index.
         * \return The source-file of the selected stacktrace-entry.
         */
        [[nodiscard]]
        constexpr std::string source_file(const std::size_t at) const
        {
            return std::invoke(m_SourceFileFn, m_Inner, at);
        }

        /**
         * \brief Queries the underlying stacktrace-backend for the source-line of the selected stacktrace-entry.
         * \param at The stacktrace-entry index.
         * \return The source-line of the selected stacktrace-entry.
         */
        [[nodiscard]]
        constexpr std::size_t source_line(const std::size_t at) const
        {
            return std::invoke(m_SourceLineFn, m_Inner, at);
        }

        [[nodiscard]]
        friend bool operator==(const Stacktrace& lhs, const Stacktrace& rhs)
        {
            return lhs.size() == rhs.size()
                && std::ranges::all_of(
                       std::views::iota(0u, lhs.size()),
                       [&](const std::size_t index) {
                           return lhs.description(index) == rhs.description(index)
                               && lhs.source_file(index) == rhs.source_file(index)
                               && lhs.source_line(index) == rhs.source_line(index);
                       });
        }

    private:
        std::any m_Inner;
        size_fn m_SizeFn;
        empty_fn m_EmptyFn;
        description_fn m_DescriptionFn;
        source_file_fn m_SourceFileFn;
        source_line_fn m_SourceLineFn;
    };
}

namespace mimicpp::stacktrace::detail::current_hook
{
    template <typename FindBackend, template <typename> typename Traits>
    concept existing_backend = requires {
        {
            Traits<
                typename FindBackend::type>::current(std::size_t{})
        } -> backend;
    };

    template <
        template <typename> typename Traits,
        existing_backend<Traits> FindBackendT = custom::find_stacktrace_backend>
    [[nodiscard]]
    constexpr auto current([[maybe_unused]] util::priority_tag<2> const, std::size_t const skip)
    {
        return Traits<
            typename FindBackendT::type>::current(skip + 1u);
    }

    template <
        template <typename> typename Traits,
        existing_backend<Traits> FindBackendT = find_backend>
    [[nodiscard]]
    constexpr auto current([[maybe_unused]] util::priority_tag<1> const, std::size_t const skip)
    {
        return Traits<
            typename FindBackendT::type>::current(skip + 1u);
    }

    template <template <typename> typename Traits>
    constexpr auto current([[maybe_unused]] util::priority_tag<0> const, [[maybe_unused]] std::size_t const skip)
    {
        static_assert(
            util::always_false<Traits<void>>{},
            "mimic++ does not have a registered stacktrace-backend.");
    }

    constexpr util::priority_tag<2> maxTag{};

    struct current_fn
    {
        template <typename... Canary, template <typename> typename Traits = backend_traits>
        [[nodiscard]]
        Stacktrace operator()(std::size_t const skip) const
        {
            return Stacktrace{
                current_hook::current<Traits>(maxTag, skip + 1u)};
        }

        template <typename... Canary, template <typename> typename Traits = backend_traits>
        [[nodiscard]]
        Stacktrace operator()() const
        {
            return Stacktrace{
                current_hook::current<Traits>(maxTag, 1u)};
        }
    };
}

namespace mimicpp::stacktrace
{
    /**
     * \brief Function object, which generates the current-stacktrace.
     * \ingroup STACKTRACE
     * \details This function skips at least all internal stacktrace-entries.
     * Callers may specify the optional ``skip`` parameter to remove additional entries.
     */
    [[maybe_unused]]
    constexpr detail::current_hook::current_fn current{};
}

namespace mimicpp::stacktrace::detail
{
    template <print_iterator OutIter>
    OutIter print_entry(OutIter out, Stacktrace const& stacktrace, std::size_t const index)
    {
        MIMICPP_ASSERT(index < stacktrace.size(), "Index out of bounds.");

        out = format::format_to(std::move(out), "`");
        out = print_path(std::move(out), stacktrace.source_file(index));
        out = format::format_to(std::move(out), "`");
        out = format::format_to(
            std::move(out),
            "#L{}, `",
            stacktrace.source_line(index));
        out = printing::type::prettify_identifier(std::move(out), stacktrace.description(index));
        out = format::format_to(std::move(out), "`");

        return out;
    }
}

template <>
struct mimicpp::printing::detail::state::common_type_printer<mimicpp::Stacktrace>
{
    template <print_iterator OutIter>
    static OutIter print(OutIter out, const Stacktrace& stacktrace)
    {
        if (stacktrace.empty())
        {
            return format::format_to(std::move(out), "empty");
        }

        for (const std::size_t i : std::views::iota(0u, stacktrace.size()))
        {
            out = format::format_to(std::move(out), "#{} ", i);
            out = stacktrace::detail::print_entry(
                std::move(out),
                stacktrace,
                i);
            out = format::format_to(std::move(out), "\n");
        }

        return out;
    }
};

namespace mimicpp::stacktrace
{
    /**
     * \brief The fallback stacktrace-backend.
     * \details In fact, it's only use is to reduce the "defined" branching in the production code.
     */
    class NullBackend
    {
    };
}

template <>
struct mimicpp::stacktrace::backend_traits<mimicpp::stacktrace::NullBackend>
{
    [[nodiscard]]
    static NullBackend current([[maybe_unused]] const std::size_t skip) noexcept
    {
        return NullBackend{};
    }

    [[nodiscard]]
    static constexpr std::size_t size([[maybe_unused]] const NullBackend& stacktrace) noexcept
    {
        return 0u;
    }

    [[nodiscard]]
    static constexpr bool empty([[maybe_unused]] const NullBackend& stacktrace) noexcept
    {
        return true;
    }

    static std::string description([[maybe_unused]] const NullBackend& stacktrace, [[maybe_unused]] const std::size_t at)
    {
        raise_unsupported_operation();
    }

    static std::string source_file([[maybe_unused]] const NullBackend& stacktrace, [[maybe_unused]] const std::size_t at)
    {
        raise_unsupported_operation();
    }

    [[nodiscard]]
    static std::size_t source_line([[maybe_unused]] const NullBackend& stacktrace, [[maybe_unused]] const std::size_t at)
    {
        raise_unsupported_operation();
    }

private:
    [[noreturn]]
    static void raise_unsupported_operation()
    {
        throw std::runtime_error{"stacktrace::NullBackend doesn't support this operation."};
    }
};

static_assert(
    mimicpp::stacktrace::backend<mimicpp::stacktrace::NullBackend>,
    "stacktrace::NullBackend does not satisfy the stacktrace::backend concept");

#if defined(MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE) \
    && not defined(NDEBUG)

    #ifdef MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE

        #if __has_include(<cpptrace/basic.hpp>)
            #include <cpptrace/basic.hpp>
        #elif __has_include(<cpptrace/cpptrace.hpp>)
            // this is necessary for old cpptrace versions.
            // see: https://github.com/jeremy-rifkin/libassert/issues/110
            #include <cpptrace/cpptrace.hpp>
        #else
            #error "The cpptrace stacktrace backend is explicitly enabled, but the the required include-file can not be found."
        #endif

namespace mimicpp::stacktrace
{
    class CpptraceBackend
    {
    public:
        ~CpptraceBackend() = default;

        [[nodiscard]]
        explicit CpptraceBackend(cpptrace::raw_trace&& trace) noexcept
            : m_Trace{std::move(trace)}
        {
        }

        CpptraceBackend(const CpptraceBackend&) = default;
        CpptraceBackend& operator=(const CpptraceBackend&) = default;
        CpptraceBackend(CpptraceBackend&&) = default;
        CpptraceBackend& operator=(CpptraceBackend&&) = default;

        [[nodiscard]]
        const cpptrace::stacktrace& data() const
        {
            if (const auto* raw = std::get_if<cpptrace::raw_trace>(&m_Trace))
            {
                m_Trace = raw->resolve();
            }

            return std::get<cpptrace::stacktrace>(m_Trace);
        }

    private:
        using TraceT = std::variant<cpptrace::raw_trace, cpptrace::stacktrace>;
        mutable TraceT m_Trace;
    };
}

struct mimicpp::stacktrace::find_backend
{
    using type = CpptraceBackend;
};

template <>
struct mimicpp::stacktrace::backend_traits<mimicpp::stacktrace::CpptraceBackend>
{
    [[nodiscard]]
    static CpptraceBackend current(const std::size_t skip)
    {
        return CpptraceBackend{cpptrace::generate_raw_trace(skip + 1)};
    }

    [[nodiscard]]
    static std::size_t size(const CpptraceBackend& stacktrace)
    {
        return stacktrace.data().frames.size();
    }

    [[nodiscard]]
    static bool empty(const CpptraceBackend& stacktrace)
    {
        return stacktrace.data().empty();
    }

    [[nodiscard]]
    static std::string description(const CpptraceBackend& stacktrace, const std::size_t at)
    {
        return frame(stacktrace, at).symbol;
    }

    [[nodiscard]]
    static std::string source_file(const CpptraceBackend& stacktrace, const std::size_t at)
    {
        return frame(stacktrace, at).filename;
    }

    [[nodiscard]]
    static std::size_t source_line(const CpptraceBackend& stacktrace, const std::size_t at)
    {
        return frame(stacktrace, at).line.value_or(0u);
    }

    [[nodiscard]]
    static const cpptrace::stacktrace_frame& frame(const CpptraceBackend& stacktrace, const std::size_t at)
    {
        return stacktrace.data().frames.at(at);
    }
};

static_assert(
    mimicpp::stacktrace::backend<mimicpp::stacktrace::CpptraceBackend>,
    "stacktrace::CpptraceBackend does not satisfy the stacktrace::backend concept");

        #define MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND 1

    #elif defined(__cpp_lib_stacktrace)

        #include <stacktrace>

struct mimicpp::stacktrace::find_backend
{
    using type = std::stacktrace;
};

template <typename Allocator>
struct mimicpp::stacktrace::backend_traits<std::basic_stacktrace<Allocator>>
{
    using BackendT = std::basic_stacktrace<Allocator>;

    [[nodiscard]]
    static BackendT current(const std::size_t skip)
    {
        return BackendT::current(skip + 1);
    }

    [[nodiscard]]
    static std::size_t size(const BackendT& backend)
    {
        return backend.size();
    }

    [[nodiscard]]
    static bool empty(const BackendT& backend)
    {
        return backend.empty();
    }

    [[nodiscard]]
    static std::string description(const BackendT& backend, const std::size_t at)
    {
        return entry(backend, at).description();
    }

    [[nodiscard]]
    static std::string source_file(const BackendT& backend, const std::size_t at)
    {
        return entry(backend, at).source_file();
    }

    [[nodiscard]]
    static std::size_t source_line(const BackendT& backend, const std::size_t at)
    {
        return entry(backend, at).source_line();
    }

    [[nodiscard]]
    static const std::stacktrace_entry& entry(const BackendT& backend, const std::size_t at)
    {
        return backend.at(at);
    }
};

static_assert(
    mimicpp::stacktrace::backend<std::stacktrace>,
    "std::stacktrace does not satisfy the stacktrace::backend concept");

        #define MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND 1

    #else

        // neither backend is available, but maybe a custom type?
        // so, we should not emit an error here.

    #endif

#endif

// This is enabled as fallback solution, when neither std::stacktrace nor cpptrace is available,
// or MIMICPP_CONFIG_EXPERIMENTAL_STACKTRACE simply not defined.
#if not MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

struct mimicpp::stacktrace::find_backend
{
    using type = NullBackend;
};

#endif

#endif
