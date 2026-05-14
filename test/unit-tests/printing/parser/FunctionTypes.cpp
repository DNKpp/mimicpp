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
    std::array const cvTable = std::to_array<std::tuple<state::CVQualifierSeq, std::string>>({
        {state::CVQualifierSeq{.isConst = true},                     "const"         },
        {state::CVQualifierSeq{.isVolatile = true},                  "volatile"      },
        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "volatile const"},
        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "const volatile"},
    });

    std::array const refTable = std::to_array<std::tuple<state::RefQualifier, std::string>>({
        {state::RefQualifier::id_ref,    "&" },
        {state::RefQualifier::id_refref, "&&"},
    });

    std::array const ptrTable = std::to_array<std::tuple<state::PtrOperator, std::string>>({
        {state::PointerDeclarator{},                                                                         "*"               },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true}},                     "*const"          },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isVolatile = true}},                  "* volatile"      },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true, .isVolatile = true}}, "*volatile const" },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true, .isVolatile = true}}, "* const volatile"},
    });
}

TEST_CASE(
    "parsing::parse_type supports parameterless function-types.",
    "[print][print::type]")
{
    std::string const input = "void ()";
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(!id->qualifications.isConst);
    CHECK(!id->qualifications.isVolatile);
    CHECK_THAT(
        id->base,
        variant_equals(state::BuiltinType{.base = lexing::keyword{"void"}}));

    state::AbstractDeclarator::Layer const expected{
        .function = state::FunctionDeclarator{}};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports function-types with noexcept specification.",
    "[print][print::type]")
{
    std::string const input = "void () noexcept";
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(!id->qualifications.isConst);
    CHECK(!id->qualifications.isVolatile);
    CHECK_THAT(
        id->base,
        variant_equals(state::BuiltinType{.base = lexing::keyword{"void"}}));

    state::AbstractDeclarator::Layer const expected{
        .function = state::FunctionDeclarator{.isNoexcept = true}};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports function-types with cv specification.",
    "[print][print::type]")
{
    auto const [expectedCV, cvText] = GENERATE(from_range(cvTable));

    std::string const input = "void ()" + cvText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(!id->qualifications.isConst);
    CHECK(!id->qualifications.isVolatile);
    CHECK_THAT(
        id->base,
        variant_equals(state::BuiltinType{.base = lexing::keyword{"void"}}));

    state::AbstractDeclarator::Layer const expected{
        .function = state::FunctionDeclarator{.qualifiers = expectedCV}};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports function-types with ref specification.",
    "[print][print::type]")
{
    auto const [expectedRef, refText] = GENERATE(from_range(refTable));

    std::string const input = "void ()" + refText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(!id->qualifications.isConst);
    CHECK(!id->qualifications.isVolatile);
    CHECK_THAT(
        id->base,
        variant_equals(state::BuiltinType{.base = lexing::keyword{"void"}}));

    state::AbstractDeclarator::Layer const expected{
        .function = state::FunctionDeclarator{.refQualifier = expectedRef}};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports function-types with arbitrary return-types.",
    "[print][print::type]")
{
    using Id = state::Identifier;
    using KW = lexing::keyword;
    using UId = state::UnqualifiedId;
    using QId = state::QualifiedId;

    auto const [expectedReturn, returnText] = GENERATE((table<state::BaseType, std::string>)({
        {state::BuiltinType{KW{"int"}},                                                                                                           "int"     },
        {QId{.identifier = {.name = Id{"foo"}}},                                                                                                  "foo"     },
        {QId{.scopes = {.explicitRoot = true}, .identifier = {.name = Id{"foo"}}},                                                                "::foo"   },
        {QId{.scopes = {.scopes = {UId{Id{"bar"}}}}, .identifier = {.name = Id{"foo"}}},                                                          "bar::foo"},
        {QId{.identifier = {.name = Id{"foo"}, .templateArgs = state::TemplateArgumentList{}}},                                              "foo<>"   },
        {QId{.identifier = {.name = Id{"foo"}, .templateArgs = {{state::Recursive{state::TypeId{.base = state::BuiltinType{KW{"int"}}}}}}}}, "foo<int>"},
    }));

    SECTION("When a plain return-type is given.")
    {
        std::string const input = returnText + " ()";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(id->base == expectedReturn);

        state::AbstractDeclarator::Layer const expected{
            .function = state::FunctionDeclarator{}};
        CHECK(expected == id->declarator.root);
    }

    SECTION("When a cv qualified return-type is given.")
    {
        auto const [expectedCV, cvText] = GENERATE(from_range(cvTable));

        std::string const input = returnText + " " + cvText + " ()";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(id->qualifications == expectedCV);
        CHECK(id->base == expectedReturn);

        state::AbstractDeclarator::Layer const expected{
            .function = state::FunctionDeclarator{}};
        CHECK(expected == id->declarator.root);
    }

    SECTION("When a return-type reference is given.")
    {
        auto const [expectedRef, refText] = GENERATE(from_range(refTable));

        std::string const input = returnText + " " + refText + " ()";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(id->base == expectedReturn);

        state::AbstractDeclarator::Layer const expected{
            .decorations = {state::ReferenceDeclarator{expectedRef}},
            .function = state::FunctionDeclarator{}};
        CHECK(expected == id->declarator.root);
    }

    SECTION("When a return-type pointer is given.")
    {
        auto const [expectedPtr, ptrText] = GENERATE(from_range(ptrTable));

        std::string const input = returnText + " " + ptrText + " ()";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(id->base == expectedReturn);

        state::AbstractDeclarator::Layer const expected{
            .decorations = {expectedPtr},
            .function = state::FunctionDeclarator{}};
        CHECK(expected == id->declarator.root);
    }
}

TEST_CASE(
    "parsing::parse_type supports function-types where the return-type is a pointer or ref to an array.",
    "[print][print::type]")
{
    using CVEntry = std::ranges::range_value_t<decltype(cvTable)>;
    auto const [expectedCV, cvText] = GENERATE(cat(
        from_range(std::views::single(CVEntry{{}, {}})),
        from_range(cvTable)));
    auto const [expectedPtrDecoration, ptrDeclText] = GENERATE(cat(
        from_range(ptrTable),
        table<state::PtrOperator, std::string>({
            {state::PointerDeclarator{},                                                                         "*"              },
            {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true}},                     "*const"         },
            {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true, .isVolatile = true}}, "*volatile const"}
    })));

    state::BuiltinType const expectedType{.base = lexing::keyword{"int"}};

    using ADecl = state::ArrayDeclarator;
    using lit = lexing::literal;
    auto const [arrayDeclarators, arrayText] = GENERATE((table<std::vector<ADecl>, std::string>)({
        {{ADecl{}},                                       "[]"          },
        {{ADecl{lit{"1337"}}},                            "[1337]"      },
        {{ADecl{}, ADecl{}},                              "[] []"       },
        {{ADecl{lit{"1337"}}, ADecl{}},                   "[1337][]"    },
        {{ADecl{lit{"42"}}, ADecl{}, ADecl{lit{"1337"}}}, "[42][][1337]"},
    }));

    std::string const input = "int " + cvText + "(" + ptrDeclText + "())" + arrayText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(expectedCV == id->qualifications);
    CHECK_THAT(id->base, variant_equals(expectedType));
    state::AbstractDeclarator::Layer const expected{
        .nested = state::Recursive{state::AbstractDeclarator::Layer{
            .decorations = {expectedPtrDecoration},
            .function = state::FunctionDeclarator{}}},
        .arrays = arrayDeclarators};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports arbitrary function-params.",
    "[print][print::type]")
{
    std::string const input = "void (int const long&, foo<short, unsigned> &&, char (* const)[][1337])";
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(!id->qualifications.isConst);
    CHECK(!id->qualifications.isVolatile);
    CHECK_THAT(
        id->base,
        variant_equals(state::BuiltinType{.base = lexing::keyword{"void"}}));

    state::FunctionDeclarator const functionDeclarator{
        .params = {
                   // `int const long&`
            state::Recursive{
                state::TypeId{
                    .qualifications{.isConst = true},
                    .base = state::BuiltinType{.base = lexing::keyword{"int"}, .sizeSpec = state::BuiltinType::SizeSpec::id_long},
                    .declarator = {.root = {.decorations = {state::ReferenceDeclarator{state::RefQualifier::id_ref}}}}}},

                   // `foo<short, unsigned> &&`
            state::Recursive{
                state::TypeId{
                    .base = state::QualifiedId{
                        .identifier = {
                            .name = {state::Identifier{"foo"}},
                            .templateArgs = {
                                {state::Recursive{state::TypeId{.base = state::BuiltinType{.sizeSpec = state::BuiltinType::SizeSpec::id_short}}},
                                 state::Recursive{state::TypeId{.base = state::BuiltinType{.signedSpec = state::BuiltinType::SignedSpec::id_unsigned}}}}}}},
                    .declarator = {.root = {.decorations = {state::ReferenceDeclarator{state::RefQualifier::id_refref}}}}}},

                   // `char (* const)[][1337]`
            state::Recursive{[] {
                return state::TypeId{
                    .base = state::BuiltinType{.base = lexing::keyword{"char"}},
                    .declarator = {
                        .root = {
                            .nested = state::Recursive{
                                state::AbstractDeclarator::Layer{
                                    .decorations = {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true}}}}},
                            .arrays = {state::ArrayDeclarator{}, state::ArrayDeclarator{.size = state::ConstantExpression{"1337"}}}}}};
            }()}}
    };
    state::AbstractDeclarator::Layer const expected{.function = functionDeclarator};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports function-pointers.",
    "[print][print::type]")
{
    using KW = lexing::keyword;
    using ID = state::Identifier;
    using SignedSpec = state::BuiltinType::SignedSpec;
    using SizeSpec = state::BuiltinType::SizeSpec;
    auto const [expectedCV, cvText] = GENERATE(from_range(cvTable));
    auto const [expectedRef, refText] = GENERATE(from_range(refTable));
    auto const [expectedIsNoexcept, noexceptText] = GENERATE((table<bool, std::string>)({
        {false, ""        },
        {true,  "noexcept"}
    }));
    auto const [expectedReturn, returnText] = GENERATE((table<state::BaseType, std::string>)({
        {state::BuiltinType{.base = KW{"void"}},                                                                                 "void"                  },
        {state::BuiltinType{.base = KW{"int"}, .sizeSpec = SizeSpec::id_longlong, .signedSpec = SignedSpec::id_unsigned},        "long unsigned int long"},
        {state::QualifiedId{.scopes = {.explicitRoot = true}, .identifier = {.name = ID{"foo"}}},                                "::foo"                 },
        {state::QualifiedId{.scopes = {.scopes = {state::UnqualifiedId{.name = ID{"bar"}}}}, .identifier = {.name = ID{"foo"}}}, "bar::foo"              },
    }));

    std::string const input = returnText + " (*)()" + cvText + refText + " " + noexceptText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(!id->qualifications.isConst);
    CHECK(!id->qualifications.isVolatile);
    CHECK(expectedReturn == id->base);

    // clang-format off
    state::AbstractDeclarator::Layer const expected{
        .nested = state::Recursive{
            state::AbstractDeclarator::Layer{
                .decorations = {state::PointerDeclarator{}}}},
        .function = state::FunctionDeclarator{
            .qualifiers = expectedCV,
            .refQualifier = expectedRef,
            .isNoexcept = expectedIsNoexcept
        }};
    // clang-format on
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports member function-pointers.",
    "[print][print::type]")
{
    using ID = state::Identifier;
    auto const [expectedCV, cvText] = GENERATE(from_range(cvTable));
    auto const [expectedRef, refText] = GENERATE(from_range(refTable));
    auto const [expectedIsNoexcept, noexceptText] = GENERATE((table<bool, std::string>)({
        {false, ""        },
        {true,  "noexcept"}
    }));
    auto const [expectedScopes, scopeText] = GENERATE((table<state::ScopeSequence, std::string>)({
        {state::ScopeSequence{.explicitRoot = true, .scopes = {state::UnqualifiedId{.name = ID{"foo"}}}},             "::foo"},
        {state::ScopeSequence{.scopes = {state::UnqualifiedId{.name = ID{"foo"}, .templateArgs = state::TemplateArgumentList{}}}}, "foo<>"},
    }));

    std::string const input = "void (" + scopeText + "::*)()" + cvText + refText + " " + noexceptText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(!id->qualifications.isConst);
    CHECK(!id->qualifications.isVolatile);
    CHECK_THAT(
        id->base,
        variant_equals(state::BuiltinType{.base = lexing::keyword{"void"}}));

    // clang-format off
    state::AbstractDeclarator::Layer const expected{
        .nested = state::Recursive{
            state::AbstractDeclarator::Layer{
                .decorations = {state::PointerDeclarator{.scopes = expectedScopes}}}},
        .function = state::FunctionDeclarator{
            .qualifiers = expectedCV,
            .refQualifier = expectedRef,
            .isNoexcept = expectedIsNoexcept
        }};
    // clang-format on
    CHECK(expected == id->declarator.root);
}
