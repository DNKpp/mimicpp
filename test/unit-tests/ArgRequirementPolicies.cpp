// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ExpectationPolicies.hpp"
#include "mimic++/Mock.hpp"

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

    using ArgSelectorT = InvocableMock<std::tuple<int&>, const CallInfoT&>;
    using ProjectionT = InvocableMock<int&, int&>;
    using ApplyStrategyT = expectation_policies::arg_list_apply<std::reference_wrapper<ProjectionT>>;
    using DescriberT = InvocableMock<StringT, StringViewT>;
    using MatcherT = MatcherMock<int&>;
    STATIC_CHECK(matcher_for<MatcherT, int&>);

    MatcherT matcher{};
    ArgSelectorT argSelector{};
    ProjectionT projection{};
    DescriberT describer{};
    expectation_policies::ArgsRequirement<
        std::reference_wrapper<ArgSelectorT>,
        ApplyStrategyT,
        MatcherFacade<std::reference_wrapper<MatcherT>, UnwrapReferenceWrapper>,
        std::reference_wrapper<DescriberT>>
        policy{
            MatcherFacade{std::ref(matcher), UnwrapReferenceWrapper{}},
            describer,
            argSelector,
            ApplyStrategyT{std::make_tuple(std::ref(projection))}
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

    SECTION("Testing matches.")
    {
        REQUIRE_CALL(argSelector, Invoke(_))
            .LR_WITH(&_1 == &info)
            .RETURN(_1.args);
        REQUIRE_CALL(projection, Invoke(_))
            .LR_WITH(&_1 == &arg0)
            .LR_RETURN(std::ref(arg0));
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
    "expectation_policies::ArgsRequirement checks whether the given call::Info matches.",
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
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
    };

    using Projection0T = InvocableMock<int&, int&>;
    using Projection1T = InvocableMock<const std::string&, const std::string&>;
    using Projection2T = InvocableMock<double&, double&>;
    using ApplyStrategyT = expectation_policies::arg_list_apply<
        std::reference_wrapper<Projection0T>,
        std::reference_wrapper<Projection1T>,
        std::reference_wrapper<Projection2T>>;
    using DescriberT = InvocableMock<StringT, StringViewT>;
    using MatcherT = VariadicMatcherMock<int&, const std::string&, double&>;
    STATIC_CHECK(matcher_for<MatcherT, int&, const std::string&, double&>);

    using ArgSelectorT = InvocableMock< // let's just extract the args as they are
        std::tuple<int&, const std::string&, double&>,
        const CallInfoT&>;
    Projection0T projection0{};
    Projection1T projection1{};
    Projection2T projection2{};
    ArgSelectorT argSelector{};
    DescriberT describer{};
    MatcherT matcher{};
    expectation_policies::ArgsRequirement<
        std::reference_wrapper<ArgSelectorT>,
        ApplyStrategyT,
        MatcherFacade<std::reference_wrapper<MatcherT>, UnwrapReferenceWrapper>,
        std::reference_wrapper<DescriberT>>
        policy{
            MatcherFacade{std::ref(matcher), UnwrapReferenceWrapper{}},
            describer,
            argSelector,
            ApplyStrategyT{std::make_tuple(std::ref(projection0), std::ref(projection1), std::ref(projection2))}
    };

    STATIC_REQUIRE(expectation_policy_for<decltype(policy), SignatureT>);

    REQUIRE(std::as_const(policy).is_satisfied());
    REQUIRE_NOTHROW(policy.consume(info));

    SECTION("Policy description.")
    {
        SCOPED_EXP matcher.describe.expect_call()
            and finally::returns<StringT>("matcher description");
        REQUIRE_CALL(describer, Invoke("matcher description"))
            .RETURN("expect that: matcher description");

        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals("expect that: matcher description"));
    }

    SECTION("Testing matches.")
    {
        REQUIRE_CALL(argSelector, Invoke(_))
            .LR_WITH(&_1 == &info)
            .RETURN(_1.args);
        REQUIRE_CALL(projection0, Invoke(_))
            .LR_WITH(&_1 == &arg0)
            .LR_RETURN(std::ref(_1));
        REQUIRE_CALL(projection1, Invoke(_))
            .LR_WITH(&_1 == &arg1)
            .LR_RETURN(std::ref(_1));
        REQUIRE_CALL(projection2, Invoke(_))
            .LR_WITH(&_1 == &arg2)
            .LR_RETURN(std::ref(_1));

        const bool expected = GENERATE(true, false);
        using matches::instance;
        SCOPED_EXP matcher.matches.expect_call(instance(arg0), instance(arg1), instance(arg2))
            and finally::returns(expected);

        REQUIRE(expected == std::as_const(policy).matches(info));
    }
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
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

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
