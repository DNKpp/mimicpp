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
        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "const volatile"},
        {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "volatile const"},
    });
}

TEST_CASE(
    "parsing::parse_type supports pointers and references.",
    "[print][print::type]")
{
    auto const [expectedType, typeText] = GENERATE((table<state::BaseType, std::string>)({
        {state::BuiltinType{lexing::keyword{"int"}},                                                    "int"    },
        {state::QualifiedId{.identifier = lexing::identifier{"_foo123"}},                               "_foo123"},
        {state::QualifiedId{.scopes = {.explicitRoot = true}, .identifier = lexing::identifier{"foo"}}, "::foo"  },
        {state::QualifiedId{.identifier = state::SimpleTemplateId{.name = lexing::identifier{"foo"}}},  "foo<>"  },
    }));

    auto const [expectedDeclarator, declaratorText] = GENERATE((table<state::PtrOperator, std::string>)({
        {state::PointerDeclarator{},                                                                         "*"              },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true}},                     "*const"         },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isVolatile = true}},                  "* volatile"     },
        {state::PointerDeclarator{.qualifiers = state::CVQualifierSeq{.isConst = true, .isVolatile = true}}, "*volatile const"},
        {state::ReferenceDeclarator{.qualifier = state::RefQualifier::id_ref},                               "&"              },
        {state::ReferenceDeclarator{.qualifier = state::RefQualifier::id_refref},                            "&&"             },
    }));
    CAPTURE(typeText, declaratorText);

    SECTION("When it's provided as plain type.")
    {
        std::string const input = typeText + declaratorText;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);
        CHECK(expectedType == id->base);

        CHECK_THAT(id->declarator.root.core, variant_equals(std::monostate{}));
        CHECK_THAT(
            id->declarator.root.decorations,
            Catch::Matchers::RangeEquals(std::array{expectedDeclarator}));
    }

    SECTION("When they are provided with arbitrary cv prefix qualification.")
    {
        auto const [expectedCV, qualifierPrefix] = GENERATE(from_range(cvTable));
        std::string const input = qualifierPrefix + " " + typeText + declaratorText;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->leadingQualifications);
        CHECK(expectedType == id->base);

        CHECK_THAT(id->declarator.root.core, variant_equals(std::monostate{}));
        CHECK_THAT(
            id->declarator.root.decorations,
            Catch::Matchers::RangeEquals(std::array{expectedDeclarator}));
    }
}
