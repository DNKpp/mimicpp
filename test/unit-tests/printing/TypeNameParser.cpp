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

        Mock<void()> end_return_type{{.name = "VisitorMock::end_return_type"}};

        Mock<void()> open_parenthesis{{.name = "VisitorMock::open_parenthesis"}};
        Mock<void()> end_function{{.name = "VisitorMock::end_function"}};
        Mock<void()> end_function_ptr{{.name = "VisitorMock::end_function_ptr"}};

        Mock<void()> add_const{{.name = "VisitorMock::add_const"}};
        Mock<void()> add_volatile{{.name = "VisitorMock::add_volatile"}};
        Mock<void()> add_noexcept{{.name = "VisitorMock::add_noexcept"}};
        Mock<void()> add_ptr{{.name = "VisitorMock::add_ptr"}};
        Mock<void()> add_lvalue_ref{{.name = "VisitorMock::add_lvalue_ref"}};
        Mock<void()> add_rvalue_ref{{.name = "VisitorMock::add_rvalue_ref"}};

        Mock<void(StringViewT)> add_identifier{{.name = "VisitorMock::add_identifier"}};
        Mock<void()> add_scope{{.name = "VisitorMock::add_scope"}};
        Mock<void()> add_argument{{.name = "VisitorMock::add_argument"}};

        Mock<void()> begin_operator_identifier{{.name = "VisitorMock::begin_operator_identifier"}};
        Mock<void()> end_operator_identifier{{.name = "VisitorMock::end_operator_identifier"}};

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

        sequence += visitor.add_identifier.expect_call(input);
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

        sequence += visitor.add_identifier.expect_call(firstScope);
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(secondScope);
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(thirdScope);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When root scope is explicitly given.")
    {
        StringViewT const scope = GENERATE(from_range(identifiers));
        StringT const input = "::" + StringT{scope};
        CAPTURE(input);

        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(scope);
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

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.expect_spec_call(spec);
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

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.expect_spec_call(spec);
        sequence += visitor.expect_spec_call(indirection);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When given after the actual identifier.")
    {
        StringT const spec = GENERATE("const", "volatile");
        StringT const input{"foo " + spec};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.expect_spec_call(spec);
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

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.expect_spec_call(spec);
        sequence += visitor.expect_spec_call(indirection);
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("Coming all together")
    {
        StringT const input{"volatile foo const * const&"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
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

        sequence += visitor.add_identifier.expect_call("foo");
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

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");

        sequence += visitor.end_template.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When templated identifier with multiple args with specs is given.")
    {
        StringViewT const input{"foo<const int volatile&, const std::string>"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();

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
    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When function identifier with 0 args but without return type is given.")
    {
        StringViewT const input{"foo()"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function identifier with 0 args and return type is given.")
    {
        StringViewT const input{"void foo()"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function identifier with single arg and return type is given.")
    {
        StringViewT const input{"float foo(const std::string)"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("float");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.expect_spec_call("const");
        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function identifier with multiple args and return type is given.")
    {
        StringViewT const input{"float foo(const std::string&&, const int)"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("float");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_rvalue_ref.expect_call();
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_const.expect_call();

        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function identifier with return type and specs is given.")
    {
        StringT const spec = GENERATE("const", "volatile", "noexcept", "&", "&&");
        StringT const input = "float foo()" + spec;
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("float");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();

        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function identifier with more complex return type is given.")
    {
        StringT const input = "const std::string* volatile& foo()";
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function type is given.")
    {
        StringT const input = "const std::string* volatile& ()";
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When noexcept function pointer without arguments is given.")
    {
        StringT const input = "void (*)()noexcept";
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();

        sequence += visitor.add_noexcept.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function pointer-ref without arguments is given.")
    {
        StringT const input = "void (*&)()noexcept";
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();

        sequence += visitor.add_noexcept.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function pointer with arguments is given.")
    {
        StringT const input = "void (*)(const std::string&&, const int)";
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.open_parenthesis.expect_call();

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_rvalue_ref.expect_call();
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_const.expect_call();

        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When member function pointer without arguments is given.")
    {
        StringT const input = "void (foo::bar::*)()";
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When member function pointer with qualifications is given.")
    {
        StringT const spec = GENERATE("const", "volatile", "noexcept", "&", "&&");
        StringT const input = "void (foo::bar::*)()" + spec;
        CAPTURE(input, spec);

        sequence += visitor.add_identifier.expect_call("void");
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_ptr.expect_call();
        sequence += visitor.end_function_ptr.expect_call();

        sequence += visitor.open_parenthesis.expect_call();
        sequence += visitor.end_function.expect_call();
        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }
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
        sequence += visitor.add_identifier.expect_call(operatorText);
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
        sequence += visitor.add_identifier.expect_call(operatorText);
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

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.begin_operator_identifier.expect_call();
        sequence += visitor.add_identifier.expect_call(operatorText);
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
            sequence += visitor.add_identifier.expect_call(operatorText);
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
            sequence += visitor.add_identifier.expect_call(operatorText);
            sequence += visitor.end_operator_identifier.expect_call();

            sequence += visitor.begin_template.expect_call();
            sequence += visitor.add_identifier.expect_call("int");
            sequence += visitor.add_argument.expect_call();
            sequence += visitor.add_identifier.expect_call("float");
            sequence += visitor.end_template.expect_call();

            sequence += visitor.end.expect_call();

            printing::type::parsing::NameParser parser{std::ref(visitor), input};
            parser();
        }
    }
}

TEST_CASE(
    "parsing::NameParser handles arbitrarily scoped identifiers.",
    "[print][print::type]")
{
    VisitorMock visitor{};
    ScopedSequence sequence{};

    sequence += visitor.begin.expect_call();

    SECTION("When function local type is given.")
    {
        constexpr StringViewT input{"foo(std::string const&, int volatile) noexcept::my_type"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.open_parenthesis.expect_call();

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_volatile.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.add_noexcept.expect_call();

        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When qualified function local type is given.")
    {
        constexpr StringViewT input{"volatile foo(std::string const&, int volatile) noexcept::my_type const&"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.open_parenthesis.expect_call();

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_volatile.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.add_noexcept.expect_call();

        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("my_type");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When nested function local type is given.")
    {
        constexpr StringViewT input{"foo::bar(std::string const&, int volatile) noexcept::my_type"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");

        sequence += visitor.open_parenthesis.expect_call();

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_volatile.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.add_noexcept.expect_call();

        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When deeply nested function local type is given.")
    {
        constexpr StringViewT input{"foo(int volatile, std::string const&) const &&::bar(std::string const&, int volatile) noexcept::my_type"};
        CAPTURE(input);

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.open_parenthesis.expect_call();

        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_rvalue_ref.expect_call();

        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");

        sequence += visitor.open_parenthesis.expect_call();

        sequence += visitor.add_identifier.expect_call("std");
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("string");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.add_argument.expect_call();

        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.add_volatile.expect_call();

        sequence += visitor.end_function.expect_call();
        sequence += visitor.add_noexcept.expect_call();

        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("my_type");

        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }
}
