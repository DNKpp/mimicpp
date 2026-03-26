//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/expectation/Expectation.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

namespace
{
    inline reporting::control_state_t const commonApplicableState = reporting::state_applicable{0, 1, 0};
    inline reporting::control_state_t const commonUnsatisfiedState = reporting::state_applicable{1, 1, 0};
    inline reporting::control_state_t const commonInapplicableState = reporting::state_inapplicable{0, 1, 0, {}, {{sequence::Tag{1337}}}};
    inline reporting::control_state_t const commonSaturatedState = reporting::state_saturated{0, 1, 1};

    template <typename Signature>
    [[nodiscard]]
    reporting::TargetReport make_common_target_report()
    {
        return reporting::TargetReport{
            .name = "Mock-Name",
            .overloadReport = reporting::TypeReport::make<Signature>()};
    }
}

TEST_CASE(
    "expectation::Expectation stores general infos.",
    "[expectation]")
{
    using ControlPolicy = ControlPolicyFake;
    using FinalizePolicy = FinalizerFake<void()>;

    util::SourceLocation const from{};
    reporting::TargetReport const target = make_common_target_report<void()>();
    expectation::Expectation const expectation{
        std::in_place_type<void()>,
        from,
        target,
        ControlPolicy{},
        FinalizePolicy{}};

    CHECK(from == expectation.from());
    CHECK_THAT(
        expectation.mock_name(),
        Catch::Matchers::Equals(target.name));
}

TEST_CASE(
    "Control policy of expectation::Expectation controls, how often its expectations must match.",
    "[expectation]")
{
    using trompeloeil::_;
    using Signature = void();
    using FinalizePolicy = FinalizerFacade<Signature, std::reference_wrapper<FinalizerMock<Signature>>, UnwrapReferenceWrapper>;
    using PolicyMock = PolicyMock<Signature>;
    using PolicyRef = PolicyFacade<Signature, std::reference_wrapper<PolicyMock>, UnwrapReferenceWrapper>;
    using ControlPolicy = ControlPolicyFacade<std::reference_wrapper<ControlPolicyMock>, UnwrapReferenceWrapper>;

    call::info_for_signature_t<Signature> call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    ControlPolicyMock times{};
    FinalizerMock<Signature> finalize{};

    SECTION("expectation_infos are gathered.")
    {
        reporting::TargetReport const target = make_common_target_report<void()>();

        expectation::Expectation exp{
            std::in_place_type<Signature>,
            {},
            target,
            ControlPolicyFake{},
            FinalizePolicy{std::ref(finalize)}};

        reporting::ExpectationReport const report = exp.report();
        CHECK(target == report.target);
    }

    SECTION("With no other expectation policies.")
    {
        expectation::Expectation exp{
            std::in_place_type<Signature>,
            {},
            make_common_target_report<Signature>(),
            ControlPolicy{std::ref(times)},
            FinalizePolicy{std::ref(finalize)}};

        SECTION("When calling is_satisfied.")
        {
            bool const isSatisfied = GENERATE(false, true);
            REQUIRE_CALL(times, is_satisfied())
                .RETURN(isSatisfied);
            CHECK(isSatisfied == std::as_const(exp).is_satisfied());
        }

        SECTION("When calling is_applicable.")
        {
            auto const [expected, state] = GENERATE(
                table<bool, reporting::control_state_t>({
                    { true,   commonApplicableState},
                    { true,  commonUnsatisfiedState},
                    {false, commonInapplicableState},
                    {false,    commonSaturatedState}
            }));
            REQUIRE_CALL(times, state())
                .RETURN(state);
            CHECK(expected == std::as_const(exp).is_applicable());
        }

        SECTION("When times is applicable, call is matched.")
        {
            REQUIRE_CALL(times, state())
                .RETURN(commonApplicableState);
            reporting::RequirementOutcomes const outcomes = std::as_const(exp).matches(call).value();
            CHECK_THAT(
                outcomes.outcomes,
                Catch::Matchers::IsEmpty());
            reporting::ExpectationReport const expectationReport = std::as_const(exp).report();
            CHECK(commonApplicableState == expectationReport.controlReport);
        }

        SECTION("When times is not applicable => inapplicable.")
        {
            auto const controlState = GENERATE(commonSaturatedState, commonInapplicableState);
            REQUIRE_CALL(times, state())
                .LR_RETURN(controlState);
            reporting::RequirementOutcomes const outcomes = std::as_const(exp).matches(call).value();
            CHECK_THAT(
                outcomes.outcomes,
                Catch::Matchers::IsEmpty());
            reporting::ExpectationReport const expectationReport = std::as_const(exp).report();
            CHECK(controlState == expectationReport.controlReport);
        }

        SECTION("Consume calls times.consume().")
        {
            REQUIRE_CALL(times, consume());
            REQUIRE_CALL(finalize, finalize_call(_))
                .LR_WITH(&_1 == &call);
            CHECK_NOTHROW(exp.consume(call));
        }
    }

    SECTION("With other expectation policies.")
    {
        PolicyMock policy{};
        expectation::Expectation exp{
            std::in_place_type<Signature>,
            {},
            make_common_target_report<Signature>(),
            ControlPolicy{std::ref(times)},
            FinalizePolicy{std::ref(finalize)},
            PolicyRef{std::ref(policy)}};

        SECTION("When calling is_applicable.")
        {
            auto const [expected, state] = GENERATE(
                table<bool, reporting::control_state_t>({
                    { true,   commonApplicableState},
                    { true,  commonUnsatisfiedState},
                    {false, commonInapplicableState},
                    {false,    commonSaturatedState}
            }));
            REQUIRE_CALL(times, state())
                .RETURN(state);
            CHECK(expected == std::as_const(exp).is_applicable());
        }

        SECTION("When times is not satisfied.")
        {
            REQUIRE_CALL(times, is_satisfied())
                .RETURN(false);
            CHECK(!std::as_const(exp).is_satisfied());
        }

        SECTION("When times is satisfied, result depends on expectation policy.")
        {
            REQUIRE_CALL(times, is_satisfied())
                .RETURN(true);
            bool const isSatisfied = GENERATE(false, true);
            REQUIRE_CALL(policy, is_satisfied())
                .RETURN(isSatisfied);
            CHECK(isSatisfied == std::as_const(exp).is_satisfied());
        }

        SECTION("When matches is called, the requirement policy is queried.")
        {
            bool const matches = GENERATE(false, true);
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(matches);
            reporting::RequirementOutcomes const outcomes = std::as_const(exp).matches(call).value();
            CHECK_THAT(
                outcomes.outcomes,
                Catch::Matchers::RangeEquals(std::array{matches}));
        }

        SECTION("Consume calls times.consume().")
        {
            REQUIRE_CALL(times, consume());
            REQUIRE_CALL(finalize, finalize_call(_))
                .LR_WITH(&_1 == &call);
            REQUIRE_CALL(policy, consume(_))
                .LR_WITH(&_1 == &call);
            CHECK_NOTHROW(exp.consume(call));
        }
    }
}

TEST_CASE(
    "expectation::Expectation::report gathers information about the expectation.",
    "[expectation]")
{
    using Signature = void();
    using FinalizerPolicy = FinalizerFake<Signature>;

    SECTION("SourceLocation is gathered.")
    {
        util::SourceLocation const loc{};

        expectation::Expectation exp{
            std::in_place_type<Signature>,
            loc,
            make_common_target_report<Signature>(),
            ControlPolicyFake{},
            FinalizerPolicy{}};

        reporting::ExpectationReport const report = exp.report();
        CHECK(loc == report.from);
    }

    SECTION("reporting::TargetReport is gathered.")
    {
        reporting::TargetReport const target = make_common_target_report<Signature>();

        expectation::Expectation const exp{
            std::in_place_type<Signature>,
            {},
            target,
            ControlPolicyFake{},
            FinalizerPolicy{}};

        reporting::ExpectationReport const report = exp.report();
        CHECK(target == report.target);
    }

    SECTION("Finalizer policy has no description.")
    {
        // Todo:
    }

    SECTION("Times policy is queried.")
    {
        using Control = ControlPolicyFacade<std::reference_wrapper<ControlPolicyMock>, UnwrapReferenceWrapper>;

        ControlPolicyMock controlPolicy{};
        expectation::Expectation const exp{
            std::in_place_type<Signature>,
            {},
            make_common_target_report<Signature>(),
            Control{std::ref(controlPolicy)},
            FinalizerPolicy{}};

        auto const state = GENERATE(
            commonApplicableState,
            commonUnsatisfiedState,
            commonInapplicableState,
            commonSaturatedState);
        REQUIRE_CALL(controlPolicy, state())
            .RETURN(state);

        reporting::ExpectationReport const report = exp.report();
        CHECK(report.controlReport == state);
    }

    SECTION("Expectation policies are queried.")
    {
        using Policy = PolicyFacade<
            Signature,
            std::reference_wrapper<PolicyMock<Signature>>,
            UnwrapReferenceWrapper>;

        PolicyMock<void()> policy{};
        expectation::Expectation exp{
            std::in_place_type<Signature>,
            {},
            make_common_target_report<Signature>(),
            ControlPolicyFake{},
            FinalizerPolicy{},
            Policy{std::ref(policy)}};

        REQUIRE_CALL(policy, describe())
            .RETURN("expectation description");

        reporting::ExpectationReport const report = exp.report();
        CHECK_THAT(
            report.requirementDescriptions,
            Catch::Matchers::SizeIs(1));
        CHECK(report.requirementDescriptions.front());
        CHECK_THAT(
            *report.requirementDescriptions.front(),
            Catch::Matchers::Equals("expectation description"));
    }

    SECTION("Expectation policies without a description are supported.")
    {
        struct Policy
        {
            [[nodiscard]]
            static constexpr bool is_satisfied() noexcept
            {
                return true;
            }

            [[nodiscard]]
            static constexpr bool matches([[maybe_unused]] call::info_for_signature_t<Signature> const& call) noexcept
            {
                return true;
            }

            [[nodiscard]]
            static constexpr std::nullopt_t describe() noexcept
            {
                return std::nullopt;
            }

            static constexpr void consume([[maybe_unused]] call::info_for_signature_t<Signature> const& call) noexcept
            {
            }
        };

        static_assert(expectation::expectation_policy_for<Policy, Signature>);

        expectation::Expectation const exp{
            std::in_place_type<Signature>,
            {},
            make_common_target_report<Signature>(),
            ControlPolicyFake{},
            FinalizerPolicy{},
            Policy{}};

        reporting::ExpectationReport const report = exp.report();
        CHECK_THAT(
            report.requirementDescriptions,
            Catch::Matchers::SizeIs(1));
        CHECK_FALSE(report.requirementDescriptions.front());
    }
}

TEMPLATE_TEST_CASE(
    "expectation::Expectation finalizer can be exchanged.",
    "[expectation]",
    void(),
    int())
{
    using trompeloeil::_;
    using Signature = TestType;
    using Finalizer = FinalizerMock<Signature>;
    using FinalizerRef = FinalizerFacade<Signature, std::reference_wrapper<Finalizer>, UnwrapReferenceWrapper>;

    call::info_for_signature_t<Signature> call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    Finalizer finalizer{};
    expectation::Expectation exp{
        std::in_place_type<Signature>,
        {},
        make_common_target_report<Signature>(),
        ControlPolicyFake{},
        FinalizerRef{std::ref(finalizer)}};

    class Exception
    {
    };

    REQUIRE_CALL(finalizer, finalize_call(_))
        .LR_WITH(&_1 == &call)
        .THROW(Exception{});
    CHECK_THROWS_AS(exp.consume(call), Exception);
}

TEMPLATE_TEST_CASE(
    "expectation::Expectation can be extended with an arbitrary policy amount.",
    "[expectation]",
    void(),
    int())
{
    using trompeloeil::_;
    using Signature = TestType;
    using FinalizePolicy = FinalizerFake<Signature>;
    using PolicyMock = PolicyMock<Signature>;
    using PolicyRef = PolicyFacade<Signature, std::reference_wrapper<PolicyMock>, UnwrapReferenceWrapper>;
    using CallInfo = call::info_for_signature_t<Signature>;

    CallInfo const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};
    auto const targetReport = make_common_target_report<TestType>();
    ControlPolicyFake const controlPolicy{.isSatisfied = true, .stateData = commonApplicableState};

    SECTION("With no policies at all.")
    {
        expectation::Expectation expectation{
            std::in_place_type<Signature>,
            {},
            targetReport,
            controlPolicy,
            FinalizePolicy{}};

        CHECK(std::as_const(expectation).is_satisfied());
        reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call).value();
        CHECK_THAT(
            outcomes.outcomes,
            Catch::Matchers::IsEmpty());
        CHECK_NOTHROW(expectation.consume(call));
    }

    SECTION("With one policy.")
    {
        PolicyMock policy{};
        expectation::Expectation expectation{
            std::in_place_type<Signature>,
            {},
            targetReport,
            controlPolicy,
            FinalizePolicy{},
            PolicyRef{std::ref(policy)}};

        bool const isSatisfied = GENERATE(false, true);
        REQUIRE_CALL(policy, is_satisfied())
            .RETURN(isSatisfied);
        REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());

        SECTION("When matched => ok match")
        {
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(true);
            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call).value();
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::RangeEquals(std::array{true}));
        }

        SECTION("When not matched => no match")
        {
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(false);
            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call).value();
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::RangeEquals(std::array{false}));
        }

        REQUIRE_CALL(policy, consume(_))
            .LR_WITH(&_1 == &call);
        (void)expectation.consume(call);
    }

    SECTION("With two policies.")
    {
        PolicyMock policy1{};
        PolicyMock policy2{};
        expectation::Expectation expectation{
            std::in_place_type<Signature>,
            {},
            targetReport,
            controlPolicy,
            FinalizePolicy{},
            PolicyRef{std::ref(policy1)},
            PolicyRef{std::ref(policy2)}};

        SECTION("When calling is_satisfied()")
        {
            bool const isSatisfied1 = GENERATE(false, true);
            bool const isSatisfied2 = GENERATE(false, true);
            bool const expectedIsSatisfied = isSatisfied1 && isSatisfied2;
            REQUIRE_CALL(policy1, is_satisfied())
                .RETURN(isSatisfied1);
            auto policy2Expectation = std::invoke(
                [&]() -> std::unique_ptr<trompeloeil::expectation> {
                    if (isSatisfied1)
                    {
                        return NAMED_REQUIRE_CALL(policy2, is_satisfied())
                            .RETURN(isSatisfied2);
                    }
                    return nullptr;
                });

            REQUIRE(expectedIsSatisfied == std::as_const(expectation).is_satisfied());
        }

        SECTION("When both matches => ok match")
        {
            REQUIRE_CALL(policy1, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(true);
            REQUIRE_CALL(policy2, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(true);

            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call).value();
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::RangeEquals(std::array{true, true}));
        }

        SECTION("When at least one not matches => no match")
        {
            auto const [isMatching1, isMatching2] = GENERATE(
                (table<bool, bool>)({
                    {false,  true},
                    { true, false},
                    {false, false}
            }));

            REQUIRE_CALL(policy1, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(isMatching1);
            REQUIRE_CALL(policy2, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(isMatching2);

            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call).value();
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::RangeEquals(std::array{isMatching1, isMatching2}));
        }

        SECTION("When calling consume()")
        {
            REQUIRE_CALL(policy1, consume(_))
                .LR_WITH(&_1 == &call);
            REQUIRE_CALL(policy2, consume(_))
                .LR_WITH(&_1 == &call);
            (void)expectation.consume(call);
        }
    }
}
