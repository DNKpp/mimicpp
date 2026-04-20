//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/type/Parser2.hpp"

#include "TestTypes.hpp"

using mimicpp::printing::type::parse_type;
namespace lexing = mimicpp::printing::type::lexing;
namespace state = mimicpp::printing::type::parsing::v2::state;

TEST_CASE(
    "parsing::parse_type supports unsigned and signed builtin-types.",
    "[print][print::type]")
{
    using SignedSpec = state::BuiltinType::SignedSpec;
    auto const [expectedSpec, type] = GENERATE((table<SignedSpec, std::string>)({
        {  SignedSpec::id_signed,   "signed"},
        {SignedSpec::id_unsigned, "unsigned"},
    }));
    CAPTURE(type);

    SECTION("When they are provided as plain type.")
    {
        auto const id = parse_type(type);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{.signedSpec = expectedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are combined with a size-spec.")
    {
        using SizeSpec = state::BuiltinType::SizeSpec;
        auto const [expectedSizeSpec, specText] = GENERATE((table<SizeSpec, std::string>)({
            {   SizeSpec::id_short,     "short"},
            {    SizeSpec::id_long,      "long"},
            {SizeSpec::id_longlong, "long long"},
        }));
        std::string const input = specText + " " + type;
        CAPTURE(specText, input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{.sizeSpec = expectedSizeSpec, .signedSpec = expectedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are provided with arbitrary cv prefix qualification.")
    {
        auto const [expectedCV, qualifierPrefix] = GENERATE((table<state::CVQualifierSeq, std::string>)({
            {                    state::CVQualifierSeq{.isConst = true},          "const"},
            {                 state::CVQualifierSeq{.isVolatile = true},       "volatile"},
            {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "const volatile"},
            {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "volatile const"},
        }));
        std::string const input = qualifierPrefix + " " + type;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->leadingQualifications);
        state::BuiltinType const expected{.signedSpec = expectedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports long and short builtin-types.",
    "[print][print::type]")
{
    using SizeSpec = state::BuiltinType::SizeSpec;
    auto const [expectedSpec, type] = GENERATE((table<SizeSpec, std::string>)({
        {   SizeSpec::id_short,     "short"},
        {    SizeSpec::id_long,      "long"},
        {SizeSpec::id_longlong, "long long"},
    }));
    CAPTURE(type);

    SECTION("When they are provided as plain type.")
    {
        auto const id = parse_type(type);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{.sizeSpec = expectedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are combined with a signed-spec.")
    {
        using SignedSpec = state::BuiltinType::SignedSpec;
        auto const [expectedSignedSpec, specText] = GENERATE((table<SignedSpec, std::string>)({
            {  SignedSpec::id_signed,   "signed"},
            {SignedSpec::id_unsigned, "unsigned"},
        }));
        std::string const input = specText + " " + type;
        CAPTURE(specText, input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{.sizeSpec = expectedSpec, .signedSpec = expectedSignedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are provided with arbitrary cv prefix qualification.")
    {
        auto const [expectedCV, qualifierPrefix] = GENERATE((table<state::CVQualifierSeq, std::string>)({
            {                    state::CVQualifierSeq{.isConst = true},          "const"},
            {                 state::CVQualifierSeq{.isVolatile = true},       "volatile"},
            {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "const volatile"},
            {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "volatile const"},
        }));
        std::string const input = qualifierPrefix + " " + type;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->leadingQualifications);
        state::BuiltinType const expected{.sizeSpec = expectedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports int as builtin-type.",
    "[print][print::type]")
{
    SECTION("When it's provided as plain type.")
    {
        auto const id = parse_type("int");
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{.base = lexing::keyword{"int"}};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When it is combined with arbitrary specs.")
    {
        using SignedSpec = state::BuiltinType::SignedSpec;
        using SizeSpec = state::BuiltinType::SizeSpec;
        auto const [expectedSignedSpec, expectedSizeSpec, input] = GENERATE(
            (table<std::optional<SignedSpec>, std::optional<SizeSpec>, std::string>)({
                {  SignedSpec::id_signed,          std::nullopt,             "signed int"},
                {  SignedSpec::id_signed,          std::nullopt,             "int signed"},
                {SignedSpec::id_unsigned,          std::nullopt,           "unsigned int"},
                {SignedSpec::id_unsigned,          std::nullopt,           "int unsigned"},

                {           std::nullopt,     SizeSpec::id_long,               "long int"},
                {           std::nullopt,     SizeSpec::id_long,               "int long"},
                {           std::nullopt,    SizeSpec::id_short,              "short int"},
                {           std::nullopt,    SizeSpec::id_short,              "int short"},

                {           std::nullopt, SizeSpec::id_longlong,          "long long int"},
                {           std::nullopt, SizeSpec::id_longlong,          "int long long"},
                {           std::nullopt, SizeSpec::id_longlong,          "long int long"},

                {  SignedSpec::id_signed,     SizeSpec::id_long,        "signed long int"},
                {  SignedSpec::id_signed,     SizeSpec::id_long,        "signed int long"},
                {  SignedSpec::id_signed,     SizeSpec::id_long,        "long int signed"},
                {  SignedSpec::id_signed,     SizeSpec::id_long,        "int long signed"},
                {  SignedSpec::id_signed, SizeSpec::id_longlong,   "long int long signed"},
                {SignedSpec::id_unsigned, SizeSpec::id_longlong, "unsigned long int long"},
        }));
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{
            .base = lexing::keyword{"int"},
            .sizeSpec = expectedSizeSpec,
            .signedSpec = expectedSignedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports long as builtin-type.",
    "[print][print::type]")
{
    using SizeSpec = state::BuiltinType::SizeSpec;

    SECTION("When it's provided as plain type.")
    {
        auto const id = parse_type("long");
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{.sizeSpec = SizeSpec::id_long};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When it is combined with arbitrary specs.")
    {
        using SignedSpec = state::BuiltinType::SignedSpec;
        auto const [expectedSignedSpec, expectedSizeSpec, input] = GENERATE(
            (table<std::optional<SignedSpec>, std::optional<SizeSpec>, std::string>)({
                {  SignedSpec::id_signed,     SizeSpec::id_long,        "signed long"},
                {  SignedSpec::id_signed,     SizeSpec::id_long,        "long signed"},
                {SignedSpec::id_unsigned,     SizeSpec::id_long,      "unsigned long"},
                {SignedSpec::id_unsigned,     SizeSpec::id_long,      "long unsigned"},

                {  SignedSpec::id_signed, SizeSpec::id_longlong,   "signed long long"},
                {SignedSpec::id_unsigned, SizeSpec::id_longlong, "long long unsigned"},
                {  SignedSpec::id_signed, SizeSpec::id_longlong,   "long signed long"},
        }));
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{
            .sizeSpec = expectedSizeSpec,
            .signedSpec = expectedSignedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When they are provided with arbitrary cv prefix qualification.")
    {
        auto const [expectedCV, qualifierPrefix] = GENERATE((table<state::CVQualifierSeq, std::string>)({
            {                    state::CVQualifierSeq{.isConst = true},          "const"},
            {                 state::CVQualifierSeq{.isVolatile = true},       "volatile"},
            {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "const volatile"},
            {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "volatile const"},
        }));
        std::string const input = qualifierPrefix + " long";
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->leadingQualifications);
        state::BuiltinType const expected{.sizeSpec = SizeSpec::id_long};
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports double as builtin-type.",
    "[print][print::type]")
{
    SECTION("When it's provided as plain type.")
    {
        auto const id = parse_type("double");
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{.base = lexing::keyword{"double"}};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When it is combined with the long spec.")
    {
        using SizeSpec = state::BuiltinType::SizeSpec;
        std::string const input = GENERATE("long double", "double long");
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{
            .base = lexing::keyword{"double"},
            .sizeSpec = SizeSpec::id_long};
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports char as builtin-type.",
    "[print][print::type]")
{
    SECTION("When it's provided as plain type.")
    {
        auto const id = parse_type("char");
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{.base = lexing::keyword{"char"}};
        CHECK_THAT(id->base, variant_equals(expected));
    }

    SECTION("When it is combined with arbitrary specs.")
    {
        using SignedSpec = state::BuiltinType::SignedSpec;
        auto const [expectedSignedSpec, input] = GENERATE((table<std::optional<SignedSpec>, std::string>)({
            {  SignedSpec::id_signed,   "signed char"},
            {  SignedSpec::id_signed,   "char signed"},
            {SignedSpec::id_unsigned, "unsigned char"},
            {SignedSpec::id_unsigned, "char unsigned"},
        }));
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);

        state::BuiltinType const expected{
            .base = lexing::keyword{"char"},
            .signedSpec = expectedSignedSpec};
        CHECK_THAT(id->base, variant_equals(expected));
    }
}

TEST_CASE(
    "parsing::parse_type supports void as builtin-type.",
    "[print][print::type]")
{
    auto const id = parse_type("void");
    REQUIRE(id);

    CHECK(!id->leadingQualifications.isConst);
    CHECK(!id->leadingQualifications.isVolatile);

    state::BuiltinType const expected{.base = lexing::keyword{"void"}};
    CHECK_THAT(id->base, variant_equals(expected));
}

TEST_CASE(
    "parsing::parse_type_id supports all builtin-types.",
    "[print][print::type]")
{
    std::string const type = GENERATE(
        "auto",
        //"void",
        "bool",
        "int",
        "float",
        "double",
        //"unsigned", this is a specifier, which is tested separately
        //"signed",   ^
        //"short",    ^
        //"long",     ^
        "char",
        "wchar_t",
        "char8_t",
        "char16_t",
        "char32_t");
    CAPTURE(type);

    SECTION("When they are provided as plain type.")
    {
        auto const id = parse_type(type);
        REQUIRE(id);

        CHECK(!id->leadingQualifications.isConst);
        CHECK(!id->leadingQualifications.isVolatile);
        CHECK_THAT(id->base, variant_equals(state::BuiltinType{.base{type}}));
    }

    SECTION("When they are provided with arbitrary cv prefix qualification.")
    {
        auto const [expectedCV, qualifierPrefix] = GENERATE((table<state::CVQualifierSeq, std::string>)({
            {                    state::CVQualifierSeq{.isConst = true},          "const"},
            {                 state::CVQualifierSeq{.isVolatile = true},       "volatile"},
            {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "const volatile"},
            {state::CVQualifierSeq{.isConst = true, .isVolatile = true}, "volatile const"},
        }));
        std::string const input = qualifierPrefix + " " + type;
        CAPTURE(input);

        auto const id = parse_type(input);
        REQUIRE(id);

        CHECK(expectedCV == id->leadingQualifications);
        CHECK_THAT(id->base, variant_equals(state::BuiltinType{.base{type}}));
    }
}
