//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/type/Parser2.hpp"

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
        .function = state::FunctionDeclarator{.base = {.isNoexcept = true}}};
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
        .function = state::FunctionDeclarator{.base = {.qualifiers = expectedCV}}};
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
        .function = state::FunctionDeclarator{.base = {.refQualifier = expectedRef}}};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports function-types with arbitrary return-types.",
    "[print][print::type]")
{
    using Id = lexing::identifier;
    using KW = lexing::keyword;
    using TId = state::SimpleTemplateId;
    using UQId = state::UnqualifiedId;
    using QId = state::QualifiedId;

    auto const [expectedReturn, returnText] = GENERATE((table<state::BaseType, std::string>)({
        {state::BuiltinType{KW{"int"}},                                                                                                    "int"     },
        {QId{.identifier = Id{"foo"}},                                                                                                     "foo"     },
        {QId{.scopes = {.explicitRoot = true}, .identifier = Id{"foo"}},                                                                   "::foo"   },
        {QId{.scopes = {.scopes = {UQId{Id{"bar"}}}}, .identifier = Id{"foo"}},                                                            "bar::foo"},
        {QId{.identifier = TId{.name = Id{"foo"}}},                                                                                        "foo<>"   },
        {QId{.identifier = TId{.name = Id{"foo"}, .args = {state::RecursiveState{state::TypeId{.base = state::BuiltinType{KW{"int"}}}}}}}, "foo<int>"},
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
        .nested = state::RecursiveState{state::AbstractDeclarator::Layer{
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

    state::ParametersAndQualifiers const paramsAndQualifiers{
        .params = {
                   // `int const long&`
            state::RecursiveState{
                state::TypeId{
                    .qualifications{.isConst = true},
                    .base = state::BuiltinType{.base = lexing::keyword{"int"}, .sizeSpec = state::BuiltinType::SizeSpec::id_long},
                    .declarator = {.root = {.decorations = {state::ReferenceDeclarator{state::RefQualifier::id_ref}}}}}},

                   // `foo<short, unsigned> &&`
            state::RecursiveState{
                state::TypeId{
                    .base = state::QualifiedId{
                        .identifier = state::SimpleTemplateId{
                            .name = lexing::identifier{"foo"},
                            .args = {
                                state::RecursiveState{
                                    state::TypeId{.base = state::BuiltinType{.sizeSpec = state::BuiltinType::SizeSpec::id_short}}},
                                state::RecursiveState{
                                    state::TypeId{.base = state::BuiltinType{.signedSpec = state::BuiltinType::SignedSpec::id_unsigned}}}}}},
                    .declarator = {.root = {.decorations = {state::ReferenceDeclarator{state::RefQualifier::id_refref}}}}}},

                   // `char (* const)[][1337]`
            state::RecursiveState{[] {
                return state::TypeId{
                    .base = state::BuiltinType{.base = lexing::keyword{"char"}},
                    .declarator = {
                        .root = {
                            .nested = state::RecursiveState{
                                state::AbstractDeclarator::Layer{
                                    .decorations = {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true}}}}},
                            .arrays = {state::ArrayDeclarator{}, state::ArrayDeclarator{.size = state::ConstantExpression{"1337"}}}}}};
            }()}}
    };
    state::AbstractDeclarator::Layer const expected{
        .function = state::FunctionDeclarator{.base = paramsAndQualifiers}};
    CHECK(expected == id->declarator.root);
}
