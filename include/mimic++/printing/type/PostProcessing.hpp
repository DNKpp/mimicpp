//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_POST_PROCESSING_HPP
#define MIMICPP_PRINTING_TYPE_POST_PROCESSING_HPP

#pragma once

#include "mimic++/Config.hpp"
#include "mimic++/Fwd.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/utilities/Algorithm.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/Regex.hpp"

#include <cassert>
#include <iterator>
#include <ranges>
#include <tuple>

#ifdef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

namespace mimicpp::printing::type::detail
{
    template <typename OutIter>
    constexpr OutIter prettify_identifier(OutIter out, StringT name)
    {
        out = std::ranges::copy(name, std::move(out)).out;

        return out;
    }
}

#else

namespace mimicpp::printing::type::detail
{
    template <print_iterator OutIter>
    constexpr OutIter prettify_lambda_scope(OutIter out, auto const& matches)
    {
        assert(matches.size() == 2 && "Regex out-of-sync.");

        auto const& lambdaId = matches[1];

        out = format::format_to(std::move(out), "lambda#");
        out = std::ranges::copy(lambdaId.first, lambdaId.second, std::move(out)).out;
        out = format::format_to(std::move(out), "::");

        return out;
    }

    template <print_iterator OutIter>
    [[nodiscard]]
    std::tuple<OutIter, std::size_t> prettify_lambda_scope(
        OutIter out,
        RegexT const& suffixRegex,
        StringViewT const prefix,
        StringViewT const fullName)
    {
        assert(fullName.data() <= prefix.data() && "Prefix must be part of fullName.");
        assert(prefix.data() + prefix.size() <= fullName.data() + fullName.size() && "Prefix must be part of fullName.");

        StringViewT rest{prefix.data() + prefix.size(), fullName.data() + fullName.size()};
        auto const closingIter = util::find_closing_token(rest, '(', ')');
        assert(closingIter != rest.cend() && "No corresponding closing-token found.");

        SVMatchT suffixMatches{};
        rest = StringViewT{closingIter, rest.cend()};
        std::regex_search(rest.cbegin(), rest.cend(), suffixMatches, suffixRegex);
        assert(!suffixMatches.empty() && "No function suffix found.");
        assert(suffixMatches.size() == 2 && "Regex out-of-sync.");
        StringViewT const suffix{suffixMatches[0].first, suffixMatches[0].second};

        out = std::ranges::copy(StringViewT{"lambda#"}, std::move(out)).out;

        StringViewT const lambdaNo = std::invoke([&] {
    #if MIMICPP_DETAIL_USES_LIBCXX
            // no number set?, just default to 1
            if (0 == suffixMatches[1].length())
            {
                return StringViewT{"1"};
            }
    #endif

            return StringViewT{suffixMatches[1].first, suffixMatches[1].second};
        });
        out = std::ranges::copy(lambdaNo, std::move(out)).out;

        return {
            std::ranges::copy(StringViewT{"::"}, std::move(out)).out,
            static_cast<std::size_t>(std::ranges::distance(prefix.data(), suffix.data() + suffix.size()))};
    }

    template <print_iterator OutIter>
    [[nodiscard]]
    std::tuple<OutIter, std::size_t> prettify_function_scope(
        OutIter out,
        RegexT const& functionSuffixRegex,
        auto const& matches,
        StringViewT const fullName)
    {
        assert(matches.size() == 2 && "Regex out-of-sync.");

        StringViewT const functionName{matches[1].first, matches[1].second};
        StringViewT const prefix{matches[0].first, matches[0].second};
        StringViewT rest{prefix.data() + prefix.size(), fullName.data() + fullName.size()};
        auto const closingIter = util::find_closing_token(rest, '(', ')');
        assert(closingIter != rest.cend() && "No corresponding closing-token found.");

        SVMatchT suffixMatches{};
        rest = StringViewT{closingIter, rest.cend()};
        std::regex_search(rest.cbegin(), rest.cend(), suffixMatches, functionSuffixRegex);
        assert(!suffixMatches.empty() && "No function suffix found.");
        StringViewT const suffix{suffixMatches[0].first, suffixMatches[0].second};

        return {
            format::format_to(std::move(out), "({})::", functionName),
            static_cast<std::size_t>(std::ranges::distance(prefix.data(), suffix.data() + suffix.size()))};
    }
}

    #if MIMICPP_DETAIL_IS_GCC \
        || MIMICPP_DETAIL_IS_CLANG
namespace mimicpp::printing::type::detail
{
    template <print_iterator OutIter>
    std::tuple<OutIter, std::size_t, std::size_t> consume_next_scope(OutIter out, StringViewT const scope, StringViewT const fullName)
    {
        assert(scope.data() == fullName.data() && "Scope and fullName are not aligned.");

        SVMatchT matches{};

        constexpr StringViewT anonymousNamespaceToken{"(anonymous namespace)::"};
        if (scope == anonymousNamespaceToken)
        {
            return std::tuple{
                format::format_to(std::move(out), "(anon ns)::"),
                scope.size() - anonymousNamespaceToken.size(),
                anonymousNamespaceToken.size()};
        }

        // libc++ sometimes also uses `'lambda'()` for lambdas
        #if MIMICPP_DETAIL_USES_LIBCXX
        if (constexpr StringViewT libcxxLambdaScopePrefix{"'lambda'("};
            scope.starts_with(libcxxLambdaScopePrefix))
        {
            // use a fake group for the lambda no
            static RegexT lambdaEndRegex{R"(\)()::)"};

            std::size_t count{};
            std::tie(out, count) = prettify_lambda_scope(
                std::move(out),
                lambdaEndRegex,
                StringViewT{fullName.cbegin(), fullName.cbegin() + libcxxLambdaScopePrefix.size()},
                fullName);
            return std::tuple{std::move(out), 0u, count};
        }
        // while libstdc++ sometimes uses `{lambda()#N}`
        #else
        if (constexpr StringViewT lambdaScopePrefix{"{lambda("};
            scope.starts_with(lambdaScopePrefix))
        {
            static RegexT lambdaEndRegex{R"(\)#(\d+)\}::)"};

            std::size_t count{};
            std::tie(out, count) = prettify_lambda_scope(
                std::move(out),
                lambdaEndRegex,
                StringViewT{fullName.cbegin(), fullName.cbegin() + lambdaScopePrefix.size()},
                fullName);
            return std::tuple{std::move(out), 0u, count};
        }
        #endif

        // clang sometimes also uses `$_N` for lambdas
        #if MIMICPP_DETAIL_IS_CLANG
        static const RegexT lambda2Scope{R"(\$_(\d+)::)"};
        if (std::regex_match(scope.cbegin(), scope.cend(), matches, lambda2Scope))
        {
            return std::tuple{
                prettify_lambda_scope(std::move(out), matches),
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }
        #endif

        static RegexT const functionScopePrefix{
            R"(^(?:\w+\s+)?)"        // return type (optional)
            R"((operator.+?|\w+)\()" // function-name + (
        };
        if (std::regex_search(scope.cbegin(), scope.cend(), matches, functionScopePrefix))
        {
            static RegexT const functionSuffixRegex{
                R"(^\)\s*)"     // )
                R"((?:const)?)" // const (optional)
                R"(\s*&{0,2})"  // ref specifier
                R"(::)"         //
            };

            std::size_t count{};
            std::tie(out, count) = prettify_function_scope(
                std::move(out),
                functionSuffixRegex,
                matches,
                fullName);
            return std::tuple{std::move(out), 0u, count};
        }

        // Probably a regular c++-scope
        return std::tuple{
            std::ranges::copy(scope, std::move(out)).out,
            0u,
            scope.size()};
    }

    [[nodiscard]]
    inline StringT apply_general_prettification(StringT name)
    {
        static const RegexT unifyClosingAngleBrackets{R"(\s+>)"};
        name = std::regex_replace(name, unifyClosingAngleBrackets, ">");

        static const RegexT stdImplNamespace{
        #if MIMICPP_DETAIL_USES_LIBCXX
            "std::__1::"
        #else
            "std::__cxx11::"
        #endif
        };
        name = std::regex_replace(name, stdImplNamespace, "std::");

        return name;
    }
}
    #else

namespace mimicpp::printing::type::detail
{
    template <print_iterator OutIter>
    std::tuple<OutIter, std::size_t, std::size_t> consume_next_scope(OutIter out, StringViewT const scope, StringViewT const fullName)
    {
        assert(scope.data() == fullName.data() && "Scope and fullName are not aligned.");

        SVMatchT matches{};

        static RegexT const functionScopePrefix{
            "`"
            R"((?:\w+\s+)?)"         // return type (optional)
            R"((operator.+?|\w+)\()" // function-name + (
        };
        if (std::regex_search(scope.cbegin(), scope.cend(), matches, functionScopePrefix))
        {
            static RegexT const functionSuffixRegex{
                R"(^\)\s*)"     // )
                R"((?:const)?)" // const (optional)
                R"(\s*&{0,2})"  // ref specifier
                R"(\s*'::)"     //
            };

            std::size_t count{};
            std::tie(out, count) = prettify_function_scope(
                std::move(out),
                functionSuffixRegex,
                matches,
                fullName);
            return std::tuple{
                std::move(out),
                std::ranges::distance(scope.cbegin(), matches[0].first),
                count};
        }

        constexpr StringViewT anonymousNamespaceToken{"`anonymous namespace'::"};
        if (scope.ends_with(anonymousNamespaceToken))
        {
            return std::tuple{
                format::format_to(std::move(out), "(anon ns)::"),
                scope.size() - anonymousNamespaceToken.size(),
                anonymousNamespaceToken.size()};
        }

        static RegexT const virtualScope{"`\\d+'::$"};
        if (std::regex_search(scope.cbegin(), scope.cend(), matches, virtualScope))
        {
            return std::tuple{
                std::move(out),
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }

        static RegexT const regularScope{R"(\w+::$)"};
        if (std::regex_search(scope.cbegin(), scope.cend(), matches, regularScope))
        {
            return std::tuple{
                std::ranges::copy(matches[0].first, matches[0].second, std::move(out)).out,
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }

        static RegexT const lambdaScope{R"(<lambda_(\d+)>::$)"};
        if (std::regex_search(scope.cbegin(), scope.cend(), matches, lambdaScope))
        {
            return std::tuple{
                prettify_lambda_scope(std::move(out), matches),
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }

        // unknown scope. Ideally, this should never be used, but we leave it here as a fallback.
        // Output will probably quite odd.
        return std::tuple{
            std::ranges::copy(scope, std::move(out)).out,
            0u,
            scope.size()};
    }

    [[nodiscard]]
    inline StringT apply_general_prettification(StringT name)
    {
        static RegexT const omitClassStructEnum{R"(\b(class|struct|enum)\s+)"};
        name = std::regex_replace(name, omitClassStructEnum, "");

        static RegexT const omitAccessSpecifiers{R"(\b(?:public|private|protected):\s+)"};
        name = std::regex_replace(name, omitAccessSpecifiers, "");

        #if MIMICPP_DETAIL_IS_CLANG_CL
        static RegexT const omitAutoTokens{R"(<auto>\s*)"};
        name = std::regex_replace(name, omitAutoTokens, "");
        #endif

        static RegexT const omitStaticSpecifier{R"(\bstatic\s+)"};
        name = std::regex_replace(name, omitStaticSpecifier, "");

        static RegexT const unifyComma{R"(\s*,\s*)"};
        name = std::regex_replace(name, unifyComma, ", ");

        static RegexT const unifyClosingAngles{R"(\s*>)"};
        name = std::regex_replace(name, unifyClosingAngles, ">");

        // something like call-convention and __ptr64
        static RegexT const omitImplementationSpecifiers{R"(\b__\w+\b\s*)"};
        name = std::regex_replace(name, omitImplementationSpecifiers, "");

        return name;
    }
}

    #endif

namespace mimicpp::printing::type::detail
{
    constexpr StringViewT scopeToken{"::"};

    template <typename OutIter>
    constexpr OutIter prettify_scopes(OutIter out, StringT name)
    {
        while (auto const firstScopeDelimiter = std::ranges::search(name, scopeToken))
        {
            std::size_t index{};
            std::size_t count{};
            std::tie(out, index, count) = consume_next_scope(
                out,
                StringViewT{name.cbegin(), firstScopeDelimiter.end()},
                name);
            name.erase(index, count);
        }

        return out;
    }

    template <typename OutIter>
    constexpr OutIter prettify_identifier(OutIter out, StringT name)
    {
        name = apply_general_prettification(std::move(name));

        if (auto const lastMatch = std::ranges::search(name | std::views::reverse, detail::scopeToken))
        {
            std::size_t const splitIndex = std::ranges::distance(name.cbegin(), lastMatch.begin().base());
            StringT topLevelIdentifier = name.substr(splitIndex);

            // includes last ::
            name.erase(splitIndex);
            out = prettify_scopes(std::move(out), std::move(name));

            name = std::move(topLevelIdentifier);
        }

        return std::ranges::copy(name, std::move(out)).out;
    }
}

#endif

#endif
