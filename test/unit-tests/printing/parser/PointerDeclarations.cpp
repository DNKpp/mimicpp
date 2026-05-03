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
    "parsing::parse_type supports pointers and references.",
    "[print][print::type]")
{
    auto const [expectedType, typeText] = GENERATE((table<state::BaseType, std::string>)({
        {state::BuiltinType{lexing::keyword{"int"}},                                                             "int"    },
        {state::QualifiedId{.identifier = {state::Identifier{"_foo123"}}},                                       "_foo123"},
        {state::QualifiedId{.scopes = {.explicitRoot = true}, .identifier = {state::Identifier{"foo"}}},         "::foo"  },
        {state::QualifiedId{.identifier = {.name = state::Identifier{"foo"}, .templateArgs{std::in_place, 0u}}}, "foo<>"  },
    }));

    auto const [expectedDecoration, declaratorText] = GENERATE(cat(
        from_range(ptrTable),
        from_range(refTable)));
    CAPTURE(typeText, declaratorText);

    state::AbstractDeclarator::Layer const expectedDeclarator{
        .decorations = {expectedDecoration}};

    SECTION("When it's provided as plain type.")
    {
        std::string const input = typeText + declaratorText;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(expectedType == id->base);
        CHECK(expectedDeclarator == id->declarator.root);
    }

    SECTION("When they are provided with arbitrary cv qualification.")
    {
        auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(from_range(cvTable));
        std::string const input = qualifierPrefix + " " + typeText + " " + qualifierSuffix + declaratorText;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->qualifications);
        CHECK(expectedType == id->base);
        CHECK(expectedDeclarator == id->declarator.root);
    }
}

TEST_CASE(
    "parsing::parse_type supports pointers of pointers and references of pointers.",
    "[print][print::type]")
{
    auto const [expectedType, typeText] = GENERATE((table<state::BaseType, std::string>)({
        {state::BuiltinType{lexing::keyword{"int"}},                                                             "int"    },
        {state::QualifiedId{.identifier = {state::Identifier{"_foo123"}}},                                       "_foo123"},
        {state::QualifiedId{.scopes = {.explicitRoot = true}, .identifier = {state::Identifier{"foo"}}},         "::foo"  },
        {state::QualifiedId{.identifier = {.name = state::Identifier{"foo"}, .templateArgs{std::in_place, 0u}}}, "foo<>"  },
    }));

    auto const [expectedFirstLvlDecoration, firstLevelPtrOpText] = GENERATE(from_range(ptrTable));
    auto const [expectedSecondLvlDecoration, secondLevelPtrOpText] = GENERATE(cat(
        from_range(ptrTable),
        from_range(refTable)));
    CAPTURE(typeText, firstLevelPtrOpText, secondLevelPtrOpText);

    state::AbstractDeclarator::Layer const expectedDeclarator{
        .decorations = {expectedFirstLvlDecoration, expectedSecondLvlDecoration}
    };

    SECTION("When it's provided as plain type.")
    {
        std::string const input = typeText + firstLevelPtrOpText + secondLevelPtrOpText;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->qualifications.isConst);
        CHECK(!id->qualifications.isVolatile);
        CHECK(expectedType == id->base);
        CHECK(expectedDeclarator == id->declarator.root);
    }

    SECTION("When they are provided with arbitrary cv qualification.")
    {
        auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(from_range(cvTable));
        std::string const input = qualifierPrefix + " " + typeText + " " + qualifierSuffix + firstLevelPtrOpText + secondLevelPtrOpText;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->qualifications);
        CHECK(expectedType == id->base);
        CHECK(expectedDeclarator == id->declarator.root);
    }
}

TEST_CASE(
    "parsing::parse_type supports arrays of pointer types.",
    "[print][print::type]")
{
    using CVEntry = std::ranges::range_value_t<decltype(cvTable)>;
    auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(cat(
        from_range(std::views::single(CVEntry{{}, "", ""})),
        from_range(cvTable)));
    auto const [expectedPtrDecoration, ptrDeclText] = GENERATE(from_range(ptrTable));

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

    std::string const input = qualifierPrefix + " int " + qualifierSuffix + ptrDeclText + arrayText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(expectedCV == id->qualifications);
    CHECK_THAT(id->base, variant_equals(expectedType));
    state::AbstractDeclarator::Layer const expected{
        .decorations = {expectedPtrDecoration},
        .arrays = arrayDeclarators};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports arrays of pointers to pointer types.",
    "[print][print::type]")
{
    using CVEntry = std::ranges::range_value_t<decltype(cvTable)>;
    auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(cat(
        from_range(std::views::single(CVEntry{{}, "", ""})),
        from_range(cvTable)));
    auto const [expectedFirstPtrDecoration, firstPtrDeclText] = GENERATE(from_range(ptrTable));
    auto const [expectedSecondPtrDecoration, secondPtrDeclText] = GENERATE(from_range(ptrTable));

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

    std::string const input = qualifierPrefix + " int " + qualifierSuffix + firstPtrDeclText + secondPtrDeclText + arrayText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(expectedCV == id->qualifications);
    CHECK_THAT(id->base, variant_equals(expectedType));
    state::AbstractDeclarator::Layer const expected{
        .decorations = {expectedFirstPtrDecoration, expectedSecondPtrDecoration},
        .arrays = arrayDeclarators
    };
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports pointers to array types.",
    "[print][print::type]")
{
    using CVEntry = std::ranges::range_value_t<decltype(cvTable)>;
    auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(cat(
        from_range(std::views::single(CVEntry{{}, "", ""})),
        from_range(cvTable)));
    auto const [expectedPtrDecoration, ptrDeclText] = GENERATE(from_range(ptrTable));

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

    std::string const input = qualifierPrefix + " int " + qualifierSuffix + "(" + ptrDeclText + ")" + arrayText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(expectedCV == id->qualifications);
    CHECK_THAT(id->base, variant_equals(expectedType));
    state::AbstractDeclarator::Layer const expected{
        .nested = state::RecursiveState{state::AbstractDeclarator::Layer{.decorations = {expectedPtrDecoration}}},
        .arrays = arrayDeclarators};
    CHECK(expected == id->declarator.root);
}

TEST_CASE(
    "parsing::parse_type supports pointers to pointers to an array.",
    "[print][print::type]")
{
    using CVEntry = std::ranges::range_value_t<decltype(cvTable)>;
    auto const [expectedCV, qualifierPrefix, qualifierSuffix] = GENERATE(cat(
        from_range(std::views::single(CVEntry{{}, "", ""})),
        from_range(cvTable)));
    auto const [expectedFirstPtrDecoration, firstPtrDeclText] = GENERATE(from_range(ptrTable));
    auto const [expectedSecondPtrDecoration, secondPtrDeclText] = GENERATE(from_range(ptrTable));

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

    std::string const input = qualifierPrefix + " int " + qualifierSuffix + "(" + firstPtrDeclText + secondPtrDeclText + ")" + arrayText;
    CAPTURE(input);

    auto const id = parse_type(input);
    REQUIRE(id);

    CHECK(expectedCV == id->qualifications);
    CHECK_THAT(id->base, variant_equals(expectedType));
    state::AbstractDeclarator::Layer const expected{
        .nested = state::RecursiveState{state::AbstractDeclarator::Layer{.decorations = {expectedFirstPtrDecoration, expectedSecondPtrDecoration}}},
        .arrays = arrayDeclarators};
    CHECK(expected == id->declarator.root);
}
