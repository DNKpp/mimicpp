//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/type/Parser.hpp"

#include "TestTypes.hpp"

using mimicpp::printing::type::parse_type;
namespace lexing = mimicpp::printing::type::lexing;
namespace state = mimicpp::printing::type::parsing::v2::state;

namespace
{
    std::array const cvTable = std::to_array<std::tuple<state::CVQualifierSeq, std::string, std::string>>({
        {state::CVQualifierSeq{.isConst = true},                     "const",          ""              },
        {state::CVQualifierSeq{.isConst = true},                     "",               "const"         },
        {state::CVQualifierSeq{.isVolatile = true},                  "volatile",       ""              },
        {state::CVQualifierSeq{.isVolatile = true},                  "",               "volatile"      },

        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "const volatile", ""              },
        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "",               "const volatile"},
        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "volatile const", ""              },
        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "",               "volatile const"},

        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "const",          "volatile"      },
        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "volatile",       "const"         },
    });
}

TEST_CASE(
    "parsing::parse_type supports arbitrary unqualified types.",
    "[print][print::type]")
{
    std::string const type = GENERATE("foo", "_123", "foo456", "const_", "_const");
    CAPTURE(type);

    state::QualifiedId const expected{.identifier = {state::Identifier{type}}};

    SECTION("When it's provided as plain type.")
    {
        auto const id = parse_type(type);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are provided with arbitrary cv qualification.")
    {
        auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(from_range(cvTable));
        std::string const input = qualifierPrefix + " " + type + " " + qualifierSuffix;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->qualifications);
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports qualified types with simple identifier scopes.",
    "[print][print::type]")
{
    std::string const type = GENERATE("foo", "_123", "foo456", "const_", "_const");
    using Id = state::Identifier;
    auto const [expectedScopes, scopeText] = GENERATE((table<state::ScopeSequence, std::string>)({
        {{.explicitRoot = true},                                                                                                                                          "::"                 },
        {{.scopes = {state::UnqualifiedId{.name = Id{"tmp"}}, state::UnqualifiedId{.name = Id{"foo"}}, state::UnqualifiedId{.name = Id{"_5432"}}}},                       "tmp::foo::_5432::"  },
        {{.explicitRoot = true, .scopes = {state::UnqualifiedId{.name = Id{"tmp"}}, state::UnqualifiedId{.name = Id{"foo"}}, state::UnqualifiedId{.name = Id{"_5432"}}}}, "::tmp::foo::_5432::"},
    }));
    CAPTURE(type, scopeText);

    state::QualifiedId const expected{
        .scopes = expectedScopes,
        .identifier = {state::Identifier{type}}};

    SECTION("When it's provided as plain type.")
    {
        std::string const input = scopeText + type;

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are provided with arbitrary cv qualification.")
    {
        auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(from_range(cvTable));
        std::string const input = qualifierPrefix + " " + scopeText + type + " " + qualifierSuffix;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->qualifications);
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports arbitrary unqualified template types.",
    "[print][print::type]")
{
    static constexpr auto make_builtin = [](lexing::keyword const& keyword) -> state::TemplateArgument {
        return state::Recursive{
            state::TypeId{.base = state::BuiltinType{keyword}}};
    };

    static constexpr auto make_type = [](lexing::identifier const& id) -> state::TemplateArgument {
        return state::Recursive{
            state::TypeId{.base = state::QualifiedId{.identifier = {.name = state::Identifier{id.content}}}}};
    };

    using KW = lexing::keyword;
    using ID = lexing::identifier;

    std::string const templateId = GENERATE("foo", "_123", "foo456", "const_", "_const");
    auto const [expectedArgs, argListText] = GENERATE((table<state::TemplateArgumentList, std::string>)({
        {state::TemplateArgumentList{},                     "<>"           },
        {{make_builtin(KW{"int"})},                         "<int>"        },
        {{make_type(ID{"_foo_"}), make_builtin(KW{"int"})}, "< _foo_,int >"},
    }));

    state::QualifiedId const expected{
        .identifier = {
                       .name = state::Identifier{templateId},
                       .templateArgs = expectedArgs}
    };

    SECTION("When they are provided as plain type.")
    {
        std::string const input = templateId + argListText;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are provided with arbitrary cv qualification.")
    {
        auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(from_range(cvTable));
        std::string const input = qualifierPrefix + " " + templateId + argListText + " " + qualifierSuffix;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->qualifications);
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports qualified types with template scopes.",
    "[print][print::type]")
{
    std::string const type = GENERATE("foo", "_123", "foo456", "const_", "_const");
    using ID = state::Identifier;
    using KW = lexing::keyword;
    using UId = state::UnqualifiedId;
    using Rec = state::Recursive<state::TypeId>;
    auto const [expectedScopes, scopeText] = GENERATE((table<state::ScopeSequence, std::string>)({
        {{.scopes = {UId{.name = ID{"tmp"}, .templateArgs = {{Rec{state::TypeId{.base = state::BuiltinType{.base = KW{"int"}}}}}}}}}, "tmp<int>"     },
        {{.explicitRoot = true, .scopes = {UId{.name = ID{"tmp"}, .templateArgs = state::TemplateArgumentList{}}}},                                                   "::tmp<>"      },
        {{.scopes = {UId{.name = ID{"tmp"}, .templateArgs = state::TemplateArgumentList{}}, UId{.name = ID{"tmp2"}, .templateArgs = state::TemplateArgumentList{}}}}, "tmp<>::tmp2<>"},
    }));
    CAPTURE(type, scopeText);

    SECTION("When it's provided as plain type.")
    {
        std::string const input = scopeText + "::" + type;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);

        state::QualifiedId const expected{
            .scopes = expectedScopes,
            .identifier = {.name = state::Identifier{type}}};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are provided with arbitrary cv qualification.")
    {
        auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(from_range(cvTable));
        std::string const input = qualifierPrefix + " " + scopeText + "::" + type + " " + qualifierSuffix;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->qualifications);
        state::QualifiedId const expected{
            .scopes = expectedScopes,
            .identifier = {.name = state::Identifier{type}}};
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports qualified types with function scopes.",
    "[print][print::type]")
{
    using ID = state::Identifier;
    using KW = lexing::keyword;
    using UId = state::UnqualifiedId;
    using Rec = state::Recursive<state::TypeId>;

    std::string const type = GENERATE("foo", "_123", "foo456", "const_", "_const");
    auto const [expectedScopes, scopeText] = GENERATE((table<state::ScopeSequence, std::string>)({
        {{.scopes = {UId{.name = ID{"tmp"}, .functionDeclarator = state::FunctionDeclarator{.params = {Rec{state::TypeId{.base = state::BuiltinType{.base = KW{"int"}}}}}}}}}, "tmp(int)"     },
        {{.explicitRoot = true, .scopes = {UId{.name = ID{"tmp"}, .functionDeclarator = state::FunctionDeclarator{}}}},                                                        "::tmp()"      },
        {{.scopes = {UId{.name = ID{"tmp"}, .functionDeclarator = state::FunctionDeclarator{}}, UId{.name = ID{"tmp2"}, .functionDeclarator = state::FunctionDeclarator{}}}},  "tmp()::tmp2()"},
    }));
    CAPTURE(type, scopeText);

    std::string const input = scopeText + "::" + type;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(!id->qualifications.isConst);
    CHECK(!id->qualifications.isVolatile);

    state::QualifiedId const expected{
        .scopes = expectedScopes,
        .identifier = {.name = state::Identifier{type}}};
    CHECK_THAT(id->base, variant_equals(expected));
}
