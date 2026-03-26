//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/expectation/Owner.hpp"
#include "mimic++/policies/GeneralPolicies.hpp"

#include "SuppressionMacros.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

TEST_CASE(
    "expectation::Owner is a non-copyable, but movable type.",
    "[expectation][expectation::owner]")
{
    STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<expectation::Owner>);
    STATIC_REQUIRE_FALSE(std::is_copy_assignable_v<expectation::Owner>);

    STATIC_REQUIRE_FALSE(std::is_nothrow_destructible_v<expectation::Owner>);
    STATIC_REQUIRE_FALSE(std::is_default_constructible_v<expectation::Owner>);

    // is_nothrow_move_assignable requires the destructor to be noexcept, which it is not for this type.
    STATIC_REQUIRE(std::is_move_constructible_v<expectation::Owner>);
    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<expectation::Owner>);
}

TEST_CASE(
    "expectation::Owner handles expectation lifetime and removes it from the Registry during destruction.",
    "[expectation][expectation::owner]")
{
    using trompeloeil::_;
    using Signature = void();

    constexpr util::SourceLocation loc{};
    auto registry = std::make_shared<expectation::Registry>();

    ControlPolicyMock controlPolicy{};
    auto innerExpectation = registry->create(
        std::in_place_type<Signature>,
        loc,
        reporting::TargetReport{.name = "MyMock", .overloadReport = reporting::TypeReport::make<Signature>()},
        ControlPolicyFacade{std::ref(controlPolicy), UnwrapReferenceWrapper{}},
        expectation_policies::InitFinalize{});

    std::optional<expectation::Owner> expectation{std::in_place, registry, innerExpectation};

    SECTION("When calling is_satisfied()")
    {
        bool const isSatisfied = GENERATE(false, true);
        REQUIRE_CALL(controlPolicy, is_satisfied())
            .RETURN(isSatisfied);
        CHECK(isSatisfied == std::as_const(expectation)->is_satisfied());
    }

    SECTION("When calling is_applicable.")
    {
        auto const [expected, state] = GENERATE((table<bool, reporting::control_state_t>)({
            { true,   reporting::state_applicable{}},
            {false, reporting::state_inapplicable{}},
            {false,    reporting::state_saturated{}},
        }));
        REQUIRE_CALL(controlPolicy, state())
            .RETURN(state);
        CHECK(expected == std::as_const(expectation)->is_applicable());
    }

    SECTION("When calling from()")
    {
        CHECK(loc == std::as_const(expectation)->from());
    }

    SECTION("When calling mock_name()")
    {
        CHECK_THAT(
            std::as_const(expectation)->mock_name(),
            Catch::Matchers::Equals("MyMock"));
    }

    SECTION("When ScopedExpectation is moved.")
    {
        ScopedExpectation otherExpectation = *std::move(expectation);

        REQUIRE_CALL(controlPolicy, is_satisfied())
            .RETURN(false);
        CHECK(false == std::as_const(otherExpectation).is_satisfied());

        SECTION("And then move assigned.")
        {
            expectation = std::move(otherExpectation);

            REQUIRE_CALL(controlPolicy, is_satisfied())
                .RETURN(true);
            CHECK(true == std::as_const(expectation)->is_satisfied());

            // just move back, so we can unify the cleanup process
            otherExpectation = *std::move(expectation);
        }

        SECTION("And then self move assigned.")
        {
            START_WARNING_SUPPRESSION
            SUPPRESS_SELF_MOVE
            otherExpectation = std::move(otherExpectation);
            STOP_WARNING_SUPPRESSION

            REQUIRE_CALL(controlPolicy, is_satisfied())
                .RETURN(true);
            CHECK(true == std::as_const(otherExpectation).is_satisfied());
        }

        // just move back, so we can unify the cleanup process
        expectation = std::move(otherExpectation);
    }

    // indirectly via remove
    REQUIRE_CALL(controlPolicy, is_satisfied())
        .RETURN(true);
    expectation.reset();
}

