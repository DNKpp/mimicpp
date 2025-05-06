//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/type/NameLexer.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

namespace
{
    template <std::equality_comparable TokenClass>
        requires std::constructible_from<printing::type::lexing::token, StringViewT, TokenClass>
    class TokenMatcher final
        : public Catch::Matchers::MatcherGenericBase
    {
    public:
        [[nodiscard]]
        explicit constexpr TokenMatcher(TokenClass tokenClass)
            : m_ClassMatcher{std::move(tokenClass)}
        {
        }

        [[nodiscard]]
        explicit constexpr TokenMatcher(StringViewT content, TokenClass tokenClass)
            : m_ClassMatcher{std::move(tokenClass)},
              m_Content{std::move(content)}
        {
        }

        [[nodiscard]]
        constexpr bool match(printing::type::lexing::token const& token) const
        {
            return m_ClassMatcher.match(token.classification)
                && (!m_Content || token.content == m_Content.value());
        }

        [[nodiscard]]
        std::string describe() const override
        {
            std::string description = std::string{"Lexing-Token equals class: "}
                                    + mimicpp::print_type<TokenClass>();
            if (m_Content)
            {
                description += " and contains content: '";
                description.append(*m_Content);
                description += "'";
            }

            return description;
        }

    private:
        VariantEqualsMatcher<TokenClass> m_ClassMatcher;
        std::optional<StringViewT> m_Content{};
    };

    template <typename TokenClass>
    [[nodiscard]]
    constexpr auto matches_class(TokenClass token)
    {
        return TokenMatcher<TokenClass>{std::move(token)};
    }

    template <typename TokenClass>
    [[nodiscard]]
    constexpr auto matches_token(StringViewT const& content, TokenClass token)
    {
        return TokenMatcher<TokenClass>{content, std::move(token)};
    }

    [[nodiscard]]
    auto matches_end_token()
    {
        return matches_token("", printing::type::lexing::end{});
    }
}

TEST_CASE(
    "printing::type::lexing::is_space determines, whether the given character is a space.",
    "[print][print::type]")
{
    SECTION("When a space is given, returns true.")
    {
        char const input = GENERATE(' ', '\t');

        CHECK(printing::type::lexing::is_space(input));
    }

    SECTION("When no space is given, returns false.")
    {
        char const input = GENERATE('a', '_', '-', '1');

        CHECK(!printing::type::lexing::is_space(input));
    }
}

TEST_CASE(
    "printing::type::lexing::NameLexer extracts tokens from given input.",
    "[print][print::type]")
{
    using namespace printing::type::lexing;

    SECTION("Empty input is supported.")
    {
        NameLexer lexer{""};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Single spaces are detected.")
    {
        NameLexer lexer{" "};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(" ", space{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(" ", space{}));
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Multiple spaces or any non-standard space is ignored.")
    {
        StringViewT const input = GENERATE(
            // " ", single spaces are treated specially.
            "\t",
            "  ",
            "\t\t",
            "\t \t",
            " \t ");
        CAPTURE(input);

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common brace-likes are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::braceLikes));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common comparison-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::comparison));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common assignment-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::assignment));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common increment- and decrement-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::incOrDec));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common arithmetic-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::arithmetic));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common bit-arithmetic-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::bitArithmetic));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common logical-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::logical));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common access-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::access));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Common special angles are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::specialAngles));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("All other operators or punctuators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::rest));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Keywords are detected.")
    {
        StringViewT const input = GENERATE(from_range(keywordCollection));
        CAPTURE(input);

        auto const expectedToken = matches_token(input, keyword{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Arbitrary identifiers are detected.")
    {
        StringViewT const input = GENERATE("foo", "_123", "foo456", "const_", "_const");
        CAPTURE(input);

        auto const expectedToken = matches_token(input, identifier{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }
}

TEST_CASE(
    "printing::type::lexing::NameLexer supports token compositions.",
    "[print][print::type]")
{
    using namespace printing::type::lexing;

    SECTION("tab + identifier.")
    {
        constexpr StringViewT input = "\ttest";
        CAPTURE(input);

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_class(identifier{"test"}));

        CHECK_THAT(
            lexer.next(),
            matches_class(identifier{"test"}));
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("Operator + operator.")
    {
        constexpr StringViewT input = "++--";
        CAPTURE(input);

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_class(operator_or_punctuator{"++"}));

        CHECK_THAT(
            lexer.next(),
            matches_class(operator_or_punctuator{"++"}));
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_class(operator_or_punctuator{"--"}));

        CHECK_THAT(
            lexer.next(),
            matches_class(operator_or_punctuator{"--"}));
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_end_token());

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }

    SECTION("keyword + space + identifier + operator + operator + space + keyword + operator.")
    {
        constexpr StringViewT input = "const\t foo123[] volatile&&";
        CAPTURE(input);

        std::tuple const sequence = {
            matches_class(keyword{"const"}),
            matches_class(identifier{"foo123"}),
            matches_class(operator_or_punctuator{"["}),
            matches_class(operator_or_punctuator{"]"}),
            matches_class(space{}),
            matches_class(keyword{"volatile"}),
            matches_class(operator_or_punctuator{"&&"})};

        NameLexer lexer{input};

        std::apply(
            [&](auto&... matchers) {
                auto check = [&, i = 0](auto const& matcher) mutable {
                    CAPTURE(i);
                    ++i;
                    CHECK_THAT(
                        std::as_const(lexer).peek(),
                        matcher);
                    CHECK_THAT(
                        lexer.next(),
                        matcher);
                };

                (check(matchers), ...);
            },
            sequence);

        CHECK_THAT(
            lexer.next(),
            matches_end_token());
    }
}

TEST_CASE(
    "lexing::operator_or_punctuator::text yields the token text.",
    "[print][print::type]")
{
    StringT const tokenText{GENERATE(from_range(printing::type::lexing::operatorOrPunctuatorCollection))};
    printing::type::lexing::operator_or_punctuator const token{tokenText};

    CHECK_THAT(
        StringT{token.text()},
        Catch::Matchers::Equals(tokenText));
}

TEST_CASE(
    "lexing::keyword::text yields the token text.",
    "[print][print::type]")
{
    StringT const tokenText{GENERATE(from_range(printing::type::lexing::keywordCollection))};
    printing::type::lexing::keyword const token{tokenText};

    CHECK_THAT(
        StringT{token.text()},
        Catch::Matchers::Equals(tokenText));
}
