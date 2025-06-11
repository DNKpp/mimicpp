//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_PATH_PRINTER_HPP
#define MIMICPP_PRINTING_PATH_PRINTER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Format.hpp"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <utility>

namespace mimicpp::printing::detail
{
    constexpr int maxPathElements{3};

    class PathPrinterFn
    {
    public:
        template <print_iterator OutIter>
        OutIter operator()(OutIter out, std::filesystem::path const& path) const
        {
            static constexpr CharT separator{std::filesystem::path::preferred_separator};

            auto const pathString = path.lexically_normal().string();
            auto const rend = pathString.crend();
            auto iter = std::ranges::find(pathString.crbegin(), rend, separator);
            for (int i = 1;
                 i < maxPathElements
                 && iter != rend;
                 ++i, iter = std::ranges::find(iter, rend, separator))
            {
                ++iter;
            }

            return std::ranges::copy(iter.base(), pathString.cend(), std::move(out)).out;
        }

        [[nodiscard]]
        StringT operator()(std::filesystem::path const& path) const
        {
            StringStreamT stream{};
            std::invoke(
                *this,
                std::ostreambuf_iterator{stream},
                path);
            return std::move(stream).str();
        }
    };
}

namespace mimicpp
{
    /**
     * \defgroup PRINTING_PATH path
     * \ingroup PRINTING
     * \brief Printing paths in a specific format.
     * \details ``mimic++`` currently limits all paths to a level of 3.
     * E.g. ``/a/very/long/absolute/file.path`` will be printed as ``long/absolute/file.path``.
     *
     *\{
     */

    /**
     * \brief Functional object that outputs paths in a specific format.
     */
    [[maybe_unused]] inline constexpr printing::detail::PathPrinterFn print_path{};

    /**
     * \}
     */
}

#endif
