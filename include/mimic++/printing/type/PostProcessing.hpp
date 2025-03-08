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
#include "mimic++/utilities/C++23Backports.hpp"

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
    constexpr StringViewT scopeDelimiter{"::"};
    constexpr StringViewT argumentDelimiter{","};
    constexpr std::array openingBrackets{'<', '(', '[', '{'};
    constexpr std::array closingBrackets{'>', ')', ']', '}'};

    // see: https://en.cppreference.com/w/cpp/string/byte/isspace
    constexpr auto is_space = [](char const c) noexcept {
        return static_cast<bool>(std::isspace(static_cast<unsigned char>(c)));
    };

    // see: https://en.cppreference.com/w/cpp/string/byte/isalpha
    constexpr auto is_alpha = [](char const c) noexcept {
        return static_cast<bool>(std::isalpha(static_cast<unsigned char>(c)));
    };

    // see: https://en.cppreference.com/w/cpp/string/byte/isalpha
    constexpr auto is_digit = [](char const c) noexcept {
        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(c)));
    };

    // see: https://eel.is/c++draft/lex.name#nt:identifier
    // This is just an approximation, as we do not care for the `identifier-start`
    constexpr auto is_identifier_element = [](char const c) noexcept {
        return is_alpha(c)
            || is_digit(c)
            || c == '_';
    };

    [[nodiscard]]
    inline StringViewT trimmed(StringViewT const str)
    {
        return {
            std::ranges::find_if_not(str, is_space),
            std::ranges::find_if_not(str | std::views::reverse, is_space).base()};
    }

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

    constexpr StringViewT templateScopeSuffix{"<>::"};

    template <print_iterator OutIter>
    [[nodiscard]]
    OutIter prettify_template_scope(OutIter out, StringViewT scope)
    {
        scope.remove_suffix(templateScopeSuffix.size());
        out = format::format_to(std::move(out), "{}::", scope);

        return out;
    }

    constexpr StringViewT functionScopeSuffix{"()::"};

    template <print_iterator OutIter>
    [[nodiscard]]
    OutIter prettify_function_scope(OutIter out, StringViewT scope)
    {
    #if MIMICPP_DETAIL_USES_LIBCXX

        // these lambdas sometimes have and sometimes have not an id.
        constexpr StringViewT libcxxLambdaPrefix{"'lambda"};
        constexpr StringViewT libcxxLambdaSuffix{"'()::"};
        if (scope.starts_with(libcxxLambdaPrefix)
            && scope.ends_with(libcxxLambdaSuffix))
        {
            StringViewT const id{
                scope.begin() + std::ranges::ssize(libcxxLambdaPrefix),
                scope.end() - std::ranges::ssize(libcxxLambdaSuffix)};
            // no number assigned, just assign 0
            if (id.empty())
            {
                return format::format_to(std::move(out), "lambda#0::");
            }

            return format::format_to(std::move(out), "lambda#{}::", id);
        }

    #endif

        scope.remove_suffix(functionScopeSuffix.size());
        out = format::format_to(std::move(out), "{{{}}}::", scope);

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
    /**
     * \brief Erases the arg list, return type, ``` and `'` pair, and all specifiers, but keeps the `()` as a marker for later steps.
     */
    constexpr StringT::const_iterator simplify_special_function_scope(StringT& name, StringT::const_iterator const wrapBegin)
    {
        constexpr std::array opening{'`', '<', '(', '{'};
        constexpr std::array closing{'\'', '>', ')', '}'};

        auto const wrapEnd = util::find_closing_token(
            std::ranges::subrange{wrapBegin + 1, name.cend()},
            '`',
            '\'');
        MIMICPP_ASSERT(wrapEnd != name.cend(), "No corresponding end found.");

        auto contentBegin = std::ranges::find_if_not(wrapBegin + 1, name.cend(), is_space);
        auto contentEnd = std::ranges::find_if_not(
            std::make_reverse_iterator(wrapEnd),
            std::make_reverse_iterator(contentBegin),
            is_space).base();
        // If a delimiter is found, it may split the `identifier(...)...` part and the return type (which we want to remove).
        if (auto const delimiter = util::find_next_unwrapped_token(
                std::ranges::subrange{contentBegin, contentEnd},
                " ",
                opening,
                closing))
        {
            auto const potentialNewContentBegin = std::ranges::find_if_not(delimiter.end(), contentEnd, is_space);

            // Is it actually a return-type (so, it's before `(`) or a specified (and thus after `)`)?
            // If it's a specifier, the range will never contain a pair of `()`, so let's detect it here.
            if (contentEnd != std::ranges::find(potentialNewContentBegin, contentEnd, '('))
            {
                contentBegin = potentialNewContentBegin;
            }
        }

        StringT::const_iterator const to = std::ranges::copy(
                                               contentBegin,
                                               contentEnd,
                                               name.begin() + std::ranges::distance(name.cbegin(), wrapBegin))
                                               .out;
        name.erase(to, wrapEnd + 1);

        // return the current position, as we may have copied another special scope to the beginning
        return wrapBegin;
    }

    constexpr StringT simplify_special_functions(StringT name)
    {
        for (auto iter = std::ranges::find(std::as_const(name), '`');
            iter != name.cend();
            iter = std::ranges::find(iter, name.cend(), '`'))
        {
            iter = simplify_special_function_scope(name, iter);
        }

        return name;
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

        static RegexT const omitStaticSpecifier{R"(\bstatic\s+)"};
        name = std::regex_replace(name, omitStaticSpecifier, "");

        // something like call-convention and __ptr64
        static RegexT const omitImplementationSpecifiers{R"(\b__\w+\b\s*)"};
        name = std::regex_replace(name, omitImplementationSpecifiers, "");

        name = simplify_special_functions(std::move(name));

        static RegexT const unifyComma{R"(\s*,\s*)"};
        name = std::regex_replace(name, unifyComma, ", ");

        static RegexT const unifyClosingAngles{R"(\s+>)"};
        name = std::regex_replace(name, unifyClosingAngles, ">");

        static RegexT const unifyAnd{R"(\s+&)"};
        name = std::regex_replace(name, unifyAnd, "&");

        static RegexT const unifyStar{R"(\s+\*)"};
        name = std::regex_replace(name, unifyStar, "*");

        return name;
    }
}

    #endif

namespace mimicpp::printing::type::detail
{
    template <print_iterator OutIter>
    OutIter prettify_function_identifier(OutIter out, [[maybe_unused]] StringViewT const scope, StringViewT identifier)
    {
    #if MIMICPP_DETAIL_USES_LIBCXX

        // these lambdas sometimes have and sometimes have not an id.
        constexpr StringViewT libcxxLambdaPrefix{"'lambda"};
        constexpr StringViewT libcxxLambdaSuffix{"'"};
        if (identifier.starts_with(libcxxLambdaPrefix)
            && identifier.ends_with(libcxxLambdaSuffix))
        {
            StringViewT id{
                identifier.begin() + std::ranges::ssize(libcxxLambdaPrefix),
                identifier.end() - std::ranges::ssize(libcxxLambdaSuffix)};
            // no number assigned, just assign 0
            if (id.empty())
            {
                id = "0";
            }

            return format::format_to(std::move(out), "lambda#{}::", id);
        }

    #elif MIMICPP_DETAIL_IS_GCC \
        || MIMICPP_DETAIL_IS_CLANG

        constexpr StringViewT libstdcxxLambdaPrefix{"{lambda"};
        constexpr StringViewT libstdcxxLambdaSuffix{"}"};

        if (identifier == libstdcxxLambdaPrefix
            && scope.ends_with(libstdcxxLambdaSuffix))
        {
            StringViewT const id{
                std::ranges::find(scope | std::views::reverse, '#').base(),
                scope.cend() - 1};

            return format::format_to(std::move(out), "lambda#{}::", id);
        }

    #endif

        // When `operator` is given, then actually `operator()` were used.
        if (identifier == "operator")
        {
            identifier = "operator()";
        }

        return format::format_to(std::move(out), "{{{}}}::", identifier);
    }

    template <print_iterator OutIter>
    OutIter prettify_scope(OutIter out, StringViewT const scope)
    {
        if (scope.ends_with('>'))
        {
            StringViewT const identifier{
                scope.cbegin(),
                std::ranges::find(scope, '<')};

            return format::format_to(std::move(out), "{}::", identifier);
        }

        if (auto const parensBegin = std::ranges::find(scope, '(');
            parensBegin != scope.cend())
        {
            StringViewT const identifier{scope.cbegin(), parensBegin};

            return prettify_function_identifier(std::move(out), scope, identifier);
        }

        // probably regular scope
        out = format::format_to(std::move(out), "{}::", scope);

        return out;
    }

    template <print_iterator OutIter>
    OutIter handle_scope(OutIter out, StringViewT const scope)
    {
        if (scope.ends_with(templateScopeSuffix))
        {
            return prettify_template_scope(std::move(out), scope);
        }

        if (scope.ends_with(functionScopeSuffix))
        {
            return prettify_function_scope(std::move(out), scope);
        }

        // Probably a regular c++-scope
        out = std::ranges::copy(scope, std::move(out)).out;

        return out;
    }

    template <typename OutIter>
    constexpr OutIter prettify_scopes(OutIter out, StringViewT const name)
    {
        auto first = name.cbegin();
        while (auto const match = std::ranges::search(
                   first,
                   name.cend(),
                   scopeDelimiter.cbegin(),
                   scopeDelimiter.cend()))
        {
            out = handle_scope(std::move(out), StringViewT{first, match.end()});
            first = match.end();
        }

        return out;
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
            R"(\)\s*)"
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
            MIMICPP_ASSERT(returnTypeDelimiter, "No return-type found.");
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
        if (name.ends_with('>'))
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

    /**
     * \brief Erases the arg list and removes the `()`.
     * `{lambda(...)#01234}` will be transformed to `lambda#01234`
     */
    constexpr StringT::const_iterator simplify_lambda_scope(StringT& name, StringT::iterator const braceBegin)
    {
        auto const braceEnd = util::find_closing_token(
            std::ranges::subrange{braceBegin + 1, name.cend()},
            '{',
            '}');
        MIMICPP_ASSERT(braceEnd != name.cend(), "No corresponding end found.");
        auto const nextDelimiter = util::find_next_unwrapped_token(
            std::ranges::subrange{braceEnd + 1, name.end()},
            scopeDelimiter,
            openingBrackets,
            closingBrackets);
        auto const delimiterSize = std::ranges::ssize(nextDelimiter);

        auto to = std::ranges::copy(StringViewT{"lambda#"}, braceBegin).out;

        // Extract the lambda-id between `)#` and `}`
        auto const noBegin = std::ranges::find_if_not(
                                 std::make_reverse_iterator(braceEnd),
                                 std::make_reverse_iterator(braceBegin + 1),
                                 is_digit)
                                 .base();
        to = std::ranges::copy(noBegin, braceEnd, to).out;

        name.erase(to, nextDelimiter.begin());

        return to + delimiterSize;
    }

    /**
     * \brief Erases the arg list and all specifiers, but keeps the `()` as a marker for later steps.
     */
    constexpr StringT::const_iterator simplify_function_scope(StringT& name, StringT::const_iterator parensBegin)
    {
        StringViewT const prefix{name.cbegin(), parensBegin};
        if (constexpr StringViewT lambdaPrefixToken{"{lambda"};
            prefix.ends_with(lambdaPrefixToken))
        {
            return simplify_lambda_scope(
                name,
                // retrieve a mutable iterator to the `{`
                name.begin()
                    + (std::ranges::distance(name.begin(), parensBegin) - std::ranges::ssize(lambdaPrefixToken)));
        }

        auto const parensEnd = util::find_closing_token(
            std::ranges::subrange{parensBegin + 1, name.cend()},
            '(',
            ')');
        MIMICPP_ASSERT(parensEnd != name.cend(), "No corresponding end found.");
        auto const nextDelimiter = util::find_next_unwrapped_token(
            std::ranges::subrange{parensEnd + 1, name.cend()},
            scopeDelimiter,
            openingBrackets,
            closingBrackets);
        auto const delimiterSize = std::ranges::ssize(nextDelimiter);

        // If it's operator(), we need to skip the first `()` pair.
        if (prefix.ends_with("operator"))
        {
            std::ranges::advance(parensBegin, 2);
        }

        name.replace(parensBegin + 1, nextDelimiter.begin(), 1u, ')');

        return parensBegin + 2 + delimiterSize;
    }

    /**
     * \brief Erases the arg list, but keeps the `<>` as a marker for later steps.
     */
    constexpr StringT::const_iterator simplify_template_scope(StringT& name, StringT::const_iterator const openingBracket)
    {
        auto const closingBracket = util::find_closing_token(
            std::ranges::subrange{openingBracket + 1, name.cend()},
            '<',
            '>');
        MIMICPP_ASSERT(closingBracket != name.cend(), "No corresponding end found.");
        name.erase(openingBracket + 1, closingBracket);

        return openingBracket + 2;
    }

    /**
     * \brief Removes all nested template and function details
     */
    #if MIMICPP_DETAIL_IS_MSVC \
        || MIMICPP_DETAIL_IS_CLANG_CL

    constexpr StringT simplify_scopes(StringT name)
    {
        constexpr std::array beginTokens{'<', '(', '`'};

        for (auto iter = std::ranges::find_first_of(std::as_const(name), beginTokens);
             iter != name.cend();
             iter = std::ranges::find_first_of(iter, name.cend(), beginTokens.cbegin(), beginTokens.cend()))
        {
            switch (*iter)
            {
            case '(':
                iter = simplify_function_scope(name, iter);
                break;

            case '<':
                iter = simplify_template_scope(name, iter);
                break;

            case '`':
                iter = simplify_special_function_scope(name, iter);
                break;

                // GCOVR_EXCL_START
            default:
                util::unreachable();
                // GCOVR_EXCL_STOP
            }
        }

        return name;
    }
    #else

    constexpr StringT simplify_scopes(StringT name)
    {
        constexpr std::array beginTokens{'<', '('};

        for (auto iter = std::ranges::find_first_of(std::as_const(name), beginTokens);
             iter != name.cend();
             iter = std::ranges::find_first_of(iter, name.cend(), beginTokens.cbegin(), beginTokens.cend()))
        {
            switch (*iter)
            {
            case '(':
                iter = simplify_function_scope(name, iter);
                break;

            case '<':
                iter = simplify_template_scope(name, iter);
                break;

                // GCOVR_EXCL_START
            default:
                util::unreachable();
                // GCOVR_EXCL_STOP
            }
        }
        return name;
    }

    #endif

    template <print_iterator OutIter>
    constexpr OutIter handle_identifier(OutIter out, StringViewT name)
    {
        // maybe (member-)function pointer?
        bool const isParensWrapped = name.starts_with("(")
                                  && name.ends_with(")");
        if (isParensWrapped)
        {
            out = format::format_to(std::move(out), "(");
            name.remove_prefix(1u);
            name.remove_suffix(1u);
        }

        while (auto const nextDelimiter = util::find_next_unwrapped_token(
                   name,
                   scopeDelimiter,
                   openingBrackets,
                   closingBrackets))
        {
            StringViewT const scope{name.cbegin(), nextDelimiter.begin()};
            out = prettify_scope(std::move(out), scope);
            name.remove_prefix(scope.size() + nextDelimiter.size());
        }

        out = std::ranges::copy(name, std::move(out)).out;

        if (isParensWrapped)
        {
            out = format::format_to(std::move(out), ")");
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

        out = detail::handle_identifier(std::move(out), std::move(name));

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
