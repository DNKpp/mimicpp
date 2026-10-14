//          Copyright Dominic (DNKpp) Koepke 2024 - 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/expectation/policies/GeneralPolicies.hpp"
#include "mimic++/expectation/Common.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "expectation::policies::InitFinalize satisfies finalize_policy_for concept.",
    "[expectation][expectation::builder]",
    ((bool dummy, typename Policy, typename Sig), dummy, Policy, Sig),
    (true, expectation::policies::InitFinalize, void()),
    (true, expectation::policies::InitFinalize, void(int)))
{
    STATIC_REQUIRE(expectation::finalize_policy_for<Policy, Sig>);
}

TEMPLATE_TEST_CASE(
    "Given types satisfy expectation_policy_for concept.",
    "[expectation][expectation::builder]",
    expectation::policies::Category<ValueCategory::lvalue>,
    expectation::policies::Category<ValueCategory::rvalue>,
    expectation::policies::Category<ValueCategory::any>,
    expectation::policies::Constness<Constness::as_const>,
    expectation::policies::Constness<Constness::non_const>,
    expectation::policies::Constness<Constness::any>)
{
    STATIC_REQUIRE(expectation::expectation_policy_for<TestType, void()>);
}

TEST_CASE(
    "expectation::policies::InitFinalize does nothing.",
    "[expectation][expectation::builder]")
{
    call::Info<void> const call{
        .args = {},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    constexpr expectation::policies::InitFinalize policy{};

    REQUIRE_NOTHROW(policy.finalize_call(call));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation::policies::Category checks whether the given call::Info matches.",
    "[expectation][expectation::policy]",
    ((ValueCategory category), category),
    (ValueCategory::lvalue),
    (ValueCategory::rvalue),
    (ValueCategory::any))
{
    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    using PolicyT = expectation::policies::Category<category>;
    STATIC_REQUIRE(expectation::expectation_policy_for<PolicyT, SignatureT>);

    constexpr PolicyT policy{};
    SECTION("Policy is always satisfied.")
    {
        REQUIRE(policy.is_satisfied());
    }

    CallInfoT const call{
        .args = {},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers)),
    };

    bool const expectedMatchResult = mimicpp::detail::is_matching(call.fromCategory, category);
    auto const expectedDescription = ValueCategory::any != category
                                       ? StringT{"expect: from "} + print(category) + " category overload"
                                       : std::optional<StringT>{};

    CHECKED_IF(expectedMatchResult)
    {
        SECTION("When call and policy category matches, success is returned.")
        {
            CHECK_THAT(
                policy.matches(call),
                variant_equals(expectation::MatchSuccess{.description = expectedDescription}));
        }

        SECTION("Policy doesn't consume, but asserts on wrong category.")
        {
            CHECK_NOTHROW(policy.consume(call));
        }
    }
    CHECKED_ELSE(expectedMatchResult)
    {
        SECTION("When call and policy category mismatch, failure is returned.")
        {
            CHECK_THAT(
                policy.matches(call),
                variant_equals(expectation::MatchFailure{.description = expectedDescription}));
        }
    }
}

TEMPLATE_TEST_CASE_SIG(
    "expectation::policies::Constness checks whether the given call::Info matches.",
    "[expectation][expectation::policy]",
    ((Constness constness), constness),
    (Constness::as_const),
    (Constness::non_const),
    (Constness::any))
{
    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    using PolicyT = expectation::policies::Constness<constness>;
    STATIC_REQUIRE(expectation::expectation_policy_for<PolicyT, SignatureT>);

    constexpr PolicyT policy{};
    SECTION("Policy is always satisfied.")
    {
        REQUIRE(policy.is_satisfied());
    }

    CallInfoT const call{
        .args = {},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers)),
    };

    bool const expectedMatchResult = mimicpp::detail::is_matching(call.fromConstness, constness);
    auto const expectedDescription = Constness::any != constness
                                       ? StringT{"expect: from "} + print(constness) + " qualified overload"
                                       : std::optional<StringT>{};

    CHECKED_IF(expectedMatchResult)
    {
        SECTION("When call and policy constness matches, success is returned.")
        {
            CHECK_THAT(
                policy.matches(call),
                variant_equals(expectation::MatchSuccess{.description = expectedDescription}));
        }

        SECTION("Policy doesn't consume, but asserts on wrong constness.")
        {
            CHECK_NOTHROW(policy.consume(call));
        }
    }
    CHECKED_ELSE(expectedMatchResult)
    {
        SECTION("When call and policy constness mismatch, failure is returned.")
        {
            CHECK_THAT(
                policy.matches(call),
                variant_equals(expectation::MatchFailure{.description = expectedDescription}));
        }
    }
}
