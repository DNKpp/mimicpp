//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationBuilder.hpp"
#include "mimic++/Printing.hpp"
#include "mimic++/Utility.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "SuppressionMacros.hpp"
#include "TestReporter.hpp"
#include "TestTypes.hpp"

#include <functional>
#include <optional>
#include <ranges>
#include <source_location>

using namespace mimicpp;

namespace
{
    inline reporting::control_state_t const commonApplicableState = reporting::state_applicable{0, 1, 0};
    inline reporting::control_state_t const commonUnsatisfiedState = reporting::state_applicable{1, 1, 0};
    inline reporting::control_state_t const commonInapplicableState = reporting::state_inapplicable{0, 1, 0, {}, {sequence::Tag{1337}}};
    inline reporting::control_state_t const commonSaturatedState = reporting::state_saturated{0, 1, 1};

    class ExpectationMock final
        : public Expectation<void()>
    {
    public:
        using ExpectationReport = reporting::ExpectationReport;
        using RequirementOutcomes = reporting::RequirementOutcomes;
        using CallInfoT = call::info_for_signature_t<void()>;

        MAKE_CONST_MOCK0(report, ExpectationReport(), override);
        MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept override);
        MAKE_CONST_MOCK0(is_applicable, bool(), noexcept override);
        MAKE_CONST_MOCK0(from, util::SourceLocation const&(), noexcept override);
        MAKE_CONST_MOCK0(mock_name, StringT const&(), noexcept override);
        MAKE_CONST_MOCK1(matches, RequirementOutcomes(const CallInfoT&), override);
        MAKE_MOCK1(consume, void(const CallInfoT&), override);
        MAKE_MOCK1(finalize_call, void(const CallInfoT&), override);
    };

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
    "mimicpp::ExpectationCollection collects expectations and reports when they are removed but unfulfilled.",
    "[expectation]")
{
    using StorageT = ExpectationCollection<void()>;

    StorageT storage{};
    auto expectation = std::make_shared<ExpectationMock>();

    REQUIRE_NOTHROW(storage.push(expectation));

    ScopedReporter reporter{};
    SECTION("When expectation is satisfied, nothing is reported.")
    {
        REQUIRE_CALL(*expectation, is_satisfied())
            .RETURN(true);
        REQUIRE_NOTHROW(storage.remove(expectation));
        REQUIRE_THAT(
            reporter.unfulfilled_expectations(),
            Catch::Matchers::IsEmpty());
    }

    SECTION("When expectation is unfulfilled, it is reported.")
    {
        reporting::ExpectationReport const expReport{
            .target = make_common_target_report<void()>(),
            .controlReport = commonUnsatisfiedState};

        REQUIRE_CALL(*expectation, is_satisfied())
            .RETURN(false);
        REQUIRE_CALL(*expectation, report())
            .RETURN(expReport);
        REQUIRE_NOTHROW(storage.remove(expectation));
        REQUIRE_THAT(
            reporter.unfulfilled_expectations(),
            Catch::Matchers::SizeIs(1));
        REQUIRE(expReport == reporter.unfulfilled_expectations().at(0));
    }
}

namespace
{
    inline reporting::RequirementOutcomes const commonMatchingOutcome{
        .outcomes = {true}};

    inline reporting::RequirementOutcomes const commonNonMatchingOutcome{
        .outcomes = {true, false}
    };
}

TEST_CASE(
    "mimicpp::ExpectationCollection queries its expectations, whether they match the call, in reverse order of construction.",
    "[expectation]")
{
    using namespace mimicpp::call;
    using StorageT = ExpectationCollection<void()>;
    using CallInfoT = Info<void>;
    using trompeloeil::_;

    ScopedReporter reporter{};
    StorageT storage{};
    std::vector<std::shared_ptr<ExpectationMock>> expectations(4);
    for (auto& exp : expectations)
    {
        exp = std::make_shared<ExpectationMock>();
        storage.push(exp);
    }

    CallInfoT const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any,
        .fromSourceLocation = std::source_location::current()};
    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>()};

    SECTION("If a full match is found.")
    {
        trompeloeil::sequence sequence{};
        REQUIRE_CALL(*expectations[3], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);
        REQUIRE_CALL(*expectations[2], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);
        REQUIRE_CALL(*expectations[1], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonMatchingOutcome);
        REQUIRE_CALL(*expectations[0], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);
        REQUIRE_CALL(*expectations[1], consume(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence);
        REQUIRE_CALL(*expectations[1], finalize_call(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence);

        REQUIRE_CALL(*expectations[1], is_applicable())
            .RETURN(true);
        REQUIRE_CALL(*expectations[1], report())
            .RETURN(expectationReport);

        REQUIRE_NOTHROW(storage.handle_call(make_common_target_report<void()>(), call));
        REQUIRE_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::IsEmpty());
        REQUIRE_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::IsEmpty());
        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1));
    }

    SECTION("If at least one matches but is inapplicable.")
    {
        trompeloeil::sequence sequence{};
        REQUIRE_CALL(*expectations[3], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);
        REQUIRE_CALL(*expectations[2], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonMatchingOutcome);
        REQUIRE_CALL(*expectations[1], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);
        REQUIRE_CALL(*expectations[0], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);

        REQUIRE_CALL(*expectations[2], is_applicable())
            .RETURN(false);
        REQUIRE_CALL(*expectations[2], report())
            .RETURN(expectationReport);

        REQUIRE_THROWS_AS(
            storage.handle_call(make_common_target_report<void()>(), call),
            NonApplicableMatchError);
        REQUIRE_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::IsEmpty());
        REQUIRE_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::SizeIs(1u));
        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::IsEmpty());
    }

    SECTION("If none matches.")
    {
        trompeloeil::sequence sequence{};
        REQUIRE_CALL(*expectations[3], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);
        REQUIRE_CALL(*expectations[2], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);
        REQUIRE_CALL(*expectations[1], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);
        REQUIRE_CALL(*expectations[0], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(commonNonMatchingOutcome);

        REQUIRE_CALL(*expectations[3], report())
            .RETURN(expectationReport);
        REQUIRE_CALL(*expectations[2], report())
            .RETURN(expectationReport);
        REQUIRE_CALL(*expectations[1], report())
            .RETURN(expectationReport);
        REQUIRE_CALL(*expectations[0], report())
            .RETURN(expectationReport);

        REQUIRE_THROWS_AS(
            storage.handle_call(make_common_target_report<void()>(), call),
            NoMatchError);
        REQUIRE_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::SizeIs(4));
        REQUIRE_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::IsEmpty());
        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::IsEmpty());
    }
}

TEST_CASE(
    "Unhandled exceptions during mimicpp::ExpectationCollection::handle_call are reported.",
    "[expectation]")
{
    namespace Matches = Catch::Matchers;

    using namespace mimicpp::call;
    using StorageT = ExpectationCollection<void()>;
    using CallInfoT = Info<void>;
    using trompeloeil::_;

    ScopedReporter reporter{};
    StorageT storage{};
    auto throwingExpectation = std::make_shared<ExpectationMock>();
    auto otherExpectation = std::make_shared<ExpectationMock>();
    storage.push(otherExpectation);
    storage.push(throwingExpectation);

    CallInfoT const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    struct Exception
    {
    };

    reporting::ExpectationReport const throwingReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonApplicableState};

    reporting::ExpectationReport const otherReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonApplicableState};

    auto const matches = [&](auto const& info) {
        try
        {
            std::rethrow_exception(info.exception);
        }
        catch (Exception const&)
        {
            return info.call == reporting::make_call_report(make_common_target_report<void()>(), call)
                && info.expectation == throwingReport;
        }
        catch (...)
        {
            return false;
        }
    };

    SECTION("When an exception is thrown during matches.")
    {
        REQUIRE_CALL(*throwingExpectation, matches(_))
            .THROW(Exception{});
        REQUIRE_CALL(*throwingExpectation, report())
            .RETURN(throwingReport);
        REQUIRE_CALL(*otherExpectation, matches(_))
            .RETURN(commonMatchingOutcome);
        REQUIRE_CALL(*otherExpectation, is_applicable())
            .RETURN(true);
        REQUIRE_CALL(*otherExpectation, report())
            .RETURN(otherReport);
        REQUIRE_CALL(*otherExpectation, consume(_));
        REQUIRE_CALL(*otherExpectation, finalize_call(_));
        REQUIRE_NOTHROW(storage.handle_call(make_common_target_report<void()>(), call));

        CHECK_THAT(
            reporter.full_match_reports(),
            Matches::SizeIs(1));
        CHECK_THAT(
            reporter.no_match_reports(),
            Matches::IsEmpty());
        CHECK_THAT(
            reporter.inapplicable_match_reports(),
            Matches::IsEmpty());

        REQUIRE_THAT(
            reporter.unhandled_exceptions(),
            Matches::SizeIs(1));
        REQUIRE(matches(reporter.unhandled_exceptions().front()));
    }
}

TEMPLATE_TEST_CASE_SIG(
    "mimicpp::expectation_policy_for determines, whether the type can be used in combination with the given signature.",
    "[expectation]",
    ((bool expected, typename Policy, typename Signature), expected, Policy, Signature),
    (false, PolicyFake<void()>, int()),     // incompatible return type
    (false, PolicyFake<void()>, void(int)), // incompatible param
    (true, PolicyFake<void()>, void()),
    (true, PolicyFacade<void(), std::reference_wrapper<PolicyFake<void()>>, UnwrapReferenceWrapper>, void()))
{
    STATIC_REQUIRE(expected == mimicpp::expectation_policy_for<Policy, Signature>);
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
    STATIC_REQUIRE(expected == mimicpp::finalize_policy_for<Policy, Signature>);
}

TEMPLATE_TEST_CASE_SIG(
    "mimicpp::control_policy determines, whether the type satisfies the requirements.",
    "[expectation]",
    ((bool expected, typename Policy), expected, Policy),
    (true, ControlPolicyFake),
    (true, ControlPolicyFacade<std::reference_wrapper<ControlPolicyFake>, UnwrapReferenceWrapper>))
{
    STATIC_REQUIRE(expected == mimicpp::control_policy<Policy>);
}

TEST_CASE(
    "mimicpp::BasicExpectation stores general infos.",
    "[expectation]")
{
    using ControlPolicyT = ControlPolicyFake;
    using FinalizerT = FinalizerFake<void()>;

    util::SourceLocation const from{};
    reporting::TargetReport const target = make_common_target_report<void()>();
    BasicExpectation<void(), ControlPolicyT, FinalizerT> expectation{
        from,
        target,
        ControlPolicyT{},
        FinalizerT{}};

    REQUIRE(from == expectation.from());
    REQUIRE_THAT(
        target.name,
        Catch::Matchers::Equals(expectation.mock_name()));
}

TEST_CASE(
    "Control policy of mimicpp::BasicExpectation controls, how often its expectations must be matched.",
    "[expectation]")
{
    using trompeloeil::_;
    using SignatureT = void();
    using FinalizerT = FinalizerFake<SignatureT>;
    using PolicyMockT = PolicyMock<SignatureT>;
    using PolicyRefT = PolicyFacade<SignatureT, std::reference_wrapper<PolicyMock<SignatureT>>, UnwrapReferenceWrapper>;
    using ControlPolicyT = ControlPolicyFacade<std::reference_wrapper<ControlPolicyMock>, UnwrapReferenceWrapper>;
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    CallInfoT const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    ControlPolicyMock times{};

    SECTION("expectation_infos are gathered.")
    {
        reporting::TargetReport const target = make_common_target_report<void()>();

        BasicExpectation<SignatureT, ControlPolicyFake, FinalizerT> expectation{
            {},
            target,
            ControlPolicyFake{},
            FinalizerT{}};

        reporting::ExpectationReport const report = expectation.report();
        REQUIRE(target == report.target);
    }

    SECTION("With no other expectation policies.")
    {
        BasicExpectation<SignatureT, ControlPolicyT, FinalizerT> expectation{
            {},
            make_common_target_report<SignatureT>(),
            std::ref(times),
            FinalizerT{}};

        SECTION("When calling is_satisfied.")
        {
            bool const isSatisfied = GENERATE(false, true);
            REQUIRE_CALL(times, is_satisfied())
                .RETURN(isSatisfied);
            REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());
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
            REQUIRE(expected == std::as_const(expectation).is_applicable());
        }

        SECTION("When times is applicable, call is matched.")
        {
            REQUIRE_CALL(times, state())
                .RETURN(commonApplicableState);
            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::IsEmpty());
            reporting::ExpectationReport const expectationReport = std::as_const(expectation).report();
            REQUIRE(commonApplicableState == expectationReport.controlReport);
        }

        SECTION("When times is not applicable => inapplicable.")
        {
            auto const controlState = GENERATE(commonSaturatedState, commonInapplicableState);
            REQUIRE_CALL(times, state())
                .LR_RETURN(controlState);
            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::IsEmpty());
            reporting::ExpectationReport const expectationReport = std::as_const(expectation).report();
            REQUIRE(controlState == expectationReport.controlReport);
        }

        SECTION("Consume calls times.consume().")
        {
            REQUIRE_CALL(times, consume());
            REQUIRE_NOTHROW(expectation.consume(call));
        }
    }

    SECTION("With other expectation policies.")
    {
        PolicyMockT policy{};
        BasicExpectation<SignatureT, ControlPolicyT, FinalizerT, PolicyRefT> expectation{
            {},
            make_common_target_report<SignatureT>(),
            std::ref(times),
            FinalizerT{},
            std::ref(policy)};

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
            REQUIRE(expected == std::as_const(expectation).is_applicable());
        }

        SECTION("When times is not satisfied.")
        {
            REQUIRE_CALL(times, is_satisfied())
                .RETURN(false);
            REQUIRE(!std::as_const(expectation).is_satisfied());
        }

        SECTION("When times is satisfied, result depends on expectation policy.")
        {
            REQUIRE_CALL(times, is_satisfied())
                .RETURN(true);
            bool const isSatisfied = GENERATE(false, true);
            REQUIRE_CALL(policy, is_satisfied())
                .RETURN(isSatisfied);
            REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());
        }

        SECTION("When matches is called, the requirement policy is queried.")
        {
            bool const matches = GENERATE(false, true);
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(matches);
            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::RangeEquals(std::array{matches}));
        }

        SECTION("Consume calls times.consume().")
        {
            REQUIRE_CALL(times, consume());
            REQUIRE_CALL(policy, consume(_))
                .LR_WITH(&_1 == &call);
            REQUIRE_NOTHROW(expectation.consume(call));
        }
    }
}

TEMPLATE_TEST_CASE(
    "mimicpp::BasicExpectation can be extended with an arbitrary policy amount.",
    "[expectation]",
    void(),
    int())
{
    using trompeloeil::_;
    using FinalizerT = FinalizerFake<TestType>;
    using PolicyMockT = PolicyMock<TestType>;
    using PolicyRefT = PolicyFacade<TestType, std::reference_wrapper<PolicyMock<TestType>>, UnwrapReferenceWrapper>;
    using CallInfoT = call::info_for_signature_t<TestType>;

    const CallInfoT call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("With no policies at all.")
    {
        BasicExpectation<TestType, ControlPolicyFake, FinalizerT> expectation{
            {},
            make_common_target_report<TestType>(),
            ControlPolicyFake{
             .isSatisfied = true,
             .stateData = commonApplicableState},
            FinalizerT{}
        };

        REQUIRE(std::as_const(expectation).is_satisfied());
        reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call);
        REQUIRE_THAT(
            outcomes.outcomes,
            Catch::Matchers::IsEmpty());
        REQUIRE_NOTHROW(expectation.consume(call));
    }

    SECTION("With one policy.")
    {
        PolicyMockT policy{};
        BasicExpectation<TestType, ControlPolicyFake, FinalizerT, PolicyRefT> expectation{
            {},
            make_common_target_report<TestType>(),
            ControlPolicyFake{
             .isSatisfied = true,
             .stateData = commonApplicableState},
            FinalizerT{},
            PolicyRefT{std::ref(policy)}
        };

        bool const isSatisfied = GENERATE(false, true);
        REQUIRE_CALL(policy, is_satisfied())
            .RETURN(isSatisfied);
        REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());

        SECTION("When matched => ok match")
        {
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(true);
            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::RangeEquals(std::array{true}));
        }

        SECTION("When not matched => no match")
        {
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(false);
            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                outcomes.outcomes,
                Catch::Matchers::RangeEquals(std::array{false}));
        }

        REQUIRE_CALL(policy, consume(_))
            .LR_WITH(&_1 == &call);
        expectation.consume(call);
    }

    SECTION("With two policies.")
    {
        PolicyMockT policy1{};
        PolicyMockT policy2{};
        BasicExpectation<TestType, ControlPolicyFake, FinalizerT, PolicyRefT, PolicyRefT> expectation{
            {},
            make_common_target_report<TestType>(),
            ControlPolicyFake{
             .isSatisfied = true,
             .stateData = commonApplicableState},
            FinalizerT{},
            PolicyRefT{std::ref(policy1)},
            PolicyRefT{std::ref(policy2)}
        };

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

            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call);
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

            reporting::RequirementOutcomes const outcomes = std::as_const(expectation).matches(call);
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
            expectation.consume(call);
        }
    }
}

TEST_CASE(
    "mimicpp::BasicExpectation::report gathers information about the expectation.",
    "[expectation]")
{
    namespace Matches = Catch::Matchers;

    using FinalizerPolicyT = FinalizerFake<void()>;

    SECTION("SourceLocation is gathered.")
    {
        util::SourceLocation const loc{};

        BasicExpectation<
            void(),
            ControlPolicyFake,
            FinalizerPolicyT>
            expectation{
                loc,
                make_common_target_report<void()>(),
                ControlPolicyFake{},
                FinalizerPolicyT{}};

        reporting::ExpectationReport const report = expectation.report();
        REQUIRE(loc == report.from);
    }

    SECTION("reporting::TargetReport is gathered.")
    {
        reporting::TargetReport const target = make_common_target_report<void()>();

        BasicExpectation<
            void(),
            ControlPolicyFake,
            FinalizerPolicyT>
            expectation{
                {},
                target,
                ControlPolicyFake{},
                FinalizerPolicyT{}};

        reporting::ExpectationReport const report = expectation.report();
        REQUIRE(target == report.target);
    }

    SECTION("Finalizer policy has no description.")
    {
        // Todo:
    }

    SECTION("Times policy is queried.")
    {
        using ControlT = ControlPolicyFacade<std::reference_wrapper<ControlPolicyMock>, UnwrapReferenceWrapper>;

        ControlPolicyMock controlPolicy{};
        BasicExpectation<
            void(),
            ControlT,
            FinalizerPolicyT>
            expectation{
                {},
                make_common_target_report<void()>(),
                ControlT{std::ref(controlPolicy)},
                FinalizerPolicyT{}};

        auto const state = GENERATE(
            commonApplicableState,
            commonUnsatisfiedState,
            commonInapplicableState,
            commonSaturatedState);
        REQUIRE_CALL(controlPolicy, state())
            .RETURN(state);

        const reporting::ExpectationReport report = expectation.report();
        REQUIRE(report.controlReport == state);
    }

    SECTION("Expectation policies are queried.")
    {
        using PolicyT = PolicyFacade<
            void(),
            std::reference_wrapper<PolicyMock<void()>>,
            UnwrapReferenceWrapper>;

        PolicyMock<void()> policy{};
        BasicExpectation<
            void(),
            ControlPolicyFake,
            FinalizerPolicyT,
            PolicyT>
            expectation{
                {},
                make_common_target_report<void()>(),
                ControlPolicyFake{},
                FinalizerPolicyT{},
                PolicyT{std::ref(policy)}};

        REQUIRE_CALL(policy, describe())
            .RETURN("expectation description");

        const reporting::ExpectationReport report = expectation.report();
        REQUIRE_THAT(
            report.requirementDescriptions,
            Matches::SizeIs(1));
        REQUIRE(report.requirementDescriptions.front());
        REQUIRE_THAT(
            *report.requirementDescriptions.front(),
            Matches::Equals("expectation description"));
    }
}

TEMPLATE_TEST_CASE(
    "mimicpp::BasicExpectation finalizer can be exchanged.",
    "[expectation]",
    void(),
    int())
{
    using trompeloeil::_;
    using SignatureT = TestType;
    using FinalizerT = FinalizerMock<SignatureT>;
    using FinalizerRefT = FinalizerFacade<SignatureT, std::reference_wrapper<FinalizerT>, UnwrapReferenceWrapper>;
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    const CallInfoT call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    FinalizerT finalizer{};
    BasicExpectation<SignatureT, ControlPolicyFake, FinalizerRefT> expectation{
        {},
        make_common_target_report<SignatureT>(),
        ControlPolicyFake{},
        std::ref(finalizer)};

    class Exception
    {
    };

    REQUIRE_CALL(finalizer, finalize_call(_))
        .LR_WITH(&_1 == &call)
        .THROW(Exception{});
    REQUIRE_THROWS_AS(expectation.finalize_call(call), Exception);
}

TEST_CASE("ScopedExpectation is a non-copyable, but movable type.")
{
    STATIC_REQUIRE(!std::is_copy_constructible_v<mimicpp::ScopedExpectation>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<mimicpp::ScopedExpectation>);
    STATIC_REQUIRE(std::is_move_constructible_v<mimicpp::ScopedExpectation>);
    STATIC_REQUIRE(std::is_move_assignable_v<mimicpp::ScopedExpectation>);
}

TEST_CASE(
    "ScopedExpectation handles expectation lifetime and removes from the ExpectationCollection.",
    "[expectation]")
{
    using trompeloeil::_;
    using SignatureT = void();
    using ExpectationT = ExpectationMock;
    using CollectionT = ExpectationCollection<SignatureT>;

    auto collection = std::make_shared<CollectionT>();
    auto innerExpectation = std::make_shared<ExpectationT>();
    std::optional<ScopedExpectation> expectation{
        std::in_place,
        collection,
        innerExpectation};

    SECTION("When calling is_satisfied()")
    {
        bool const isSatisfied = GENERATE(false, true);
        REQUIRE_CALL(*innerExpectation, is_satisfied())
            .RETURN(isSatisfied);
        REQUIRE(isSatisfied == std::as_const(expectation)->is_satisfied());
    }

    SECTION("When calling is_applicable.")
    {
        bool const isApplicable = GENERATE(false, true);
        REQUIRE_CALL(*innerExpectation, is_applicable())
            .RETURN(isApplicable);
        REQUIRE(isApplicable == std::as_const(expectation)->is_applicable());
    }

    SECTION("When calling from()")
    {
        util::SourceLocation const loc{};
        REQUIRE_CALL(*innerExpectation, from())
            .RETURN(loc);
        REQUIRE(loc == std::as_const(expectation)->from());
    }

    SECTION("When calling mock_name()")
    {
        StringT const mockName = "MyMock";
        REQUIRE_CALL(*innerExpectation, mock_name())
            .LR_RETURN(std::ref(mockName));
        REQUIRE(mockName == std::as_const(expectation)->mock_name());
    }

    SECTION("When ScopedExpectation is moved.")
    {
        ScopedExpectation otherExpectation = *std::move(expectation);

        SECTION("When calling is_satisfied()")
        {
            bool const isSatisfied = GENERATE(false, true);
            REQUIRE_CALL(*innerExpectation, is_satisfied())
                .RETURN(isSatisfied);
            REQUIRE(isSatisfied == std::as_const(otherExpectation).is_satisfied());
        }

        SECTION("When calling is_applicable.")
        {
            bool const isApplicable = GENERATE(false, true);
            REQUIRE_CALL(*innerExpectation, is_applicable())
                .RETURN(isApplicable);
            REQUIRE(isApplicable == std::as_const(otherExpectation).is_applicable());
        }

        SECTION("And then move assigned.")
        {
            expectation = std::move(otherExpectation);

            SECTION("When calling is_satisfied()")
            {
                bool const isSatisfied = GENERATE(false, true);
                REQUIRE_CALL(*innerExpectation, is_satisfied())
                    .RETURN(isSatisfied);
                REQUIRE(isSatisfied == std::as_const(expectation)->is_satisfied());
            }

            SECTION("When calling is_applicable.")
            {
                bool const isApplicable = GENERATE(false, true);
                REQUIRE_CALL(*innerExpectation, is_applicable())
                    .RETURN(isApplicable);
                REQUIRE(isApplicable == std::as_const(expectation)->is_applicable());
            }

            // just move back, so we can unify the cleanup process
            otherExpectation = *std::move(expectation);
        }

        SECTION("And then self move assigned.")
        {
            START_WARNING_SUPPRESSION
            SUPPRESS_SELF_MOVE
            otherExpectation = std::move(otherExpectation);
            STOP_WARNING_SUPPRESSION

            SECTION("When calling is_satisfied()")
            {
                bool const isSatisfied = GENERATE(false, true);
                REQUIRE_CALL(*innerExpectation, is_satisfied())
                    .RETURN(isSatisfied);
                REQUIRE(isSatisfied == std::as_const(otherExpectation).is_satisfied());
            }

            SECTION("When calling is_applicable.")
            {
                bool const isApplicable = GENERATE(false, true);
                REQUIRE_CALL(*innerExpectation, is_applicable())
                    .RETURN(isApplicable);
                REQUIRE(isApplicable == std::as_const(otherExpectation).is_applicable());
            }
        }

        // just move back, so we can unify the cleanup process
        expectation = std::move(otherExpectation);
    }

    // indirectly via remove
    REQUIRE_CALL(*innerExpectation, is_satisfied())
        .RETURN(true);
    expectation.reset();
}

TEST_CASE(
    "ExpectationCollection disambiguates multiple possible matches in a deterministic manner.",
    "[expectation]")
{
    using SignatureT = int();
    using CollectionT = ExpectationCollection<SignatureT>;
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    auto collection = std::make_shared<CollectionT>();

    ScopedReporter reporter{};

    reporting::TargetReport const targetReport{
        .name = "Test",
        .overloadReport = reporting::TypeReport::make<SignatureT>()};

    CallInfoT const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("GreedySequence prefers younger expectations.")
    {
        GreedySequence sequence{};

        ScopedExpectation exp1 = detail::make_expectation_builder(collection, targetReport)
                              && expect::times(0, 1)
                              && expect::in_sequence(sequence)
                              && finally::returns(42);

        ScopedExpectation exp2 = detail::make_expectation_builder(collection, targetReport)
                              && expect::times(0, 1)
                              && expect::in_sequence(sequence)
                              && finally::returns(1337);

        REQUIRE(1337 == collection->handle_call(make_common_target_report<void()>(), call));
    }

    SECTION("LazySequence prefers older expectations.")
    {
        LazySequence sequence{};

        ScopedExpectation exp1 = detail::make_expectation_builder(collection, targetReport)
                              && expect::times(0, 1)
                              && expect::in_sequence(sequence)
                              && finally::returns(42);

        ScopedExpectation exp2 = detail::make_expectation_builder(collection, targetReport)
                              && expect::times(0, 1)
                              && expect::in_sequence(sequence)
                              && finally::returns(1337);

        REQUIRE(42 == collection->handle_call(make_common_target_report<void()>(), call));
    }
}
