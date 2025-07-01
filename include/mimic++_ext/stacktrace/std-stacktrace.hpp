//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXT_STACKTRACE_STD_STACKTRACE_HPP
#define MIMICPP_EXT_STACKTRACE_STD_STACKTRACE_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <limits>
    #include <stacktrace>
    #include <string>
#endif

struct mimicpp::stacktrace::find_backend
{
    using type = std::stacktrace;
};

template <typename Allocator>
struct mimicpp::stacktrace::backend_traits<std::basic_stacktrace<Allocator>>
{
    using BackendT = std::basic_stacktrace<Allocator>;

    [[nodiscard]]
    static BackendT current(std::size_t const skip, std::size_t const max) noexcept
    {
        MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max() - max, "Skip + max is too high.");

        return BackendT::current(skip + 1, max);
    }

    [[nodiscard]]
    static BackendT current(std::size_t const skip) noexcept
    {
        MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max(), "Skip is too high.");

        return BackendT::current(skip + 1);
    }

    [[nodiscard]]
    static std::size_t size(BackendT const& backend) noexcept
    {
        return backend.size();
    }

    [[nodiscard]]
    static bool empty(BackendT const& backend) noexcept
    {
        return backend.empty();
    }

    [[nodiscard]]
    static std::string description(BackendT const& backend, std::size_t const at)
    {
        return entry(backend, at).description();
    }

    [[nodiscard]]
    static std::string source_file(BackendT const& backend, std::size_t const at)
    {
        return entry(backend, at).source_file();
    }

    [[nodiscard]]
    static std::size_t source_line(BackendT const& backend, std::size_t const at)
    {
        return entry(backend, at).source_line();
    }

    [[nodiscard]]
    static std::stacktrace_entry const& entry(BackendT const& backend, std::size_t const at)
    {
        return backend.at(at);
    }
};

static_assert(
    mimicpp::stacktrace::backend<std::stacktrace>,
    "std::stacktrace does not satisfy the stacktrace::backend concept");

namespace mimicpp::stacktrace
{
    using InstalledBackend = std::stacktrace;
}

#define MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND 1

#endif
