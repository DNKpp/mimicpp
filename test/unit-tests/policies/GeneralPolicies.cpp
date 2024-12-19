// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/policies/GeneralPolicies.hpp"
#include "mimic++/Expectation.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::InitFinalize satisfies finalize_policy_for concept.",
    "[expectation][expectation::builder]",
    ((bool dummy, typename Policy, typename Sig), dummy, Policy, Sig),
    (true, expectation_policies::InitFinalize, void()),
    (true, expectation_policies::InitFinalize, void(int)))
{
    STATIC_REQUIRE(finalize_policy_for<Policy, Sig>);
}

TEMPLATE_TEST_CASE(
    "Given types satisfy expectation_policy_for concept.",
    "[expectation][expectation::builder]",
    expectation_policies::Category<ValueCategory::lvalue>,
    expectation_policies::Category<ValueCategory::rvalue>,
    expectation_policies::Category<ValueCategory::any>,
    expectation_policies::Constness<Constness::as_const>,
    expectation_policies::Constness<Constness::non_const>,
    expectation_policies::Constness<Constness::any>)
{
    STATIC_REQUIRE(expectation_policy_for<TestType, void()>);
}

TEST_CASE(
    "expectation_policies::InitFinalize does nothing.",
    "[expectation][expectation::builder]")
{
    const call::Info<void> call{
        .args = {},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    constexpr expectation_policies::InitFinalize policy{};

    REQUIRE_NOTHROW(policy.finalize_call(call));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::Category checks whether the given call::Info matches.",
    "[expectation][expectation::policy]",
    ((ValueCategory category), category),
    (ValueCategory::lvalue),
    (ValueCategory::rvalue),
    (ValueCategory::any))
{
    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    using PolicyT = expectation_policies::Category<category>;
    STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

    constexpr PolicyT policy{};
    SECTION("Policy is always satisfied.")
    {
        REQUIRE(policy.is_satisfied());
    }

    SECTION("Policy description.")
    {
        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals(format::format("expect: from {} category overload", category)));
    }

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    if (is_matching(call.fromCategory, category))
    {
        SECTION("When call and policy category matches, success is returned.")
        {
            REQUIRE(policy.matches(call));
        }

        SECTION("Policy doesn't consume, but asserts on wrong category.")
        {
            REQUIRE_NOTHROW(policy.consume(call));
        }
    }
    else
    {
        SECTION("When call and policy category mismatch, failure is returned.")
        {
            REQUIRE(!policy.matches(call));
        }
    }
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::Constness checks whether the given call::Info matches.",
    "[expectation][expectation::policy]",
    ((Constness constness), constness),
    (Constness::as_const),
    (Constness::non_const),
    (Constness::any))
{
    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    using PolicyT = expectation_policies::Constness<constness>;
    STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

    constexpr PolicyT policy{};
    SECTION("Policy is always satisfied.")
    {
        REQUIRE(policy.is_satisfied());
    }

    SECTION("Policy description.")
    {
        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals(format::format("expect: from {} qualified overload", constness)));
    }

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    if (is_matching(call.fromConstness, constness))
    {
        SECTION("When call and policy constness matches, success is returned.")
        {
            REQUIRE(policy.matches(call));
        }

        SECTION("Policy doesn't consume, but asserts on wrong constness.")
        {
            REQUIRE_NOTHROW(policy.consume(call));
        }
    }
    else
    {
        SECTION("When call and policy constness mismatch, failure is returned.")
        {
            REQUIRE(!policy.matches(call));
        }
    }
}
