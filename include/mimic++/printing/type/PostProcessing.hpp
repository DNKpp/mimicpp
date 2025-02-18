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
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/Regex.hpp"

#include <cassert>
#include <iterator>
#include <ranges>
#include <tuple>

#ifdef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

namespace mimicpp::printing::detail
{
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
    constexpr OutIter prettify_param_list(OutIter out, StringViewT const paramList)
    {
        if (paramList.starts_with("void"))
        {
            return out;
        }

        // This is just a very basic approach. To make it correct, we would have to parse all params separately
        // and of course distinguish between templates and scoped identifiers. Probably not worth the effort.
        out = std::ranges::copy(paramList, std::move(out)).out;

        return out;
    }

    template <print_iterator OutIter>
    constexpr OutIter prettify_function_scope(OutIter out, auto const& matches)
    {
        assert(matches.size() == 5 && "Regex out-of-sync.");

        auto const& functionName = matches[1];
        auto const& paramList = matches[2];
        auto const& specs = matches[3];
        auto const& refSpecs = matches[4];

        out = format::format_to(std::move(out), "(");
        out = std::ranges::copy(functionName.first, functionName.second, std::move(out)).out;
        out = format::format_to(std::move(out), "(");
        out = prettify_param_list(std::move(out), StringViewT{paramList.first, paramList.second});
        out = format::format_to(std::move(out), ")");

        if (0 != specs.length())
        {
            out = format::format_to(std::move(out), " ");
            out = std::ranges::copy(specs.first, specs.second, std::move(out)).out;
        }

        if (0 != refSpecs.length())
        {
            out = format::format_to(std::move(out), " ");
            out = std::ranges::copy(refSpecs.first, refSpecs.second, std::move(out)).out;
        }
        out = format::format_to(std::move(out), ")::");

        return out;
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

        constexpr StringViewT anonymousNamespaceToken{"(anonymous namespace)::"};
        static RegexT const lambdaScope{R"(\{lambda\(\)#(\d+)\}::)"};
        static RegexT const functionScope{
            R"(^(?:\w+\s+)?)"      // return type (optional)
            R"((operator.+?|\w+))" // function-name
            R"(\((.*?)\)\s*)"      // arg-list
            R"(((?:const)?))"      // const (optional)
            R"(\s*(&{0,2}))"       // ref specifier
            R"(::)"                //
        };

        SVMatchT matches{};
        // Apply on the full-name, because otherwise fun(std::string will result in std::fun(string
        if (std::regex_search(fullName.cbegin(), fullName.cend(), matches, functionScope))
        {
            return std::tuple{
                prettify_function_scope(std::move(out), matches),
                std::ranges::distance(fullName.cbegin(), matches[0].first),
                matches[0].length()};
        }

        if (scope.ends_with(anonymousNamespaceToken))
        {
            return std::tuple{
                format::format_to(std::move(out), "(anon ns)::"),
                scope.size() - anonymousNamespaceToken.size(),
                anonymousNamespaceToken.size()};
        }

        if (std::regex_search(scope.cbegin(), scope.cend(), matches, lambdaScope))
        {
            return std::tuple{
                prettify_lambda_scope(std::move(out), matches),
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
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
        static const RegexT unifyClosingAngleBrackets{R"(\s*>)"};
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
    constexpr std::tuple<OutIter, std::size_t, std::size_t> consume_next_scope(OutIter out, StringViewT const scope, StringViewT const fullName)
    {
        assert(scope.data() == fullName.data() && "Scope and fullName are not aligned.");

        constexpr StringViewT anonymousNamespaceToken{"`anonymous namespace'::"};
        static RegexT const virtualScope{"`\\d+'::"};
        static RegexT const regularScope{R"(\w+::)"};
        static RegexT const lambdaScope{R"(<lambda_(\d+)>::)"};
        static RegexT const functionScope{
            "`"
            R"((?:\w+\s+)?)"       // return type (optional)
            R"((operator.+?|\w+))" // function-name
            R"(\((.*?)\))"         // arg-list
            R"(((?:const)?))"      // const (optional)
            R"(\s*(&{0,2}))"       // ref specifier
            R"(\s*'::)"            //
        };

        SVMatchT matches{};
        // Apply on the full-name, because otherwise fun(std::string will result in std::fun(string
        if (std::regex_search(fullName.cbegin(), fullName.cend(), matches, functionScope))
        {
            return std::tuple{
                prettify_function_scope(std::move(out), matches),
                std::ranges::distance(fullName.cbegin(), matches[0].first),
                matches[0].length()};
        }

        if (scope.ends_with(anonymousNamespaceToken))
        {
            return std::tuple{
                format::format_to(std::move(out), "(anon ns)::"),
                scope.size() - anonymousNamespaceToken.size(),
                anonymousNamespaceToken.size()};
        }

        if (std::regex_search(scope.cbegin(), scope.cend(), matches, virtualScope))
        {
            return std::tuple{
                std::move(out),
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }

        if (std::regex_search(scope.cbegin(), scope.cend(), matches, regularScope))
        {
            return std::tuple{
                std::ranges::copy(matches[0].first, matches[0].second, std::move(out)).out,
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }

        if (std::regex_search(scope.cbegin(), scope.cend(), matches, lambdaScope))
        {
            return std::tuple{
                prettify_lambda_scope(std::move(out), matches),
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }

        util::unreachable();
    }

    [[nodiscard]]
    inline StringT apply_general_prettification(StringT name)
    {
        static RegexT const omitClassStructEnum{R"(\b(class|struct|enum)\s+)"};
        name = std::regex_replace(name, omitClassStructEnum, "");

        static RegexT const omitAccessSpecifiers{R"(\b(?:public|private|protected):\s+)"};
        name = std::regex_replace(name, omitAccessSpecifiers, "");

        static RegexT const omitStaticSpecifier{R"(\bstatic\s+)"};
        name = std::regex_replace(name, omitStaticSpecifier, "");

        static RegexT const unifyComma{R"(\s*,\s*)"};
        name = std::regex_replace(name, unifyComma, ", ");

        static RegexT const unifyClosingAngles{R"(\s*>)"};
        name = std::regex_replace(name, unifyClosingAngles, ">");

        // something like call-convention and __ptr64
        static RegexT const omitImplementationSpecifiers{R"(\s+__\w+\b)"};
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
