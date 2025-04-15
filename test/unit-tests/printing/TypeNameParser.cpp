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

        Mock<void(StringViewT)> add_identifier{{.name = "VisitorMock::add_identifier"}};
        Mock<void()> add_arg{{.name = "VisitorMock::add_arg"}};

        Mock<void()> begin_scope{{.name = "VisitorMock::begin_scope"}};
        Mock<void()> end_scope{{.name = "VisitorMock::end_scope"}};

        Mock<void()> begin_type{{.name = "VisitorMock::begin_type"}};
        Mock<void()> end_type{{.name = "VisitorMock::end_type"}};

        Mock<void()> begin_template_args{{.name = "VisitorMock::begin_template_args"}};
        Mock<void()> end_template_args{{.name = "VisitorMock::end_template_args"}};

        Mock<void()> begin_function{{.name = "VisitorMock::begin_function"}};
        Mock<void()> end_function{{.name = "VisitorMock::end_function"}};
        Mock<void()> begin_return_type{{.name = "VisitorMock::begin_return_type"}};
        Mock<void()> end_return_type{{.name = "VisitorMock::end_return_type"}};
        Mock<void()> begin_function_args{{.name = "VisitorMock::begin_function_args"}};
        Mock<void()> end_function_args{{.name = "VisitorMock::end_function_args"}};

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
        parser();
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
        parser();
    }

    SECTION("When root scope is explicitly given.")
    {
        StringViewT const scope = GENERATE(from_range(identifiers));
        StringT const input = "::" + StringT{scope};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call(scope);
        sequence += visitor.end_type.expect_call();

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

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.expect_spec_call(spec);

        sequence += visitor.end_type.expect_call();
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

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.expect_spec_call(spec);
        sequence += visitor.expect_spec_call(indirection);

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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
        parser();
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
        parser();
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

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call();
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When qualified templated identifier is given.")
    {
        StringViewT const input{"volatile foo<> const&"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call();
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When templated identifier with multiple args is given.")
    {
        StringViewT const input{"foo<int, std::string>"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call();

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
        parser();
    }

    SECTION("When templated identifier with multiple args with specs is given.")
    {
        StringViewT const input{"foo<const int volatile&, const std::string>"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call();

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
        parser();
    }

    SECTION("When templated identifier has templated arg.")
    {
        StringViewT const input{"foo<const bar<int*> volatile&>"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_template_args.expect_call();

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");

        {
            sequence += visitor.begin_template_args.expect_call();

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
        parser();
    }

    SECTION("When template is part of a scope.")
    {
        StringViewT const input{"foo<>::bar"};
        CAPTURE(input);

        sequence += visitor.begin_type.expect_call();

        sequence += visitor.begin_scope.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.begin_template_args.expect_call();
        sequence += visitor.end_template_args.expect_call();
        sequence += visitor.end_scope.expect_call();

        sequence += visitor.add_identifier.expect_call("bar");

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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
    sequence += visitor.begin_type.expect_call();
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

        sequence += visitor.begin_function_args.expect_call();
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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

        sequence += visitor.begin_template_args.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_template_args.expect_call();

        sequence += visitor.begin_function_args.expect_call();
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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

        sequence += visitor.begin_function_args.expect_call();

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
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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

        sequence += visitor.begin_function_args.expect_call();

        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("bar");

        sequence += visitor.begin_template_args.expect_call();
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
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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

        sequence += visitor.begin_function_args.expect_call();

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
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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
                sequence += visitor.begin_template_args.expect_call();
                sequence += visitor.end_template_args.expect_call();
            }

            sequence += visitor.begin_function_args.expect_call();
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

        sequence += visitor.begin_function_args.expect_call();
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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

        sequence += visitor.begin_function_args.expect_call();
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function identifier with templated return type is given.")
    {
        StringT const input = "bar<int>& foo()" + suffix;
        CAPTURE(input);

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();

        sequence += visitor.add_identifier.expect_call("bar");
        sequence += visitor.begin_template_args.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("int");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_template_args.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();

        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.add_identifier.expect_call("foo");

        sequence += visitor.begin_function_args.expect_call();
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
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
    sequence += visitor.begin_type.expect_call();
    sequence += visitor.begin_function.expect_call();

    SECTION("When function has an unqualified return-type.")
    {
        StringT const input = "foo ()" + suffix;
        CAPTURE(input);

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_args.expect_call();
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }

    SECTION("When function has a qualified return-type.")
    {
        StringT const input = "volatile foo const& ()" + suffix;
        CAPTURE(input);

        sequence += visitor.begin_return_type.expect_call();
        sequence += visitor.begin_type.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.add_const.expect_call();
        sequence += visitor.add_volatile.expect_call();
        sequence += visitor.add_lvalue_ref.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end_return_type.expect_call();

        sequence += visitor.begin_function_args.expect_call();
        sequence += visitor.end_function_args.expect_call();

        CHECKED_IF(!spec.empty())
        {
            sequence += visitor.expect_spec_call(spec);
        }

        sequence += visitor.end_function.expect_call();
        sequence += visitor.end_type.expect_call();
        sequence += visitor.end.expect_call();

        printing::type::parsing::NameParser parser{std::ref(visitor), input};
        parser();
    }
}
