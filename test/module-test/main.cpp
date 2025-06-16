//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

import mimicpp;

#include <string>

#include "mimic++/MacroExports.hpp"

TEST_CASE("mimic++ supports modules!")
{
    mimicpp::Mock<int(std::string), void()> mock{};
    mimicpp::ScopedSequence sequence{};

    SCOPED_EXP mock.expect_call(mimicpp::matches::str::eq("Hello, mimic++!"))
        and mimicpp::expect::in_sequence(sequence)
        and mimicpp::expect::times(2)
        and mimicpp::expect::arg<0>(!mimicpp::matches::range::is_empty())
        and mimicpp::then::invoke([] {})
        and mimicpp::finally::returns(42);

    sequence += mock.expect_call("Test")
            and mimicpp::finally::throws(std::runtime_error{"Exception"});

    CHECK(42 == mock("Hello, mimic++!"));
    CHECK(42 == mock("Hello, mimic++!"));

    CHECK_THROWS(mock("Test"));
}

TEST_CASE("mimic++ interface mocks support modules!")
{
    class Interface
    {
    public:
        virtual ~Interface() = default;
        virtual void foo() = 0;
    };

    struct Derived
        : public Interface
    {
    public:
        MOCK_METHOD(foo, void, ());
    };

    auto mock = std::make_unique<Derived>();
    Interface& interface{*mock};

    SCOPED_EXP mock->foo_.expect_call();

    interface.foo();
}
