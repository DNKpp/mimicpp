//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_PRINTING_TYPE_NAME_LEXER_HPP
#define MIMICPP_PRINTING_TYPE_NAME_LEXER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/Algorithm.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <concepts>
#include <functional>
#include <tuple>
#include <variant>

namespace mimicpp::printing::type::lexing
{
    // see: https://en.cppreference.com/w/cpp/string/byte/isspace
    constexpr auto is_space = [](char const c) noexcept {
        return static_cast<bool>(std::isspace(static_cast<unsigned char>(c)));
    };

    // see: https://en.cppreference.com/w/cpp/string/byte/isalpha
    constexpr auto is_digit = [](char const c) noexcept {
        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(c)));
    };

    // see: https://en.cppreference.com/w/cpp/string/byte/isdigit
    constexpr auto is_alpha = [](char const c) noexcept {
        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(c)));
    };

    // see: https://en.cppreference.com/w/cpp/string/byte/isxdigit
    constexpr auto is_hex_digit = [](char const c) noexcept {
        return static_cast<bool>(std::isxdigit(static_cast<unsigned char>(c)));
    };

    namespace texts
    {
        // just list the noteworthy ones here
        constexpr std::array visibilityKeywords = std::to_array<StringViewT>({"public", "protected", "private"});
        constexpr std::array specKeywords = std::to_array<StringViewT>({"const", "constexpr", "volatile", "noexcept"});
        constexpr std::array contextKeywords = std::to_array<StringViewT>({"operator", "struct", "class", "enum"});
        constexpr std::array otherKeywords = std::to_array<StringViewT>({"new", "delete", "co_await"});
        constexpr std::array digraphs = std::to_array<StringViewT>({"and", "or", "xor", "not", "bitand", "bitor", "compl", "and_eq", "or_eq", "xor_eq", "not_eq"});

        constexpr std::array braceLikes = std::to_array<StringViewT>({"{", "}", "[", "]", "(", ")", "`", "'"});
        constexpr std::array comparison = std::to_array<StringViewT>({"==", "!=", "<", "<=", ">", ">=", "<=>"});
        constexpr std::array assignment = std::to_array<StringViewT>({"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="});
        constexpr std::array incOrDec = std::to_array<StringViewT>({"++", "--"});
        constexpr std::array arithmetic = std::to_array<StringViewT>({"+", "-", "*", "/", "%"});
        constexpr std::array bitArithmetic = std::to_array<StringViewT>({"~", "&", "|", "^", "<<", ">>"});
        constexpr std::array logical = std::to_array<StringViewT>({"!", "&&", "||"});
        constexpr std::array access = std::to_array<StringViewT>({".", ".*", "->", "->*"});
        constexpr std::array specialAngles = std::to_array<StringViewT>({"<:", ":>", "<%", "%>"});
        constexpr std::array rest = std::to_array<StringViewT>({"::", ";", ",", ":", "...", "?"});
    }

    // GCOVR_EXCL_START

    // These functions are only executed at compile-time and thus be reported as uncovered.

    constexpr std::array keywordCollection = std::invoke(
        [] {
            std::array collection = util::concat_arrays(
                texts::visibilityKeywords,
                texts::specKeywords,
                texts::contextKeywords,
                texts::otherKeywords,
                texts::digraphs);

            std::ranges::sort(collection);
            MIMICPP_ASSERT(collection.cend() == std::ranges::unique(collection).begin(), "Fix your input!");

            return collection;
        });

    // see: https://eel.is/c++draft/lex.operators#nt:operator-or-punctuator
    constexpr std::array operatorOrPunctuatorCollection = std::invoke(
        [] {
            std::array collection = util::concat_arrays(
                texts::braceLikes,
                texts::comparison,
                texts::assignment,
                texts::incOrDec,
                texts::arithmetic,
                texts::bitArithmetic,
                texts::logical,
                texts::access,
                texts::specialAngles,
                texts::rest);
            std::ranges::sort(collection);
            MIMICPP_ASSERT(collection.cend() == std::ranges::unique(collection).begin(), "Fix your input!");

            return collection;
        });

    // GCOVR_EXCL_STOP

    struct space
    {
        [[nodiscard]]
        bool operator==(space const&) const = default;
    };

    struct keyword
    {
    public:
        static constexpr auto& textCollection = keywordCollection;

        [[nodiscard]]
        explicit constexpr keyword(StringViewT const& text) noexcept
            : keyword{
                  std::ranges::distance(
                      textCollection.cbegin(),
                      util::binary_find(textCollection, text))}
        {
        }

        [[nodiscard]]
        explicit constexpr keyword(std::ptrdiff_t const keywordIndex) noexcept
            : m_KeywordIndex{keywordIndex}
        {
            MIMICPP_ASSERT(0 <= m_KeywordIndex && m_KeywordIndex < std::ranges::ssize(textCollection), "Invalid keyword.");
        }

        [[nodiscard]]
        constexpr StringViewT text() const noexcept
        {
            return textCollection[m_KeywordIndex];
        }

        [[nodiscard]]
        bool operator==(keyword const&) const = default;

    private:
        std::ptrdiff_t m_KeywordIndex;
    };

    struct operator_or_punctuator
    {
    public:
        static constexpr auto& textCollection = operatorOrPunctuatorCollection;

        [[nodiscard]]
        explicit constexpr operator_or_punctuator(StringViewT const& text) noexcept
            : operator_or_punctuator{
                  std::ranges::distance(
                      textCollection.cbegin(),
                      util::binary_find(textCollection, text))}
        {
        }

        [[nodiscard]]
        explicit constexpr operator_or_punctuator(std::ptrdiff_t const textIndex) noexcept
            : m_TextIndex{textIndex}
        {
            MIMICPP_ASSERT(0 <= m_TextIndex && m_TextIndex < std::ranges::ssize(textCollection), "Invalid operator or punctuator.");
        }

        [[nodiscard]]
        constexpr StringViewT text() const noexcept
        {
            return textCollection[m_TextIndex];
        }

        [[nodiscard]]
        bool operator==(operator_or_punctuator const&) const = default;

    private:
        std::ptrdiff_t m_TextIndex;
    };

    struct identifier
    {
        StringViewT content;

        [[nodiscard]]
        bool operator==(identifier const&) const = default;
    };

    struct end
    {
        [[nodiscard]]
        bool operator==(end const&) const = default;
    };

    using token_class = std::variant<
        end,
        space,
        keyword,
        operator_or_punctuator,
        identifier>;

    struct token
    {
        StringViewT content;
        token_class classification;
    };

    class NameLexer
    {
    public:
        [[nodiscard]]
        explicit constexpr NameLexer(StringViewT text) noexcept
            : m_Text{std::move(text)},
              m_Next{find_next()}
        {
        }

        [[nodiscard]]
        constexpr token next() noexcept
        {
            return std::exchange(m_Next, find_next());
        }

        [[nodiscard]]
        constexpr token const& peek() const noexcept
        {
            return m_Next;
        }

    private:
        StringViewT m_Text;
        token m_Next;

        [[nodiscard]]
        constexpr token find_next() noexcept
        {
            if (m_Text.empty())
            {
                return token{
                    .content = {m_Text.cend(), m_Text.cend()},
                    .classification = end{}
                };
            }

            if (is_space(m_Text.front()))
            {
                // Multiple consecutive spaces or any whitespace character other than a single space
                // carry no meaningful semantic value beyond delimitation.
                // Although single spaces may sometimes influence the result and sometimes not,
                // complicating the overall process, we filter out all non-single whitespace characters here.
                if (StringViewT const content = next_as_space();
                    " " == content)
                {
                    return token{
                        .content = content,
                        .classification = space{}};
                }

                return find_next();
            }

            if (auto const options = util::prefix_range(
                    operatorOrPunctuatorCollection,
                    m_Text.substr(0u, 1u)))
            {
                return next_as_op_or_punctuator(options);
            }

            StringViewT const content = next_as_identifier();
            // As we do not perform any prefix-checks, we need to check now whether the token actually denotes a keyword.
            if (auto const iter = util::binary_find(keywordCollection, content);
                iter != keywordCollection.cend())
            {
                return token{
                    .content = content,
                    .classification = keyword{std::ranges::distance(keywordCollection.begin(), iter)}};
            }

            return token{
                .content = content,
                .classification = identifier{.content = content}};
        }

        [[nodiscard]]
        constexpr StringViewT next_as_space() noexcept
        {
            auto const end = std::ranges::find_if_not(m_Text.cbegin() + 1, m_Text.cend(), is_space);
            StringViewT const content{m_Text.cbegin(), end};
            m_Text = StringViewT{end, m_Text.cend()};

            return content;
        }

        /**
         * \brief Extracts the next operator or punctuator.
         * \details Performs longest-prefix matching.
         */
        [[nodiscard]]
        constexpr token next_as_op_or_punctuator(std::span<StringViewT const> options) noexcept
        {
            MIMICPP_ASSERT(m_Text.substr(0u, 1u) == options.front(), "Assumption does not hold.");

            auto const try_advance = [&, this](std::size_t const n) {
                if (n <= m_Text.size())
                {
                    return util::prefix_range(
                        options,
                        StringViewT{m_Text.cbegin(), m_Text.cbegin() + n});
                }

                return std::ranges::subrange{options.end(), options.end()};
            };

            std::size_t length{1u};
            StringViewT const* lastMatch = &options.front();
            while (auto const nextOptions = try_advance(length + 1))
            {
                ++length;
                options = {nextOptions.begin(), nextOptions.end()};

                // If the first string is exactly the size of the prefix, it's a match.
                if (auto const& front = options.front();
                    length == front.size())
                {
                    lastMatch = &front;
                }
            }

            MIMICPP_ASSERT(!options.empty(), "Invalid state.");
            MIMICPP_ASSERT(lastMatch, "Invalid state.");

            auto const index = std::ranges::distance(operatorOrPunctuatorCollection.data(), lastMatch);
            StringViewT const content{m_Text.substr(0u, lastMatch->size())};
            m_Text.remove_prefix(lastMatch->size());

            return token{
                .content = content,
                .classification = operator_or_punctuator{index}};
        }

        /**
         * \brief Extracts the next identifier.
         * \details This approach differs a lot from the general c++ process. Instead of utilizing a specific alphabet
         * of valid characters (and thus performing a whitelist-test), we use a more permissive approach here and check
         * whether the next character is not a space and not prefix of a operator or punctuator.
         * This has to be done, because demangled names may (and will!) contain various non-allowed tokens.
         *
         * As we make the assumption that the underlying name is actually correct, we do not need to check for validity
         * here. Just treat everything else as identifier and let the parser do the rest.
         */
        [[nodiscard]]
        constexpr StringViewT next_as_identifier() noexcept
        {
            auto const last = std::ranges::find_if_not(
                m_Text.cbegin() + 1,
                m_Text.cend(),
                [](auto const c) {
                    return !is_space(c)
                        && !std::ranges::binary_search(operatorOrPunctuatorCollection, StringViewT{&c, 1u});
                });

            StringViewT const content{m_Text.cbegin(), last};
            m_Text = {last, m_Text.cend()};

            return content;
        }
    };
}

#endif
