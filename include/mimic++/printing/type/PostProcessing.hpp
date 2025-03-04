//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_POST_PROCESSING_HPP
#define MIMICPP_PRINTING_TYPE_POST_PROCESSING_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/utilities/Algorithm.hpp"

#include <array>
#include <iterator>
#include <optional>
#include <ranges>
#include <tuple>

namespace mimicpp::printing::type
{
    /**
     * \brief Prettifies a demangled name.
     * \ingroup PRINTING_TYPE
     * \tparam OutIter The print-iterator type.
     * \param out The print iterator.
     * \param name The demangled name to be prettified.
     * \return The current print iterator.
     *
     * \details This function formats a type or template name for better readability.
     * The primary strategy is to minimize unnecessary details while retaining essential information.
     * Although this may introduce some ambiguity, it is generally more beneficial to provide an approximate name.
     *
     * For example, when a template-dependent type is provided, the template arguments are omitted:
     * `std::vector<int, std::allocator>::iterator` => `std::vector::iterator`
     *
     * \attention Providing a mangled name will result in unexpected behavior.
     * \note When `MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING` is enabled,
     * this function simply outputs the provided name without any modifications.
     */
    template <print_iterator OutIter>
    constexpr OutIter prettify_identifier(OutIter out, StringT name);
}

#ifdef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

namespace mimicpp::printing::type
{
    template <print_iterator OutIter>
    constexpr OutIter prettify_identifier(OutIter out, StringT name)
    {
        out = std::ranges::copy(name, std::move(out)).out;

        return out;
    }
}

#else

    #include <regex>

namespace mimicpp
{
    using RegexT = std::regex;
    using SMatchT = std::smatch;
    using SVMatchT = std::match_results<StringViewT::const_iterator>;
}

namespace mimicpp::printing::type::detail
{
    constexpr StringViewT anonymousNamespaceTargetScopeText{"{anon-ns}::"};

    template <print_iterator OutIter>
    [[nodiscard]]
    std::tuple<OutIter, std::size_t> prettify_function_scope(
        OutIter out,
        auto const& prefixMatches,
        auto const& suffixMatches)
    {
        MIMICPP_ASSERT(prefixMatches.size() == 2, "Regex out-of-sync.");
        MIMICPP_ASSERT(suffixMatches.size() == 1, "Regex out-of-sync.");

        StringViewT const functionName{prefixMatches[1].first, prefixMatches[1].second};
        StringViewT const prefix{prefixMatches[0].first, prefixMatches[0].second};
        StringViewT const suffix{suffixMatches[0].first, suffixMatches[0].second};

        return {
            format::format_to(std::move(out), "{{{}}}::", functionName),
            static_cast<std::size_t>(std::ranges::distance(prefix.data(), suffix.data() + suffix.size()))};
    }

    template <print_iterator OutIter>
    [[nodiscard]]
    OutIter prettify_template_scope(
        OutIter out,
        StringViewT const scope,
        StringViewT const templateScopeSuffix)
    {
        auto reversedName = scope
                          | std::views::reverse
                          | std::views::drop(templateScopeSuffix.size());
        auto const iter = util::find_closing_token(reversedName, '>', '<');
        MIMICPP_ASSERT(iter != reversedName.end(), "No template begin found.");
        out = format::format_to(
            std::move(out),
            "{}::",
            StringViewT{scope.cbegin(), iter.base() - 1});

        return out;
    }
}

    #if MIMICPP_DETAIL_IS_GCC \
        || MIMICPP_DETAIL_IS_CLANG
namespace mimicpp::printing::type::detail
{
    template <print_iterator OutIter>
    [[nodiscard]]
    std::tuple<OutIter, std::size_t> prettify_lambda_scope(
        OutIter out,
        RegexT const& suffixRegex,
        StringViewT const prefix,
        StringViewT const scope)
    {
        MIMICPP_ASSERT(scope.data() == prefix.data(), "Prefix and scope must be aligned.");

        StringViewT rest{prefix.data() + prefix.size(), scope.data() + scope.size()};
        auto const closingIter = util::find_closing_token(rest, '(', ')');
        MIMICPP_ASSERT(closingIter != rest.cend(), "No corresponding closing-token found.");

        SVMatchT suffixMatches{};
        rest = StringViewT{closingIter, rest.cend()};
        std::regex_search(rest.cbegin(), rest.cend(), suffixMatches, suffixRegex);
        MIMICPP_ASSERT(!suffixMatches.empty(), "No function suffix found.");
        MIMICPP_ASSERT(suffixMatches.size() == 2, "Regex out-of-sync.");
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
    std::tuple<OutIter, std::size_t, std::size_t> consume_next_scope(OutIter out, StringViewT const scope)
    {
        SVMatchT matches{};

        if (constexpr StringViewT templateScopeSuffix{">::"};
            scope.ends_with(templateScopeSuffix))
        {
            return std::tuple{
                prettify_template_scope(std::move(out), scope, templateScopeSuffix),
                0u,
                scope.size()};
        }

        // libc++ sometimes also uses `'lambda'()` for lambdas
        #if MIMICPP_DETAIL_USES_LIBCXX
        if (constexpr StringViewT libcxxLambdaScopePrefix{"'lambda'("};
            scope.starts_with(libcxxLambdaScopePrefix))
        {
            // use a fake group for the lambda number
            static RegexT lambdaEndRegex{R"(\)()::$)"};

            std::size_t count{};
            std::tie(out, count) = prettify_lambda_scope(
                std::move(out),
                lambdaEndRegex,
                StringViewT{scope.cbegin(), scope.cbegin() + libcxxLambdaScopePrefix.size()},
                scope);
            return std::tuple{std::move(out), 0u, count};
        }
        // while libstdc++ sometimes uses `{lambda()#N}`
        #else
        if (constexpr StringViewT lambdaScopePrefix{"{lambda("};
            scope.starts_with(lambdaScopePrefix))
        {
            static RegexT lambdaEndRegex{R"(\)#(\d+)\}::$)"};

            std::size_t count{};
            std::tie(out, count) = prettify_lambda_scope(
                std::move(out),
                lambdaEndRegex,
                StringViewT{scope.cbegin(), scope.cbegin() + lambdaScopePrefix.size()},
                scope);
            return std::tuple{std::move(out), 0u, count};
        }
        #endif

        static const RegexT functionSuffix{
            R"(\))"
            R"(\s*(?:const)?)"
            R"(\s*(?:volatile)?)"
            R"(\s*&{0,2})"
            R"(\s*::$)"};
        if (std::regex_search(scope.cbegin(), scope.cend(), matches, functionSuffix))
        {
            static RegexT const functionScopePrefix{
                R"(^(?:\w+\s+)?)"        // return type (optional)
                R"((operator.+?|\w+)\()" // function-name + (
            };
            StringViewT const rest{scope.cbegin(), matches[0].first};
            SVMatchT prefixMatches{};
            std::regex_search(rest.cbegin(), rest.cend(), prefixMatches, functionScopePrefix);
            MIMICPP_ASSERT(!prefixMatches.empty(), "No corresponding function prefix found.");

            std::size_t count{};
            std::tie(out, count) = prettify_function_scope(
                std::move(out),
                prefixMatches,
                matches);
            return std::tuple{
                std::move(out),
                std::ranges::distance(rest.cbegin(), prefixMatches[0].first),
                count};
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

        static const RegexT terseAnonymousNamespace{R"(\(anonymous namespace\)::)"};
        name = std::regex_replace(name, terseAnonymousNamespace, anonymousNamespaceTargetScopeText.data());

        #if MIMICPP_DETAIL_IS_CLANG
        static const RegexT prettifyTerseLambdas{R"(\$_(\d+))"};
        name = std::regex_replace(name, prettifyTerseLambdas, "lambda#$1");
        #endif

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
    std::tuple<OutIter, std::size_t, std::size_t> consume_next_scope(OutIter out, StringViewT const scope)
    {
        SVMatchT matches{};

        // the anonymous namespace was already prettified, so just print it as-is
        if (scope.ends_with(anonymousNamespaceTargetScopeText))
        {
            return std::tuple{
                std::ranges::copy(anonymousNamespaceTargetScopeText, std::move(out)).out,
                scope.size() - anonymousNamespaceTargetScopeText.size(),
                anonymousNamespaceTargetScopeText.size()};
        }

        if (constexpr StringViewT templateScopeSuffix{">::"};
            scope.ends_with(templateScopeSuffix))
        {
            return std::tuple{
                prettify_template_scope(std::move(out), scope, templateScopeSuffix),
                0u,
                scope.size()};
        }

        // was already prettified, thus return as-is
        if (static RegexT const lambdaScope{R"(lambda#\d+::$)"};
            std::regex_search(scope.cbegin(), scope.cend(), matches, lambdaScope))
        {
            return std::tuple{
                std::ranges::copy(matches[0].first, matches[0].second, std::move(out)).out,
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }

        if (static RegexT const regularScope{R"(\w+::$)"};
            std::regex_search(scope.cbegin(), scope.cend(), matches, regularScope))
        {
            return std::tuple{
                std::ranges::copy(matches[0].first, matches[0].second, std::move(out)).out,
                std::ranges::distance(scope.cbegin(), matches[0].first),
                matches[0].length()};
        }

        static const RegexT functionSuffix{
            R"(\))"
            R"(\s*(?:const)?)"
            R"(\s*(?:volatile)?)"
            R"(\s*&{0,2})"
            R"(\s*'::$)"};
        if (std::regex_search(scope.cbegin(), scope.cend(), matches, functionSuffix))
        {
            static RegexT const functionScopePrefix{
                "`"
                R"((?:\w+\s+)?)"         // return type (optional)
                R"((operator.+?|\w+)\()" // function-name + (
            };
            StringViewT const rest{scope.cbegin(), matches[0].first};
            SVMatchT prefixMatches{};
            std::regex_search(rest.cbegin(), rest.cend(), prefixMatches, functionScopePrefix);
            MIMICPP_ASSERT(!prefixMatches.empty(), "No corresponding function prefix found.");

            std::size_t count{};
            std::tie(out, count) = prettify_function_scope(
                std::move(out),
                prefixMatches,
                matches);
            return std::tuple{
                std::move(out),
                std::ranges::distance(rest.cbegin(), prefixMatches[0].first),
                count};
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

        static const RegexT terseAnonymousNamespace{"`anonymous namespace'::"};
        name = std::regex_replace(name, terseAnonymousNamespace, anonymousNamespaceTargetScopeText.data());

        static const RegexT omitVirtualNamespace{R"(`\d+'::)"};
        name = std::regex_replace(name, omitVirtualNamespace, "");

        static const RegexT prettifyLambda{R"(<lambda_(\d+)>)"};
        name = std::regex_replace(name, prettifyLambda, "lambda#$1");

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
    constexpr StringViewT scopeDelimiter{"::"};
    constexpr StringViewT argumentDelimiter{","};
    constexpr std::array openingBrackets{'<', '(', '[', '{'};
    constexpr std::array closingBrackets{'>', ')', ']', '}'};

    template <typename OutIter>
    constexpr OutIter prettify_scopes(OutIter out, StringT name)
    {
        while (auto const match = util::find_next_unwrapped_token(
                   name,
                   scopeDelimiter,
                   openingBrackets,
                   closingBrackets))
        {
            std::size_t index{};
            std::size_t count{};
            std::tie(out, index, count) = consume_next_scope(
                out,
                StringViewT{name.cbegin(), match.end()});
            name.erase(index, count);
        }

        return out;
    }

    constexpr auto is_space = [](CharT const c) {
        return static_cast<bool>(std::isspace(static_cast<int>(c)));
    };

    [[nodiscard]]
    inline StringViewT trimmed(StringViewT const str)
    {
        return {
            std::ranges::find_if_not(str, is_space),
            std::ranges::find_if_not(str | std::views::reverse, is_space).base()};
    }

    struct special_type_info
    {
        struct function_t
        {
            StringT returnType;
            StringT argList;
            StringT specifiers;
        };

        std::optional<function_t> functionInfo{};

        struct template_t
        {
            StringT argList;
        };

        std::optional<template_t> templateInfo{};
    };

    [[nodiscard]]
    inline std::tuple<StringT, std::optional<special_type_info::function_t>> detect_function_type_info(StringT name)
    {
        static const RegexT functionSuffix{
            R"(\))"
            R"(((?:\s*\w+\b)*)"
            R"(\s*&{0,2}))"
            R"(\s*$)"};
        if (SMatchT matches{};
            std::regex_search(name, matches, functionSuffix))
        {
            special_type_info::function_t functionInfo{};
            auto& [returnType, argList, specs] = functionInfo;

            specs.assign(matches[1].first, matches[1].second);

            auto reversedName = name
                              | std::views::reverse
                              | std::views::drop(matches[0].length());
            auto const argListBeginIter = util::find_closing_token(reversedName, ')', '(');
            MIMICPP_ASSERT(argListBeginIter != reversedName.end(), "No function begin found.");
            if (StringViewT const args{argListBeginIter.base(), matches[0].first};
                args != "void")
            {
                argList.assign(args.cbegin(), args.cend());
            }

            auto const returnTypeDelimiter = util::find_next_unwrapped_token(
                std::ranges::subrange{argListBeginIter + 1, reversedName.end()},
                " ",
                closingBrackets,
                openingBrackets);
            DEBUG_ASSERT(returnTypeDelimiter, "No return-type found.");
            returnType.assign(reversedName.end().base(), returnTypeDelimiter.end().base());

            name = StringT{returnTypeDelimiter.begin().base(), argListBeginIter.base() - 1};

            return {
                std::move(name),
                std::move(functionInfo)};
        }

        return {
            std::move(name),
            std::nullopt};
    }

    [[nodiscard]]
    inline std::tuple<StringT, std::optional<special_type_info::template_t>> detect_template_type_info(StringT name)
    {
        if (name.back() == '>')
        {
            auto reversedName = name
                              | std::views::reverse
                              | std::views::drop(1);
            auto const iter = util::find_closing_token(reversedName, '>', '<');
            MIMICPP_ASSERT(iter != reversedName.end(), "No template begin found.");
            special_type_info::template_t info{
                {iter.base(), name.end() - 1}
            };

            name.erase(iter.base() - 1, name.cend());

            return {
                std::move(name),
                std::move(info)};
        }

        return {
            std::move(name),
            std::nullopt};
    }

    [[nodiscard]]
    inline std::tuple<StringT, special_type_info> detect_special_type_info(StringT name)
    {
    #if MIMICPP_DETAIL_IS_GCC

        static RegexT lambdaSuffixRegex{R"(#(\d+)}$)"};
        // it's a lambda
        if (SMatchT matches{};
            std::regex_search(name, matches, lambdaSuffixRegex))
        {
            auto reversedName = name
                              | std::views::reverse
                              | std::views::drop(matches[0].length());
            auto const iter = util::find_closing_token(reversedName, '}', '{');
            MIMICPP_ASSERT(iter != reversedName.end(), "No lambda begin found.");

            auto outIter = std::ranges::copy(StringViewT{"lambda#"}, iter.base() - 1).out;
            outIter = std::ranges::copy(matches[1].first, matches[1].second, outIter).out;
            name.erase(outIter, name.cend());

            return {
                std::move(name),
                {}};
        }

    #endif

        special_type_info info{};
        std::tie(name, info.functionInfo) = detect_function_type_info(std::move(name));
        std::tie(name, info.templateInfo) = detect_template_type_info(std::move(name));

        return {
            std::move(name),
            std::move(info)};
    }

    template <print_iterator OutIter>
    constexpr OutIter prettify_arg_list(OutIter out, StringViewT const argList)
    {
        bool isFirst{true};
        StringViewT pendingArgList{argList};
        while (!pendingArgList.empty())
        {
            if (!std::exchange(isFirst, false))
            {
                out = std::ranges::copy(StringViewT{", "}, std::move(out)).out;
            }

            auto const tokenDelimiter = util::find_next_unwrapped_token(
                pendingArgList,
                argumentDelimiter,
                openingBrackets,
                closingBrackets);
            StringViewT const arg = trimmed(StringViewT{pendingArgList.begin(), tokenDelimiter.begin()});
            out = type::prettify_identifier(std::move(out), StringT{arg});
            pendingArgList = StringViewT{tokenDelimiter.end(), pendingArgList.end()};
        }

        return out;
    }
}

namespace mimicpp::printing::type
{
    template <print_iterator OutIter>
    constexpr OutIter prettify_identifier(OutIter out, StringT name)
    {
        name = detail::apply_general_prettification(std::move(name));

        detail::special_type_info specialTypeInfo{};
        std::tie(name, specialTypeInfo) = detail::detect_special_type_info(std::move(name));

        if (specialTypeInfo.functionInfo)
        {
            out = type::prettify_identifier(
                std::move(out),
                std::move(specialTypeInfo.functionInfo->returnType));
            out = format::format_to(std::move(out), " ");
        }

        // maybe (member-)function pointer
        bool const isParensWrapped = name.starts_with("(")
                                  && name.ends_with(")");
        if (isParensWrapped)
        {
            out = format::format_to(std::move(out), "(");
            name.pop_back();
            name.erase(0, 1);
        }

        if (auto reversedName = name | std::views::reverse;
            auto const lastScopeMatch = util::find_next_unwrapped_token(
                reversedName,
                detail::scopeDelimiter,
                detail::closingBrackets,
                detail::openingBrackets))
        {
            std::size_t const splitIndex = std::ranges::distance(name.cbegin(), lastScopeMatch.begin().base());
            StringT topLevelIdentifier = name.substr(splitIndex);

            // includes last ::
            name.erase(splitIndex);
            out = detail::prettify_scopes(std::move(out), std::move(name));

            name = std::move(topLevelIdentifier);
        }

        out = std::ranges::copy(name, std::move(out)).out;

        if (isParensWrapped)
        {
            out = format::format_to(std::move(out), ")");
        }

        if (specialTypeInfo.templateInfo)
        {
            out = format::format_to(std::move(out), "<");
            out = detail::prettify_arg_list(std::move(out), specialTypeInfo.templateInfo->argList);
            out = format::format_to(std::move(out), ">");
        }

        if (specialTypeInfo.functionInfo)
        {
            auto const& [ret, args, specs] = *specialTypeInfo.functionInfo;

            out = format::format_to(std::move(out), "(");
            out = detail::prettify_arg_list(std::move(out), args);
            out = format::format_to(std::move(out), ")");
            out = std::ranges::copy(specs, std::move(out)).out;
        }

        return out;
    }
}

#endif

#endif
