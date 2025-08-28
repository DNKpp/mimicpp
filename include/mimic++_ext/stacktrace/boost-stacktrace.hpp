//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXT_STACKTRACE_BOOST_STACKTRACE_HPP
#define MIMICPP_EXT_STACKTRACE_BOOST_STACKTRACE_HPP

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #if __has_include(<boost/stacktrace.hpp>)
        #include <boost/stacktrace.hpp>
    #else
        #error "The boost::stacktrace backend is explicitly enabled, but the the required include-file can not be found."
    #endif

    #include <limits>
    #include <string>
#endif

struct mimicpp::util::stacktrace::find_backend
{
    using type = boost::stacktrace::stacktrace;
};

template <>
struct mimicpp::util::stacktrace::backend_traits<boost::stacktrace::stacktrace>
{
    using Backend = boost::stacktrace::stacktrace;

    [[nodiscard]]
    static Backend current(std::size_t const skip, std::size_t const max) noexcept
    {
        MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max() - max, "Skip + max is too high.");

        return Backend{skip + 1, max};
    }

    [[nodiscard]]
    static Backend current(std::size_t const skip)
    {
        MIMICPP_ASSERT(skip < std::numeric_limits<std::size_t>::max(), "Skip is too high.");

        return Backend{skip + 1, std::numeric_limits<std::size_t>::max()};
    }

    [[nodiscard]]
    static std::size_t size(Backend const& stacktrace)
    {
        return stacktrace.size();
    }

    [[nodiscard]]
    static bool empty(Backend const& stacktrace)
    {
        return stacktrace.empty();
    }

    [[nodiscard]]
    static std::string description(Backend const& stacktrace, std::size_t const at)
    {
        return stacktrace[at].name();
    }

    [[nodiscard]]
    static std::string source_file(Backend const& stacktrace, std::size_t const at)
    {
        return stacktrace[at].source_file();
    }

    [[nodiscard]]
    static std::size_t source_line(Backend const& stacktrace, std::size_t const at)
    {
        return stacktrace[at].source_line();
    }
};

#endif
