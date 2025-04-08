//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/ScopedSequence.hpp"
#include "mimic++/printing/type/NameParser.hpp"

using namespace mimicpp;

namespace
{
    struct VisitorMock
    {
        Mock<void()> begin{{.name = "VisitorMock::begin"}};
        Mock<void()> end{{.name = "VisitorMock::end"}};

        Mock<void()> begin_template{{.name = "VisitorMock::begin_template"}};
        Mock<void()> end_template{{.name = "VisitorMock::end_template"}};

        Mock<void()> begin_function{{.name = "VisitorMock::begin_function"}};
        Mock<void()> end_function{{.name = "VisitorMock::end_function"}};
        Mock<void(StringViewT)> push_spec{{.name = "VisitorMock::push_spec"}};

        Mock<void(StringViewT)> push_identifier{{.name = "VisitorMock::push_identifier"}};
        Mock<void()> push_scope{{.name = "VisitorMock::push_scope"}};
        Mock<void()> push_argument{{.name = "VisitorMock::push_argument"}};

        Mock<void()> begin_operator_identifier{{.name = "VisitorMock::begin_operator_identifier"}};
        Mock<void()> end_operator_identifier{{.name = "VisitorMock::end_operator_identifier"}};
    };
}

TEST_CASE(
    "parsing::NameParser detects arbitrary identifiers.",
    "[print][print::type]")
{
    static constexpr std::array identifiers = std::to_array<StringViewT>({"foo", "_123", "foo456", "const_", "_const"});

    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When single identifier is given.")
    {
        StringViewT const input = GENERATE(from_range(identifiers));
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call(input);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When multiple scopes are given.")
    {
        StringViewT const firstScope = GENERATE(from_range(identifiers));
        StringViewT const secondScope = GENERATE(from_range(identifiers));
        StringViewT const thirdScope = GENERATE(from_range(identifiers));

        StringT const input = StringT{firstScope} + "::" + StringT{secondScope} + "::" + StringT{thirdScope};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call(firstScope);
        sequence += visitor.push_scope.expect_call();
        sequence += visitor.push_identifier.expect_call(secondScope);
        sequence += visitor.push_scope.expect_call();
        sequence += visitor.push_identifier.expect_call(thirdScope);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When root scope is explicitly given.")
    {
        StringViewT const scope = GENERATE(from_range(identifiers));
        StringT const input = "::" + StringT{scope};
        CAPTURE(input);

        sequence += visitor.push_scope.expect_call();
        sequence += visitor.push_identifier.expect_call(scope);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }
}

TEST_CASE(
    "parsing::NameParser detects specifications.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When given before the actual identifier.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const input{spec + " foo"};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call("foo");
        sequence += visitor.push_spec.expect_call(spec);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When given before the actual identifier with ref/pointer.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const indirection = GENERATE("&", "&&", "*");
        StringT const input{spec + " foo " + indirection};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call("foo");
        sequence += visitor.push_spec.expect_call(spec);
        sequence += visitor.push_spec.expect_call(indirection);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When given after the actual identifier.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const input{"foo " + spec};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call("foo");
        sequence += visitor.push_spec.expect_call(spec);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When given after the actual identifier with ref/pointer.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const indirection = GENERATE("&", "&&", "*");
        StringT const input{"foo " + spec + indirection};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call("foo");
        sequence += visitor.push_spec.expect_call(spec);
        sequence += visitor.push_spec.expect_call(indirection);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("Coming all together")
    {
        StringT const input{"const foo volatile * const&"};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call("foo");
        sequence += visitor.push_spec.expect_call("const");
        sequence += visitor.push_spec.expect_call("volatile");
        sequence += visitor.push_spec.expect_call("*");
        sequence += visitor.push_spec.expect_call("const");
        sequence += visitor.push_spec.expect_call("&");
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }
}

TEST_CASE(
    "parsing::NameParser detects templates.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When templated identifier with 0 args is given.")
    {
        StringViewT const input{"foo<>"};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call("foo");
        sequence += visitor.begin_template.expect_call();
        sequence += visitor.end_template.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When templated identifier with multiple args is given.")
    {
        StringViewT const input{"foo<int, std::string>"};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call("foo");

        sequence += visitor.begin_template.expect_call();
        sequence += visitor.push_identifier.expect_call("int");
        sequence += visitor.push_argument.expect_call();

        sequence += visitor.push_identifier.expect_call("std");
        sequence += visitor.push_scope.expect_call();
        sequence += visitor.push_identifier.expect_call("string");

        sequence += visitor.end_template.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When templated identifier with multiple args with specs is given.")
    {
        StringViewT const input{"foo<const int volatile&, const std::string>"};
        CAPTURE(input);

        sequence += visitor.push_identifier.expect_call("foo");

        sequence += visitor.begin_template.expect_call();
        sequence += visitor.push_identifier.expect_call("int");
        sequence += visitor.push_spec.expect_call("const");
        sequence += visitor.push_spec.expect_call("volatile");
        sequence += visitor.push_spec.expect_call("&");
        sequence += visitor.push_argument.expect_call();

        sequence += visitor.push_identifier.expect_call("std");
        sequence += visitor.push_scope.expect_call();
        sequence += visitor.push_identifier.expect_call("string");
        sequence += visitor.push_spec.expect_call("const");

        sequence += visitor.end_template.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }
}

TEST_CASE(
    "parsing::NameParser detects functions.",
    "[print][print::type]")
{

}

TEST_CASE(
    "parsing::NameParser detects operators as identifiers.",
    "[print][print::type]")
{
    // see: https://en.cppreference.com/w/cpp/language/operators
    StringViewT const operatorText = GENERATE(
        "+",
        "-",
        "*",
        "/",
        "%",
        "^",
        "&",
        "|",
        "~",
        "!",
        "=",
        "<",
        ">",
        "+=",
        "-=",
        "*=",
        "/=",
        "%=",
        "^=",
        "&=",
        "|=",
        "<<",
        ">>",
        "<<=",
        ">>=",
        "==",
        "!=",
        "<=",
        ">=",
        "<=>",
        "&&",
        "||",
        "++",
        "--",
        ",",
        "->*",
        "->",
        "()",
        "[]");

    VisitorMock visitor{};
    ScopedSequence sequence{};

    SECTION("When operator-keyword and -symbol are not separated by space.")
    {
        StringT const input = StringT{"operator"} + StringT{operatorText};
        CAPTURE(input);

        sequence += visitor.begin.expect_call();

        sequence += visitor.begin_operator_identifier.expect_call();
        sequence += visitor.push_identifier.expect_call(operatorText);
        sequence += visitor.end_operator_identifier.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When operator-keyword and -symbol are separated by space.")
    {
        StringT const input = StringT{"operator"} + " " + StringT{operatorText};
        CAPTURE(input);

        sequence += visitor.begin.expect_call();

        sequence += visitor.begin_operator_identifier.expect_call();
        sequence += visitor.push_identifier.expect_call(operatorText);
        sequence += visitor.end_operator_identifier.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When prefixed by another scope.")
    {
        StringT const input = StringT{"foo::operator"} + StringT{operatorText};
        CAPTURE(input);

        sequence += visitor.begin.expect_call();

        sequence += visitor.push_identifier.expect_call("foo");
        sequence += visitor.push_scope.expect_call();
        sequence += visitor.begin_operator_identifier.expect_call();
        sequence += visitor.push_identifier.expect_call(operatorText);
        sequence += visitor.end_operator_identifier.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When operator template with 0 args is given.")
    {
        StringT const space = GENERATE("", " ");
        StringT const input = StringT{"operator"} + StringT{operatorText} + space + "<>";
        CAPTURE(input);

        // operator<<> is no valid syntax (fortunately)
        CHECKED_IF((operatorText != "<" || !space.empty()))
        {
            sequence += visitor.begin.expect_call();

            sequence += visitor.begin_operator_identifier.expect_call();
            sequence += visitor.push_identifier.expect_call(operatorText);
            sequence += visitor.end_operator_identifier.expect_call();
            sequence += visitor.begin_template.expect_call();
            sequence += visitor.end_template.expect_call();

            sequence += visitor.end.expect_call();

            printing::type::parsing::NameParser parser{std::ref(visitor), input};
            parser();
        }
    }

    SECTION("When operator template with arbitrary args is given.")
    {
        StringT const space = GENERATE("", " ");
        StringT const input = StringT{"operator"} + StringT{operatorText} + space + "<int, float>";
        CAPTURE(input);

        // operator<<> is no valid syntax (fortunately)
        CHECKED_IF((operatorText != "<" || !space.empty()))
        {
            sequence += visitor.begin.expect_call();

            sequence += visitor.begin_operator_identifier.expect_call();
            sequence += visitor.push_identifier.expect_call(operatorText);
            sequence += visitor.end_operator_identifier.expect_call();

            sequence += visitor.begin_template.expect_call();
            sequence += visitor.push_identifier.expect_call("int");
            sequence += visitor.push_argument.expect_call();
            sequence += visitor.push_identifier.expect_call("float");
            sequence += visitor.end_template.expect_call();

            sequence += visitor.end.expect_call();

            printing::type::parsing::NameParser parser{std::ref(visitor), input};
            parser();
        }
    }
}
