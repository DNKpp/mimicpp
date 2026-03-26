//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/expectation/Registry.hpp"
#include "mimic++/expectation/Builder.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

namespace
{
    inline reporting::control_state_t const commonApplicableState = reporting::state_applicable{0, 1, 0};
    inline reporting::control_state_t const commonUnsatisfiedState = reporting::state_applicable{1, 1, 0};
    inline reporting::control_state_t const commonInapplicableState = reporting::state_inapplicable{0, 1, 0, {}, {{sequence::Tag{1337}}}};

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
    "expectation::Registry collects expectations and reports when they are removed but unfulfilled.",
    "[expectation]")
{
    using Signature = void();

    expectation::Registry registry{};

    ControlPolicyMock controlPolicy{};
    constexpr util::SourceLocation loc{};
    auto exp = registry.create(
        std::in_place_type<Signature>,
        loc,
        make_common_target_report<Signature>(),
        ControlPolicyFacade{std::ref(controlPolicy)},
        FinalizerFake<Signature>{});

    ScopedReporter reporter{};
    SECTION("When expectation is satisfied, nothing is reported.")
    {
        REQUIRE_CALL(controlPolicy, is_satisfied())
            .RETURN(true);
        CHECK_NOTHROW(registry.remove(exp));
        CHECK_THAT(
            reporter.unfulfilled_expectations(),
            Catch::Matchers::IsEmpty());
    }

    SECTION("When expectation is unfulfilled, it is reported.")
    {
        reporting::ExpectationReport const expReport{
            .from = loc,
            .target = make_common_target_report<Signature>(),
            .controlReport = commonUnsatisfiedState};

        REQUIRE_CALL(controlPolicy, is_satisfied())
            .RETURN(false);
        REQUIRE_CALL(controlPolicy, state())
            .RETURN(commonUnsatisfiedState);
        REQUIRE_NOTHROW(registry.remove(exp));
        CHECK_THAT(
            reporter.unfulfilled_expectations(),
            Catch::Matchers::RangeEquals({expReport}));
    }
}

TEST_CASE(
    "expectation::Registry queries its expectations, whether they match the call, in reverse order of construction.",
    "[expectation]")
{
    using namespace mimicpp::call;
    using Signature = void();
    using trompeloeil::_;

    ScopedReporter reporter{};
    expectation::Registry registry{};
    std::vector<PolicyMock<Signature>> policies(4);
    std::vector<ControlPolicyFake> controlPolicies{};
    controlPolicies.reserve(policies.size());
    std::vector<expectation::Expectation> expectations{};
    for (auto& policy : policies)
    {
        using Policy = PolicyFacade<Signature, std::reference_wrapper<PolicyMock<Signature>>, UnwrapReferenceWrapper>;

        expectations.emplace_back(
            registry.create(
                std::in_place_type<Signature>,
                util::SourceLocation{},
                make_common_target_report<Signature>(),
                ControlPolicyFacade{std::ref(controlPolicies.emplace_back())},
                FinalizerFake<Signature>{},
                Policy{std::ref(policy)}));
    }

    info_for_signature_t<Signature> const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};
    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>()};

    SECTION("If a full match is found.")
    {
        trompeloeil::sequence sequence{};
        REQUIRE_CALL(policies[3], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);
        REQUIRE_CALL(policies[2], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);
        REQUIRE_CALL(policies[1], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(true);
        REQUIRE_CALL(policies[0], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);
        REQUIRE_CALL(policies[1], consume(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence);

        controlPolicies[1u].stateData = commonApplicableState;
        REQUIRE_CALL(policies[1], describe())
            .RETURN("Policy1");

        CHECK_NOTHROW(registry.handle_call<Signature>(make_common_target_report<Signature>(), call));
        CHECK_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1));
    }

    SECTION("If at least one matches but is inapplicable.")
    {
        trompeloeil::sequence sequence{};
        REQUIRE_CALL(policies[3], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);
        REQUIRE_CALL(policies[2], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(true);
        REQUIRE_CALL(policies[1], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);
        REQUIRE_CALL(policies[0], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);

        controlPolicies[2u].stateData = commonInapplicableState;
        REQUIRE_CALL(policies[2u], describe())
            .RETURN("Policy2");

        CHECK_THROWS_AS(
            registry.handle_call<Signature>(make_common_target_report<Signature>(), call),
            NonApplicableMatchError);
        CHECK_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::SizeIs(1u));
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::IsEmpty());
    }

    SECTION("If none matches.")
    {
        trompeloeil::sequence sequence{};
        REQUIRE_CALL(policies[3], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);
        REQUIRE_CALL(policies[2], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);
        REQUIRE_CALL(policies[1], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);
        REQUIRE_CALL(policies[0], matches(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
            .IN_SEQUENCE(sequence)
            .RETURN(false);

        REQUIRE_CALL(policies[3u], describe())
            .RETURN("Policy3");
        REQUIRE_CALL(policies[2u], describe())
            .RETURN("Policy2");
        REQUIRE_CALL(policies[1u], describe())
            .RETURN("Policy1");
        REQUIRE_CALL(policies[0u], describe())
            .RETURN("Policy0");

        CHECK_THROWS_AS(
            registry.handle_call<Signature>(make_common_target_report<Signature>(), call),
            NoMatchError);
        CHECK_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::SizeIs(1u));
        CHECK_THAT(
            std::get<1>(reporter.no_match_reports().front()),
            Catch::Matchers::SizeIs(4u));
        CHECK_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::IsEmpty());
    }
}

TEST_CASE(
    "expectation::Registry::handle_call does not report matches, when settings::reportSuccess is false.",
    "[expectation]")
{
    using namespace mimicpp::call;
    using Signature = void();
    using trompeloeil::_;

    ScopedReporter reporter{};
    expectation::Registry registry{};
    auto exp = registry.create(
        std::in_place_type<Signature>,
        util::SourceLocation{},
        make_common_target_report<Signature>(),
        ControlPolicyFake{.stateData = commonApplicableState},
        FinalizerFake<Signature>{});

    info_for_signature_t<Signature> const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    settings::report_success().store(false);

    CHECK_NOTHROW(registry.handle_call<Signature>(make_common_target_report<Signature>(), call));
    CHECK_THAT(
        reporter.no_match_reports(),
        Catch::Matchers::IsEmpty());
    CHECK_THAT(
        reporter.inapplicable_match_reports(),
        Catch::Matchers::IsEmpty());
    CHECK_THAT(
        reporter.full_match_reports(),
        Catch::Matchers::IsEmpty());
}

TEST_CASE(
    "expectation::Registry::handle_call ignores expectations with mismatching signatures.",
    "[expectation]")
{
    using namespace mimicpp::call;
    using Signature = void();
    using trompeloeil::_;

    ScopedReporter reporter{};
    expectation::Registry registry{};
    auto exp = registry.create(
        std::in_place_type<Signature>,
        util::SourceLocation{},
        make_common_target_report<Signature>(),
        ControlPolicyFake{.stateData = commonApplicableState},
        FinalizerFake<Signature>{});

    using OtherSignature = int();
    info_for_signature_t<OtherSignature> const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    settings::report_success().store(false);

    REQUIRE_THROWS_AS(
        registry.handle_call<OtherSignature>(make_common_target_report<OtherSignature>(), call),
        NoMatchError);
    REQUIRE_THAT(
        reporter.no_match_reports(),
        Catch::Matchers::SizeIs(1u));
    CHECK_THAT(
        std::get<1u>(reporter.no_match_reports().front()),
        Catch::Matchers::IsEmpty());
    CHECK_THAT(
        reporter.inapplicable_match_reports(),
        Catch::Matchers::IsEmpty());
    CHECK_THAT(
        reporter.full_match_reports(),
        Catch::Matchers::IsEmpty());
}

TEST_CASE(
    "Unhandled exceptions during expectation::Registry::handle_call are reported.",
    "[expectation]")
{
    using namespace mimicpp::call;
    using Signature = void();
    using trompeloeil::_;

    ScopedReporter reporter{};
    expectation::Registry registry{};

    using PolicyRef = PolicyFacade<Signature, std::reference_wrapper<PolicyMock<Signature>>, UnwrapReferenceWrapper>;
    using FinalizeRef = FinalizerFacade<Signature, std::reference_wrapper<FinalizerMock<Signature>>, UnwrapReferenceWrapper>;
    PolicyMock<Signature> throwingPolicy{};
    constexpr util::SourceLocation throwingLoc{};
    auto throwingExp = registry.create(
        std::in_place_type<Signature>,
        throwingLoc,
        make_common_target_report<Signature>(),
        ControlPolicyFake{.stateData = commonApplicableState},
        FinalizerFake<Signature>{},
        PolicyRef{std::ref(throwingPolicy)});

    PolicyMock<Signature> otherPolicy{};
    FinalizerMock<Signature> otherFinalizePolicy{};
    auto otherExp = registry.create(
        std::in_place_type<Signature>,
        util::SourceLocation{},
        make_common_target_report<Signature>(),
        ControlPolicyFake{.stateData = commonApplicableState},
        FinalizeRef{std::ref(otherFinalizePolicy)},
        PolicyRef{std::ref(otherPolicy)});

    info_for_signature_t<Signature> const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    struct Exception
    {
    };

    reporting::ExpectationReport const throwingReport{
        .from = throwingLoc,
        .target = make_common_target_report<void()>(),
        .controlReport = commonApplicableState,
        .requirementDescriptions{"Throwing"}};

    auto const matches = [&](auto const& info) {
        try
        {
            std::rethrow_exception(info.exception);
        }
        catch (Exception const&)
        {
            auto const expected = reporting::make_call_report(
                make_common_target_report<void()>(),
                call,
                // Just use the existing stacktrace, because we can not construct it ourselves.
                info.call.stacktrace);
            return info.call == expected
                && info.expectation == throwingReport;
        }
        catch (...)
        {
            return false;
        }
    };

    SECTION("When an exception is thrown during matches.")
    {
        REQUIRE_CALL(throwingPolicy, matches(_))
            .THROW(Exception{});
        REQUIRE_CALL(throwingPolicy, describe())
            .RETURN("Throwing");
        REQUIRE_CALL(otherPolicy, matches(_))
            .RETURN(true);
        REQUIRE_CALL(otherPolicy, describe())
            .RETURN("Other");
        REQUIRE_CALL(otherPolicy, consume(_));
        REQUIRE_CALL(otherFinalizePolicy, finalize_call(_));
        REQUIRE_NOTHROW(registry.handle_call<Signature>(make_common_target_report<Signature>(), call));

        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1));
        CHECK_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::IsEmpty());

        REQUIRE_THAT(
            reporter.unhandled_exceptions(),
            Catch::Matchers::SizeIs(1));
        CHECK(matches(reporter.unhandled_exceptions().front()));
    }
}

TEST_CASE(
    "expectation::Registry::handle_call disambiguates multiple possible matches in a deterministic manner.",
    "[expectation]")
{
    using Signature = int();

    auto registry = std::make_shared<expectation::Registry>();

    ScopedReporter reporter{};

    reporting::TargetReport const targetReport{
        .name = "Test",
        .overloadReport = reporting::TypeReport::make<Signature>()};

    call::info_for_signature_t<Signature> const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("GreedySequence prefers younger expectations.")
    {
        GreedySequence sequence{};

        ScopedExpectation exp1 = expectation::detail::make_builder<Signature>(registry, targetReport)
                              && expect::times(0, 1)
                              && expect::in_sequence(sequence)
                              && finally::returns(42);

        ScopedExpectation exp2 = expectation::detail::make_builder<Signature>(registry, targetReport)
                              && expect::times(0, 1)
                              && expect::in_sequence(sequence)
                              && finally::returns(1337);

        CHECK(1337 == registry->handle_call<Signature>(targetReport, call));
    }

    SECTION("LazySequence prefers older expectations.")
    {
        LazySequence sequence{};

        ScopedExpectation exp1 = expectation::detail::make_builder<Signature>(registry, targetReport)
                              && expect::times(0, 1)
                              && expect::in_sequence(sequence)
                              && finally::returns(42);

        ScopedExpectation exp2 = expectation::detail::make_builder<Signature>(registry, targetReport)
                              && expect::times(0, 1)
                              && expect::in_sequence(sequence)
                              && finally::returns(1337);

        CHECK(42 == registry->handle_call<Signature>(targetReport, call));
    }
}
