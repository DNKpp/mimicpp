//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_UTILITIES_REGEX_HPP
#define MIMICPP_UTILITIES_REGEX_HPP

#include <regex>
#include <string>
#include <string_view>

namespace mimicpp
{
    using RegexT = std::regex;
}

namespace mimicpp::util
{
    template <typename Char, typename CharTraits, typename Allocator, typename RegexTraits>
    [[nodiscard]]
    constexpr std::basic_string<Char, CharTraits, Allocator> regex_replace_all(
        std::basic_string<Char, CharTraits, Allocator> str,
        std::basic_regex<Char, RegexTraits> const& regex,
        std::basic_string_view<Char, CharTraits> const fmt)
    {
        using match_result_t = std::match_results<typename std::basic_string<Char, CharTraits, Allocator>::const_iterator>;

        // just advance by one, so that we can find matches which involves the replaced fmt
        // like: `> > >` with regex `> >` and fmt = `>>`
        //      => `>> >` => `>>>`
        // When fmt is empty, we need a step size of 0.
        auto const advanceStep = static_cast<std::ptrdiff_t>(!fmt.empty());

        auto processedIter = str.cbegin();
        match_result_t match{};
        while (std::regex_search(processedIter, str.cend(), match, regex))
        {
            auto const matchBegin = match[0].first;
            auto const matchEnd = match[0].second;

            auto const nextIndex = std::distance(str.cbegin(), matchBegin + advanceStep);
            str.replace(matchBegin, matchEnd, fmt);
            processedIter = str.cbegin() + nextIndex;
        }

        return str;
    }
}

#endif
