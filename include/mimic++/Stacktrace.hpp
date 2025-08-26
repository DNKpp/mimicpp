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
#include "mimic++/utilities/Concepts.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <functional> // std::invoke
    #include <limits>
    #include <memory>
    #include <ranges>
    #include <stdexcept>
    #include <type_traits>
    #include <utility>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::custom
{
    /**
     * \brief Users may define this type to enable their own stacktrace-backend
     * \note See \ref STACKTRACE "stacktrace" documentation for an example.
     * \ingroup STACKTRACE
     */
    struct find_stacktrace_backend;
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util::stacktrace
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
     * Additionally, a specialization for the `util::stacktrace::backend_traits` template must be added, which defines at least the
     * following functions:
     * ```cpp
     * template <>
     * struct mimicpp::util::stacktrace::backend_traits<MyStacktraceBackend>
     * {
     *    static MyStacktraceBackend current(std::size_t skip);
     *    static MyStacktraceBackend current(std::size_t skip, std::size_t max);
     *    static std::size_t size(MyStacktraceBackend const& backend);
     *    static bool empty(MyStacktraceBackend const& backend);
     *    static std::string description(MyStacktraceBackend const& backend, std::size_t index);
     *    static std::string source_file(MyStacktraceBackend const& backend, std::size_t index);
     *    static std::size_t source_line(MyStacktraceBackend const& backend, std::size_t index);
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
        std::semiregular<std::remove_cvref_t<T>>
        && requires(backend_traits<std::remove_cvref_t<T>> traits, std::remove_cvref_t<T> const& backend, std::size_t const value) {
               { decltype(traits)::current(value) } -> std::convertible_to<std::remove_cvref_t<T>>;
               { decltype(traits)::current(value, value) } -> std::convertible_to<std::remove_cvref_t<T>>;
               { decltype(traits)::size(backend) } -> std::convertible_to<std::size_t>;
               { decltype(traits)::empty(backend) } -> util::boolean_testable;
               { decltype(traits)::description(backend, value) } -> std::convertible_to<std::string>;
               { decltype(traits)::source_file(backend, value) } -> std::convertible_to<std::string>;
               { decltype(traits)::source_line(backend, value) } -> std::convertible_to<std::size_t>;
           };

    /**
     * \brief The fallback stacktrace-backend.
     * \details In fact, it's only use is to reduce the "defined" branching in the production code.
     */
    class NullBackend
    {
    };

    /**
     * \}
     */
}

template <>
struct mimicpp::util::stacktrace::backend_traits<mimicpp::util::stacktrace::NullBackend>
{
    [[nodiscard]]
    static constexpr NullBackend current([[maybe_unused]] std::size_t const skip, [[maybe_unused]] std::size_t const max) noexcept
    {
        return NullBackend{};
    }

    [[nodiscard]]
    static constexpr NullBackend current([[maybe_unused]] std::size_t const skip) noexcept
    {
        return NullBackend{};
    }

    [[nodiscard]]
    static constexpr std::size_t size([[maybe_unused]] NullBackend const& stacktrace) noexcept
    {
        return 0u;
    }

    [[nodiscard]]
    static constexpr bool empty([[maybe_unused]] NullBackend const& stacktrace) noexcept
    {
        return true;
    }

    static std::string description([[maybe_unused]] NullBackend const& stacktrace, [[maybe_unused]] std::size_t const at)
    {
        raise_unsupported_operation();
    }

    static std::string source_file([[maybe_unused]] NullBackend const& stacktrace, [[maybe_unused]] std::size_t const at)
    {
        raise_unsupported_operation();
    }

    [[nodiscard]]
    static std::size_t source_line([[maybe_unused]] NullBackend const& stacktrace, [[maybe_unused]] std::size_t const at)
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
    mimicpp::util::stacktrace::backend<mimicpp::util::stacktrace::NullBackend>,
    "stacktrace::NullBackend does not satisfy the stacktrace::backend concept");

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util
{
    /**
     * \brief A simple type-erased stacktrace abstraction.
     * \ingroup STACKTRACE
     */
    class Stacktrace
    {
    private:
        class Concept
        {
        public:
            [[nodiscard]]
            constexpr virtual bool empty() const = 0;

            [[nodiscard]]
            constexpr virtual std::size_t size() const = 0;

            [[nodiscard]]
            constexpr virtual std::string description(std::size_t at) const = 0;

            [[nodiscard]]
            constexpr virtual std::string source_file(std::size_t at) const = 0;

            [[nodiscard]]
            constexpr virtual std::size_t source_line(std::size_t at) const = 0;

        protected:
            virtual ~Concept() = default;
            Concept() = default;
            Concept(Concept const&) = default;
            Concept& operator=(Concept const&) = default;
            Concept(Concept&&) = default;
            Concept& operator=(Concept&&) = default;
        };

        template <typename Backend>
        class Model final
            : public Concept
        {
        public:
            using BackendTraits = stacktrace::backend_traits<Backend>;

            [[nodiscard]]
            explicit Model(Backend&& backend) noexcept(std::is_nothrow_move_constructible_v<Backend>)
                : m_Backend{std::move(backend)}
            {
            }

            [[nodiscard]]
            constexpr bool empty() const override
            {
                return std::invoke(BackendTraits::empty, m_Backend);
            }

            [[nodiscard]]
            constexpr std::size_t size() const override
            {
                return std::invoke(BackendTraits::size, m_Backend);
            }

            [[nodiscard]]
            constexpr std::string description(std::size_t const at) const override
            {
                return std::invoke(BackendTraits::description, m_Backend, at);
            }

            [[nodiscard]]
            constexpr std::string source_file(std::size_t const at) const override
            {
                return std::invoke(BackendTraits::source_file, m_Backend, at);
            }

            [[nodiscard]]
            constexpr std::size_t source_line(std::size_t const at) const override
            {
                return std::invoke(BackendTraits::source_line, m_Backend, at);
            }

        private:
            Backend m_Backend;
        };

    public:
        /**
         * \brief Defaulted destructor.
         */
        ~Stacktrace() = default;

        /**
         * \brief Default constructor.
         */
        Stacktrace()
            : Stacktrace{stacktrace::NullBackend{}}
        {
        }

        /**
         * \brief Constructor storing the given stacktrace-backend.
         * \param backend The actual stacktrace-backend object.
         */
        template <typename Backend>
            requires(!std::same_as<Stacktrace, std::remove_cvref_t<Backend>>)
                 && stacktrace::backend<Backend>
        [[nodiscard]]
        explicit Stacktrace(Backend backend)
            : m_Backend{std::make_shared<Model<Backend>>(std::move(backend))}
        {
        }

        /**
         * \brief Defaulted copy-constructor.
         */
        Stacktrace(Stacktrace const&) = default;

        /**
         * \brief Defaulted copy-assignment-operator.
         */
        Stacktrace& operator=(Stacktrace const&) = default;

        /**
         * \brief Defaulted move-constructor.
         */
        [[nodiscard]]
        Stacktrace(Stacktrace&&) = default;

        /**
         * \brief Defaulted move-assignment-operator.
         */
        Stacktrace& operator=(Stacktrace&&) = default;

        /**
         * \brief Queries the underlying stacktrace-backend for its size.
         * \return The stacktrace-entry size.
         */
        [[nodiscard]]
        std::size_t size() const
        {
            return backend().size();
        }

        /**
         * \brief Queries the underlying stacktrace-backend whether it's empty.
         * \return `True` if no stacktrace-entries exist.
         */
        [[nodiscard]]
        bool empty() const
        {
            return backend().empty();
        }

        /**
         * \brief Queries the underlying stacktrace-backend for the description of the selected stacktrace-entry.
         * \param at The stacktrace-entry index.
         * \return The description of the selected stacktrace-entry.
         */
        [[nodiscard]]
        std::string description(std::size_t const at) const
        {
            return backend().description(at);
        }

        /**
         * \brief Queries the underlying stacktrace-backend for the source-file of the selected stacktrace-entry.
         * \param at The stacktrace-entry index.
         * \return The source-file of the selected stacktrace-entry.
         */
        [[nodiscard]]
        std::string source_file(std::size_t const at) const
        {
            return backend().source_file(at);
        }

        /**
         * \brief Queries the underlying stacktrace-backend for the source-line of the selected stacktrace-entry.
         * \param at The stacktrace-entry index.
         * \return The source-line of the selected stacktrace-entry.
         */
        [[nodiscard]]
        std::size_t source_line(std::size_t const at) const
        {
            return backend().source_line(at);
        }

        [[nodiscard]]
        friend bool operator==(Stacktrace const& lhs, Stacktrace const& rhs)
        {
            return lhs.size() == rhs.size()
                && std::ranges::all_of(
                       std::views::iota(0u, lhs.size()),
                       [&](std::size_t const index) {
                           return lhs.description(index) == rhs.description(index)
                               && lhs.source_file(index) == rhs.source_file(index)
                               && lhs.source_line(index) == rhs.source_line(index);
                       });
        }

    private:
        std::shared_ptr<Concept> m_Backend;

        [[nodiscard]]
        constexpr Concept const& backend() const noexcept
        {
            MIMICPP_ASSERT(m_Backend, "Invalid state");

            return *m_Backend;
        }
    };
}

namespace mimicpp::util::stacktrace::detail
{
    template <
        template <typename> typename Traits,
#if MIMICPP_CONFIG_EXPERIMENTAL_USE_CUSTOM_STACKTRACE
        typename FindBackend = custom::find_stacktrace_backend>
#else
        typename FindBackend = stacktrace::find_backend>
#endif
    // ReSharper disable once CppFunctionIsNotImplemented
    Traits<typename FindBackend::type> find_traits();

    struct current_fn
    {
        template <typename... Canary, template <typename> typename TraitsTemplate = backend_traits>
        [[nodiscard]]
        Stacktrace operator()(std::size_t const skip, std::size_t const max) const
        {
            MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max() - max, "Skip + max is too high.");
            using Traits = decltype(find_traits<TraitsTemplate>());

            return Stacktrace{Traits::current(skip + 1u, max)};
        }

        template <typename... Canary, template <typename> typename TraitsTemplate = backend_traits>
        [[nodiscard]]
        Stacktrace operator()(std::size_t const skip) const
        {
            MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max(), "Skip is too high.");
            using Traits = decltype(find_traits<TraitsTemplate>());

            return Stacktrace{Traits::current(skip + 1u)};
        }

        template <typename... Canary, template <typename> typename TraitsTemplate = backend_traits>
        [[nodiscard]]
        Stacktrace operator()() const
        {
            using Traits = decltype(find_traits<TraitsTemplate>());

            return Stacktrace{Traits::current(1u)};
        }
    };
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::util::stacktrace
{
    /**
     * \brief Function object, which generates the current-stacktrace.
     * \ingroup STACKTRACE
     * \details This function skips at least all internal stacktrace-entries.
     * Callers may specify the optional ``skip`` parameter to remove additional entries.
     */
    [[maybe_unused]]
    inline constexpr detail::current_fn current{};
}

namespace mimicpp::util::stacktrace::detail
{
    template <print_iterator OutIter>
    constexpr OutIter print_entry(OutIter out, Stacktrace const& stacktrace, std::size_t const index)
    {
        MIMICPP_ASSERT(index < stacktrace.size(), "Index out of bounds.");

        out = format::format_to(std::move(out), "`");
        out = print_path(std::move(out), stacktrace.source_file(index));
        out = format::format_to(std::move(out), "`");
        out = format::format_to(
            std::move(out),
            "#L{}, `",
            stacktrace.source_line(index));
        out = printing::type::prettify_function(std::move(out), stacktrace.description(index));
        out = format::format_to(std::move(out), "`");

        return out;
    }
}

template <>
struct mimicpp::printing::detail::state::common_type_printer<mimicpp::util::Stacktrace>
{
    template <print_iterator OutIter>
    static constexpr OutIter print(OutIter out, util::Stacktrace const& stacktrace)
    {
        if (stacktrace.empty())
        {
            return format::format_to(std::move(out), "empty");
        }

        for (std::size_t const i : std::views::iota(0u, stacktrace.size()))
        {
            out = format::format_to(std::move(out), "#{} ", i);
            out = util::stacktrace::detail::print_entry(
                std::move(out),
                stacktrace,
                i);
            out = format::format_to(std::move(out), "\n");
        }

        return out;
    }
};

#if MIMICPP_CONFIG_EXPERIMENTAL_USE_CXX23_STACKTRACE
    #include "mimic++_ext/stacktrace/std-stacktrace.hpp"
    #define MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND 1
#elif MIMICPP_CONFIG_EXPERIMENTAL_USE_CPPTRACE
    #include "mimic++_ext/stacktrace/cpptrace.hpp"
    #define MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND 1
#else
namespace mimicpp::util::stacktrace
{
    struct find_backend
    {
        using type = NullBackend;
    };
}
#endif

#endif
