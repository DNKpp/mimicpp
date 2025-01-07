//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/policies/ArgRequirementPolicies.hpp"
#include "mimic++/Expectation.hpp"
#include "mimic++/Mock.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

TEST_CASE(
    "expect::detail::arg_requirement_describer generates a description.",
    "[detail][expectation][expectation::policy]")
{
    const auto matcherDescription = GENERATE(
        as<StringT>{},
        "",
        "Hello, World!");

    SECTION("When single index is given.")
    {
        constexpr expect::detail::arg_requirement_describer<42> describer{};
        REQUIRE_THAT(
            describer(matcherDescription),
            Catch::Matchers::Equals("expect: arg[42] " + matcherDescription));
    }

    SECTION("When two indices are given.")
    {
        constexpr expect::detail::arg_requirement_describer<42, 1337> describer{};
        REQUIRE_THAT(
            describer(matcherDescription),
            Catch::Matchers::Equals("expect: arg[42, 1337] " + matcherDescription));
    }

    SECTION("When three indices are given.")
    {
        constexpr expect::detail::arg_requirement_describer<42, 1337, 0> describer{};
        REQUIRE_THAT(
            describer(matcherDescription),
            Catch::Matchers::Equals("expect: arg[42, 1337, 0] " + matcherDescription));
    }
}

TEST_CASE(
    "expect::detail::all_args_requirement_describer generates a description.",
    "[detail][expectation][expectation::policy]")
{
    const auto matcherDescription = GENERATE(
        as<StringT>{},
        "",
        "Hello, World!");

    constexpr expect::detail::all_args_requirement_describer describer{};
    REQUIRE_THAT(
        describer(matcherDescription),
        Catch::Matchers::Equals("expect: arg[all] " + matcherDescription));
}

TEST_CASE(
    "expectation_policies::ArgsRequirement checks whether the given call::Info matches.",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;
    namespace Matches = Catch::Matchers;

    using SignatureT = void(int);
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    int arg0{42};
    const CallInfoT info{
        .args = {arg0},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    using DescriberStrategyT = InvocableMock<StringT, StringViewT>;
    using MatcherT = MatcherMock<int&>;
    using MatchesStrategyT = InvocableMock<
        bool,
        expectation_policies::matcher_matches_fn<MatcherFacade<std::reference_wrapper<MatcherT>, UnwrapReferenceWrapper>>,
        const CallInfoT&>;
    STATIC_CHECK(matcher_for<MatcherT, int&>);

    MatcherT matcher{};
    MatchesStrategyT matchesStrategy{};
    DescriberStrategyT describeStrategy{};
    expectation_policies::ArgsRequirement policy{
        MatcherFacade{std::ref(matcher), UnwrapReferenceWrapper{}},
        std::ref(matchesStrategy),
        std::ref(describeStrategy)
    };

    STATIC_REQUIRE(expectation_policy_for<decltype(policy), SignatureT>);

    REQUIRE(std::as_const(policy).is_satisfied());
    REQUIRE_NOTHROW(policy.consume(info));

    SECTION("Policy description.")
    {
        REQUIRE_CALL(matcher, describe())
            .RETURN("matcher description");
        REQUIRE_CALL(describeStrategy, Invoke("matcher description"))
            .RETURN("expect that: matcher description");

        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals("expect that: matcher description"));
    }

    SECTION("Testing matches.")
    {
        REQUIRE_CALL(matchesStrategy, Invoke(_, _))
            .LR_WITH(&_2 == &info)
            .LR_RETURN(_1.matcher.matches(arg0));
        const bool expected = GENERATE(true, false);
        REQUIRE_CALL(matcher, matches(_))
            .LR_WITH(&_1 == &arg0)
            .RETURN(expected);

        REQUIRE(expected == std::as_const(policy).matches(info));
    }
}

namespace
{
    // using mimic++ mock here, because not possible with trompeloeil
    template <typename... Args>
    class VariadicMatcherMock
    {
    public:
        Mock<bool(Args...) const> matches{};
        Mock<StringT() const> describe{};
    };
}

TEST_CASE(
    "expectation_policies::ArgsRequirement supports variadic arguments.",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;
    namespace Matches = Catch::Matchers;

    using SignatureT = void(int, const std::string&, double);
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    int arg0{42};
    const std::string arg1{"Hello, World!"};
    double arg2{1337.};
    const CallInfoT info{
        .args = {arg0, arg1, arg2},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))
    };

    using MatcherT = VariadicMatcherMock<int&, const std::string&, double&>;
    STATIC_CHECK(matcher_for<MatcherT, int&, const std::string&, double&>);
    MatcherT matcher{};

    expectation_policies::ArgsRequirement policy{
        MatcherFacade{std::ref(matcher), UnwrapReferenceWrapper{}},
        detail::apply_args_fn{
                      detail::all_args_selector_fn<std::add_lvalue_reference_t>{},
                      detail::arg_list_forward_apply_fn{}},
        expect::detail::arg_requirement_describer<0u, 1u, 2u>{}
    };

    STATIC_REQUIRE(expectation_policy_for<decltype(policy), SignatureT>);

    SECTION("Testing matches.")
    {
        const bool expected = GENERATE(true, false);
        using matches::instance;
        SCOPED_EXP matcher.matches.expect_call(instance(arg0), instance(arg1), instance(arg2))
            and finally::returns(expected);

        REQUIRE(expected == std::as_const(policy).matches(info));
    }
}

TEST_CASE(
    "expect::arg creates an expectation_policies::ArgRequirement policy.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;
    namespace Matches = Catch::Matchers;

    using SignatureT = void(int);
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    int arg0{42};
    const CallInfoT info{
        .args = {arg0},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    using MatcherT = MatcherMock<int&>;
    STATIC_CHECK(matcher_for<MatcherT, int&>);
    MatcherT matcher{};

    expectation_policies::ArgsRequirement policy = expect::arg<0>(
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
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    using MatcherT = MatcherMock<const std::string&>;
    STATIC_CHECK(matcher_for<MatcherT, std::string>);
    MatcherT matcher{};
    using ProjectionT = InvocableMock<std::string, int&>;

    ProjectionT projection{};

    expectation_policies::ArgsRequirement policy = expect::arg<0>(
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
    "expect::all_args creates an expectation_policies::ArgRequirement policy.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;
    namespace Matches = Catch::Matchers;

    using SignatureT = void(int, std::string);
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    int arg0{42};
    std::string arg1{"Test"};
    const CallInfoT info{
        .args = {arg0, arg1},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))
    };

    using MatcherT = VariadicMatcherMock<int&, std::string&>;
    STATIC_CHECK(matcher_for<MatcherT, int&, std::string&>);
    MatcherT matcher{};

    expectation_policies::ArgsRequirement policy = expect::all_args(
        MatcherFacade{
            std::ref(matcher),
            UnwrapReferenceWrapper{}});
    STATIC_REQUIRE(expectation_policy_for<decltype(policy), SignatureT>);

    REQUIRE(std::as_const(policy).is_satisfied());
    REQUIRE_NOTHROW(policy.consume(info));

    SECTION("Policy description.")
    {
        SCOPED_EXP std::as_const(matcher).describe.expect_call()
            and finally::returns<std::string>("matcher description");

        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals("expect: arg[all] matcher description"));
    }

    SECTION("Policy matches().")
    {
        const bool match = GENERATE(true, false);
        SCOPED_EXP std::as_const(matcher).matches.expect_call(matches::instance(arg0), matches::instance(arg1))
            and finally::returns(match);

        REQUIRE(match == std::as_const(policy).matches(info));
    }
}
