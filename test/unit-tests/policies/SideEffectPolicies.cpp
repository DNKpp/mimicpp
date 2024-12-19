// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/policies/SideEffectPolicies.hpp"
#include "mimic++/Expectation.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

TEST_CASE(
    "Invalid configurations of expectation_policies::SideEffectAction do not satisfy expectation_policy_for concept.",
    "[expectation][expectation::policy]")
{
    using ActionT = InvocableMock<void, int>;

    SECTION("When action is not applicable.")
    {
        STATIC_REQUIRE(
            !expectation_policy_for<
                expectation_policies::SideEffectAction<ActionT>,
                void(int)>);
    }
}

TEST_CASE(
    "expectation_policies::SideEffectAction invokes the specified function on consume.",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;

    const call::Info<void> info{
        .args = {},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    InvocableMock<void, const call::Info<void>&> action{};
    expectation_policies::SideEffectAction policy{std::ref(action)};
    STATIC_REQUIRE(expectation_policy_for<decltype(policy), void()>);
    REQUIRE(std::as_const(policy).is_satisfied());
    REQUIRE(std::as_const(policy).matches(info));
    REQUIRE(std::optional<StringT>{} == std::as_const(policy).describe());

    REQUIRE_CALL(action, Invoke(_))
        .LR_WITH(&info == &_1);
    REQUIRE_NOTHROW(policy.consume(info));
}

TEST_CASE(
    "then::apply_arg creates expectation_policies::SideEffectAction.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    int param0{1337};
    double param1{4.2};
    std::string param2{"Hello, World!"};
    const call::Info<void, int&, double&, std::string&> info{
        .args = {param0, param1, param2}
    };

    SECTION("Index 0.")
    {
        InvocableMock<void, int&> action{};
        expectation_policies::SideEffectAction policy = then::apply_arg<0>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param0);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("Index 1.")
    {
        InvocableMock<void, const double&> action{};
        expectation_policies::SideEffectAction policy = then::apply_arg<1>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param1);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("Index 2.")
    {
        InvocableMock<void, std::string> action{};
        expectation_policies::SideEffectAction policy = then::apply_arg<2>(std::ref(action));
        REQUIRE_CALL(action, Invoke("Hello, World!"));
        REQUIRE_NOTHROW(policy.consume(info));
    }
}

TEST_CASE(
    "then::apply_args creates expectation_policies::SideEffectAction.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    int param0{1337};
    double param1{4.2};
    std::string param2{"Hello, World!"};
    const call::Info<void, int&, double&, std::string&> info{
        .args = {param0, param1, param2}
    };

    SECTION("Indices 0, 1, 2.")
    {
        InvocableMock<void, int&, double&, std::string&> action{};
        expectation_policies::SideEffectAction policy = then::apply_args<0, 1, 2>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_, _, _))
            .LR_WITH(&_1 == &param0)
            .LR_WITH(&_2 == &param1)
            .LR_WITH(&_3 == &param2);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("Indices 2, 1, 0")
    {
        InvocableMock<void, std::string&, double&, int&> action{};
        expectation_policies::SideEffectAction policy = then::apply_args<2, 1, 0>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_, _, _))
            .LR_WITH(&_1 == &param2)
            .LR_WITH(&_2 == &param1)
            .LR_WITH(&_3 == &param0);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("Arbitrarily mixed.")
    {
        InvocableMock<void, double&, int&, std::string&, std::string&, int&> action{};
        expectation_policies::SideEffectAction policy = then::apply_args<1, 0, 2, 2, 0>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_, _, _, _, _))
            .LR_WITH(&_1 == &param1)
            .LR_WITH(&_2 == &param0)
            .LR_WITH(&_3 == &param2)
            .LR_WITH(&_4 == &param2)
            .LR_WITH(&_5 == &param0);
        REQUIRE_NOTHROW(policy.consume(info));
    }
}

TEST_CASE(
    "then::apply_all creates expectation_policies::SideEffectAction.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    SECTION("When signature has zero params.")
    {
        const call::Info<void> info{
            .args = {}};

        InvocableMock<void> action{};
        expectation_policies::SideEffectAction policy = then::apply_all(std::ref(action));
        REQUIRE_CALL(action, Invoke());
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("When signature has one param.")
    {
        int param0{1337};
        const call::Info<void, int&> info{
            .args = {param0}};

        InvocableMock<void, int&> action{};
        expectation_policies::SideEffectAction policy = then::apply_all(std::ref(action));
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param0);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("When signature has multiple params.")
    {
        int param0{1337};
        double param1{4.2};
        const call::Info<void, int&, double&> info{
            .args = {param0, param1}
        };

        InvocableMock<void, int&, double&> action{};
        expectation_policies::SideEffectAction policy = then::apply_all(std::ref(action));
        REQUIRE_CALL(action, Invoke(_, _))
            .LR_WITH(&_1 == &param0)
            .LR_WITH(&_2 == &param1);
        REQUIRE_NOTHROW(policy.consume(info));
    }
}

TEST_CASE(
    "then::invoke creates expectation_policies::SideEffectAction.",
    "[expectation][expectation::factories]")
{
    int param0{1337};
    double param1{4.2};
    std::string param2{"Hello, World!"};
    const call::Info<void, int&, double&, std::string&> info{
        .args = {param0, param1, param2}
    };

    InvocableMock<void> action{};
    expectation_policies::SideEffectAction policy = then::invoke(std::ref(action));
    REQUIRE_CALL(action, Invoke());
    REQUIRE_NOTHROW(policy.consume(info));
}
