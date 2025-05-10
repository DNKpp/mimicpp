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
        Mock<void(StringViewT)> unrecognized{{.name = "VisitorMock::unrecognized"}};

        Mock<void()> begin{{.name = "VisitorMock::begin"}};
        Mock<void()> end{{.name = "VisitorMock::end"}};

        Mock<void(StringViewT)> add_identifier{{.name = "VisitorMock::add_identifier"}};
        Mock<void()> add_arg{{.name = "VisitorMock::add_arg"}};

        Mock<void()> begin_scope{{.name = "VisitorMock::begin_scope"}};
        Mock<void()> end_scope{{.name = "VisitorMock::end_scope"}};

        Mock<void()> begin_type{{.name = "VisitorMock::begin_type"}};
        Mock<void()> end_type{{.name = "VisitorMock::end_type"}};

        Mock<void(std::ptrdiff_t)> begin_template_args{{.name = "VisitorMock::begin_template_args"}};
        Mock<void()> end_template_args{{.name = "VisitorMock::end_template_args"}};

        Mock<void()> begin_function{{.name = "VisitorMock::begin_function"}};
        Mock<void()> end_function{{.name = "VisitorMock::end_function"}};
        Mock<void()> begin_return_type{{.name = "VisitorMock::begin_return_type"}};
        Mock<void()> end_return_type{{.name = "VisitorMock::end_return_type"}};
        Mock<void(std::ptrdiff_t)> begin_function_args{{.name = "VisitorMock::begin_function_args"}};
        Mock<void()> end_function_args{{.name = "VisitorMock::end_function_args"}};

        Mock<void()> begin_function_ptr{{.name = "VisitorMock::begin_function_ptr"}};
        Mock<void()> end_function_ptr{{.name = "VisitorMock::end_function_ptr"}};

        Mock<void()> begin_operator_identifier{{.name = "VisitorMock::begin_operator_identifier"}};
        Mock<void()> end_operator_identifier{{.name = "VisitorMock::end_operator_identifier"}};

        Mock<void()> add_const{{.name = "VisitorMock::add_const"}};
        Mock<void()> add_volatile{{.name = "VisitorMock::add_volatile"}};
        Mock<void()> add_noexcept{{.name = "VisitorMock::add_noexcept"}};
        Mock<void()> add_ptr{{.name = "VisitorMock::add_ptr"}};
        Mock<void()> add_lvalue_ref{{.name = "VisitorMock::add_lvalue_ref"}};
        Mock<void()> add_rvalue_ref{{.name = "VisitorMock::add_rvalue_ref"}};

        [[nodiscard]]
        auto expect_spec_call(StringViewT const content)
        {
            if ("const" == content)
            {
                return add_const.expect_call();
            }

            if ("volatile" == content)
            {
                return add_volatile.expect_call();
            }

            if ("noexcept" == content)
            {
                return add_noexcept.expect_call();
            }

            if ("*" == content)
            {
                return add_ptr.expect_call();
            }

            if ("&" == content)
            {
                return add_lvalue_ref.expect_call();
            }

            if ("&&" == content)
            {
                return add_rvalue_ref.expect_call();
            }

            util::unreachable();
        }
    };

    constexpr std::array placeholderCollection = std::to_array<StringViewT>({
        "{placeholder}",
        "{__placeholder}",
        "{place holder}",
        "{place-holder}",
        "{anon class}",

        //"(placeholder)", this will never be supported as we can not reliably distinguish that from FunctionArgs
        //"(__placeholder)",
        "(place holder)",
        "(place-holder)",
        "(anon class)",

        "<placeholder>",
        "<__placeholder>",
        "<place holder>",
        "<place-holder>",
        "<anon class>",

        "`placeholder'",
        "`__placeholder'",
        "`place holder'",
        "`place-holder'",
        "`anon class'",

        "'placeholder'",
        "'__placeholder'",
        "'place holder'",
        "'place-holder'",
        "'anon class'",
    });
}

TEST_CASE(
    "parsing::NameParser rejects unrecognizable input.",
    "[print][print::type]")
{
    StringViewT constexpr input{"Hello, World!"};

    VisitorMock visitor{};

    SCOPED_EXP visitor.unrecognized.expect_call("Hello, World!");

    printing::type::parsing::NameParser parser{std::ref(visitor), input};
    SECTION("When parsing type.")
    {
        parser.parse_type();
    }

    SECTION("When parsing function.")
    {
        parser.parse_function();
    }
}

TEST_CASE(
    "parsing::NameParser supports builtin-types.",
    "[print][print::type]")
{
    StringT const type = GENERATE(
        "auto",
        "int",
        "float",
        "double",
        "unsigned",
        "signed",
        "unsigned char",
        "signed char",
        "long long",
        "unsigned long long",
        "signed long long");

    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When plain type is given.")
    {
        StringT const input = type;
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(input);
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When qualified type is given.")
    {
        StringT const qualification = GENERATE("&", "&&", "*");
        StringT const spacing = GENERATE("", " ");
        StringT const input = type + spacing + qualification;
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(type);
        sequence += visitor.expect_spec_call(qualification);
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When type is used as template-argument.")
    {
        StringT const input = "foo<" + type + ">";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(1);
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(type);
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When type is used as function-argument.")
    {
        StringT const input = "ret (" + type + ")";
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("ret");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_args.expect_call(1);
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(type);
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When type is used as return-type.")
    {
        StringT const input = type + " ()";
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(type);
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser detects identifiers.",
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

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(input);
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When multiple scopes are given.")
    {
        StringViewT const firstScope = GENERATE(from_range(identifiers));
        StringViewT const secondScope = GENERATE(from_range(identifiers));
        StringViewT const thirdScope = GENERATE(from_range(identifiers));

        StringT const input = StringT{firstScope} + "::" + StringT{secondScope} + "::" + StringT{thirdScope};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(firstScope);
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(secondScope);
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call(thirdScope);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
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

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When given before the actual identifier with ref/pointer.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const indirection = GENERATE("&", "&&", "*");
        StringT const input{spec + " foo " + indirection};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.expect_spec_call(spec);
        sequence += visitor.expect_spec_call(indirection);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When given after the actual identifier.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const input{"foo " + spec};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When given after the actual identifier with ref/pointer.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const indirection = GENERATE("&", "&&", "*");
        StringT const input{"foo " + spec + indirection};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.expect_spec_call(spec);
        sequence += visitor.expect_spec_call(indirection);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Coming all together")
    {
        StringT const input{"volatile foo const* volatile** const&"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser handles decorated types.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    StringT const indirection = GENERATE("&", "&&", "*");
    StringT const spacing = GENERATE("", " ");
    StringT const input = "int* __ptr64" + spacing + indirection + " __ptr64";
    CAPTURE(indirection, input);

    sequence += visitor.begin.expect_call();
    sequence += visitor.begin_type.expect_call();

    sequence += visitor.add_identifier.expect_call("int");
    sequence += visitor.add_ptr.expect_call();
    sequence += visitor.expect_spec_call(indirection);

    sequence += visitor.end_type.expect_call();
    sequence += visitor.end.expect_call();

    printing::type::parsing::NameParser parser{std::ref(visitor), input};
    parser.parse_type();
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

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(0);
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When templated identifier with placeholder arg is given.")
    {
        StringT const placeholder{GENERATE(from_range(placeholderCollection))};
        StringT const input = "foo<" + placeholder + ">";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(1);
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(placeholder);
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When qualified templated identifier is given.")
    {
        StringViewT const input{"volatile foo<> const&"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(0);
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When templated identifier with multiple args is given.")
    {
        StringViewT const input{"foo<int, std::string>"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(2);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.add_arg.expect_call();

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When templated identifier with multiple args with specs is given.")
    {
        StringViewT const input{"foo<const int volatile&, const std::string>"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(2);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.add_arg.expect_call();

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When templated identifier has templated arg.")
    {
        StringViewT const input{"foo<bar<>>"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(1);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");
        sequence += visitor.begin_template_args.expect_call(0);
        sequence += visitor.end_template_args.expect_call();
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When templated identifier has qualified templated arg.")
    {
        StringViewT const input{"foo<const bar<int*> volatile&>"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(1);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");

        {
            sequence += visitor.begin_template_args.expect_call(1);

            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("int");
            sequence += visitor.add_ptr.expect_call();
            sequence += visitor.end_type.expect_call();

            sequence += visitor.end_template_args.expect_call();
        }

        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When template is part of a scope.")
    {
        StringViewT const input{"foo<>::bar"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.begin_template_args.expect_call(0);
        sequence += visitor.end_template_args.expect_call();
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("bar");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser detects named functions.",
    "[print][print::type]")
{
    StringT const spec = GENERATE("", "const", "volatile", "noexcept", "&", "&&");
    StringT const specSpace = GENERATE("", " ");
    StringT const suffix = specSpace + spec;

    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();
    sequence += visitor.begin_function.expect_call();

    SECTION("When function identifier with 0 args is given.")
    {
        StringT const returnType = GENERATE("", "void");
        StringT const prefix = returnType + (returnType.empty() ? "" : " ");
        StringT const input = prefix + "foo()" + suffix;
        CAPTURE(input);

        CHECKED_IF(!returnType.empty())
        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call(returnType);
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When templated function identifier is given.")
    {
        StringT const returnType = GENERATE("", "void");
        StringT const prefix = returnType + (returnType.empty() ? "" : " ");
        StringT const input = prefix + "foo<int>()" + suffix;
        CAPTURE(input);

        CHECKED_IF(!returnType.empty())
        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call(returnType);
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(1);
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When function identifier with single arg is given.")
    {
        StringT const returnType = GENERATE("", "void");
        StringT const prefix = returnType + (returnType.empty() ? "" : " ");
        StringT const input = prefix + "foo(const std::string)" + suffix;
        CAPTURE(input);

        CHECKED_IF(!returnType.empty())
        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call(returnType);
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call(1);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When function identifier with templated arg is given.")
    {
        StringT const returnType = GENERATE("", "void");
        StringT const prefix = returnType + (returnType.empty() ? "" : " ");
        StringT const input = prefix + "foo(volatile bar<int> const&)" + suffix;
        CAPTURE(input);

        CHECKED_IF(!returnType.empty())
        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call(returnType);
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call(1);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");

        sequence += visitor.begin_template_args.expect_call(1);
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When function identifier with multiple args is given.")
    {
        StringT const returnType = GENERATE("", "void");
        StringT const prefix = returnType + (returnType.empty() ? "" : " ");
        StringT const input = prefix + "foo(const char&&, const int)" + suffix;
        CAPTURE(input);

        CHECKED_IF(!returnType.empty())
        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call(returnType);
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call(2);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("char");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_rvalue_ref.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.add_arg.expect_call();

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When function is a scope.")
    {
        StringT const returnType = GENERATE("", "void");
        StringT const prefix = returnType + (returnType.empty() ? "" : " ");
        StringT const templateExpr = GENERATE("", "<>");
        StringT const scopeSpec = GENERATE("", "const", "volatile", "&", "&&", "noexcept");
        StringT const input = prefix + "foo" + templateExpr + "()" + scopeSpec + "::bar::fun()" + suffix;
        CAPTURE(input);

        CHECKED_IF(!returnType.empty())
        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call(returnType);
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        {
            sequence += visitor.begin_scope.expect_call();
            sequence += visitor.begin_function.expect_call();
            sequence += visitor.add_identifier.expect_call("foo");

            CHECKED_IF(!templateExpr.empty())
            {
                sequence += visitor.begin_template_args.expect_call(0);
                sequence += visitor.end_template_args.expect_call();
            }

            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();

            CHECKED_IF(!scopeSpec.empty())
            {
                sequence += visitor.expect_spec_call(scopeSpec);
            }

            sequence += visitor.end_function.expect_call();
            sequence += visitor.end_scope.expect_call();
        }

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("fun");

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When function identifier with qualified return type is given.")
    {
        StringT const input = "const std::string* volatile& foo()" + suffix;
        CAPTURE(input);

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");

        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When function identifier with templated return type is given.")
    {
        StringT const input = "bar<int>& foo()" + suffix;
        CAPTURE(input);

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call("bar");
        sequence += visitor.begin_template_args.expect_call(1);
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_template_args.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }
}

TEST_CASE(
    "parsing::NameParser detects function types.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    StringT const spec = GENERATE("", "const", "volatile", "noexcept", "&", "&&");
    StringT const specSpace = GENERATE("", " ");
    StringT const suffix = specSpace + spec;

    sequence += visitor.begin.expect_call();

    SECTION("When function has an unqualified return-type.")
    {
        StringT const input = "foo ()" + suffix;
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When function has a qualified return-type.")
    {
        StringT const input = "volatile foo const& ()" + suffix;
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When function is template argument.")
    {
        StringT const input = "foo<void ()" + suffix + ">";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(1);
        {
            sequence += visitor.begin_function.expect_call();

            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("void");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();

            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();

            CHECKED_IF(!spec.empty())
            {
                sequence += visitor.expect_spec_call(spec);
            }

            sequence += visitor.end_function.expect_call();
        }
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser detects placeholders.",
    "[print][print::type]")
{
    StringT const placeholder{GENERATE(from_range(placeholderCollection))};

    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("Plain placeholders are detected.")
    {
        StringViewT const input = placeholder;
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(input);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Templated placeholders are detected.")
    {
        StringT const input = placeholder + "<>";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(placeholder);

        sequence += visitor.begin_template_args.expect_call(0);
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Prefix qualified placeholders are detected.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const input = spec + " " + placeholder;
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(placeholder);
        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Suffix qualified placeholders are detected.")
    {
        StringT const spec = GENERATE("const", "volatile", "&", "&&", "*");
        StringT const input = placeholder + " " + spec;
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(placeholder);
        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Fully qualified placeholders are detected.")
    {
        StringT const input = "volatile " + placeholder + " const&";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(placeholder);
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Scoped placeholders are detected.")
    {
        StringT const input = "foo::" + placeholder + "::my_type";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(placeholder);
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Placeholder return types are detected.")
    {
        StringT const input = placeholder + " foo()";
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(placeholder);
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("Functions with placeholder names are detected.")
    {
        StringT const returnType = GENERATE("", "void");
        StringT const prefix = returnType + (returnType.empty() ? "" : " ");
        StringT const input = prefix + placeholder + "()";
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        CHECKED_IF(!returnType.empty())
        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call(returnType);
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.add_identifier.expect_call(placeholder);
        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("Functions with placeholder scoped names are detected.")
    {
        StringT const returnType = GENERATE("", "void");
        StringT const prefix = returnType + (returnType.empty() ? "" : " ");
        StringT const input = prefix + placeholder + "::foo()";
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        CHECKED_IF(!returnType.empty())
        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call(returnType);
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(placeholder);
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }
}

TEST_CASE(
    "parsing::NameParser detects function pointers.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When function pointer without arguments is given.")
    {
        StringT const input = "const std::string* volatile& (*)()";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");

        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When noexcept function pointer without arguments is given.")
    {
        StringT const input = "void (*)()noexcept";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.add_noexcept.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When function pointer-ref without arguments is given.")
    {
        StringT const input = "void (*&)()noexcept";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.add_noexcept.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When function pointer with arguments is given.")
    {
        StringT const input = "void (*)(const std::string&&, const int)";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(2);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_rvalue_ref.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.add_arg.expect_call();

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.end_type.expect_call();

        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When member function pointer without arguments is given.")
    {
        StringT const input = "void (foo::bar::*)()";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When member function pointer with qualifications is given.")
    {
        StringT const spec = GENERATE("const", "volatile", "noexcept", "&", "&&");
        StringT const input = "void (foo::bar::*)()" + spec;
        CAPTURE(input, spec);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When return-type is a function pointer.")
    {
        StringT const input = "void (*(float))(int)";
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        { // handles the `void (*)(int)` return-type.
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();

            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("void");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();

            sequence += visitor.begin_function_ptr.expect_call();
            sequence += visitor.add_ptr.expect_call();
            sequence += visitor.end_function_ptr.expect_call();

            sequence += visitor.begin_function_args.expect_call(1);
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("int");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_function_args.expect_call();

            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.begin_function_args.expect_call(1);
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("float");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When Function-Ptr with a function-ptr return-type is given.")
    {
        StringT const input = "void (*(*)(float))(int)";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();

        { // Handles the `void (*)(float)` return-type
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("void");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();

            sequence += visitor.begin_function_ptr.expect_call();
            sequence += visitor.add_ptr.expect_call();
            sequence += visitor.end_function_ptr.expect_call();

            sequence += visitor.begin_function_args.expect_call(1);
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("int");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_function_args.expect_call();
        }

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(1);
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("float");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When function-ptr with function-ptr parameter is given.")
    {
        StringT const input = "void (*)(void (*)())";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(1);

        {
            sequence += visitor.begin_type.expect_call();

            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("void");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();

            sequence += visitor.begin_function_ptr.expect_call();
            sequence += visitor.add_ptr.expect_call();
            sequence += visitor.end_function_ptr.expect_call();
            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();

            sequence += visitor.end_type.expect_call();
        }

        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When parameter is a template argument.")
    {
        StringT const input = "foo<void (*)()>";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call(1);

        {
            sequence += visitor.begin_type.expect_call();

            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("void");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();

            sequence += visitor.begin_function_ptr.expect_call();
            sequence += visitor.add_ptr.expect_call();
            sequence += visitor.end_function_ptr.expect_call();
            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();

            sequence += visitor.end_type.expect_call();
        }

        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser handles decorated function-ptrs.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When function-ptr is given.")
    {
        StringT const input = "void (__cdecl*)()";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When member function-ptr is given.")
    {
        StringT const input = "void (__cdecl foo::*)()";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_ptr.expect_call();
        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_scope.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser handles arbitrarily scoped identifiers.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    auto expect_args = [&] {
        {
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.begin_scope.expect_call();
            sequence += visitor.add_identifier.expect_call("std");
            sequence += visitor.end_scope.expect_call();
            sequence += visitor.add_identifier.expect_call("string");
            sequence += visitor.add_const.expect_call();
            sequence += visitor.add_lvalue_ref.expect_call();
            sequence += visitor.end_type.expect_call();

            sequence += visitor.add_arg.expect_call();

            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("int");
            sequence += visitor.add_volatile.expect_call();
            sequence += visitor.end_type.expect_call();
        }
    };

    sequence += visitor.begin.expect_call();
    sequence += visitor.begin_type.expect_call();

    SECTION("When function local type is given.")
    {
        constexpr StringViewT input{"foo(std::string const&, int volatile) noexcept::my_type"};
        CAPTURE(input);

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.begin_function.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call(2);
        expect_args();
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.add_noexcept.expect_call();
        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When qualified function local type is given.")
    {
        constexpr StringViewT input{"volatile foo(std::string const&, int volatile) noexcept::my_type const&"};
        CAPTURE(input);

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.begin_function.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call(2);
        expect_args();
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.add_noexcept.expect_call();
        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("my_type");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When nested function local type is given.")
    {
        constexpr StringViewT input{"foo::bar(std::string const&, int volatile) noexcept::my_type"};
        CAPTURE(input);

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.begin_function.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");

        sequence += visitor.begin_function_args.expect_call(2);
        expect_args();
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.add_noexcept.expect_call();
        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When deeply nested function local type is given.")
    {
        constexpr StringViewT input{"foo<int volatile, std::string const&>() const &&::bar(std::string const&, int volatile) noexcept::my_type"};
        CAPTURE(input);

        { // foo<int volatile, std::string const&>() const &&
            sequence += visitor.begin_scope.expect_call();
            sequence += visitor.begin_function.expect_call();
            sequence += visitor.add_identifier.expect_call("foo");

            sequence += visitor.begin_template_args.expect_call(2);

            {
                sequence += visitor.begin_type.expect_call();
                sequence += visitor.add_identifier.expect_call("int");
                sequence += visitor.add_volatile.expect_call();
                sequence += visitor.end_type.expect_call();

                sequence += visitor.add_arg.expect_call();

                sequence += visitor.begin_type.expect_call();
                sequence += visitor.begin_scope.expect_call();
                sequence += visitor.add_identifier.expect_call("std");
                sequence += visitor.end_scope.expect_call();
                sequence += visitor.add_identifier.expect_call("string");
                sequence += visitor.add_const.expect_call();
                sequence += visitor.add_lvalue_ref.expect_call();
                sequence += visitor.end_type.expect_call();
            }

            sequence += visitor.end_template_args.expect_call();

            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();

            sequence += visitor.add_const.expect_call();
            sequence += visitor.add_rvalue_ref.expect_call();
            sequence += visitor.end_function.expect_call();
            sequence += visitor.end_scope.expect_call();
        }

        { // bar(std::string const&, int volatile) noexcept
            sequence += visitor.begin_scope.expect_call();
            sequence += visitor.begin_function.expect_call();
            sequence += visitor.add_identifier.expect_call("bar");

            sequence += visitor.begin_function_args.expect_call(2);
            expect_args();
            sequence += visitor.end_function_args.expect_call();

            sequence += visitor.add_noexcept.expect_call();
            sequence += visitor.end_function.expect_call();
            sequence += visitor.end_scope.expect_call();
        }

        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser handles msvc-like wrapped scopes.",
    "[print][print::type]")
{
    StringT const spacing = GENERATE("", " ");
    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When scoped identifier is given, it's unwrapped.")
    {
        StringT const input = "`foo::bar'";

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("bar");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When function local type is given.")
    {
        StringT const input{"`void foo()" + spacing + "'::my_type"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        {
            sequence += visitor.begin_function.expect_call();

            /*sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("void");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();*/

            sequence += visitor.add_identifier.expect_call("foo");

            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();

            sequence += visitor.end_function.expect_call();
        }
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Deeply nested function local type is given.")
    {
        StringT const input{
            "`ret2 `ret1 `ret0 inner::fn0(int const)'::`my_placeholder'::middle::fn1()const'::outer::fn2()const &" + spacing + "'::my_type"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("inner");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        { // ``ret0 fn0(int const)'`
            sequence += visitor.begin_function.expect_call();

            /*sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("ret0");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();*/

            sequence += visitor.add_identifier.expect_call("fn0");

            sequence += visitor.begin_function_args.expect_call(1);
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("int");
            sequence += visitor.add_const.expect_call();
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_function_args.expect_call();

            sequence += visitor.end_function.expect_call();
        }
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("`my_placeholder'");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("middle");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        { // ``ret1 fn1()const'`
            sequence += visitor.begin_function.expect_call();

            /*sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("ret1");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();*/

            sequence += visitor.add_identifier.expect_call("fn1");

            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();
            sequence += visitor.add_const.expect_call();

            sequence += visitor.end_function.expect_call();
        }
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("outer");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        { // ``ret2 fn2()const'`
            sequence += visitor.begin_function.expect_call();

            /*sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("ret2");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();*/

            sequence += visitor.add_identifier.expect_call("fn2");

            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();
            sequence += visitor.add_const.expect_call();
            sequence += visitor.add_lvalue_ref.expect_call();

            sequence += visitor.end_function.expect_call();
        }
        sequence += visitor.end_scope.expect_call();
        // end `outer::fn2()const`

        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When decorated function local type is given.")
    {
        StringT const visibility = GENERATE("public", "private", "protected");
        StringT const typeClass = GENERATE("struct", "class", "enum");
        StringT const refness = GENERATE("", "&", "&&");
        StringT const input = typeClass + " `" + visibility + ": void __cdecl foo() __ptr64" + spacing + refness + "'::my_type";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        {
            sequence += visitor.begin_function.expect_call();

            /*sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();
            sequence += visitor.add_identifier.expect_call("void");
            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();*/

            sequence += visitor.add_identifier.expect_call("foo");

            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();

            CHECKED_IF(!refness.empty())
            {
                sequence += visitor.expect_spec_call(refness);
            }

            sequence += visitor.end_function.expect_call();
        }
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("When decorated lambda is given.")
    {
        StringT const input = "struct std::source_location __cdecl <lambda_11>::operator ()(void) const";
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        {
            sequence += visitor.begin_return_type.expect_call();
            sequence += visitor.begin_type.expect_call();

            sequence += visitor.begin_scope.expect_call();
            sequence += visitor.add_identifier.expect_call("std");
            sequence += visitor.end_scope.expect_call();

            sequence += visitor.add_identifier.expect_call("source_location");

            sequence += visitor.end_type.expect_call();
            sequence += visitor.end_return_type.expect_call();
        }

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("<lambda_11>");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_operator_identifier.expect_call();
        sequence += visitor.add_identifier.expect_call("()");
        sequence += visitor.end_operator_identifier.expect_call();

        // `void` function-arg is omitted
        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.add_const.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }
}

TEST_CASE(
    "parsing::NameParser handles msvc's std::stacktrace special characteristics.",
    "[print][print::type]")
{
    StringT const identifier = "executable!foo+0x1337";

    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();
    sequence += visitor.begin_type.expect_call();

    sequence += visitor.add_identifier.expect_call("foo");

    sequence += visitor.end_type.expect_call();
    sequence += visitor.end.expect_call();

    printing::type::parsing::NameParser parser{std::ref(visitor), identifier};
    parser.parse_type();
}

TEST_CASE(
    "parsing::NameParser keeps meaningful reserved identifiers.",
    "[print][print::type]")
{
    StringT const identifier = "__identifier";

    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("Just a single identifier.")
    {
        StringT const input = identifier;
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(identifier);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), identifier};
        parser.parse_type();
    }

    SECTION("Prefix qualified identifiers.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const input = spec + " " + identifier;
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(identifier);
        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Suffix qualified identifiers.")
    {
        StringT const spec = GENERATE("const", "volatile", "&", "&&", "*");
        StringT const input = identifier + " " + spec;
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(identifier);
        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("Fully qualified identifiers.")
    {
        StringT const input = "volatile " + identifier + " const&";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(identifier);
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }

    SECTION("As function name.")
    {
        StringT const input = identifier + "()";
        CAPTURE(input);

        sequence += visitor.begin_function.expect_call();

        sequence += visitor.add_identifier.expect_call(identifier);

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("As template.")
    {
        StringT const input = identifier + "<>";
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call(identifier);

        sequence += visitor.begin_template_args.expect_call(0);
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser detects operators as identifiers.",
    "[print][print::type]")
{
    // see: https://en.cppreference.com/w/cpp/language/operators
    auto const [requireSpacing, operatorSymbol] = GENERATE(
        (table<bool, StringViewT>)({
            {false,         "+"},
            {false,         "-"},
            {false,         "*"},
            {false,         "/"},
            {false,         "%"},
            {false,         "^"},
            {false,         "&"},
            {false,         "|"},
            {false,         "~"},
            {false,         "!"},
            {false,         "="},
            {false,         "<"},
            {false,         ">"},
            {false,        "+="},
            {false,        "-="},
            {false,        "*="},
            {false,        "/="},
            {false,        "%="},
            {false,        "^="},
            {false,        "&="},
            {false,        "|="},
            {false,        "<<"},
            {false,        ">>"},
            {false,       "<<="},
            {false,       ">>="},
            {false,        "=="},
            {false,        "!="},
            {false,        "<="},
            {false,        ">="},
            {false,       "<=>"},
            {false,        "&&"},
            {false,        "||"},
            {false,        "++"},
            {false,        "--"},
            {false,         ","},
            {false,       "->*"},
            {false,        "->"},
            {false,        "()"},
            {false,        "[]"},
            { true,       "new"},
            { true,     "new[]"},
            { true,    "new []"},
            { true,    "delete"},
            { true,  "delete[]"},
            { true, "delete []"},
            { true,  "co_await"}
    }));

    StringT const spacing = requireSpacing
                              ? " "
                              : GENERATE("", " ");

    VisitorMock visitor{};
    ScopedSequence sequence{};

    SECTION("When operator + symbol form a function.")
    {
        StringT const input = StringT{"operator"} + spacing + StringT{operatorSymbol} + "()";
        CAPTURE(input);

        sequence += visitor.begin.expect_call();
        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_operator_identifier.expect_call();
        sequence += visitor.add_identifier.expect_call(operatorSymbol);
        sequence += visitor.end_operator_identifier.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When templated operator is given.")
    {
        StringT const templateSpacing = GENERATE("", " ");
        StringT const input = StringT{"operator"} + spacing + StringT{operatorSymbol} + templateSpacing + "<>" + "()";
        CAPTURE(input);

        sequence += visitor.begin.expect_call();
        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_operator_identifier.expect_call();
        sequence += visitor.add_identifier.expect_call(operatorSymbol);
        sequence += visitor.end_operator_identifier.expect_call();

        sequence += visitor.begin_template_args.expect_call(0);
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When operator is scope.")
    {
        StringT const input = "foo::" + StringT{"operator"} + spacing + StringT{operatorSymbol} + "()::my_type";
        CAPTURE(input);

        sequence += visitor.begin.expect_call();
        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.begin_scope.expect_call();
        {
            sequence += visitor.begin_function.expect_call();

            sequence += visitor.begin_operator_identifier.expect_call();
            sequence += visitor.add_identifier.expect_call(operatorSymbol);
            sequence += visitor.end_operator_identifier.expect_call();

            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();

            sequence += visitor.end_function.expect_call();
        }
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_type();
    }
}

TEST_CASE(
    "parsing::NameParser detects conversion operators.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    SECTION("When converting to simple type-name.")
    {
        StringT const input = "operator bool()";
        CAPTURE(input);

        sequence += visitor.begin.expect_call();
        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_operator_identifier.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("bool");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_operator_identifier.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }

    SECTION("When converting to complex type-name.")
    {
        StringT const input = "operator volatile foo::bar()::my_type const&()";
        CAPTURE(input);

        sequence += visitor.begin.expect_call();
        sequence += visitor.begin_function.expect_call();

        sequence += visitor.begin_operator_identifier.expect_call();
        {
            sequence += visitor.begin_type.expect_call();

            sequence += visitor.begin_scope.expect_call();
            sequence += visitor.add_identifier.expect_call("foo");
            sequence += visitor.end_scope.expect_call();

            sequence += visitor.begin_scope.expect_call();
            sequence += visitor.begin_function.expect_call();
            sequence += visitor.add_identifier.expect_call("bar");
            sequence += visitor.begin_function_args.expect_call(0);
            sequence += visitor.end_function_args.expect_call();
            sequence += visitor.end_function.expect_call();
            sequence += visitor.end_scope.expect_call();

            sequence += visitor.add_identifier.expect_call("my_type");
            sequence += visitor.add_const.expect_call();
            sequence += visitor.add_volatile.expect_call();
            sequence += visitor.add_lvalue_ref.expect_call();

            sequence += visitor.end_type.expect_call();
        }
        sequence += visitor.end_operator_identifier.expect_call();

        sequence += visitor.begin_function_args.expect_call(0);
        sequence += visitor.end_function_args.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser.parse_function();
    }
}
