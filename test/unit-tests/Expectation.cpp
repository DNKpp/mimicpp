//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationBuilder.hpp"
#include "mimic++/Printing.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "SuppressionMacros.hpp"
#include "TestReporter.hpp"
#include "TestTypes.hpp"

#include <functional>
#include <optional>
#include <ranges>

using namespace mimicpp;

namespace
{
    inline reporting::control_state_t const commonApplicableState = reporting::state_applicable{0, 1, 0};
    inline reporting::control_state_t const commonUnsatisfiedState = reporting::state_applicable{1, 1, 0};
    inline reporting::control_state_t const commonInapplicableState = reporting::state_inapplicable{0, 1, 0, {}, {{sequence::Tag{1337}}}};
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
        .fromConstness = Constness::any};
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
            Catch::Matchers::SizeIs(1u));
        REQUIRE_THAT(
            std::get<1>(reporter.no_match_reports().front()),
            Catch::Matchers::SizeIs(4u));
        REQUIRE_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::IsEmpty());
        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::IsEmpty());
    }
}

TEST_CASE(
    "mimicpp::ExpectationCollection::handle_call does not report matches, when settings::reportSuccess is false.",
    "[expectation]")
{
    using namespace mimicpp::call;
    using StorageT = ExpectationCollection<void()>;
    using CallInfoT = Info<void>;
    using trompeloeil::_;

    ScopedReporter reporter{};
    StorageT storage{};
    auto expectation = std::make_shared<ExpectationMock>();
    storage.push(expectation);

    CallInfoT const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};
    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>()};

    settings::report_success().store(false);

    REQUIRE_CALL(*expectation, matches(_))
        .RETURN(commonMatchingOutcome);
    REQUIRE_CALL(*expectation, is_applicable())
        .RETURN(true);
    REQUIRE_CALL(*expectation, consume(_));
    REQUIRE_CALL(*expectation, finalize_call(_));
    REQUIRE_CALL(*expectation, report())
        .RETURN(expectationReport);
    REQUIRE_NOTHROW(storage.handle_call(make_common_target_report<void()>(), call));
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







TEST_CASE("ScopedExpectation is a non-copyable, but movable type.")
{
    STATIC_REQUIRE(!std::is_copy_constructible_v<mimicpp::ScopedExpectation>);
    STATIC_REQUIRE(!std::is_copy_assignable_v<mimicpp::ScopedExpectation>);

    STATIC_REQUIRE(!std::is_nothrow_destructible_v<mimicpp::ScopedExpectation>);
    STATIC_REQUIRE(!std::is_default_constructible_v<mimicpp::ScopedExpectation>);

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
