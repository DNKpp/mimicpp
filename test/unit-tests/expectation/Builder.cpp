//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/expectation/Builder.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;
using expectation::policies::detail::TimesConfig;

namespace
{
    template <typename Signature>
    using BaseBuilder = expectation::BasicBuilder<
        false,
        sequence::detail::Config<>,
        Signature,
        expectation_policies::InitFinalize>;

    template <typename Signature>
    [[nodiscard]]
    auto make_builder(expectation::Registry::Ptr registry)
    {
        return BaseBuilder<Signature>{
            std::move(registry),
            reporting::TargetReport{"Test-Mock", reporting::TypeReport::make<Signature>()},
            TimesConfig{},
            sequence::detail::Config<>{},
            expectation_policies::InitFinalize{},
            std::tuple{}
        };
    }

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
    "expectation::BasicBuilder times-limits can be configured.",
    "[expectation][expectation::builder]")
{
    using Signature = void();
    using CallInfo = call::info_for_signature_t<Signature>;

    ScopedReporter reporter{};

    auto const registry = std::make_shared<expectation::Registry>();
    CallInfo const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("It is allowed to omit the times policy.")
    {
        ScopedExpectation const expectation = make_builder<Signature>(registry);

        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(registry->handle_call<Signature>(make_common_target_report<Signature>(), call));
        REQUIRE(expectation.is_satisfied());
    }

    SECTION("Or exchange it once.")
    {
        ScopedExpectation const expectation = make_builder<Signature>(registry)
                                           && TimesConfig{0, 0};

        REQUIRE(expectation.is_satisfied());
    }
}

TEST_CASE(
    "expectation::BasicBuilder sequences can be configured.",
    "[expectation][expectation::builder]")
{
    using Signature = void();
    using CallInfo = call::info_for_signature_t<Signature>;

    ScopedReporter reporter{};

    auto const registry = std::make_shared<expectation::Registry>();
    CallInfo const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    Sequence sequence{};

    SECTION("Can be specified once.")
    {
        ScopedExpectation const expectation = make_builder<Signature>(registry)
                                           && expect::in_sequence(sequence);

        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(registry->handle_call<Signature>(make_common_target_report<Signature>(), call));
        REQUIRE(expectation.is_satisfied());
    }

    SECTION("Can be specified with times.")
    {
        ScopedExpectation const expectation = make_builder<Signature>(registry)
                                           && expect::twice()
                                           && expect::in_sequence(sequence);

        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(registry->handle_call<Signature>(make_common_target_report<Signature>(), call));
        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(registry->handle_call<Signature>(make_common_target_report<Signature>(), call));
        REQUIRE(expectation.is_satisfied());
    }

    SECTION("Can be specified multiple times.")
    {
        Sequence secondSequence{};
        ScopedExpectation const expectation = make_builder<Signature>(registry)
                                           && expect::in_sequence(sequence)
                                           && expect::in_sequence(secondSequence);

        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(registry->handle_call<Signature>(make_common_target_report<Signature>(), call));
        REQUIRE(expectation.is_satisfied());
    }
}

TEST_CASE(
    "Finalize policy of expectation::BasicBuilder for void return may be exchanged.",
    "[expectation][expectation::builder]")
{
    using trompeloeil::_;

    using Signature = void();
    using CallInfo = call::info_for_signature_t<Signature>;

    auto const registry = std::make_shared<expectation::Registry>();
    CallInfo const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("It is allowed to omit the finalize policy.")
    {
        ScopedExpectation expectation = make_builder<Signature>(registry);

        REQUIRE_NOTHROW(expectation.is_satisfied());
        REQUIRE_NOTHROW(registry->handle_call<Signature>(make_common_target_report<Signature>(), call));
    }

    SECTION("Or exchange it once.")
    {
        using FinalizerT = FinalizerMock<Signature>;
        using FinalizerPolicyT = FinalizerFacade<
            Signature,
            std::reference_wrapper<FinalizerMock<Signature>>,
            UnwrapReferenceWrapper>;
        FinalizerT finalizer{};
        ScopedExpectation const expectation = make_builder<Signature>(registry)
                                           && FinalizerPolicyT{std::ref(finalizer)};

        REQUIRE_CALL(finalizer, finalize_call(_))
            .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation);

        REQUIRE_NOTHROW(expectation.is_satisfied());
        REQUIRE_NOTHROW(registry->handle_call<Signature>(make_common_target_report<Signature>(), call));
    }
}

TEST_CASE(
    "Finalize policy of expectation::BasicBuilder for non-void return must be exchanged.",
    "[expectation][expectation::builder]")
{
    using trompeloeil::_;

    using Signature = int();
    using CallInfo = call::info_for_signature_t<Signature>;

    auto const registry = std::make_shared<expectation::Registry>();
    CallInfo const call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    using Finalizer = FinalizerMock<Signature>;
    using FinalizePolicy = FinalizerFacade<
        Signature,
        std::reference_wrapper<FinalizerMock<Signature>>,
        UnwrapReferenceWrapper>;
    Finalizer finalizer{};
    ScopedExpectation const expectation = make_builder<Signature>(registry)
                                       && FinalizePolicy{std::ref(finalizer)};

    REQUIRE_CALL(finalizer, finalize_call(_))
        .LR_WITH(_1.fromSourceLocation == call.fromSourceLocation)
        .RETURN(0);

    REQUIRE_NOTHROW(expectation.is_satisfied());
    REQUIRE_NOTHROW(registry->handle_call<Signature>(make_common_target_report<Signature>(), call));
}

TEST_CASE(
    "expectation::BasicBuilder allows expectation extension via suitable polices.",
    "[expectation][expectation::builder]")
{
    using Signature = void();
    using ExpectationPolicy = PolicyMock<Signature>;
    using Policy = PolicyFacade<Signature, std::reference_wrapper<ExpectationPolicy>, UnwrapReferenceWrapper>;

    auto const registry = std::make_shared<expectation::Registry>();

    SECTION("Just once.")
    {
        ExpectationPolicy policy{};

        // in ExpectationCollection::remove
        REQUIRE_CALL(policy, is_satisfied())
            .RETURN(true);

        ScopedExpectation const expectation = make_builder<Signature>(registry)
                                           && TimesConfig{0, 0}
                                           && Policy{std::ref(policy)};

        REQUIRE_CALL(policy, is_satisfied())
            .RETURN(true);
        REQUIRE(expectation.is_satisfied());
    }

    SECTION("Just twice.")
    {
        ExpectationPolicy policy1{};
        ExpectationPolicy policy2{};

        // in ExpectationCollection::remove
        REQUIRE_CALL(policy1, is_satisfied())
            .RETURN(true);
        REQUIRE_CALL(policy2, is_satisfied())
            .RETURN(true);

        const ScopedExpectation expectation = make_builder<Signature>(registry)
                                           && TimesConfig{0, 0}
                                           && Policy{std::ref(policy1)}
                                           && Policy{std::ref(policy2)};

        REQUIRE_CALL(policy1, is_satisfied())
            .RETURN(true);
        REQUIRE_CALL(policy2, is_satisfied())
            .RETURN(true);
        REQUIRE(expectation.is_satisfied());
    }
}

TEST_CASE(
    "ScopedExpectation forwards source_location to finalize.",
    "[expectation][expectation::builder]")
{
    namespace Matches = Catch::Matchers;

    using Signature = void();

    auto const registry = std::make_shared<expectation::Registry>();

    constexpr util::SourceLocation beforeLoc{};
    ScopedExpectation const expectation = make_builder<Signature>(registry)
                                       && TimesConfig{0, 0};
    constexpr util::SourceLocation afterLoc{};

    REQUIRE_THAT(
        std::string{expectation.from().file_name()},
        Matches::Equals(std::string{beforeLoc.file_name()}));
    REQUIRE_THAT(
        std::string{expectation.from().function_name()},
        Matches::Equals(std::string{beforeLoc.function_name()}));
    REQUIRE(beforeLoc.line() < expectation.from().line());
    REQUIRE(expectation.from().line() < afterLoc.line());
}

TEST_CASE(
    "MIMICPP_SCOPED_EXPECTATION ScopedExpectation with unique name from a builder.",
    "[expectation][expectation::builder]")
{
    using Signature = void();

    ScopedReporter reporter{};

    {
        auto const registry = std::make_shared<expectation::Registry>();

        MIMICPP_SCOPED_EXPECTATION make_builder<Signature>(registry)
            && TimesConfig{0, 0};

        MIMICPP_SCOPED_EXPECTATION make_builder<Signature>(registry)
            && TimesConfig{0, 0};
    }

    REQUIRE_THAT(
        reporter.unfulfilled_expectations(),
        Catch::Matchers::IsEmpty());
}
