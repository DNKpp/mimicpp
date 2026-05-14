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

    std::array const ptrTable = std::to_array<std::tuple<state::PtrOperator, std::string>>({
        {state::PointerDeclarator{},                                                                         "*"              },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true}},                     "*const"         },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isVolatile = true}},                  "* volatile"     },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true, .isVolatile = true}}, "*volatile const"},
    });

    std::array const refTable = std::to_array<std::tuple<state::PtrOperator, std::string>>({
        {state::ReferenceDeclarator{.qualifier = state::RefQualifier::id_ref},    "&" },
        {state::ReferenceDeclarator{.qualifier = state::RefQualifier::id_refref}, "&&"},
    });
}

TEST_CASE(
    "parsing::parse_type supports conversion functions to builtin-types.",
    "[print][print::type]")
{
    std::string const input = "foo::operator int()::bar";
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
                    .name = state::ConversionFunctionId{
                        .target = state::Recursive{state::TypeId{.base = state::BuiltinType{lexing::keyword{"int"}}}}},
                    .functionDeclarator = state::FunctionDeclarator{}}}},
        .identifier = state::UnqualifiedId{.name = state::Identifier{.content = "bar"}}};
    CHECK_THAT(id->base, variant_equals(expected));
}

TEST_CASE(
    "parsing::parse_type supports conversion functions to qualified types.",
    "[print][print::type]")
{
    auto const [expectedCV, prefixCVText, suffixCVText] = GENERATE(from_range(cvTable));
    auto const [expectedFirstLvlDecoration, firstLevelPtrOpText] = GENERATE(from_range(ptrTable));
    auto const [expectedSecondLvlDecoration, secondLevelPtrOpText] = GENERATE(cat(
        from_range(ptrTable),
        from_range(refTable)));
    CAPTURE(firstLevelPtrOpText, secondLevelPtrOpText);

    std::string const input = "foo::operator " + prefixCVText + " int " + suffixCVText + firstLevelPtrOpText + secondLevelPtrOpText + "()::bar";
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
                    .name = state::ConversionFunctionId{
                        .target = state::Recursive{state::TypeId{
                            .qualifications = expectedCV,
                            .base = state::BuiltinType{lexing::keyword{"int"}},
                            .declarator = {.root = {.decorations = {expectedFirstLvlDecoration, expectedSecondLvlDecoration}}}}}},
                    .functionDeclarator = state::FunctionDeclarator{}}}},
        .identifier = state::UnqualifiedId{.name = state::Identifier{.content = "bar"}}};
    CHECK_THAT(id->base, variant_equals(expected));
}

// This case is broken because the whole `::_test_::foo123()::bar` is treated as the conversion-type-id.
// However, this is a very synthetical example, which can probably not even produces via valid c++ code,
// because there is no way returning a function-local type from a conversion operator.
TEST_CASE(
    "parsing::parse_type supports conversion functions to general types.",
    "[!shouldfail][print][print::type]")
{
    std::string const input = "foo::operator ::_test_::foo123()::bar";
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
                    .name = state::ConversionFunctionId{
                        .target = state::Recursive{state::TypeId{
                            .base = state::QualifiedId{
                                .scopes = {.explicitRoot = true, .scopes = {{.name = state::Identifier{"_test_"}}}},
                                .identifier = state::UnqualifiedId{.name = state::Identifier{"foo123"}}}}}},
                    .functionDeclarator = state::FunctionDeclarator{}}}},
        .identifier = state::UnqualifiedId{.name = state::Identifier{.content = "bar"}}};

    CHECK_THAT(id->base, variant_equals(expected));
}
