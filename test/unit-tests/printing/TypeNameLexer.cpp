//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/type/NameLexer.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

namespace
{
    template <typename Value>
    [[nodiscard]]
    constexpr auto matches_token(Value token)
    {
        return VariantEqualsMatcher<Value>{std::move(token)};
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
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Single spaces are detected.")
    {
        auto const expectedToken = matches_token(space{" "});
        NameLexer lexer{" "};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
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
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Comma is detected.")
    {
        auto const expectedToken = matches_token(comma{","});

        NameLexer lexer{","};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Scope-resolution is detected.")
    {
        auto const expectedToken = matches_token(scope_resolution{"::"});

        NameLexer lexer{"::"};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common brace-likes are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::braceLikes));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common comparison-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::comparison));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common assignment-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::assignment));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common increment- and decrement-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::incOrDec));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common arithmetic-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::arithmetic));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common bit-arithmetic-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::bitArithmetic));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common logical-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::logical));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common access-operators are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::access));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Common special angles are detected.")
    {
        StringViewT const input = GENERATE(from_range(texts::specialAngles));
        CAPTURE(input);

        auto const expectedToken = matches_token(operator_or_punctuator{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Keywords are detected.")
    {
        StringViewT const input = GENERATE(from_range(keywordCollection));
        CAPTURE(input);

        auto const expectedToken = matches_token(keyword{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Arbitrary Keywords are detected.")
    {
        StringViewT const input = GENERATE("foo", "_123", "foo456", "const_", "_const");
        CAPTURE(input);

        auto const expectedToken = matches_token(identifier{input});

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            expectedToken);

        CHECK_THAT(
            lexer.next(),
            expectedToken);
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }
}

TEST_CASE(
    "printing::type::lexing::NameLexer supports token compositions.",
    "[print][print::type]")
{
    using namespace printing::type::lexing;

    SECTION("space + identifier.")
    {
        constexpr StringViewT input = "\ttest";
        CAPTURE(input);

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(identifier{"test"}));

        CHECK_THAT(
            lexer.next(),
            matches_token(identifier{"test"}));
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("Operator + operator.")
    {
        constexpr StringViewT input = "++--";
        CAPTURE(input);

        NameLexer lexer{input};
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(operator_or_punctuator{"++"}));

        CHECK_THAT(
            lexer.next(),
            matches_token(operator_or_punctuator{"++"}));
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(operator_or_punctuator{"--"}));

        CHECK_THAT(
            lexer.next(),
            matches_token(operator_or_punctuator{"--"}));
        CHECK_THAT(
            std::as_const(lexer).peek(),
            matches_token(end{}));

        CHECK_THAT(
            lexer.next(),
            matches_token(end{}));
    }

    SECTION("keyword + space + identifier + operator + operator + space + keyword + operator.")
    {
        constexpr StringViewT input = "const\t foo123[] volatile&&";
        CAPTURE(input);

        std::tuple const sequence = {
            matches_token(keyword{"const"}),
            matches_token(identifier{"foo123"}),
            matches_token(operator_or_punctuator{"["}),
            matches_token(operator_or_punctuator{"]"}),
            matches_token(space{" "}),
            matches_token(keyword{"volatile"}),
            matches_token(operator_or_punctuator{"&&"})};

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
            matches_token(end{}));
    }
}
