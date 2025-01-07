//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ExpectationBuilder.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

namespace
{
    template <typename Signature>
    using BaseBuilderT = BasicExpectationBuilder<
        false,
        sequence::detail::Config<>,
        Signature,
        expectation_policies::InitFinalize>;

    template <typename Signature>
    constexpr auto make_builder(
        std::shared_ptr<ExpectationCollection<Signature>> collection)
    {
        return BaseBuilderT<Signature>{
            std::move(collection),
            detail::TimesConfig{},
            sequence::detail::Config<>{},
            expectation_policies::InitFinalize{},
            std::tuple{}};
    }
}

TEMPLATE_TEST_CASE(
    "Times of mimicpp::BasicExpectationBuilder can be exchanged only once.",
    "[expectation][expectation::builder]",
    BaseBuilderT<void()>)
{
    using BuilderT = TestType;

    STATIC_REQUIRE(requires { std::declval<BuilderT&&>() && detail::TimesConfig{}; });
    STATIC_REQUIRE(!requires { std::declval<BuilderT&&>() && detail::TimesConfig{} && detail::TimesConfig{}; });
}

TEST_CASE(
    "BasicExpectationBuilder times-limits can be configured.",
    "[expectation][expectation::builder]")
{
    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    ScopedReporter reporter{};

    auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
    const CallInfoT call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SECTION("It is allowed to omit the times policy.")
    {
        const ScopedExpectation expectation = make_builder(collection);

        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(collection->handle_call(call));
        REQUIRE(expectation.is_satisfied());
    }

    SECTION("Or exchange it once.")
    {
        const ScopedExpectation expectation = make_builder(collection)
                                           && detail::TimesConfig{0, 0};

        REQUIRE(expectation.is_satisfied());
    }
}

TEST_CASE(
    "BasicExpectationBuilder sequences can be configured.",
    "[expectation][expectation::builder]")
{
    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    ScopedReporter reporter{};

    auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
    const CallInfoT call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    SequenceT sequence{};

    SECTION("Can be specified once.")
    {
        const ScopedExpectation expectation = make_builder(collection)
                                           && expect::in_sequence(sequence);

        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(collection->handle_call(call));
        REQUIRE(expectation.is_satisfied());
    }

    SECTION("Can be specified with times.")
    {
        const ScopedExpectation expectation = make_builder(collection)
                                           && expect::twice()
                                           && expect::in_sequence(sequence);

        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(collection->handle_call(call));
        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(collection->handle_call(call));
        REQUIRE(expectation.is_satisfied());
    }

    SECTION("Can be specified multiple times.")
    {
        SequenceT secondSequence{};
        const ScopedExpectation expectation = make_builder(collection)
                                           && expect::in_sequence(sequence)
                                           && expect::in_sequence(secondSequence);

        REQUIRE(!expectation.is_satisfied());
        REQUIRE_NOTHROW(collection->handle_call(call));
        REQUIRE(expectation.is_satisfied());
    }
}

TEMPLATE_TEST_CASE(
    "Finalize policy of mimicpp::BasicExpectationBuilder can be exchanged only once.",
    "[expectation][expectation::builder]",
    BaseBuilderT<void()>)
{
    using BuilderT = TestType;
    using FinalizerT = FinalizerFake<void()>;

    STATIC_REQUIRE(requires { std::declval<BuilderT&&>() && FinalizerT{}; });
    STATIC_REQUIRE(!requires { std::declval<BuilderT&&>() && FinalizerT{} && FinalizerT{}; });
}

TEST_CASE(
    "Finalize policy of mimicpp::BasicExpectationBuilder for void return may be exchanged.",
    "[expectation][expectation::builder]")
{
    using trompeloeil::_;

    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
    const CallInfoT call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any,
        .fromSourceLocation = std::source_location::current()};

    SECTION("It is allowed to omit the finalize policy.")
    {
        ScopedExpectation expectation = make_builder(collection);

        REQUIRE_NOTHROW(expectation.is_satisfied());
        REQUIRE_NOTHROW(collection->handle_call(call));
    }

    SECTION("Or exchange it once.")
    {
        using FinalizerT = FinalizerMock<SignatureT>;
        using FinalizerPolicyT = FinalizerFacade<
            SignatureT,
            std::reference_wrapper<FinalizerMock<SignatureT>>,
            UnwrapReferenceWrapper>;
        FinalizerT finalizer{};
        const ScopedExpectation expectation = make_builder(collection)
                                           && FinalizerPolicyT{std::ref(finalizer)};

        REQUIRE_CALL(finalizer, finalize_call(_))
            .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation));

        REQUIRE_NOTHROW(expectation.is_satisfied());
        REQUIRE_NOTHROW(collection->handle_call(call));
    }
}

TEST_CASE(
    "Finalize policy of mimicpp::BasicExpectationBuilder for non-void return must be exchanged.",
    "[expectation][expectation::builder]")
{
    using trompeloeil::_;

    using SignatureT = int();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
    const CallInfoT call{
        .args = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any,
        .fromSourceLocation = std::source_location::current()};

    using FinalizerT = FinalizerMock<SignatureT>;
    using FinalizerPolicyT = FinalizerFacade<
        SignatureT,
        std::reference_wrapper<FinalizerMock<SignatureT>>,
        UnwrapReferenceWrapper>;
    FinalizerT finalizer{};
    const ScopedExpectation expectation = make_builder(collection)
                                       && FinalizerPolicyT{std::ref(finalizer)};

    REQUIRE_CALL(finalizer, finalize_call(_))
        .LR_WITH(is_same_source_location(_1.fromSourceLocation, call.fromSourceLocation))
        .RETURN(0);

    REQUIRE_NOTHROW(expectation.is_satisfied());
    REQUIRE_NOTHROW(collection->handle_call(call));
}

TEST_CASE(
    "mimicpp::BasicExpectationBuilder allows expectation extension via suitable polices.",
    "[expectation][expectation::builder]")
{
    using SignatureT = void();
    using ExpectationPolicyT = PolicyMock<SignatureT>;
    using PolicyT = PolicyFacade<SignatureT, std::reference_wrapper<ExpectationPolicyT>, UnwrapReferenceWrapper>;

    auto collection = std::make_shared<ExpectationCollection<SignatureT>>();

    SECTION("Just once.")
    {
        ExpectationPolicyT policy{};

        // in ExpectationCollection::remove
        REQUIRE_CALL(policy, is_satisfied())
            .RETURN(true);

        const ScopedExpectation expectation = make_builder(collection)
                                           && detail::TimesConfig{0, 0}
                                           && PolicyT{std::ref(policy)};

        REQUIRE_CALL(policy, is_satisfied())
            .RETURN(true);
        REQUIRE(expectation.is_satisfied());
    }

    SECTION("Just twice.")
    {
        ExpectationPolicyT policy1{};
        ExpectationPolicyT policy2{};

        // in ExpectationCollection::remove
        REQUIRE_CALL(policy1, is_satisfied())
            .RETURN(true);
        REQUIRE_CALL(policy2, is_satisfied())
            .RETURN(true);

        const ScopedExpectation expectation = make_builder(collection)
                                           && detail::TimesConfig{0, 0}
                                           && PolicyT{std::ref(policy1)}
                                           && PolicyT{std::ref(policy2)};

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

    using SignatureT = void();

    const auto collection = std::make_shared<ExpectationCollection<SignatureT>>();

    const std::source_location beforeLoc = std::source_location::current();
    const ScopedExpectation expectation = make_builder(collection)
                                       && detail::TimesConfig{0, 0};
    const std::source_location afterLoc = std::source_location::current();

    REQUIRE_THAT(
        expectation.from().file_name(),
        Matches::Equals(beforeLoc.file_name()));
    REQUIRE_THAT(
        expectation.from().function_name(),
        Matches::Equals(beforeLoc.function_name()));
    REQUIRE(beforeLoc.line() < expectation.from().line());
    REQUIRE(expectation.from().line() < afterLoc.line());
}

TEST_CASE(
    "MIMICPP_SCOPED_EXPECTATION ScopedExpectation with unique name from a builder.",
    "[expectation][expectation::builder]")
{
    using SignatureT = void();

    ScopedReporter reporter{};

    {
        const auto collection = std::make_shared<ExpectationCollection<SignatureT>>();

        MIMICPP_SCOPED_EXPECTATION make_builder(collection)
            && detail::TimesConfig{0, 0};

        MIMICPP_SCOPED_EXPECTATION make_builder(collection)
            && detail::TimesConfig{0, 0};
    }

    REQUIRE_THAT(
        reporter.unfulfilled_expectations(),
        Catch::Matchers::IsEmpty());
}
