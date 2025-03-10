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
    constexpr StringViewT scopeDelimiter{"::"};
    constexpr StringViewT argumentDelimiter{","};
    constexpr std::array openingBrackets{'<', '(', '[', '{'};
    constexpr std::array closingBrackets{'>', ')', ']', '}'};

    // GCOVR_EXCL_START

    // see: https://en.cppreference.com/w/cpp/string/byte/isspace
    constexpr auto is_space = [](char const c) noexcept {
        return static_cast<bool>(std::isspace(static_cast<unsigned char>(c)));
    };

    // see: https://en.cppreference.com/w/cpp/string/byte/isdigit
    constexpr auto is_digit = [](char const c) noexcept {
        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(c)));
    };

    // GCOVR_EXCL_STOP

    [[nodiscard]]
    constexpr StringViewT trimmed(StringViewT const str)
    {
        return {
            std::ranges::find_if_not(str, is_space),
            std::ranges::find_if_not(str | std::views::reverse, is_space).base()};
    }
}

    #if MIMICPP_DETAIL_IS_GCC \
        || MIMICPP_DETAIL_IS_CLANG
namespace mimicpp::printing::type::detail
{
        #if MIMICPP_DETAIL_USES_LIBCXX

    constexpr StringT unify_lambdas(StringT name)
    {
        // `'lambda'(...)` => `lambda(...)`

        constexpr StringViewT lambdaPrefix{"\'lambda"};

        auto first = name.begin();
        while (auto const match = std::ranges::search(
                   first,
                   name.end(),
                   lambdaPrefix.cbegin(),
                   lambdaPrefix.cend()))
        {
            // These lambdas sometimes have and sometimes have not an id.
            auto const newParensBegin = std::shift_left(
                match.begin(),
                std::ranges::find_if_not(match.end(), name.cend(), is_digit),
                1);

            auto const parensBegin = std::ranges::find(newParensBegin, name.cend(), '(');
            MIMICPP_ASSERT(parensBegin != name.cend(), "No begin-parenthesis found.");
            auto const parensEnd = util::find_closing_token(
                std::ranges::subrange{parensBegin + 1, name.cend()},
                '(',
                ')');
            MIMICPP_ASSERT(parensEnd != name.cend(), "No end-parenthesis found.");

            auto const newEnd = std::shift_left(newParensBegin, parensEnd + 1, 2);
            name.erase(newEnd, parensEnd + 1);

            first = newParensBegin;
        }

        return name;
    }

        #else

    constexpr StringT unify_lambdas_type1(StringT name)
    {
        // `{lambda(...)#id}` => `lambda#id(...)`

        constexpr StringViewT lambdaPrefix{"{lambda("};

        auto first = name.begin();
        while (auto const match = std::ranges::search(
                   first,
                   name.end(),
                   lambdaPrefix.cbegin(),
                   lambdaPrefix.cend()))
        {
            std::ranges::subrange const rest{match.end(), name.cend()};
            auto const braceEnd = util::find_closing_token(rest, '{', '}');
            MIMICPP_ASSERT(braceEnd != name.cend(), "No corresponding end found.");

            auto const idToken = util::find_next_unwrapped_token(
                std::ranges::subrange{match.end() - 1, braceEnd},
                StringViewT{"#"},
                openingBrackets,
                closingBrackets);
            auto const idLength = std::ranges::distance(idToken.begin(), braceEnd);

            // bring id in front of `(...)`
            std::ranges::rotate(match.end() - 1, idToken.begin(), braceEnd);
            auto const newEnd = std::shift_left(match.begin(), braceEnd, 1);

            name.erase(newEnd, braceEnd + 1);

            first = match.end() - 2 + idLength; // points to `(`
        }

        return name;
    }

    constexpr StringT unify_lambdas_type2(StringT name)
    {
        // `<lambda(...)>` => `lambda(...)`

        constexpr StringViewT lambdaPrefix{"<lambda("};

        auto first = name.begin();
        while (auto const match = std::ranges::search(
                   first,
                   name.end(),
                   lambdaPrefix.cbegin(),
                   lambdaPrefix.cend()))
        {
            std::ranges::subrange const rest{match.end(), name.end()};
            auto const angleEnd = util::find_closing_token(rest, '<', '>');
            MIMICPP_ASSERT(angleEnd != name.cend(), "No corresponding end found.");

            auto const newEnd = std::shift_left(match.begin(), angleEnd, 1);

            name.erase(newEnd, angleEnd + 1);

            first = match.end() - 2; // points to `(`
        }

        return name;
    }

    constexpr StringT unify_lambdas(StringT name)
    {
        name = unify_lambdas_type1(std::move(name));
        name = unify_lambdas_type2(std::move(name));

        return name;
    }

        #endif

    [[nodiscard]]
    inline StringT apply_general_prettification(StringT name)
    {
        static const RegexT unifyClosingAngleBrackets{R"(\s+>)"};
        name = std::regex_replace(name, unifyClosingAngleBrackets, ">");

        // `{anonymous}` is emitted by source_location::function_name
        static const RegexT unifyAnonymousNamespace{R"((\{anonymous\}|\(anonymous namespace\))::)"};
        name = std::regex_replace(name, unifyAnonymousNamespace, anonymousNamespaceTargetScopeText.data());

        name = unify_lambdas(std::move(name));

        static const RegexT prettifyAnonTypes{
        #if MIMICPP_DETAIL_IS_CLANG
            R"(\$_(\d+))"
        #else
            R"(\{unnamed type#(\d+)\})"
        #endif
        };
        name = std::regex_replace(name, prettifyAnonTypes, "{anon-type#$1}");

        static const RegexT prettifyAnonClass{
        #if MIMICPP_DETAIL_IS_CLANG
            R"(\(anonymous class\))"
        #else
            R"(<unnamed class>)"
        #endif
        };
        name = std::regex_replace(name, prettifyAnonClass, "{anon-class}");

        static const RegexT prettifyAnonStruct{
        #if MIMICPP_DETAIL_IS_CLANG
            R"(\(anonymous struct\))"
        #else
            R"(<unnamed struct>)"
        #endif
        };
        name = std::regex_replace(name, prettifyAnonStruct, "{anon-struct}");

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
        constexpr std::array opening{'`', '<', '(', '{', '['};
        constexpr std::array closing{'\'', '>', ')', '}', ']'};

        auto const wrapEnd = util::find_closing_token(
            std::ranges::subrange{wrapBegin + 1, name.cend()},
            '`',
            '\'');
        MIMICPP_ASSERT(wrapEnd != name.cend(), "No corresponding end found.");

        auto contentBegin = std::ranges::find_if_not(wrapBegin + 1, name.cend(), is_space);
        auto contentEnd = std::ranges::find_if_not(
                              std::make_reverse_iterator(wrapEnd),
                              std::make_reverse_iterator(contentBegin),
                              is_space)
                              .base();
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
    constexpr OutIter prettify(OutIter out, StringViewT name);

    template <print_iterator OutIter>
    constexpr OutIter prettify_function_identifier(OutIter out, [[maybe_unused]] StringViewT const scope, StringViewT identifier)
    {
        // When `operator` is given, then actually `operator()` were used.
        if (identifier == "operator")
        {
            identifier = "operator()";
        }

        return format::format_to(std::move(out), "{{{}}}::", identifier);
    }

    template <print_iterator OutIter>
    constexpr OutIter prettify_scope(OutIter out, StringViewT const scope)
    {
        if (scope.ends_with('>'))
        {
            // if identifier is empty, the whole `<...>` is probably just a placeholder.
            if (StringViewT const identifier = trimmed(
                    {scope.cbegin(), std::ranges::find(scope, '<')});
                !identifier.empty())
            {
                return format::format_to(std::move(out), "{}::", identifier);
            }
        }
        else if (auto const parensBegin = std::ranges::find(scope, '(');
                 parensBegin != scope.cend())
        {
            // if identifier is empty, the whole `(...)` is probably just a placeholder.
            if (StringViewT const identifier{scope.cbegin(), parensBegin};
                !identifier.empty())
            {
                return prettify_function_identifier(std::move(out), scope, identifier);
            }
        }

        out = format::format_to(std::move(out), "{}::", scope);

        return out;
    }

    struct special_type_info
    {
        struct function_t
        {
            StringViewT returnType;
            StringViewT argList;
            StringViewT specifiers;
        };

        std::optional<function_t> functionInfo{};

        struct template_t
        {
            StringViewT argList;
        };

        std::optional<template_t> templateInfo{};
    };

    [[nodiscard]]
    inline std::optional<special_type_info::function_t> detect_function_type_info(StringViewT& name)
    {
        static const RegexT functionSuffix{
            R"(\)\s*)"
            R"(((?:\s*\w+\b)*)"
            R"(\s*&{0,2}))"
            R"(\s*$)"};
        if (SVMatchT matches{};
            std::regex_search(name.cbegin(), name.cend(), matches, functionSuffix))
        {
            StringViewT const specs{matches[1].first, matches[1].second};

            auto reversedName = name
                              | std::views::reverse
                              | std::views::drop(matches[0].length());
            auto const argListBeginIter = util::find_closing_token(reversedName, ')', '(');
            MIMICPP_ASSERT(argListBeginIter != reversedName.end(), "No function begin found.");

            // If no actual identifier is contained, it can not be a function and is thus probably a placeholder.
            if (StringViewT const rest = trimmed({name.cbegin(), argListBeginIter.base() - 1});
                rest.empty()
                || rest.ends_with("::"))
            {
                return std::nullopt;
            }

            StringViewT args = trimmed({argListBeginIter.base(), matches[0].first});

    #if MIMICPP_DETAIL_IS_MSVC \
        || MIMICPP_DETAIL_IS_CLANG_CL

            if (args == "void")
            {
                args = {};
            }

    #endif

            // If a whitespace exists before `(`, it's probably the return type.
            // If it's not there, just use `auto`.
            StringViewT returnType{};
            if (auto const returnTypeDelimiter = util::find_next_unwrapped_token(
                    std::ranges::subrange{argListBeginIter + 1, reversedName.end()},
                    " ",
                    closingBrackets,
                    openingBrackets))
            {
                returnType = trimmed({reversedName.end().base(), returnTypeDelimiter.end().base()});
                name = StringViewT{returnTypeDelimiter.begin().base(), argListBeginIter.base() - 1};
            }
            else
            {
                returnType = "auto";
                name = StringViewT{name.cbegin(), argListBeginIter.base() - 1};
            }

            return special_type_info::function_t{
                .returnType = returnType,
                .argList = args,
                .specifiers = specs};
        }

        return std::nullopt;
    }

    [[nodiscard]]
    constexpr std::optional<special_type_info::template_t> detect_template_type_info(StringViewT& name)
    {
        if (name.ends_with('>'))
        {
            auto reversedName = name
                              | std::views::reverse
                              | std::views::drop(1);
            auto const iter = util::find_closing_token(reversedName, '>', '<');
            MIMICPP_ASSERT(iter != reversedName.end(), "No template begin found.");
            StringViewT const args = trimmed({iter.base(), name.end() - 1});

            // If no actual identifier is contained, it can not be a template and is thus probably a placeholder.
            StringViewT const rest = trimmed({name.cbegin(), iter.base() - 1});
            if (rest.empty()
                || rest.ends_with("::"))
            {
                return std::nullopt;
            }

            name = rest;

            return special_type_info::template_t{
                .argList = args};
        }

        return std::nullopt;
    }

    [[nodiscard]]
    inline special_type_info detect_special_type_info(StringViewT& name)
    {
        return special_type_info{
            .functionInfo = detect_function_type_info(name),
            .templateInfo = detect_template_type_info(name)};
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
            out = detail::prettify(std::move(out), arg);
            pendingArgList = StringViewT{tokenDelimiter.end(), pendingArgList.end()};
        }

        return out;
    }

    template <print_iterator OutIter>
    constexpr OutIter prettify_top_level_identifier(OutIter out, StringViewT const identifier)
    {
        out = std::ranges::copy(identifier, std::move(out)).out;

        return out;
    }

    template <print_iterator OutIter>
    constexpr OutIter handle_identifier(OutIter out, StringViewT name)
    {
        // maybe (member-)function pointer?
        bool const isParensWrapped = name.starts_with("(")
                                  && name.ends_with(")");
        if (isParensWrapped)
        {
            out = format::format_to(std::move(out), "(");
            name = name.substr(1u, name.size() - 2u);
        }

        while (auto const nextDelimiter = util::find_next_unwrapped_token(
                   name,
                   scopeDelimiter,
                   openingBrackets,
                   closingBrackets))
        {
            StringViewT const scope = trimmed({name.cbegin(), nextDelimiter.begin()});
            out = prettify_scope(std::move(out), scope);
            name.remove_prefix(scope.size() + nextDelimiter.size());
        }

        out = detail::prettify_top_level_identifier(std::move(out), name);

        if (isParensWrapped)
        {
            out = format::format_to(std::move(out), ")");
        }

        return out;
    }

    template <print_iterator OutIter>
    constexpr OutIter prettify(OutIter out, StringViewT name)
    {
        const auto [functionInfo, templateInfo] = detect_special_type_info(name);

        if (functionInfo)
        {
            out = detail::prettify(std::move(out), functionInfo->returnType);
            out = format::format_to(std::move(out), " ");
        }

        out = detail::handle_identifier(std::move(out), name);

        if (templateInfo)
        {
            out = format::format_to(std::move(out), "<");
            out = detail::prettify_arg_list(std::move(out), templateInfo->argList);
            out = format::format_to(std::move(out), ">");
        }

        if (functionInfo)
        {
            auto const& [ret, args, specs] = *functionInfo;

            out = format::format_to(std::move(out), "(");
            out = detail::prettify_arg_list(std::move(out), args);
            out = format::format_to(std::move(out), ")");
            out = std::ranges::copy(specs, std::move(out)).out;
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

        return detail::prettify(std::move(out), name);
    }
}

#endif

#endif
