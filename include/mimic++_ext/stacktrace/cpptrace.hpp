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
#endif

struct mimicpp::stacktrace::find_backend
{
    using type = cpptrace::stacktrace;
};

template <>
struct mimicpp::stacktrace::backend_traits<cpptrace::stacktrace>
{
    using Backend = cpptrace::stacktrace;

    [[nodiscard]]
    static Backend current(std::size_t const skip, std::size_t const max) noexcept
    {
        MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max() - max, "Skip + max is too high.");

        return cpptrace::generate_trace(skip + 1, max);
    }

    [[nodiscard]]
    static Backend current(std::size_t const skip)
    {
        MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max(), "Skip is too high.");

        return cpptrace::generate_trace(skip + 1);
    }

    [[nodiscard]]
    static MIMICPP_DETAIL_CONSTEXPR_VECTOR std::size_t size(Backend const& stacktrace)
    {
        return stacktrace.frames.size();
    }

    [[nodiscard]]
    static MIMICPP_DETAIL_CONSTEXPR_VECTOR bool empty(Backend const& stacktrace)
    {
        return stacktrace.frames.empty();
    }

    [[nodiscard]]
    static MIMICPP_DETAIL_CONSTEXPR_STRING std::string description(Backend const& stacktrace, std::size_t const at)
    {
        return frame(stacktrace, at).symbol;
    }

    [[nodiscard]]
    static MIMICPP_DETAIL_CONSTEXPR_STRING std::string source_file(Backend const& stacktrace, std::size_t const at)
    {
        return frame(stacktrace, at).filename;
    }

    [[nodiscard]]
    static MIMICPP_DETAIL_CONSTEXPR_VECTOR std::size_t source_line(Backend const& stacktrace, std::size_t const at)
    {
        return frame(stacktrace, at).line.value_or(0u);
    }

    [[nodiscard]]
    static constexpr cpptrace::stacktrace_frame const& frame(Backend const& stacktrace, std::size_t const at)
    {
        return stacktrace.frames.at(at);
    }
};

static_assert(
    mimicpp::stacktrace::backend<cpptrace::stacktrace>,
    "cpptrace::stacktrace does not satisfy the stacktrace::backend concept");

#define MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND 1

#endif
