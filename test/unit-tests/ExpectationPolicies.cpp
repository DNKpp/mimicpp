// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ExpectationPolicies.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "Given types satisfy mimicpp::finalize_policy_for concept.",
    "[expectation][expectation::builder]",
    ((bool dummy, typename Policy, typename Sig), dummy, Policy, Sig),
    (true, expectation_policies::InitFinalize, void()),
    (true, expectation_policies::InitFinalize, void(int)),
    (true, expectation_policies::Throws<std::runtime_error>, void()),
    (true, expectation_policies::Throws<std::runtime_error>, void(int)),
    (true, expectation_policies::Throws<std::runtime_error>, double(int)))
{
    STATIC_REQUIRE(mimicpp::finalize_policy_for<Policy, Sig>);
}

TEMPLATE_TEST_CASE(
    "Given types satisfy mimicpp::expectation_policy_for concept.",
    "[expectation][expectation::builder]",
    expectation_policies::Category<ValueCategory::lvalue>,
    expectation_policies::Category<ValueCategory::rvalue>,
    expectation_policies::Category<ValueCategory::any>,
    expectation_policies::Constness<Constness::as_const>,
    expectation_policies::Constness<Constness::non_const>,
    expectation_policies::Constness<Constness::any>)
{
    STATIC_REQUIRE(mimicpp::expectation_policy_for<TestType, void()>);
}

TEST_CASE(
    "expectation_policies::InitFinalize does nothing.",
    "[expectation][expectation::builder]")
{
    const call::Info<void> call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

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
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

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
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

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

TEST_CASE(
    "expectation_policies::ReturnsResultOf forwards the invocation result during finalize_call().",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;

    using SignatureT = int&();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    int value{42};
    InvocableMock<int&, const CallInfoT&> action{};
    expectation_policies::ReturnsResultOf policy{std::ref(action)};
    STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

    REQUIRE_CALL(action, Invoke(_))
        .LR_WITH(&call == &_1)
        .LR_RETURN(value);
    int& result = policy.finalize_call(call);
    REQUIRE(&value == &result);
}

TEST_CASE(
    "expectation_policies::Throws always throws an exception during finalize_call().",
    "[expectation][expectation::policy]")
{
    struct test_exception
    {
    };

    SECTION("When void is returned.")
    {
        using SignatureT = void();
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        using PolicyT = expectation_policies::Throws<test_exception>;
        STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        PolicyT policy{test_exception{}};

        REQUIRE_THROWS_AS(
            policy.finalize_call(call),
            test_exception);
    }

    SECTION("When non-void is returned.")
    {
        using SignatureT = int && ();
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        using PolicyT = expectation_policies::Throws<test_exception>;
        STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        PolicyT policy{test_exception{}};

        REQUIRE_THROWS_AS(
            policy.finalize_call(call),
            test_exception);
    }
}

TEST_CASE(
    "expectation_policies::Requirement checks whether the given call::Info matches.",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;
    namespace Matches = Catch::Matchers;

    using SignatureT = void(int);
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    int arg0{42};
    const CallInfoT info{
        .args = {arg0},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    using ProjectionT = InvocableMock<int&, const CallInfoT&>;
    using DescriberT = InvocableMock<StringT, StringViewT>;
    using MatcherT = MatcherMock<int&>;
    STATIC_CHECK(matcher_for<MatcherT, int&>);

    ProjectionT projection{};
    DescriberT describer{};
    MatcherT matcher{};

    expectation_policies::Requirement<
        MatcherFacade<std::reference_wrapper<MatcherT>, UnwrapReferenceWrapper>,
        std::reference_wrapper<ProjectionT>,
        std::reference_wrapper<DescriberT>>
        policy{
            MatcherFacade{std::ref(matcher), UnwrapReferenceWrapper{}},
            projection,
            describer
    };

    STATIC_REQUIRE(expectation_policy_for<decltype(policy), SignatureT>);

    REQUIRE(std::as_const(policy).is_satisfied());
    REQUIRE_NOTHROW(policy.consume(info));

    SECTION("Policy description.")
    {
        REQUIRE_CALL(matcher, describe())
            .RETURN("matcher description");
        REQUIRE_CALL(describer, Invoke("matcher description"))
            .RETURN("expect that: matcher description");

        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals("expect that: matcher description"));
    }

    SECTION("When matched.")
    {
        REQUIRE_CALL(projection, Invoke(_))
            .LR_WITH(&_1 == &info)
            .LR_RETURN(std::ref(arg0));
        REQUIRE_CALL(matcher, matches(_))
            .LR_WITH(&_1 == &arg0)
            .RETURN(true);

        REQUIRE(std::as_const(policy).matches(info));
    }

    SECTION("When not matched.")
    {
        REQUIRE_CALL(projection, Invoke(_))
            .LR_WITH(&_1 == &info)
            .LR_RETURN(std::ref(arg0));
        REQUIRE_CALL(matcher, matches(_))
            .LR_WITH(&_1 == &arg0)
            .RETURN(false);

        REQUIRE(!std::as_const(policy).matches(info));
    }
}

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
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

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
    "expectation_policies::ApplyArgsAction invokes the specified function on consume.",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;

    int param0{1337};
    double param1{4.2};
    const call::Info<void, int&, double&&> info{
        .args = {param0, param1},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
    };

    SECTION("When void is returned.")
    {
        SECTION("When specified in order.")
        {
            using ActionT = InvocableMock<void, int&, double&&>;
            ActionT action{};
            expectation_policies::ApplyArgsAction<
                std::reference_wrapper<ActionT>,
                std::type_identity_t,
                0u,
                1u>
                applyAction{
                    std::ref(action)};

            REQUIRE_CALL(action, Invoke(_, _))
                .LR_WITH(&param0 == &_1)
                .LR_WITH(&param1 == &_2);
            REQUIRE_NOTHROW(applyAction(info));
        }

        SECTION("In reverse order.")
        {
            using ActionT = InvocableMock<void, double&&, int&>;
            ActionT action{};
            expectation_policies::ApplyArgsAction<
                std::reference_wrapper<ActionT>,
                std::type_identity_t,
                1u,
                0u>
                applyAction{
                    std::ref(action)};

            REQUIRE_CALL(action, Invoke(_, _))
                .LR_WITH(&param0 == &_2)
                .LR_WITH(&param1 == &_1);
            REQUIRE_NOTHROW(applyAction(info));
        }

        SECTION("Just the first.")
        {
            using ActionT = InvocableMock<void, int&>;
            ActionT action{};
            expectation_policies::ApplyArgsAction<
                std::reference_wrapper<ActionT>,
                std::type_identity_t,
                0u>
                applyAction{
                    std::ref(action)};

            REQUIRE_CALL(action, Invoke(_))
                .LR_WITH(&param0 == &_1);
            REQUIRE_NOTHROW(applyAction(info));
        }

        SECTION("Just the second.")
        {
            using ActionT = InvocableMock<void, double&&>;
            ActionT action{};
            expectation_policies::ApplyArgsAction<
                std::reference_wrapper<ActionT>,
                std::type_identity_t,
                1u>
                applyAction{
                    std::ref(action)};

            REQUIRE_CALL(action, Invoke(_))
                .LR_WITH(&param1 == &_1);
            REQUIRE_NOTHROW(applyAction(info));
        }

        SECTION("Arbitrarily mixed.")
        {
            using ActionT = InvocableMock<void, double&&, int&, int&, double&&>;
            ActionT action{};
            expectation_policies::ApplyArgsAction<
                std::reference_wrapper<ActionT>,
                std::type_identity_t,
                1u,
                0u,
                0u,
                1u>
                applyAction{
                    std::ref(action)};

            REQUIRE_CALL(action, Invoke(_, _, _, _))
                .LR_WITH(&_1 == &param1)
                .LR_WITH(&_2 == &param0)
                .LR_WITH(&_3 == &param0)
                .LR_WITH(&_4 == &param1);
            REQUIRE_NOTHROW(applyAction(info));
        }
    }

    SECTION("When non-void is returned.")
    {
        using ActionT = InvocableMock<int&, double&&>;
        ActionT action{};
        expectation_policies::ApplyArgsAction<
            std::reference_wrapper<ActionT>,
            std::type_identity_t,
            1u>
            applyAction{
                std::ref(action)};

        int value{42};
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param1)
            .LR_RETURN(std::ref(value));
        REQUIRE(&value == &applyAction(info));
    }

    SECTION("When different projection is applied.")
    {
        using ActionT = InvocableMock<int&, double&>;
        ActionT action{};
        expectation_policies::ApplyArgsAction<
            std::reference_wrapper<ActionT>,
            std::add_lvalue_reference_t,
            1u>
            applyAction{
                std::ref(action)};

        int value{42};
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param1)
            .LR_RETURN(std::ref(value));
        REQUIRE(&value == &applyAction(info));
    }
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::ApplyArgsAction takes the param category into account.",
    "[expectation][expectation::policy]",
    ((bool expectSameAddress, typename ActionParam, typename SigParam), expectSameAddress, ActionParam, SigParam),
    (false, int, int),
    (false, int, int&),
    (false, int, const int&),
    (false, int, int&&),
    (false, int, const int&&),

    (true, int&, int&),

    (false, const int&, int),
    (true, const int&, int&),
    (true, const int&, const int&),
    (true, const int&, int&&),
    (true, const int&, const int&&))
{
    using trompeloeil::_;

    int param0{1337};
    const call::Info<void, SigParam> info{
        .args = {param0},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    InvocableMock<void, ActionParam> action{};
    expectation_policies::ApplyArgsAction<
        std::reference_wrapper<InvocableMock<void, ActionParam>>,
        std::type_identity_t,
        0u>
        policy{
            std::ref(action)};

    REQUIRE_CALL(action, Invoke(param0))
        .LR_WITH(expectSameAddress == (&_1 == &param0));
    REQUIRE_NOTHROW(policy(info));
}

TEST_CASE(
    "expectation_policies::ApplyArgsAction invokes the specified function with all params.",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;

    SECTION("Signatures without params.")
    {
        const call::Info<void> info{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("When void is returned.")
        {
            InvocableMock<void> action{};
            expectation_policies::ApplyAllArgsAction<
                std::reference_wrapper<InvocableMock<void>>,
                std::type_identity_t>
                applyAction{
                    std::ref(action)};
            REQUIRE_CALL(action, Invoke());
            REQUIRE_NOTHROW(applyAction(info));
        }

        SECTION("When non-void is returned.")
        {
            InvocableMock<int&> action{};
            expectation_policies::ApplyAllArgsAction<
                std::reference_wrapper<InvocableMock<int&>>,
                std::type_identity_t>
                applyAction{
                    std::ref(action)};

            int value{42};
            REQUIRE_CALL(action, Invoke())
                .LR_RETURN(std::ref(value));
            REQUIRE(&value == &applyAction(info));
        }
    }

    SECTION("Unary signatures.")
    {
        int param0{1337};
        const call::Info<void, int&&> info{
            .args = {param0},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("When void is returned.")
        {
            InvocableMock<void, int&&> action{};
            expectation_policies::ApplyAllArgsAction<
                std::reference_wrapper<InvocableMock<void, int&&>>,
                std::type_identity_t>
                applyAction{
                    std::ref(action)};
            REQUIRE_CALL(action, Invoke(_))
                .LR_WITH(&_1 == &param0);
            REQUIRE_NOTHROW(applyAction(info));
        }

        SECTION("When non-void is returned.")
        {
            InvocableMock<int&, int&&> action{};
            expectation_policies::ApplyAllArgsAction<
                std::reference_wrapper<InvocableMock<int&, int&&>>,
                std::type_identity_t>
                applyAction{
                    std::ref(action)};

            int value{42};
            REQUIRE_CALL(action, Invoke(_))
                .LR_WITH(&_1 == &param0)
                .LR_RETURN(std::ref(value));
            REQUIRE(&value == &applyAction(info));
        }

        SECTION("When other projection is applied.")
        {
            InvocableMock<int&, int&> action{};
            expectation_policies::ApplyAllArgsAction<
                std::reference_wrapper<InvocableMock<int&, int&>>,
                std::add_lvalue_reference_t>
                applyAction{
                    std::ref(action)};

            int value{42};
            REQUIRE_CALL(action, Invoke(_))
                .LR_WITH(&_1 == &param0)
                .LR_RETURN(std::ref(value));
            REQUIRE(&value == &applyAction(info));
        }
    }

    SECTION("With arbitrary signatures.")
    {
        int param0{1337};
        double param1{4.2};
        std::string param2{"Hello, World!"};
        const call::Info<void, int&, double&, std::string&&> info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        SECTION("When void is returned.")
        {
            using ActionT = InvocableMock<void, int&, double&, std::string&&>;
            ActionT action{};
            expectation_policies::ApplyAllArgsAction<
                std::reference_wrapper<ActionT>,
                std::type_identity_t>
                applyAction{
                    std::ref(action)};

            REQUIRE_CALL(action, Invoke(_, _, _))
                .LR_WITH(&_1 == &param0)
                .LR_WITH(&_2 == &param1)
                .LR_WITH(&_3 == &param2);
            REQUIRE_NOTHROW(applyAction(info));
        }

        SECTION("When non-void is returned.")
        {
            using ActionT = InvocableMock<int&, int&, double&, std::string&&>;
            ActionT action{};
            expectation_policies::ApplyAllArgsAction<
                std::reference_wrapper<ActionT>,
                std::type_identity_t>
                applyAction{
                    std::ref(action)};

            int value{42};
            REQUIRE_CALL(action, Invoke(_, _, _))
                .LR_WITH(&_1 == &param0)
                .LR_WITH(&_2 == &param1)
                .LR_WITH(&_3 == &param2)
                .LR_RETURN(std::ref(value));
            REQUIRE(&value == &applyAction(info));
        }

        SECTION("When other projection is applied.")
        {
            using ActionT = InvocableMock<int&, int&, double&, std::string&>;
            ActionT action{};
            expectation_policies::ApplyAllArgsAction<
                std::reference_wrapper<ActionT>,
                std::add_lvalue_reference_t>
                applyAction{
                    std::ref(action)};

            int value{42};
            REQUIRE_CALL(action, Invoke(_, _, _))
                .LR_WITH(&_1 == &param0)
                .LR_WITH(&_2 == &param1)
                .LR_WITH(&_3 == &param2)
                .LR_RETURN(std::ref(value));
            REQUIRE(&value == &applyAction(info));
        }
    }
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::ApplyAllArgsAction takes the param category into account.",
    "[expectation][expectation::policy]",
    ((bool expectSameAddress, typename ActionParam, typename SigParam), expectSameAddress, ActionParam, SigParam),
    (false, int, int),
    (false, int, int&),
    (false, int, const int&),
    (false, int, int&&),
    (false, int, const int&&),

    (true, int&, int&),

    (false, const int&, int),
    (true, const int&, int&),
    (true, const int&, const int&),
    (true, const int&, int&&),
    (true, const int&, const int&&))
{
    using trompeloeil::_;

    int param0{1337};
    const call::Info<void, SigParam> info{
        .args = {param0},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    InvocableMock<void, ActionParam> action{};
    expectation_policies::ApplyAllArgsAction<
        std::reference_wrapper<InvocableMock<void, ActionParam>>,
        std::type_identity_t>
        policy{
            std::ref(action)};

    REQUIRE_CALL(action, Invoke(param0))
        .LR_WITH(expectSameAddress == (&_1 == &param0));
    REQUIRE_NOTHROW(policy(info));
}

TEST_CASE(
    "then::apply_arg creates expectation_policies::ParamsSideEffect.",
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
    "then::apply_args creates expectation_policies::ParamsSideEffect.",
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
    "then::apply_all creates expectation_policies::AllParamsSideEffect.",
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
    "then::invoke creates expectation_policies::ParamsSideEffect.",
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

TEST_CASE(
    "expect::arg creates an expectation_policies::Requirement policy.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;
    namespace Matches = Catch::Matchers;

    using SignatureT = void(int);
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    int arg0{42};
    const CallInfoT info{
        .args = {arg0},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    using MatcherT = MatcherMock<int&>;
    STATIC_CHECK(matcher_for<MatcherT, int&>);
    MatcherT matcher{};

    expectation_policies::Requirement policy = expect::arg<0>(
        MatcherFacade{
            std::ref(matcher),
            UnwrapReferenceWrapper{}});
    STATIC_REQUIRE(expectation_policy_for<decltype(policy), SignatureT>);

    REQUIRE(std::as_const(policy).is_satisfied());
    REQUIRE_NOTHROW(policy.consume(info));

    SECTION("Policy description.")
    {
        REQUIRE_CALL(matcher, describe())
            .RETURN("matcher description");

        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals("expect: arg[0] matcher description"));
    }

    SECTION("Policy matches().")
    {
        const bool match = GENERATE(true, false);
        REQUIRE_CALL(matcher, matches(_))
            .LR_WITH(&_1 == &arg0)
            .RETURN(match);

        REQUIRE(match == std::as_const(policy).matches(info));
    }
}

TEST_CASE(
    "expect::arg supports an optional projection.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;
    namespace Matches = Catch::Matchers;

    using SignatureT = void(int);
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    int arg0{42};
    const CallInfoT info{
        .args = {arg0},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    using MatcherT = MatcherMock<const std::string&>;
    STATIC_CHECK(matcher_for<MatcherT, std::string>);
    MatcherT matcher{};
    using ProjectionT = InvocableMock<std::string, int&>;

    ProjectionT projection{};

    expectation_policies::Requirement policy = expect::arg<0>(
        MatcherFacade{
            std::ref(matcher),
            UnwrapReferenceWrapper{}},
        std::ref(projection));
    STATIC_REQUIRE(expectation_policy_for<decltype(policy), SignatureT>);

    REQUIRE_CALL(projection, Invoke(_))
        .LR_WITH(&_1 == &arg0)
        .RETURN("42");

    const bool match = GENERATE(true, false);
    REQUIRE_CALL(matcher, matches(_))
        .LR_WITH(_1 == "42")
        .RETURN(match);

    REQUIRE(match == std::as_const(policy).matches(info));
}

TEST_CASE(
    "mimicpp::expect::returns_result_of creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    using SignatureT = int&();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    InvocableMock<int&> action{};
    expectation_policies::ReturnsResultOf policy = finally::returns_result_of(std::ref(action));
    STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

    int value{42};
    REQUIRE_CALL(action, Invoke())
        .LR_RETURN(value);
    int& result = policy.finalize_call(call);
    REQUIRE(&value == &result);
}

TEST_CASE(
    "mimicpp::expect::returns creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    SECTION("When a value is returned.")
    {
        using CallInfoT = call::Info<int>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        const int value = GENERATE(range(0, 5));

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(value);
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), int()>);

            REQUIRE(value == policy.finalize_call(call));
            REQUIRE(value == policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), int()>);

            REQUIRE(value == policy.finalize_call(call));
            REQUIRE(value == policy.finalize_call(call));
        }
    }

    SECTION("When a lvalue ref is returned.")
    {
        using CallInfoT = call::Info<int&>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(42);
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&()>);

            int& result = policy.finalize_call(call);
            REQUIRE(42 == result);
            REQUIRE(&result == &policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            int value{42};
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&()>);

            REQUIRE(&value == &policy.finalize_call(call));
            REQUIRE(&value == &policy.finalize_call(call));
        }
    }

    SECTION("When a const lvalue ref is returned.")
    {
        using CallInfoT = call::Info<const int&>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(42);
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), const int&()>);

            const int& result = policy.finalize_call(call);
            REQUIRE(42 == result);
            REQUIRE(&result == &policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            constexpr int value{42};
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), const int&()>);

            REQUIRE(&value == &policy.finalize_call(call));
            REQUIRE(&value == &policy.finalize_call(call));
        }
    }

    SECTION("When a rvalue ref is returned.")
    {
        using CallInfoT = call::Info<int&&>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(42);
            STATIC_REQUIRE(finalize_policy_for < decltype(policy), int && () >);

            int&& result = policy.finalize_call(call);
            REQUIRE(42 == result);
            REQUIRE(42 == policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            int value{42};
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for < decltype(policy), int && () >);

            int&& result = policy.finalize_call(call);
            REQUIRE(&value == std::addressof(result));
            int&& secondResult = policy.finalize_call(call);
            REQUIRE(&value == std::addressof(secondResult));
        }
    }

    SECTION("When a const rvalue ref is returned.")
    {
        using CallInfoT = call::Info<const int&&>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(42);
            STATIC_REQUIRE(finalize_policy_for < decltype(policy), const int && () >);

            const int&& result = policy.finalize_call(call);
            REQUIRE(42 == result);
            REQUIRE(42 == policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            int value{42};
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for < decltype(policy), const int && () >);

            const int&& result = policy.finalize_call(call);
            REQUIRE(&value == std::addressof(result));
            const int&& secondResult = policy.finalize_call(call);
            REQUIRE(&value == std::addressof(secondResult));
        }
    }
}

TEST_CASE(
    "mimicpp::expect::returns_apply_result_of creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    SECTION("When a single index is chosen.")
    {
        using SignatureT = std::string && (int, double, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;

        int param0{1337};
        double param1{4.2};
        std::string param2{"Hello, World!"};
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_apply_result_of<2>(
            [](std::string& str) noexcept -> std::string&& { return std::move(str); });
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        std::string&& result = policy.finalize_call(info);
        REQUIRE(&param2 == std::addressof(result));
    }

    SECTION("When an arbitrary index sequence is chosen")
    {
        using SignatureT = std::tuple<double, std::string&&, int, double&>(int, double, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;

        int param0{1337};
        double param1{4.2};
        std::string param2{"Hello, World!"};
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_apply_result_of<1, 2, 0, 1>(
            [](double& p0, std::string& p1, int& p2, double& p3) {
                return std::tuple<double, std::string&&, int, double&>{
                    p0,
                    std::move(p1),
                    p2,
                    p3};
            });
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        auto&& [r0, r1, r2, r3] = policy.finalize_call(info);
        REQUIRE(r0 == param1);
        REQUIRE(std::addressof(r1) == &param2);
        REQUIRE(r2 == param0);
        REQUIRE(&r3 == &param1);
    }
}

TEST_CASE(
    "expect::returns_apply_all_result_of creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    SECTION("When signature has zero params.")
    {
        const call::Info<int&> info{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        InvocableMock<int&> action{};
        expectation_policies::ReturnsResultOf policy = finally::returns_apply_all_result_of(std::ref(action));
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&()>);

        int value{42};
        REQUIRE_CALL(action, Invoke())
            .LR_RETURN(std::ref(value));
        REQUIRE(&value == &policy.finalize_call(info));
    }

    SECTION("When signature has one param.")
    {
        int param0{1337};
        const call::Info<int&, int&> info{
            .args = {std::ref(param0)},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        InvocableMock<int&, int&> action{};
        expectation_policies::ReturnsResultOf policy = finally::returns_apply_all_result_of(std::ref(action));
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&(int&)>);

        int value{42};
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param0)
            .LR_RETURN(std::ref(value));
        REQUIRE(&value == &policy.finalize_call(info));
    }

    SECTION("When signature has multiple params.")
    {
        int param0{1337};
        double param1{4.2};
        const call::Info<int&, int&, double&> info{
            .args = {param0, param1},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        InvocableMock<int&, int&, double&> action{};
        expectation_policies::ReturnsResultOf policy = finally::returns_apply_all_result_of(std::ref(action));
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&(int&, double&)>);

        int value{42};
        REQUIRE_CALL(action, Invoke(_, _))
            .LR_WITH(&_1 == &param0)
            .LR_WITH(&_2 == &param1)
            .LR_RETURN(std::ref(value));
        REQUIRE(&value == &policy.finalize_call(info));
    }
}

TEST_CASE(
    "mimicpp::expect::returns_arg creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    int param0{1337};
    double param1{4.2};
    std::string param2{"Hello, World!"};

    SECTION("When value is selected.")
    {
        using SignatureT = int(int, double&, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<0>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(1337 == policy.finalize_call(info));
    }

    SECTION("When lvalue ref is selected.")
    {
        using SignatureT = double&(int, double&, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<1>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(&param1 == &policy.finalize_call(info));
    }

    SECTION("When const lvalue ref is selected.")
    {
        using SignatureT = const double&(int, const double&, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<1>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(&param1 == &policy.finalize_call(info));
    }

    SECTION("When rvalue ref is selected.")
    {
        using SignatureT = std::string && (int, double&, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<2>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        std::string&& result = policy.finalize_call(info);
        REQUIRE(&param2 == std::addressof(result));
    }

    SECTION("When const rvalue ref is selected.")
    {
        using SignatureT = const std::string && (int, double&, const std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<2>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        const std::string&& result = policy.finalize_call(info);
        REQUIRE(&param2 == std::addressof(result));
    }
}

TEST_CASE(
    "mimicpp::expect::throws creates expectation_policies::Throws.",
    "[expectation][expectation::factories]")
{
    using CallInfoT = call::Info<int>;

    const int value = GENERATE(range(0, 5));
    expectation_policies::Throws policy = finally::throws(value);

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    REQUIRE_THROWS_AS(
        policy.finalize_call(call),
        int);
    REQUIRE_THROWS_AS(
        policy.finalize_call(call),
        int);
}
