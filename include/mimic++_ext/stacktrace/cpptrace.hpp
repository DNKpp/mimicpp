//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXT_STACKTRACE_CPPTRACE_HPP
#define MIMICPP_EXT_STACKTRACE_CPPTRACE_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #if __has_include(<cpptrace/basic.hpp>)
        #include <cpptrace/basic.hpp>
    #elif __has_include(<cpptrace/cpptrace.hpp>)
        // this is necessary for old cpptrace versions.
        // see: https://github.com/jeremy-rifkin/libassert/issues/110
        #include <cpptrace/cpptrace.hpp>
    #else
        #error "The cpptrace stacktrace backend is explicitly enabled, but the the required include-file can not be found."
    #endif

    #include <limits>
    #include <string>
    #include <variant>
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

template <>
struct mimicpp::stacktrace::backend_traits<mimicpp::stacktrace::CpptraceBackend>
{
    [[nodiscard]]
    static CpptraceBackend current(std::size_t const skip, std::size_t const max) noexcept
    {
        MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max() - max, "Skip + max is too high.");

        return CpptraceBackend{cpptrace::generate_raw_trace(skip + 1, max)};
    }

    [[nodiscard]]
    static CpptraceBackend current(std::size_t const skip)
    {
        MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max(), "Skip is too high.");

        return CpptraceBackend{cpptrace::generate_raw_trace(skip + 1)};
    }

    [[nodiscard]]
    static std::size_t size(CpptraceBackend const& stacktrace)
    {
        return stacktrace.data().frames.size();
    }

    [[nodiscard]]
    static bool empty(CpptraceBackend const& stacktrace)
    {
        return stacktrace.data().empty();
    }

    [[nodiscard]]
    static std::string description(CpptraceBackend const& stacktrace, std::size_t const at)
    {
        return frame(stacktrace, at).symbol;
    }

    [[nodiscard]]
    static std::string source_file(CpptraceBackend const& stacktrace, std::size_t const at)
    {
        return frame(stacktrace, at).filename;
    }

    [[nodiscard]]
    static std::size_t source_line(CpptraceBackend const& stacktrace, std::size_t const at)
    {
        return frame(stacktrace, at).line.value_or(0u);
    }

    [[nodiscard]]
    static const cpptrace::stacktrace_frame& frame(CpptraceBackend const& stacktrace, std::size_t const at)
    {
        return stacktrace.data().frames.at(at);
    }
};

static_assert(
    mimicpp::stacktrace::backend<mimicpp::stacktrace::CpptraceBackend>,
    "stacktrace::CpptraceBackend does not satisfy the stacktrace::backend concept");

namespace mimicpp::stacktrace
{
    using InstalledBackend = CpptraceBackend;
}

#define MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND 1

#endif
