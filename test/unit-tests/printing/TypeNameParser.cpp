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
        Mock<void()> add_scope{{.name = "VisitorMock::add_scope"}};

        Mock<void()> begin_name{{.name = "VisitorMock::begin_name"}};
        Mock<void()> end_name{{.name = "VisitorMock::end_name"}};

        Mock<void()> begin_type{{.name = "VisitorMock::begin_type"}};
        Mock<void()> end_type{{.name = "VisitorMock::end_type"}};

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
        sequence += visitor.begin_name.expect_call();
        sequence += visitor.add_identifier.expect_call(input);
        sequence += visitor.end_name.expect_call();
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
        sequence += visitor.begin_name.expect_call();

        sequence += visitor.add_identifier.expect_call(firstScope);
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(secondScope);
        sequence += visitor.add_scope.expect_call();
        sequence += visitor.add_identifier.expect_call(thirdScope);

        sequence += visitor.end_name.expect_call();
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
        sequence += visitor.begin_name.expect_call();
        sequence += visitor.add_identifier.expect_call(scope);
        sequence += visitor.end_name.expect_call();
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
        sequence += visitor.begin_name.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_name.expect_call();
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
        sequence += visitor.begin_name.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_name.expect_call();
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
        sequence += visitor.begin_name.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_name.expect_call();
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
        sequence += visitor.begin_name.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_name.expect_call();
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
        sequence += visitor.begin_name.expect_call();
        sequence += visitor.add_identifier.expect_call("foo");
        sequence += visitor.end_name.expect_call();

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
