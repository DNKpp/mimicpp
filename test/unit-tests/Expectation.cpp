//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationBuilder.hpp"
#include "mimic++/Printer.hpp"
#include "mimic++/Utility.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "SuppressionMacros.hpp"
#include "TestReporter.hpp"
#include "TestTypes.hpp"

#include <functional>
#include <optional>
#include <ranges>
#include <source_location>

namespace
{
    class ExpectationMock final
        : public mimicpp::Expectation<void()>
    {
    public:
        using CallInfoT = mimicpp::call::info_for_signature_t<void()>;

        MAKE_CONST_MOCK0(report, mimicpp::ExpectationReport(), override);
        MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept override);
        MAKE_CONST_MOCK0(from, const std::source_location&(), noexcept override);
        MAKE_CONST_MOCK0(mock_name, const std::optional<mimicpp::StringT>&(), noexcept override);
        MAKE_CONST_MOCK1(matches, mimicpp::MatchReport(const CallInfoT&), override);
        MAKE_MOCK1(consume, void(const CallInfoT&), override);
        MAKE_MOCK1(finalize_call, void(const CallInfoT&), override);
    };
}

TEST_CASE(
    "mimicpp::ExpectationCollection collects expectations and reports when they are removed but unfulfilled.",
    "[expectation]")
{
    using StorageT = mimicpp::ExpectationCollection<void()>;

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
        const mimicpp::ExpectationReport expReport{
            .timesDescription = "times description"};

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
    inline const mimicpp::MatchReport commonNoMatchReport{
        .controlReport = mimicpp::state_applicable{
                                                   .min = 0,
                                                   .max = 1337,
                                                   .count = 0},
        .expectationReports = {{.isMatching = false}}
    };

    inline const mimicpp::MatchReport commonFullMatchReport{
        .controlReport = mimicpp::state_applicable{
                                                   .min = 0,
                                                   .max = 1337,
                                                   .count = 0},
        .expectationReports = {}
    };

    inline const mimicpp::MatchReport commonInapplicableMatchReport{
        .controlReport = mimicpp::state_inapplicable{
                                                     .min = 0,
                                                     .max = 1337,
                                                     .count = 0,
                                                     .inapplicableSequences = {mimicpp::sequence::Tag{42}}},
        .expectationReports = {}
    };
}

TEST_CASE(
    "mimicpp::ExpectationCollection queries its expectations, whether they match the call, in reverse order of construction.",
    "[expectation]")
{
    using mimicpp::is_same_source_location;
    using namespace mimicpp::call;
    using StorageT = mimicpp::ExpectationCollection<void()>;
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

    const CallInfoT call{
        .args = {},
        .fromCategory = mimicpp::ValueCategory::any,
        .fromConstness = mimicpp::Constness::any,
        .fromSourceLocation = std::source_location::current()};

    SECTION("If a full match is found.")
    {
        trompeloeil::sequence sequence{};
        REQUIRE_CALL(*expectations[3], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(commonNoMatchReport);
        REQUIRE_CALL(*expectations[2], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(commonNoMatchReport);
        REQUIRE_CALL(*expectations[1], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(commonFullMatchReport);
        REQUIRE_CALL(*expectations[0], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(commonNoMatchReport);
        REQUIRE_CALL(*expectations[1], consume(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence);
        REQUIRE_CALL(*expectations[1], finalize_call(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence);

        REQUIRE_NOTHROW(storage.handle_call(call));
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
        using match_report_t = mimicpp::MatchReport;
        const auto [count, result0, result1, result2, result3] = GENERATE(
            (table<std::size_t, match_report_t, match_report_t, match_report_t, match_report_t>)({
                {1u, commonInapplicableMatchReport,           commonNoMatchReport,           commonNoMatchReport,           commonNoMatchReport},
                {1u,           commonNoMatchReport, commonInapplicableMatchReport,           commonNoMatchReport,           commonNoMatchReport},
                {1u,           commonNoMatchReport,           commonNoMatchReport, commonInapplicableMatchReport,           commonNoMatchReport},
                {1u,           commonNoMatchReport,           commonNoMatchReport,           commonNoMatchReport, commonInapplicableMatchReport},

                {2u, commonInapplicableMatchReport,           commonNoMatchReport, commonInapplicableMatchReport,           commonNoMatchReport},
                {2u,           commonNoMatchReport, commonInapplicableMatchReport,           commonNoMatchReport, commonInapplicableMatchReport},
                {3u, commonInapplicableMatchReport,           commonNoMatchReport, commonInapplicableMatchReport, commonInapplicableMatchReport},
                {4u, commonInapplicableMatchReport, commonInapplicableMatchReport, commonInapplicableMatchReport, commonInapplicableMatchReport}
        }));

        trompeloeil::sequence sequence{};
        REQUIRE_CALL(*expectations[3], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(result0);
        REQUIRE_CALL(*expectations[2], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(result1);
        REQUIRE_CALL(*expectations[1], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(result2);
        REQUIRE_CALL(*expectations[0], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(result3);

        REQUIRE_THROWS_AS(
            storage.handle_call(call),
            NonApplicableMatchError);
        REQUIRE_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::IsEmpty());
        REQUIRE_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::SizeIs(count));
        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::IsEmpty());
    }

    SECTION("If none matches.")
    {
        trompeloeil::sequence sequence{};
        REQUIRE_CALL(*expectations[3], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(commonNoMatchReport);
        REQUIRE_CALL(*expectations[2], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(commonNoMatchReport);
        REQUIRE_CALL(*expectations[1], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(commonNoMatchReport);
        REQUIRE_CALL(*expectations[0], matches(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
            .IN_SEQUENCE(sequence)
            .RETURN(commonNoMatchReport);

        REQUIRE_THROWS_AS(
            storage.handle_call(call),
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
    using StorageT = mimicpp::ExpectationCollection<void()>;
    using CallInfoT = Info<void>;
    using trompeloeil::_;

    ScopedReporter reporter{};
    StorageT storage{};
    auto throwingExpectation = std::make_shared<ExpectationMock>();
    auto otherExpectation = std::make_shared<ExpectationMock>();
    storage.push(otherExpectation);
    storage.push(throwingExpectation);

    const CallInfoT call{
        .args = {},
        .fromCategory = mimicpp::ValueCategory::any,
        .fromConstness = mimicpp::Constness::any};

    struct Exception
    {
    };

    const mimicpp::ExpectationReport throwingReport{
        .timesDescription = "times description"};

    const auto matches = [&](const auto& info) {
        try
        {
            std::rethrow_exception(info.exception);
        }
        catch (const Exception&)
        {
            return info.call == mimicpp::make_call_report(call)
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
            .RETURN(commonFullMatchReport);
        REQUIRE_CALL(*otherExpectation, consume(_));
        REQUIRE_CALL(*otherExpectation, finalize_call(_));
        REQUIRE_NOTHROW(storage.handle_call(call));

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
    "mimicpp::BasicExpectation stores detail::expectation_info.",
    "[expectation]")
{
    using ControlPolicyT = ControlPolicyFake;
    using FinalizerT = FinalizerFake<void()>;

    const mimicpp::detail::expectation_info info{
        .sourceLocation = std::source_location::current(),
        .mockName = GENERATE(as<std::optional<mimicpp::StringT>>{}, std::nullopt, "MyMock")};

    mimicpp::BasicExpectation<void(), ControlPolicyT, FinalizerT> expectation{
        info,
        ControlPolicyT{},
        FinalizerT{}};

    REQUIRE(mimicpp::is_same_source_location(info.sourceLocation, expectation.from()));
    REQUIRE(info.mockName == expectation.mock_name());
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
    using CallInfoT = mimicpp::call::info_for_signature_t<SignatureT>;

    const CallInfoT call{
        .args = {},
        .fromCategory = mimicpp::ValueCategory::any,
        .fromConstness = mimicpp::Constness::any};

    ControlPolicyMock times{};

    SECTION("expectation_infos are gathered.")
    {
        const mimicpp::detail::expectation_info info{
            .sourceLocation = std::source_location::current(),
            .mockName = GENERATE(as<std::optional<mimicpp::StringT>>{}, std::nullopt, "MyMock")};

        mimicpp::BasicExpectation<SignatureT, ControlPolicyFake, FinalizerT> expectation{
            info,
            ControlPolicyFake{},
            FinalizerT{}};

        const mimicpp::MatchReport report = expectation.matches(call);
        REQUIRE(info == report.expectationInfo);
    }

    SECTION("With no other expectation policies.")
    {
        mimicpp::BasicExpectation<SignatureT, ControlPolicyT, FinalizerT> expectation{
            {},
            std::ref(times),
            FinalizerT{}};

        SECTION("When calling is_satisfied.")
        {
            const bool isSatisfied = GENERATE(false, true);
            REQUIRE_CALL(times, is_satisfied())
                .RETURN(isSatisfied);
            REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());
        }

        SECTION("When times is applicable, call is matched.")
        {
            const mimicpp::control_state_t controlState{
                mimicpp::state_applicable{0, 1, 0}
            };
            REQUIRE_CALL(times, state())
                .RETURN(controlState);
            const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
            REQUIRE(matchReport.controlReport == controlState);
            REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(matchReport));
        }

        SECTION("When times is not applicable => inapplicable.")
        {
            const auto controlState = GENERATE(
                as<mimicpp::control_state_t>{},
                (mimicpp::state_inapplicable{0, 1, 0, {}, {mimicpp::sequence::Tag{1337}}}),
                (mimicpp::state_saturated{0, 1, 1}));
            REQUIRE_CALL(times, state())
                .LR_RETURN(controlState);
            const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
            REQUIRE(matchReport.controlReport == controlState);
            REQUIRE(mimicpp::MatchResult::inapplicable == evaluate_match_report(matchReport));
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
        mimicpp::BasicExpectation<SignatureT, ControlPolicyT, FinalizerT, PolicyRefT> expectation{
            {},
            std::ref(times),
            FinalizerT{},
            std::ref(policy)};

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
            const bool isSatisfied = GENERATE(false, true);
            REQUIRE_CALL(policy, is_satisfied())
                .RETURN(isSatisfied);
            REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());
        }

        SECTION("When policy is not matched, then the result is always no match.")
        {
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(false);
            REQUIRE_CALL(policy, describe())
                .RETURN(mimicpp::StringT{});
            const auto controlState = GENERATE(
                as<mimicpp::control_state_t>{},
                (mimicpp::state_applicable{0, 1, 0}),
                (mimicpp::state_inapplicable{0, 1, 0, {}, {mimicpp::sequence::Tag{1337}}}),
                (mimicpp::state_saturated{0, 1, 1}));
            REQUIRE_CALL(times, state())
                .RETURN(controlState);
            REQUIRE(mimicpp::MatchResult::none == evaluate_match_report(std::as_const(expectation).matches(call)));
        }

        SECTION("When policy is matched.")
        {
            SECTION("And when times is applicable => ok")
            {
                REQUIRE_CALL(times, state())
                    .RETURN(mimicpp::state_applicable{0, 1, 0});
                REQUIRE_CALL(policy, matches(_))
                    .LR_WITH(&_1 == &call)
                    .RETURN(true);
                REQUIRE_CALL(policy, describe())
                    .RETURN(mimicpp::StringT{});
                REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(std::as_const(expectation).matches(call)));
            }

            SECTION("And when times not applicable => inapplicable")
            {
                const auto controlState = GENERATE(
                    as<mimicpp::control_state_t>{},
                    (mimicpp::state_inapplicable{0, 1, 0, {}, {mimicpp::sequence::Tag{1337}}}),
                    (mimicpp::state_saturated{0, 1, 1}));
                REQUIRE_CALL(times, state())
                    .LR_RETURN(controlState);
                REQUIRE_CALL(policy, matches(_))
                    .LR_WITH(&_1 == &call)
                    .RETURN(true);
                REQUIRE_CALL(policy, describe())
                    .RETURN(mimicpp::StringT{});
                REQUIRE(mimicpp::MatchResult::inapplicable == evaluate_match_report(std::as_const(expectation).matches(call)));
            }
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
    using CallInfoT = mimicpp::call::info_for_signature_t<TestType>;
    using ExpectationReportT = mimicpp::MatchReport::Expectation;

    const CallInfoT call{
        .args = {},
        .fromCategory = mimicpp::ValueCategory::any,
        .fromConstness = mimicpp::Constness::any};

    SECTION("With no policies at all.")
    {
        mimicpp::BasicExpectation<TestType, ControlPolicyFake, FinalizerT> expectation{
            {},
            ControlPolicyFake{
                              .isSatisfied = true,
                              .stateData = commonFullMatchReport.controlReport},
            FinalizerT{}
        };

        REQUIRE(std::as_const(expectation).is_satisfied());
        const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
        REQUIRE_THAT(
            matchReport.expectationReports,
            Catch::Matchers::IsEmpty());
        REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(matchReport));
        REQUIRE_NOTHROW(expectation.consume(call));
    }

    SECTION("With one policy.")
    {
        PolicyMockT policy{};
        mimicpp::BasicExpectation<TestType, ControlPolicyFake, FinalizerT, PolicyRefT> expectation{
            {},
            ControlPolicyFake{
                              .isSatisfied = true,
                              .stateData = commonFullMatchReport.controlReport},
            FinalizerT{},
            PolicyRefT{std::ref(policy)}
        };

        const bool isSatisfied = GENERATE(false, true);
        REQUIRE_CALL(policy, is_satisfied())
            .RETURN(isSatisfied);
        REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());

        SECTION("When matched => ok match")
        {
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(true);
            REQUIRE_CALL(policy, describe())
                .RETURN("policy description");
            const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                matchReport.expectationReports,
                Catch::Matchers::RangeEquals(std::vector<ExpectationReportT>{
                    {true, "policy description"}
            }));
            REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(matchReport));
        }

        SECTION("When not matched => no match")
        {
            REQUIRE_CALL(policy, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(false);
            REQUIRE_CALL(policy, describe())
                .RETURN("policy description");
            const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                matchReport.expectationReports,
                Catch::Matchers::RangeEquals(std::vector<ExpectationReportT>{
                    {false, "policy description"}
            }));
            REQUIRE(mimicpp::MatchResult::none == evaluate_match_report(matchReport));
        }

        REQUIRE_CALL(policy, consume(_))
            .LR_WITH(&_1 == &call);
        expectation.consume(call);
    }

    SECTION("With two policies.")
    {
        PolicyMockT policy1{};
        PolicyMockT policy2{};
        mimicpp::BasicExpectation<TestType, ControlPolicyFake, FinalizerT, PolicyRefT, PolicyRefT> expectation{
            {},
            ControlPolicyFake{
                              .isSatisfied = true,
                              .stateData = commonFullMatchReport.controlReport},
            FinalizerT{},
            PolicyRefT{std::ref(policy1)},
            PolicyRefT{std::ref(policy2)}
        };

        SECTION("When calling is_satisfied()")
        {
            const bool isSatisfied1 = GENERATE(false, true);
            const bool isSatisfied2 = GENERATE(false, true);
            const bool expectedIsSatisfied = isSatisfied1 && isSatisfied2;
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
            REQUIRE_CALL(policy1, describe())
                .RETURN("policy1 description");
            REQUIRE_CALL(policy2, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(true);
            REQUIRE_CALL(policy2, describe())
                .RETURN("policy2 description");

            const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                matchReport.expectationReports,
                Catch::Matchers::UnorderedRangeEquals(
                    std::vector<ExpectationReportT>{
                        {true, "policy1 description"},
                        {true, "policy2 description"}
            }));
            REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(matchReport));
        }

        SECTION("When at least one not matches => no match")
        {
            const auto [isMatching1, isMatching2] = GENERATE(
                (table<bool, bool>)({
                    {false,  true},
                    { true, false},
                    {false, false}
            }));

            REQUIRE_CALL(policy1, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(isMatching1);
            REQUIRE_CALL(policy1, describe())
                .RETURN("policy1 description");
            REQUIRE_CALL(policy2, matches(_))
                .LR_WITH(&_1 == &call)
                .RETURN(isMatching2);
            REQUIRE_CALL(policy2, describe())
                .RETURN("policy2 description");

            const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
            REQUIRE_THAT(
                matchReport.expectationReports,
                Catch::Matchers::UnorderedRangeEquals(
                    std::vector<ExpectationReportT>{
                        {isMatching1, "policy1 description"},
                        {isMatching2, "policy2 description"}
            }));
            REQUIRE(mimicpp::MatchResult::none == evaluate_match_report(matchReport));
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

    SECTION("expectation_infos are gathered.")
    {
        const mimicpp::detail::expectation_info info{
            .sourceLocation = std::source_location::current(),
            .mockName = GENERATE(as<std::optional<mimicpp::StringT>>{}, std::nullopt, "MyMock")};

        mimicpp::BasicExpectation<
            void(),
            ControlPolicyFake,
            FinalizerPolicyT>
            expectation{
                info,
                ControlPolicyFake{},
                FinalizerPolicyT{}};

        const mimicpp::ExpectationReport report = expectation.report();
        REQUIRE(info == report.expectationInfo);
    }

    SECTION("Finalizer policy has no description.")
    {
        // Todo:
    }

    SECTION("Times policy is queried.")
    {
        using ControlT = ControlPolicyFacade<std::reference_wrapper<ControlPolicyMock>, UnwrapReferenceWrapper>;

        ControlPolicyMock controlPolicy{};
        mimicpp::BasicExpectation<
            void(),
            ControlT,
            FinalizerPolicyT>
            expectation{
                {},
                ControlT{std::ref(controlPolicy)},
                FinalizerPolicyT{}};

        REQUIRE_CALL(controlPolicy, state())
            .RETURN(mimicpp::state_applicable{0, 1, 0});

        const mimicpp::ExpectationReport report = expectation.report();
        REQUIRE(report.timesDescription);
        REQUIRE_THAT(
            *report.timesDescription,
            !Matches::IsEmpty());
    }

    SECTION("Expectation policies are queried.")
    {
        using PolicyT = PolicyFacade<
            void(),
            std::reference_wrapper<PolicyMock<void()>>,
            UnwrapReferenceWrapper>;

        PolicyMock<void()> policy{};
        mimicpp::BasicExpectation<
            void(),
            ControlPolicyFake,
            FinalizerPolicyT,
            PolicyT>
            expectation{
                {},
                ControlPolicyFake{},
                FinalizerPolicyT{},
                PolicyT{std::ref(policy)}};

        REQUIRE_CALL(policy, describe())
            .RETURN("expectation description");

        const mimicpp::ExpectationReport report = expectation.report();
        REQUIRE_THAT(
            report.expectationDescriptions,
            Matches::SizeIs(1));
        REQUIRE(report.expectationDescriptions[0]);
        REQUIRE_THAT(
            *report.expectationDescriptions[0],
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
    using CallInfoT = mimicpp::call::info_for_signature_t<SignatureT>;

    const CallInfoT call{
        .args = {},
        .fromCategory = mimicpp::ValueCategory::any,
        .fromConstness = mimicpp::Constness::any};

    FinalizerT finalizer{};
    mimicpp::BasicExpectation<SignatureT, ControlPolicyFake, FinalizerRefT> expectation{
        {},
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
    using CollectionT = mimicpp::ExpectationCollection<SignatureT>;

    auto collection = std::make_shared<CollectionT>();
    auto innerExpectation = std::make_shared<ExpectationT>();
    std::optional<mimicpp::ScopedExpectation> expectation{
        std::in_place,
        collection,
        innerExpectation};

    SECTION("When calling is_satisfied()")
    {
        const bool isSatisfied = GENERATE(false, true);
        REQUIRE_CALL(*innerExpectation, is_satisfied())
            .RETURN(isSatisfied);
        REQUIRE(isSatisfied == std::as_const(expectation)->is_satisfied());
    }

    SECTION("When calling from()")
    {
        const auto loc = std::source_location::current();
        REQUIRE_CALL(*innerExpectation, from())
            .RETURN(loc);
        REQUIRE(
            mimicpp::is_same_source_location(
                loc,
                std::as_const(expectation)->from()));
    }

    SECTION("When calling mock_name()")
    {
        const auto mockName = GENERATE(as<std::optional<mimicpp::StringT>>{}, std::nullopt, "MyMock");
        REQUIRE_CALL(*innerExpectation, mock_name())
            .LR_RETURN(std::ref(mockName));
        REQUIRE(mockName == std::as_const(expectation)->mock_name());
    }

    SECTION("When ScopedExpectation is moved.")
    {
        mimicpp::ScopedExpectation otherExpectation = *std::move(expectation);

        SECTION("When calling is_satisfied()")
        {
            const bool isSatisfied = GENERATE(false, true);
            REQUIRE_CALL(*innerExpectation, is_satisfied())
                .RETURN(isSatisfied);
            REQUIRE(isSatisfied == std::as_const(otherExpectation).is_satisfied());
        }

        SECTION("And then move assigned.")
        {
            expectation = std::move(otherExpectation);

            SECTION("When calling is_satisfied()")
            {
                const bool isSatisfied = GENERATE(false, true);
                REQUIRE_CALL(*innerExpectation, is_satisfied())
                    .RETURN(isSatisfied);
                REQUIRE(isSatisfied == std::as_const(expectation)->is_satisfied());
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
                const bool isSatisfied = GENERATE(false, true);
                REQUIRE_CALL(*innerExpectation, is_satisfied())
                    .RETURN(isSatisfied);
                REQUIRE(isSatisfied == std::as_const(otherExpectation).is_satisfied());
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
    namespace expect = mimicpp::expect;
    namespace finally = mimicpp::finally;
    using SignatureT = int();
    using CollectionT = mimicpp::ExpectationCollection<SignatureT>;
    using CallInfoT = mimicpp::call::info_for_signature_t<SignatureT>;

    auto collection = std::make_shared<CollectionT>();

    ScopedReporter reporter{};

    const CallInfoT call{
        .args = {},
        .fromCategory = mimicpp::ValueCategory::any,
        .fromConstness = mimicpp::Constness::any};

    SECTION("GreedySequence prefers younger expectations.")
    {
        mimicpp::GreedySequence sequence{};

        mimicpp::ScopedExpectation exp1 = mimicpp::detail::make_expectation_builder(collection)
                                       && expect::times(0, 1)
                                       && expect::in_sequence(sequence)
                                       && finally::returns(42);

        mimicpp::ScopedExpectation exp2 = mimicpp::detail::make_expectation_builder(collection)
                                       && expect::times(0, 1)
                                       && expect::in_sequence(sequence)
                                       && finally::returns(1337);

        REQUIRE(1337 == collection->handle_call(call));
    }

    SECTION("LazySequence prefers older expectations.")
    {
        mimicpp::LazySequence sequence{};

        mimicpp::ScopedExpectation exp1 = mimicpp::detail::make_expectation_builder(collection)
                                       && expect::times(0, 1)
                                       && expect::in_sequence(sequence)
                                       && finally::returns(42);

        mimicpp::ScopedExpectation exp2 = mimicpp::detail::make_expectation_builder(collection)
                                       && expect::times(0, 1)
                                       && expect::in_sequence(sequence)
                                       && finally::returns(1337);

        REQUIRE(42 == collection->handle_call(call));
    }
}
