//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_PATH_PRINTER_HPP
#define MIMICPP_PRINTING_PATH_PRINTER_HPP

#pragma once

#include "mimic++/Config.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/printing/Format.hpp"

#include <algorithm>
#include <filesystem>
#include <ranges>

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

            auto const pathString = weakly_canonical(path).string();
            StringViewT printPath{pathString};
            for (auto reversedPath = pathString | std::views::reverse;
                 auto const& element : std::views::split(reversedPath, separator)
                                           | std::views::take(maxPathElements))
            {
                printPath = StringViewT{
                    element.end().base(),
                    pathString.end()};
            }

            return std::ranges::copy(printPath, std::move(out)).out;
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
    [[maybe_unused]] inline constexpr printing::detail::PathPrinterFn print_path{};
}

#endif
