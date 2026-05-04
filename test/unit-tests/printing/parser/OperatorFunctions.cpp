//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/type/Parser.hpp"

#include "TestTypes.hpp"

using mimicpp::printing::type::parse_type;
namespace lexing = mimicpp::printing::type::lexing;
namespace state = mimicpp::printing::type::parsing::v2::state;

TEST_CASE(
    "parsing::parse_type supports types with a nested operator scope.",
    "[print][print::type]")
{
    // see: https://en.cppreference.com/w/cpp/language/operators
    SECTION("When a simple operator is used.")
    {
        std::string const symbol = GENERATE(
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
            "->");

        std::string const input = "foo::operator" + symbol + "()::bar";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(state::AbstractDeclarator::Layer{} == id->declarator.root);

        state::QualifiedId const expected{
            .scopes = {
                .scopes = {
                    state::UnqualifiedId{.name = state::Identifier{.content = "foo"}},
                    state::UnqualifiedId{
                        .name = state::OperatorFunctionId{.symbol = lexing::operator_or_punctuator{symbol}},
                        .functionDeclarator{std::in_place}}}},
            .identifier = state::UnqualifiedId{.name = state::Identifier{.content = "bar"}}};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When a double-token operator is used.")
    {
        using Op = lexing::operator_or_punctuator;
        auto const [expectedOps, symbolText] = GENERATE((table<std::array<Op, 2u>, std::string>)({
            {{Op{"("}, Op{")"}}, "()"},
            {{Op{"["}, Op{"]"}}, "[]"},
        }));

        std::string const input = "foo::operator" + symbolText + "()::bar";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(state::AbstractDeclarator::Layer{} == id->declarator.root);

        state::QualifiedId const expected{
            .scopes = {
                .scopes = {
                    state::UnqualifiedId{.name = state::Identifier{.content = "foo"}},
                    state::UnqualifiedId{
                        .name = state::OperatorFunctionId{.symbol = expectedOps},
                        .functionDeclarator{std::in_place}}}},
            .identifier = state::UnqualifiedId{.name = state::Identifier{.content = "bar"}}};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When a keyword operator is used.")
    {
        std::string const symbol = GENERATE(
            "new",
            "delete",
            "co_await");

        std::string const input = "foo::operator " + symbol + "()::bar";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(state::AbstractDeclarator::Layer{} == id->declarator.root);

        state::QualifiedId const expected{
            .scopes = {
                .scopes = {
                    state::UnqualifiedId{.name = state::Identifier{.content = "foo"}},
                    state::UnqualifiedId{
                        .name = state::OperatorFunctionId{.symbol = std::pair{lexing::keyword{symbol}, false}},
                        .functionDeclarator{std::in_place}}}},
            .identifier = state::UnqualifiedId{.name = state::Identifier{.content = "bar"}}};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When a keyword with subscript ops operator is used.")
    {
        std::string const symbol = GENERATE(
            "new",
            "delete");

        std::string const input = "foo::operator " + symbol + "[]" + "()::bar";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(state::AbstractDeclarator::Layer{} == id->declarator.root);

        state::QualifiedId const expected{
            .scopes = {
                .scopes = {
                    state::UnqualifiedId{.name = state::Identifier{.content = "foo"}},
                    state::UnqualifiedId{
                        .name = state::OperatorFunctionId{.symbol = std::pair{lexing::keyword{symbol}, true}},
                        .functionDeclarator{std::in_place}}}},
            .identifier = state::UnqualifiedId{.name = state::Identifier{.content = "bar"}}};
        CHECK_THAT(id->base, variant_equals(expected));
    }
}
