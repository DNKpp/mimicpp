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
     * \note When `MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES` is disabled,
     * this function simply outputs the provided name without any modifications.
     */
    template <print_iterator OutIter>
    constexpr OutIter prettify_identifier(OutIter out, StringT name);
}

#ifndef MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES

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
    constexpr StringViewT anonymousNamespaceTargetScopeText{"{anon-ns}"};
    constexpr StringViewT scopeDelimiter{"::"};
    constexpr StringViewT argumentDelimiter{","};

    #if MIMICPP_DETAIL_IS_MSVC \
        || MIMICPP_DETAIL_IS_CLANG_CL
    constexpr std::array openingBrackets{'<', '(', '[', '{', '`'};
    constexpr std::array closingBrackets{'>', ')', ']', '}', '\''};
    #else
    constexpr std::array openingBrackets{'<', '(', '[', '{'};
    constexpr std::array closingBrackets{'>', ')', ']', '}'};
    #endif

    // see: https://en.cppreference.com/w/cpp/string/byte/isspace
    constexpr auto is_space = [](char const c) noexcept {
        return static_cast<bool>(std::isspace(static_cast<unsigned char>(c)));
    };

    template <std::random_access_iterator Iter>
        requires std::constructible_from<StringViewT, Iter, Iter>
    [[nodiscard]]
    constexpr StringViewT trimmed(Iter const begin, Iter const end)
    {
        auto const trimmedBegin = std::ranges::find_if_not(begin, end, is_space);
        auto const trimmedEnd = std::ranges::find_if_not(
            std::make_reverse_iterator(end),
            std::make_reverse_iterator(trimmedBegin),
            is_space);
        return StringViewT{trimmedBegin, trimmedEnd.base()};
    }

    [[nodiscard]]
    constexpr StringViewT trimmed(StringViewT const str)
    {
        return trimmed(str.cbegin(), str.cend());
    }

    struct function_info
    {
        StringViewT returnType{};
        StringViewT argList{};
        StringViewT specs{};
    };

    struct template_info
    {
        StringViewT argList{};
        StringViewT specs{};
    };

    struct scope_info
    {
        StringViewT identifier{};
        std::optional<function_info> functionInfo{};
        std::optional<template_info> templateInfo{};
    };

    [[nodiscard]]
    constexpr std::ranges::borrowed_subrange_t<StringViewT> detect_last_enclosed(
        StringViewT const& name,
        StringViewT const opening,
        StringViewT const closing)
    {
        MIMICPP_ASSERT(!name.starts_with(' ') && !name.ends_with(' '), "Name is not trimmed.");
        MIMICPP_ASSERT(1u == opening.size(), "Opening token must have a size of one.");
        MIMICPP_ASSERT(1u == closing.size(), "Closing token must have a size of one.");

        auto reversedName = name | std::views::reverse;

        auto const delimiterMatch = util::find_next_unwrapped_token(
            reversedName,
            scopeDelimiter,
            closingBrackets,
            openingBrackets);
        auto const closingMatch = util::find_next_unwrapped_token(
            std::ranges::subrange{reversedName.begin(), delimiterMatch.begin()},
            closing,
            closingBrackets,
            openingBrackets);
        auto const openingIter = util::find_closing_token(
            std::ranges::subrange{closingMatch.end(), delimiterMatch.begin()},
            closing.front(),
            opening.front());
        // If no `opening closing` range could be found or the identifier between `scopeDelimiter opening` is empty,
        // it's not what we are looking for.
        if (openingIter == delimiterMatch.begin()
            || std::ranges::empty(trimmed(delimiterMatch.begin().base(), openingIter.base() - 1)))
        {
            return {name.cend(), name.cend()};
        }

        return {openingIter.base() - 1, closingMatch.begin().base()};
    }

    [[nodiscard]]
    constexpr std::optional<function_info> detect_function_scope_info(StringViewT& name)
    {
        MIMICPP_ASSERT(!name.starts_with(' ') && !name.ends_with(' '), "Name is not trimmed.");

        auto const enclosedMatch = detect_last_enclosed(name, "(", ")");
        if (enclosedMatch.empty())
        {
            return std::nullopt;
        }
        MIMICPP_ASSERT(enclosedMatch.front() == '(' && enclosedMatch.back() == ')', "Unexpected result.");

        function_info info{
            .argList = trimmed(enclosedMatch.begin() + 1, enclosedMatch.end() - 1),
            .specs = trimmed(enclosedMatch.end(), name.end())};
        if (info.argList == "void")
        {
            info.argList = {};
        }

        // Do not trim here, as this will otherwise lead to indistinguishable symbols.
        // Consider the function-type `r()`; Compilers will generate the name `r ()` for this (note the additional ws).
        // Due to reasons, some compilers will also generate `i()` (and thus omit the return-type)
        // for functions with complex return types. The delimiting ws is the only assumption we have.
        // Due to this, an empty identifier is valid.
        name = {name.cbegin(), enclosedMatch.begin()};

        // If a whitespace exists before `(`, it's probably the return type.
        if (auto const returnTypeDelimiter = util::find_next_unwrapped_token(
                name | std::views::reverse,
                " ",
                closingBrackets,
                openingBrackets))
        {
            info.returnType = trimmed(name.cbegin(), returnTypeDelimiter.end().base());
            name = {returnTypeDelimiter.begin().base(), name.cend()};
        }

        return info;
    }

    [[nodiscard]]
    constexpr std::optional<template_info> detect_template_scope_info(StringViewT& name)
    {
        MIMICPP_ASSERT(!name.starts_with(' ') && !name.ends_with(' '), "Name is not trimmed.");

        auto const enclosedMatch = detect_last_enclosed(name, "<", ">");
        if (enclosedMatch.empty())
        {
            return std::nullopt;
        }
        MIMICPP_ASSERT(enclosedMatch.front() == '<' && enclosedMatch.back() == '>', "Unexpected result.");

        StringViewT const specs = trimmed(enclosedMatch.end(), name.cend());
        StringViewT const args = trimmed(enclosedMatch.begin() + 1, enclosedMatch.end() - 1);
        name = trimmed(name.cbegin(), enclosedMatch.begin());

        return template_info{
            .argList = args,
            .specs = specs};
    }

    [[nodiscard]]
    constexpr scope_info gather_scope_info(StringViewT scope)
    {
        scope_info info{};
        info.functionInfo = detect_function_scope_info(scope);
        info.templateInfo = detect_template_scope_info(scope);

        info.identifier = scope;

        return info;
    }

    class ScopeIterator
    {
    public:
        [[nodiscard]]
        explicit constexpr ScopeIterator(StringViewT content) noexcept
            : m_Pending{std::move(content)}
        {
        }

        [[nodiscard]]
        constexpr StringViewT pending() const noexcept
        {
            return m_Pending;
        }

        [[nodiscard]]
        std::optional<scope_info> operator()()
        {
            if (m_Pending.empty())
            {
                return std::nullopt;
            }

            auto const delimiter = util::find_next_unwrapped_token(
                m_Pending,
                scopeDelimiter,
                openingBrackets,
                closingBrackets);
            StringViewT const scope = trimmed(m_Pending.cbegin(), delimiter.begin());
            m_Pending = StringViewT{delimiter.end(), m_Pending.end()};

            return gather_scope_info(scope);
        }

    private:
        StringViewT m_Pending;
    };

    [[nodiscard]]
    inline StringT transform_special_operators(StringT name)
    {
        static RegexT const transformOpSpaceship{R"(\boperator\s?<=>)"};
        name = std::regex_replace(name, transformOpSpaceship, "operator-spaceship");

        static RegexT const transformOpLessEq{R"(\boperator\s?<=)"};
        name = std::regex_replace(name, transformOpLessEq, "operator-le");

        static RegexT const transformOpLess{R"(\boperator\s?<)"};
        name = std::regex_replace(name, transformOpLess, "operator-lt");

        static RegexT const transformOpGreaterEq{R"(\boperator\s?>=)"};
        name = std::regex_replace(name, transformOpGreaterEq, "operator-ge");

        static RegexT const transformOpGreater{R"(\boperator\s?>)"};
        name = std::regex_replace(name, transformOpGreater, "operator-gt");

        static RegexT const transformOpInvoke{R"(\boperator\s?\(\))"};
        name = std::regex_replace(name, transformOpInvoke, "operator-invoke");

        return name;
    }

    [[nodiscard]]
    inline StringT remove_template_details(StringT name)
    {
        if (name.ends_with(']'))
        {
            auto rest = name | std::views::reverse | std::views::drop(1);
            if (auto const closingIter = util::find_closing_token(rest, ']', '[');
                closingIter != rest.end())
            {
                auto const end = std::ranges::find_if_not(closingIter + 1, rest.end(), is_space);
                name.erase(end.base(), name.end());
            }
        }

        return name;
    }
}

    #if MIMICPP_DETAIL_IS_GCC \
        || MIMICPP_DETAIL_IS_CLANG
namespace mimicpp::printing::type::detail
{
        #if MIMICPP_DETAIL_USES_LIBCXX

    // see: https://en.cppreference.com/w/cpp/string/byte/isdigit
    constexpr auto is_digit = [](char const c) noexcept {
        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(c)));
    };

    constexpr StringT unify_lambdas(StringT name)
    {
        // `'lambda'(...)` => `lambda`
        // or `'lambdaid'(...) => 'lambdaid'

        constexpr StringViewT lambdaPrefix{"\'lambda"};

        auto first = name.begin();
        while (auto const match = std::ranges::search(
                   first,
                   name.end(),
                   lambdaPrefix.cbegin(),
                   lambdaPrefix.cend()))
        {
            // These lambdas sometimes have and sometimes have not an id.
            auto const newEnd = std::shift_left(
                match.begin(),
                std::ranges::find_if_not(match.end(), name.cend(), is_digit),
                1);

            auto const parensBegin = std::ranges::find(newEnd, name.cend(), '(');
            MIMICPP_ASSERT(parensBegin != name.cend(), "No begin-parenthesis found.");
            auto const parensEnd = util::find_closing_token(
                std::ranges::subrange{parensBegin + 1, name.cend()},
                '(',
                ')');
            MIMICPP_ASSERT(parensEnd != name.cend(), "No end-parenthesis found.");

            name.erase(newEnd, parensEnd + 1);

            first = newEnd;
        }

        return name;
    }

        #else

    constexpr StringT unify_lambdas_type1(StringT name)
    {
        // `{lambda(...)#id}` => `lambda#id`

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

            // Bring `lambda#id`to the front.
            auto newEnd = std::shift_left(match.begin(), match.end() - 1, 1);
            newEnd = std::ranges::copy(idToken.begin(), braceEnd, newEnd).out;

            name.erase(newEnd, braceEnd + 1);

            first = newEnd;
        }

        return name;
    }

    constexpr StringT unify_lambdas_type2(StringT name)
    {
        // `<lambda(...)>` => `lambda`

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

            auto const newEnd = std::shift_left(match.begin(), match.end() - 1, 1);

            name.erase(newEnd, angleEnd + 1);

            first = newEnd;
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
    inline StringT apply_basic_transformations(StringT name)
    {
        name = remove_template_details(std::move(name));

        if (constexpr StringViewT constexprToken{"constexpr "};
            name.starts_with(constexprToken))
        {
            name.erase(0, constexprToken.size());
        }

        static const RegexT unifyStdImplNamespace{
        #if MIMICPP_DETAIL_USES_LIBCXX
            "std::__1::"
        #else
            "std::__cxx11::"
        #endif
        };
        name = std::regex_replace(name, unifyStdImplNamespace, "std::");

        name = transform_special_operators(std::move(name));
        name = unify_lambdas(std::move(name));

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
        // `\`ret fn(...) specs'` => `fn(...) specs`

        auto const contentBegin = std::ranges::find_if_not(wrapBegin + 1, name.cend(), is_space);
        auto const wrapEnd = util::find_closing_token(
            std::ranges::subrange{contentBegin, name.cend()},
            '`',
            '\'');
        MIMICPP_ASSERT(wrapEnd != name.cend(), "No corresponding end found.");

        auto const parensEnd = util::find_next_unwrapped_token(
            std::ranges::subrange{
                std::make_reverse_iterator(wrapEnd),
                std::make_reverse_iterator(contentBegin)},
            ")",
            closingBrackets,
            openingBrackets);
        auto const parensBegin = util::find_closing_token(
            std::ranges::subrange{parensEnd.end(), std::make_reverse_iterator(contentBegin)},
            ')',
            '(');
        if (!parensEnd
            || parensBegin.base() == contentBegin)
        {
            return wrapEnd + 1;
        }

        // The return type seems optional. So, if not delimiter can be found it may still be a function.
        auto const delimiter = util::find_next_unwrapped_token(
            std::ranges::subrange{parensBegin + 1, std::make_reverse_iterator(contentBegin)},
            " ",
            closingBrackets,
            openingBrackets);
        auto const end = std::ranges::copy(
                             delimiter.begin().base(),
                             wrapEnd,
                             name.begin() + std::ranges::distance(name.cbegin(), wrapBegin))
                             .out;

        name.erase(end, wrapEnd + 1);

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
    inline StringT apply_basic_transformations(StringT name)
    {
        name = remove_template_details(std::move(name));

        name = transform_special_operators(std::move(name));

        static RegexT const omitClassStructEnum{R"(\b(class|struct|enum)\s+)"};
        name = std::regex_replace(name, omitClassStructEnum, "");

        static RegexT const omitAccessSpecifiers{R"(\b(?:public|private|protected):\s+)"};
        name = std::regex_replace(name, omitAccessSpecifiers, "");

        static const RegexT omitVirtualNamespace{R"(`\d+'::)"};
        name = std::regex_replace(name, omitVirtualNamespace, "");

        // something like call-convention and __ptr64
        static RegexT const omitImplementationSpecifiers{R"(\b__\w+\b\s*)"};
        name = std::regex_replace(name, omitImplementationSpecifiers, "");

        static const RegexT prettifyLambda{R"(<lambda_(\d+)>)"};
        name = std::regex_replace(name, prettifyLambda, "lambda#$1");

        name = simplify_special_functions(std::move(name));

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
    constexpr OutIter prettify_arg_list(OutIter out, StringViewT const argList)
    {
        bool isFirst{true};
        StringViewT pendingArgList{argList};
        while (!pendingArgList.empty())
        {
            if (!std::exchange(isFirst, false))
            {
                out = format::format_to(std::move(out), ", ");
            }

            auto const tokenDelimiter = util::find_next_unwrapped_token(
                pendingArgList,
                argumentDelimiter,
                openingBrackets,
                closingBrackets);
            out = detail::prettify(
                std::move(out),
                trimmed(pendingArgList.begin(), tokenDelimiter.begin()));
            pendingArgList = StringViewT{tokenDelimiter.end(), pendingArgList.end()};
        }

        return out;
    }

    template <print_iterator OutIter>
    constexpr OutIter prettify_specs(OutIter out, StringViewT const specs)
    {
        out = std::ranges::copy(specs, std::move(out)).out;

        return out;
    }

    [[nodiscard]]
    inline auto& alias_map()
    {
        static const std::unordered_map<StringViewT, StringViewT> aliases{
            {"(anonymous namespace)", anonymousNamespaceTargetScopeText},
            {          "{anonymous}", anonymousNamespaceTargetScopeText},
            {"`anonymous namespace'", anonymousNamespaceTargetScopeText},
            {  "anonymous-namespace", anonymousNamespaceTargetScopeText},
            {  "anonymous namespace", anonymousNamespaceTargetScopeText},
            {          "operator-lt",                       "operator<"},
            {          "operator-le",                      "operator<="},
            {          "operator-gt",                       "operator>"},
            {          "operator-ge",                      "operator>="},
            {   "operator-spaceship",                     "operator<=>"},
            {      "operator-invoke",                      "operator()"}
        };

        return aliases;
    }

    template <print_iterator OutIter>
    constexpr OutIter handle_scope(OutIter out, scope_info const& scope)
    {
        auto const& aliases = alias_map();
        auto const& identifier = scope.identifier;

        // detect member function pointer.
        if (identifier.ends_with("::*)")
            && identifier.starts_with("("))
        {
            out = format::format_to(std::move(out), "(");
            out = detail::prettify(
                std::move(out),
                StringViewT{identifier.cbegin() + 1, identifier.cend() - 1});
            out = format::format_to(std::move(out), ")");
        }
    #if MIMICPP_DETAIL_IS_MSVC
        else if (identifier.starts_with('`') && identifier.ends_with('\''))
        {
            out = detail::prettify(
                std::move(out),
                StringViewT{identifier.cbegin() + 1, identifier.cend() - 1});
        }
    #endif
        else if (auto const iter = aliases.find(scope.identifier);
                 iter != aliases.cend())
        {
            out = std::ranges::copy(iter->second, std::move(out)).out;
        }
        else
        {
            out = std::ranges::copy(scope.identifier, std::move(out)).out;
        }

        return out;
    }

    template <print_iterator OutIter>
    constexpr OutIter handle_identifier(OutIter out, StringViewT const name)
    {
        ScopeIterator iter{name};
        bool isFirst{true};
        while (auto const scopeInfo = iter())
        {
            if (!std::exchange(isFirst, false))
            {
                out = std::ranges::copy(scopeDelimiter, std::move(out)).out;
            }

            out = handle_scope(std::move(out), *scopeInfo);
        }

        return out;
    }

    template <print_iterator OutIter>
    constexpr OutIter prettify(OutIter out, StringViewT const name)
    {
        auto&& [identifier, functionInfo, templateInfo] = gather_scope_info(name);

        if (functionInfo
            && !functionInfo->returnType.empty())
        {
            out = detail::prettify(std::move(out), functionInfo->returnType);
            out = format::format_to(std::move(out), " ");
        }

        out = detail::handle_identifier(std::move(out), identifier);

        if (templateInfo)
        {
            auto const& [args, specs] = *templateInfo;

            out = format::format_to(std::move(out), "<");
            out = detail::prettify_arg_list(std::move(out), args);
            out = format::format_to(std::move(out), ">");
            out = prettify_specs(std::move(out), specs);
        }

        if (functionInfo)
        {
            auto const& [ret, args, specs] = *functionInfo;

            out = format::format_to(std::move(out), "(");
            out = detail::prettify_arg_list(std::move(out), args);
            out = format::format_to(std::move(out), ")");
            out = prettify_specs(std::move(out), specs);
        }

        return out;
    }
}

namespace mimicpp::printing::type
{
    template <print_iterator OutIter>
    constexpr OutIter prettify_identifier(OutIter out, StringT name)
    {
        name = detail::apply_basic_transformations(std::move(name));

        return detail::prettify(std::move(out), detail::trimmed(name));
    }
}

#endif

#endif
