//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/expectation/Common.hpp"

#include "../TestTypes.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "mimicpp::expectation_policy_for determines, whether the type can be used in combination with the given signature.",
    "[expectation]",
    ((bool expected, typename Policy, typename Signature), expected, Policy, Signature),
    (false, PolicyFake<void()>, int()),     // incompatible return type
    (false, PolicyFake<void()>, void(int)), // incompatible param
    (true, PolicyFake<void()>, void()),
    (true, PolicyFacade<void(), std::reference_wrapper<PolicyFake<void()>>, UnwrapReferenceWrapper>, void()))
{
    STATIC_REQUIRE(expected == expectation::expectation_policy_for<Policy, Signature>);
}

TEMPLATE_TEST_CASE_SIG(
    "mimicpp::finalize_policy_for determines, whether the type can be used in combination with the given signature.",
    "[expectation]",
    ((bool expected, typename Policy, typename Signature), expected, Policy, Signature),
    (false, FinalizerFake<void()>, int()),     // incompatible return type
    (false, FinalizerFake<void()>, void(int)), // incompatible param
    (true, FinalizerFake<void()>, void()),
    (true, FinalizerFacade<void(), std::reference_wrapper<FinalizerFake<void()>>, UnwrapReferenceWrapper>, void()))
{
    STATIC_REQUIRE(expected == expectation::finalize_policy_for<Policy, Signature>);
}

TEMPLATE_TEST_CASE_SIG(
    "mimicpp::control_policy determines, whether the type satisfies the requirements.",
    "[expectation]",
    ((bool expected, typename Policy), expected, Policy),
    (true, ControlPolicyFake),
    (true, ControlPolicyFacade<std::reference_wrapper<ControlPolicyFake>, UnwrapReferenceWrapper>))
{
    STATIC_REQUIRE(expected == expectation::control_policy<Policy>);
}
