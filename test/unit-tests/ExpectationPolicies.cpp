// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ExpectationPolicies.hpp"
#include "mimic++/Mock.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "Given types satisfy mimicpp::finalize_policy_for concept.",
    "[expectation][expectation::builder]",
    ((bool dummy, typename Policy, typename Sig), dummy, Policy, Sig),
    (true, expectation_policies::InitFinalize, void()),
    (true, expectation_policies::InitFinalize, void(int)),
    (true, expectation_policies::Throws<std::runtime_error>, void()),
    (true, expectation_policies::Throws<std::runtime_error>, void(int)),
    (true, expectation_policies::Throws<std::runtime_error>, double(int)))
{
    STATIC_REQUIRE(mimicpp::finalize_policy_for<Policy, Sig>);
}

TEMPLATE_TEST_CASE(
    "Given types satisfy mimicpp::expectation_policy_for concept.",
    "[expectation][expectation::builder]",
    expectation_policies::Category<ValueCategory::lvalue>,
    expectation_policies::Category<ValueCategory::rvalue>,
    expectation_policies::Category<ValueCategory::any>,
    expectation_policies::Constness<Constness::as_const>,
    expectation_policies::Constness<Constness::non_const>,
    expectation_policies::Constness<Constness::any>)
{
    STATIC_REQUIRE(mimicpp::expectation_policy_for<TestType, void()>);
}

TEST_CASE(
    "expectation_policies::InitFinalize does nothing.",
    "[expectation][expectation::builder]")
{
    const call::Info<void> call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    constexpr expectation_policies::InitFinalize policy{};

    REQUIRE_NOTHROW(policy.finalize_call(call));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::Category checks whether the given call::Info matches.",
    "[expectation][expectation::policy]",
    ((ValueCategory category), category),
    (ValueCategory::lvalue),
    (ValueCategory::rvalue),
    (ValueCategory::any))
{
    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    using PolicyT = expectation_policies::Category<category>;
    STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

    constexpr PolicyT policy{};
    SECTION("Policy is always satisfied.")
    {
        REQUIRE(policy.is_satisfied());
    }

    SECTION("Policy description.")
    {
        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals(format::format("expect: from {} category overload", category)));
    }

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    if (is_matching(call.fromCategory, category))
    {
        SECTION("When call and policy category matches, success is returned.")
        {
            REQUIRE(policy.matches(call));
        }

        SECTION("Policy doesn't consume, but asserts on wrong category.")
        {
            REQUIRE_NOTHROW(policy.consume(call));
        }
    }
    else
    {
        SECTION("When call and policy category mismatch, failure is returned.")
        {
            REQUIRE(!policy.matches(call));
        }
    }
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::Constness checks whether the given call::Info matches.",
    "[expectation][expectation::policy]",
    ((Constness constness), constness),
    (Constness::as_const),
    (Constness::non_const),
    (Constness::any))
{
    using SignatureT = void();
    using CallInfoT = call::info_for_signature_t<SignatureT>;
    using PolicyT = expectation_policies::Constness<constness>;
    STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

    constexpr PolicyT policy{};
    SECTION("Policy is always satisfied.")
    {
        REQUIRE(policy.is_satisfied());
    }

    SECTION("Policy description.")
    {
        REQUIRE_THAT(
            policy.describe(),
            Catch::Matchers::Equals(format::format("expect: from {} qualified overload", constness)));
    }

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    if (is_matching(call.fromConstness, constness))
    {
        SECTION("When call and policy constness matches, success is returned.")
        {
            REQUIRE(policy.matches(call));
        }

        SECTION("Policy doesn't consume, but asserts on wrong constness.")
        {
            REQUIRE_NOTHROW(policy.consume(call));
        }
    }
    else
    {
        SECTION("When call and policy constness mismatch, failure is returned.")
        {
            REQUIRE(!policy.matches(call));
        }
    }
}

TEST_CASE(
    "expectation_policies::ReturnsResultOf forwards the invocation result during finalize_call().",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;

    using SignatureT = int&();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    int value{42};
    InvocableMock<int&, const CallInfoT&> action{};
    expectation_policies::ReturnsResultOf policy{std::ref(action)};
    STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

    REQUIRE_CALL(action, Invoke(_))
        .LR_WITH(&call == &_1)
        .LR_RETURN(value);
    int& result = policy.finalize_call(call);
    REQUIRE(&value == &result);
}

TEST_CASE(
    "expectation_policies::Throws always throws an exception during finalize_call().",
    "[expectation][expectation::policy]")
{
    struct test_exception
    {
    };

    SECTION("When void is returned.")
    {
        using SignatureT = void();
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        using PolicyT = expectation_policies::Throws<test_exception>;
        STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        PolicyT policy{test_exception{}};

        REQUIRE_THROWS_AS(
            policy.finalize_call(call),
            test_exception);
    }

    SECTION("When non-void is returned.")
    {
        using SignatureT = int && ();
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        using PolicyT = expectation_policies::Throws<test_exception>;
        STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        PolicyT policy{test_exception{}};

        REQUIRE_THROWS_AS(
            policy.finalize_call(call),
            test_exception);
    }
}

TEST_CASE(
    "Invalid configurations of expectation_policies::SideEffectAction do not satisfy expectation_policy_for concept.",
    "[expectation][expectation::policy]")
{
    using ActionT = InvocableMock<void, int>;

    SECTION("When action is not applicable.")
    {
        STATIC_REQUIRE(
            !expectation_policy_for<
                expectation_policies::SideEffectAction<ActionT>,
                void(int)>);
    }
}

TEST_CASE(
    "expectation_policies::SideEffectAction invokes the specified function on consume.",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;

    const call::Info<void> info{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    InvocableMock<void, const call::Info<void>&> action{};
    expectation_policies::SideEffectAction policy{std::ref(action)};
    STATIC_REQUIRE(expectation_policy_for<decltype(policy), void()>);
    REQUIRE(std::as_const(policy).is_satisfied());
    REQUIRE(std::as_const(policy).matches(info));
    REQUIRE(std::optional<StringT>{} == std::as_const(policy).describe());

    REQUIRE_CALL(action, Invoke(_))
        .LR_WITH(&info == &_1);
    REQUIRE_NOTHROW(policy.consume(info));
}

namespace
{
    inline constexpr std::array refQualifiers{
        ValueCategory::lvalue,
        ValueCategory::rvalue,
        ValueCategory::any};
    inline constexpr std::array constQualifiers{
        Constness::non_const,
        Constness::as_const,
        Constness::any};

    template <typename... Ts>
    [[nodiscard]]
    constexpr std::tuple<Ts&...> make_ref_tuple(std::tuple<Ts...>& values) noexcept
    {
        return std::apply(
            [](auto&... elements) { return std::tuple{std::ref(elements)...}; },
            values);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::all_args_selector_fn returns all arg-references as tuple.",
    "[expectation][expectation::policy]",
    ((bool dummy, typename Expected, template <typename> typename Projection, typename... Args), dummy, Expected, Projection, Args...),
    (true, std::tuple<int&>, std::add_lvalue_reference_t, int),
    (true, std::tuple<int&>, std::add_lvalue_reference_t, int&),
    (true, std::tuple<const int&>, std::add_lvalue_reference_t, const int&),
    (true, std::tuple<int&>, std::add_lvalue_reference_t, int&&),
    (true, std::tuple<const int&>, std::add_lvalue_reference_t, const int&&),

    (true, std::tuple<int&&>, std::add_rvalue_reference_t, int),
    (true, std::tuple<int&>, std::add_rvalue_reference_t, int&),
    (true, std::tuple<const int&>, std::add_rvalue_reference_t, const int&),
    (true, std::tuple<int&&>, std::add_rvalue_reference_t, int&&),
    (true, std::tuple<const int&&>, std::add_rvalue_reference_t, const int&&),

    (true, std::tuple<float&, int&, double&>, std::add_lvalue_reference_t, float, int, double),
    (true, std::tuple<float&, int&, double&>, std::add_lvalue_reference_t, float&, int&, double&),
    (true, std::tuple<const float&, const int&, const double&>, std::add_lvalue_reference_t, const float&, const int&, const double&),
    (true, std::tuple<float&, int&, double&>, std::add_lvalue_reference_t, float&&, int&&, double&&),
    (true, std::tuple<const float&, const int&, const double&>, std::add_lvalue_reference_t, const float&&, const int&&, const double&&),

    (true, std::tuple<float&&, int&&, double&&>, std::add_rvalue_reference_t, float, int, double),
    (true, std::tuple<float&, int&, double&>, std::add_rvalue_reference_t, float&, int&, double&),
    (true, std::tuple<const float&, const int&, const double&>, std::add_rvalue_reference_t, const float&, const int&, const double&),
    (true, std::tuple<float&&, int&&, double&&>, std::add_rvalue_reference_t, float&&, int&&, double&&),
    (true, std::tuple<const float&&, const int&&, const double&&>, std::add_rvalue_reference_t, const float&&, const int&&, const double&&))
{
    std::tuple<std::remove_cvref_t<Args>...> values{};
    using CallInfoT = call::Info<void, Args...>;
    const CallInfoT callInfo{
        .args = make_ref_tuple(values),
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    const expectation_policies::all_args_selector_fn<Projection> selector{};
    const std::tuple result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<const Expected, decltype(result)>);
    const auto same_addresses = [&]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>) {
        return (... && (&std::get<indices>(values) == &std::get<indices>(result)));
    };
    REQUIRE(same_addresses(std::index_sequence_for<Args...>{}));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::args_selector_fn returns selected arg-references as tuple.",
    "[expectation][expectation::policy]",
    ((typename Expected, template <typename> typename Projection, std::size_t index, typename... Args), Expected, Projection, index, Args...),
    (std::tuple<int&>, std::add_lvalue_reference_t, 0u, int),
    (std::tuple<int&>, std::add_lvalue_reference_t, 0u, int&),
    (std::tuple<const int&>, std::add_lvalue_reference_t, 0u, const int&),
    (std::tuple<int&>, std::add_lvalue_reference_t, 0u, int&&),
    (std::tuple<const int&>, std::add_lvalue_reference_t, 0u, const int&&),

    (std::tuple<int&&>, std::add_rvalue_reference_t, 0u, int),
    (std::tuple<int&>, std::add_rvalue_reference_t, 0u, int&),
    (std::tuple<const int&>, std::add_rvalue_reference_t, 0u, const int&),
    (std::tuple<int&&>, std::add_rvalue_reference_t, 0u, int&&),
    (std::tuple<const int&&>, std::add_rvalue_reference_t, 0u, const int&&),

    (std::tuple<int&>, std::add_lvalue_reference_t, 1u, float, int, double),
    (std::tuple<int&>, std::add_lvalue_reference_t, 1u, float&, int&, double&),
    (std::tuple<const int&>, std::add_lvalue_reference_t, 1u, const float&, const int&, const double&),
    (std::tuple<int&>, std::add_lvalue_reference_t, 1u, float&&, int&&, double&&),
    (std::tuple<const int&>, std::add_lvalue_reference_t, 1u, const float&&, const int&&, const double&&),

    (std::tuple<int&&>, std::add_rvalue_reference_t, 1u, float, int, double),
    (std::tuple<int&>, std::add_rvalue_reference_t, 1u, float&, int&, double&),
    (std::tuple<const int&>, std::add_rvalue_reference_t, 1u, const float&, const int&, const double&),
    (std::tuple<int&&>, std::add_rvalue_reference_t, 1u, float&&, int&&, double&&),
    (std::tuple<const int&&>, std::add_rvalue_reference_t, 1u, const float&&, const int&&, const double&&))
{
    std::tuple<std::remove_cvref_t<Args>...> values{};
    using CallInfoT = call::Info<void, Args...>;
    const CallInfoT callInfo{
        .args = make_ref_tuple(values),
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    const expectation_policies::args_selector_fn<Projection, std::index_sequence<index>> selector{};
    const std::tuple result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<const Expected, decltype(result)>);
    REQUIRE(&std::get<0>(result) == &std::get<index>(values));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::args_selector_fn supports arbitrary selection-sequence.",
    "[expectation][expectation::policy]",
    ((typename Expected, std::size_t... indices), Expected, indices...),
    (std::tuple<float&>, 0u),
    (std::tuple<float&, float&>, 0u, 0u),
    (std::tuple<double&, int&, double&>, 2u, 1u, 2u),
    (std::tuple<int&, float&>, 1u, 0u))
{
    std::tuple<float, int, double> values{};
    using CallInfoT = call::Info<void, float&, int&, double&>;
    const CallInfoT callInfo{
        .args = make_ref_tuple(values),
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    const expectation_policies::args_selector_fn<std::add_lvalue_reference_t, std::index_sequence<indices...>> selector{};
    const std::tuple result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<const Expected, decltype(result)>);
    REQUIRE(
        std::apply(
            [&](auto&... elements) { return (... && (&elements == &std::get<indices>(values))); },
            result));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::arg_list_forward_apply_fn forwards all elements to the given function.",
    "[expectation][expectation::policy]",
    ((bool dummy, typename... Args), dummy, Args...),
    (true, int&),
    (true, const int&),
    (true, int&&),
    (true, const int&&),

    (true, float&, int&, double&),
    (true, const float&, const int&, const double&),
    (true, float&&, int&&, double&&),
    (true, const float&&, const int&&, const double&&))
{
    std::tuple<std::remove_cvref_t<Args>...> values{};
    const auto action = [&]<typename... Ts>(Ts&&... elements) {
        STATIC_REQUIRE((... && std::same_as<Ts&&, Args>));

        const auto same_addresses = [&]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>) {
            return (... && (&elements == &std::get<indices>(values)));
        };
        REQUIRE(same_addresses(std::index_sequence_for<Args...>{}));
    };

    constexpr expectation_policies::arg_list_forward_apply_fn forwarder{};
    std::tuple refs = std::apply(
        [](auto&... elements) { return std::tuple<Args...>{static_cast<Args>(elements)...}; },
        values);
    std::invoke(
        forwarder,
        action,
        std::move(refs));
}

TEMPLATE_TEST_CASE(
    "expectation_policies::arg_list_forward_apply_fn forwards the returned value.",
    "[expectation][expectation::policy]",
    int,
    // const int, never return a const value
    int&,
    const int&,
    int&&,
    const int&&)
{
    int value{1337};
    constexpr expectation_policies::arg_list_forward_apply_fn forwarder{};

    const auto action = [](int& v) -> TestType {
        return static_cast<TestType>(v);
    };

    const auto make_refs = [&] { return std::make_tuple(std::ref(value)); };

    STATIC_REQUIRE(std::same_as<TestType, decltype(forwarder(action, make_refs()))>);
    REQUIRE(1337 == forwarder(action, make_refs()));
    CHECKED_IF(std::is_reference_v<TestType>)
    {
        auto&& r = forwarder(action, make_refs());
        REQUIRE(&value == &r);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::arg_list_indirect_apply_fn forwards all projected elements to the given function.",
    "[expectation][expectation::policy]",
    ((bool dummy, typename... Args), dummy, Args...),
    (true, int&),
    (true, const int&),
    (true, int&&),
    (true, const int&&),

    (true, float&, int&, double&),
    (true, const float&, const int&, const double&),
    (true, float&&, int&&, double&&),
    (true, const float&&, const int&&, const double&&))
{
    std::tuple<std::remove_cvref_t<Args>...> values{};
    int invokeCount{};
    const auto projection = [&](auto&& value) {
        ++invokeCount;
        return std::ref(value);
    };
    const auto action = [&]<typename... Ts>(Ts... elements) {
        STATIC_REQUIRE((... && std::same_as<Ts, std::reference_wrapper<std::remove_reference_t<Args>>>));

        const auto same_addresses = [&]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>) {
            return (... && (&elements.get() == &std::get<indices>(values)));
        };
        REQUIRE(same_addresses(std::index_sequence_for<Args...>{}));
    };

    const auto apply = std::invoke(
        [&]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>) {
            return expectation_policies::arg_list_indirect_apply_fn{
                std::tuple{[&](auto) { return std::ref(projection); }(indices)...}};
        },
        std::index_sequence_for<Args...>{});
    std::tuple refs = std::apply(
        [](auto&... elements) { return std::tuple<Args...>{static_cast<Args>(elements)...}; },
        values);
    std::invoke(
        apply,
        action,
        std::move(refs));

    REQUIRE(std::cmp_equal(invokeCount, sizeof...(Args)));
}

TEMPLATE_TEST_CASE(
    "expectation_policies::arg_list_indirect_apply_fn forwards the returned value.",
    "[expectation][expectation::policy]",
    int,
    // const int, never return a const value
    int&,
    const int&,
    int&&,
    const int&&)
{
    int value{1337};
    constexpr expectation_policies::arg_list_indirect_apply_fn<std::identity> forwarder{};

    const auto action = [](int& v) -> TestType {
        return static_cast<TestType>(v);
    };

    const auto make_refs = [&] { return std::make_tuple(std::ref(value)); };

    STATIC_REQUIRE(std::same_as<TestType, decltype(forwarder(action, make_refs()))>);
    REQUIRE(1337 == forwarder(action, make_refs()));
    CHECKED_IF(std::is_reference_v<TestType>)
    {
        auto&& r = forwarder(action, make_refs());
        REQUIRE(&value == &r);
    }
}

TEST_CASE(
    "expectation_policies::apply_args_fn invokes the given function.",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;

    SECTION("When just single param exists.")
    {
        int arg0{1337};
        using CallInfoT = call::Info<void, int&>;
        const CallInfoT callInfo{
            .args = {arg0},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

        SECTION("When single param is selected.")
        {
            using ArgSelectorT = InvocableMock<std::tuple<int&>, const CallInfoT&>;
            using ActionT = InvocableMock<void, int&>;

            ArgSelectorT argSelector{};
            ActionT action{};
            const expectation_policies::apply_args_fn fun{
                std::ref(argSelector),
                expectation_policies::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    const expectation_policies::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        expectation_policies::arg_list_forward_apply_fn>,
                    decltype(fun)>);
            STATIC_REQUIRE(std::is_void_v<decltype(fun(action, callInfo))>);

            trompeloeil::sequence sequence{};
            REQUIRE_CALL(argSelector, Invoke(_))
                .IN_SEQUENCE(sequence)
                .LR_WITH(&_1 == &callInfo)
                .RETURN(_1.args);
            REQUIRE_CALL(action, Invoke(_))
                .IN_SEQUENCE(sequence)
                .LR_WITH(&_1 == &arg0);

            REQUIRE_NOTHROW(fun(action, callInfo));
        }

        SECTION("When param is selected multiple times.")
        {
            using ArgSelectorT = InvocableMock<std::tuple<int&, int&, int&>, const CallInfoT&>;
            using ActionT = InvocableMock<void, int&, int&, int&>;

            ArgSelectorT argSelector{};
            ActionT action{};
            const expectation_policies::apply_args_fn fun{
                std::ref(argSelector),
                expectation_policies::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    const expectation_policies::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        expectation_policies::arg_list_forward_apply_fn>,
                    decltype(fun)>);
            STATIC_REQUIRE(std::is_void_v<decltype(fun(action, callInfo))>);

            trompeloeil::sequence sequence{};
            REQUIRE_CALL(argSelector, Invoke(_))
                .IN_SEQUENCE(sequence)
                .LR_WITH(&_1 == &callInfo)
                .RETURN(std::tuple_cat(_1.args, _1.args, _1.args));
            REQUIRE_CALL(action, Invoke(_, _, _))
                .IN_SEQUENCE(sequence)
                .LR_WITH(&_1 == &arg0)
                .LR_WITH(&_2 == &arg0)
                .LR_WITH(&_3 == &arg0);

            REQUIRE_NOTHROW(fun(action, callInfo));
        }
    }

    SECTION("When multiple params exist.")
    {
        int arg0{1337};
        std::string arg1{"Test"};
        double arg2{42.};
        using CallInfoT = call::Info<void, int&, std::string&, double&>;
        const CallInfoT callInfo{
            .args = {arg0, arg1, arg2},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
        };

        SECTION("When single param is selected.")
        {
            using ArgSelectorT = InvocableMock<std::tuple<std::string&>, const CallInfoT&>;
            using ActionT = InvocableMock<void, std::string&>;

            ArgSelectorT argSelector{};
            ActionT action{};
            const expectation_policies::apply_args_fn fun{
                std::ref(argSelector),
                expectation_policies::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    const expectation_policies::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        expectation_policies::arg_list_forward_apply_fn>,
                    decltype(fun)>);
            STATIC_REQUIRE(std::is_void_v<decltype(fun(action, callInfo))>);

            trompeloeil::sequence sequence{};
            REQUIRE_CALL(argSelector, Invoke(_))
                .IN_SEQUENCE(sequence)
                .LR_WITH(&_1 == &callInfo)
                .LR_RETURN(std::tie(arg1));
            REQUIRE_CALL(action, Invoke(_))
                .IN_SEQUENCE(sequence)
                .LR_WITH(&_1 == &arg1);

            REQUIRE_NOTHROW(fun(action, callInfo));
        }

        SECTION("When multiple params are selected.")
        {
            using ArgSelectorT = InvocableMock<std::tuple<double&, std::string&, double&, int&>, const CallInfoT&>;
            using ActionT = InvocableMock<void, double&, std::string&, double&, int&>;

            ArgSelectorT argSelector{};
            ActionT action{};
            const expectation_policies::apply_args_fn fun{
                std::ref(argSelector),
                expectation_policies::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    const expectation_policies::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        expectation_policies::arg_list_forward_apply_fn>,
                    decltype(fun)>);
            STATIC_REQUIRE(std::is_void_v<decltype(fun(action, callInfo))>);

            trompeloeil::sequence sequence{};
            REQUIRE_CALL(argSelector, Invoke(_))
                .IN_SEQUENCE(sequence)
                .LR_WITH(&_1 == &callInfo)
                .LR_RETURN(std::tie(arg2, arg1, arg2, arg0));
            REQUIRE_CALL(action, Invoke(_, _, _, _))
                .IN_SEQUENCE(sequence)
                .LR_WITH(&_1 == &arg2)
                .LR_WITH(&_2 == &arg1)
                .LR_WITH(&_3 == &arg2)
                .LR_WITH(&_4 == &arg0);

            REQUIRE_NOTHROW(fun(action, callInfo));
        }
    }
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::apply_args_fn preserves argument qualification.",
    "[expectation][expectation::policy]",
    ((bool expectSameAddress, typename ActionParam, typename SigParam), expectSameAddress, ActionParam, SigParam),
    (false, int, int),
    (false, int, int&),
    (false, int, const int&),
    (false, int, int&&),
    (false, int, const int&&),

    (true, int&, int&),

    (false, const int&, int),
    (true, const int&, int&),
    (true, const int&, const int&),
    (true, const int&, int&&),
    (true, const int&, const int&&))
{
    using trompeloeil::_;

    int param0{1337};
    const call::Info<void, SigParam> info{
        .args = {param0},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    InvocableMock<void, ActionParam> action{};
    constexpr expectation_policies::apply_args_fn fun{
        expectation_policies::all_args_selector_fn<std::add_rvalue_reference_t>{},
        expectation_policies::arg_list_forward_apply_fn{}};

    REQUIRE_CALL(action, Invoke(param0))
        .LR_WITH(!expectSameAddress || (&_1 == &param0));
    REQUIRE_NOTHROW(fun(action, info));
}

TEMPLATE_TEST_CASE(
    "expectation_policies::apply_args_fn forwards the returned value.",
    "[expectation][expectation::policy]",
    int,
    // const int, never return a const value
    int&,
    const int&,
    int&&,
    const int&&)
{
    using trompeloeil::_;

    int param0{1337};
    const call::Info<void, int> info{
        .args = {param0},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    constexpr expectation_policies::apply_args_fn fun{
        expectation_policies::all_args_selector_fn<std::add_rvalue_reference_t>{},
        expectation_policies::arg_list_forward_apply_fn{}};

    int value{42};
    const auto action = [&]([[maybe_unused]] int p) -> TestType {
        return static_cast<TestType>(value);
    };

    STATIC_REQUIRE(std::same_as<TestType, decltype(fun(action, info))>);
    REQUIRE(42 == fun(action, info));
    CHECKED_IF(std::is_reference_v<TestType>)
    {
        auto&& r = fun(action, info);
        REQUIRE(&value == &r);
    }
}

TEST_CASE(
    "then::apply_arg creates expectation_policies::ParamsSideEffect.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    int param0{1337};
    double param1{4.2};
    std::string param2{"Hello, World!"};
    const call::Info<void, int&, double&, std::string&> info{
        .args = {param0, param1, param2}
    };

    SECTION("Index 0.")
    {
        InvocableMock<void, int&> action{};
        expectation_policies::SideEffectAction policy = then::apply_arg<0>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param0);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("Index 1.")
    {
        InvocableMock<void, const double&> action{};
        expectation_policies::SideEffectAction policy = then::apply_arg<1>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param1);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("Index 2.")
    {
        InvocableMock<void, std::string> action{};
        expectation_policies::SideEffectAction policy = then::apply_arg<2>(std::ref(action));
        REQUIRE_CALL(action, Invoke("Hello, World!"));
        REQUIRE_NOTHROW(policy.consume(info));
    }
}

TEST_CASE(
    "then::apply_args creates expectation_policies::ParamsSideEffect.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    int param0{1337};
    double param1{4.2};
    std::string param2{"Hello, World!"};
    const call::Info<void, int&, double&, std::string&> info{
        .args = {param0, param1, param2}
    };

    SECTION("Indices 0, 1, 2.")
    {
        InvocableMock<void, int&, double&, std::string&> action{};
        expectation_policies::SideEffectAction policy = then::apply_args<0, 1, 2>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_, _, _))
            .LR_WITH(&_1 == &param0)
            .LR_WITH(&_2 == &param1)
            .LR_WITH(&_3 == &param2);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("Indices 2, 1, 0")
    {
        InvocableMock<void, std::string&, double&, int&> action{};
        expectation_policies::SideEffectAction policy = then::apply_args<2, 1, 0>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_, _, _))
            .LR_WITH(&_1 == &param2)
            .LR_WITH(&_2 == &param1)
            .LR_WITH(&_3 == &param0);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("Arbitrarily mixed.")
    {
        InvocableMock<void, double&, int&, std::string&, std::string&, int&> action{};
        expectation_policies::SideEffectAction policy = then::apply_args<1, 0, 2, 2, 0>(std::ref(action));
        REQUIRE_CALL(action, Invoke(_, _, _, _, _))
            .LR_WITH(&_1 == &param1)
            .LR_WITH(&_2 == &param0)
            .LR_WITH(&_3 == &param2)
            .LR_WITH(&_4 == &param2)
            .LR_WITH(&_5 == &param0);
        REQUIRE_NOTHROW(policy.consume(info));
    }
}

TEST_CASE(
    "then::apply_all creates expectation_policies::AllParamsSideEffect.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    SECTION("When signature has zero params.")
    {
        const call::Info<void> info{
            .args = {}};

        InvocableMock<void> action{};
        expectation_policies::SideEffectAction policy = then::apply_all(std::ref(action));
        REQUIRE_CALL(action, Invoke());
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("When signature has one param.")
    {
        int param0{1337};
        const call::Info<void, int&> info{
            .args = {param0}};

        InvocableMock<void, int&> action{};
        expectation_policies::SideEffectAction policy = then::apply_all(std::ref(action));
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param0);
        REQUIRE_NOTHROW(policy.consume(info));
    }

    SECTION("When signature has multiple params.")
    {
        int param0{1337};
        double param1{4.2};
        const call::Info<void, int&, double&> info{
            .args = {param0, param1}
        };

        InvocableMock<void, int&, double&> action{};
        expectation_policies::SideEffectAction policy = then::apply_all(std::ref(action));
        REQUIRE_CALL(action, Invoke(_, _))
            .LR_WITH(&_1 == &param0)
            .LR_WITH(&_2 == &param1);
        REQUIRE_NOTHROW(policy.consume(info));
    }
}

TEST_CASE(
    "then::invoke creates expectation_policies::ParamsSideEffect.",
    "[expectation][expectation::factories]")
{
    int param0{1337};
    double param1{4.2};
    std::string param2{"Hello, World!"};
    const call::Info<void, int&, double&, std::string&> info{
        .args = {param0, param1, param2}
    };

    InvocableMock<void> action{};
    expectation_policies::SideEffectAction policy = then::invoke(std::ref(action));
    REQUIRE_CALL(action, Invoke());
    REQUIRE_NOTHROW(policy.consume(info));
}

TEST_CASE(
    "finally::returns_result_of creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    using SignatureT = int&();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    InvocableMock<int&> action{};
    expectation_policies::ReturnsResultOf policy = finally::returns_result_of(std::ref(action));
    STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

    int value{42};
    REQUIRE_CALL(action, Invoke())
        .LR_RETURN(value);
    int& result = policy.finalize_call(call);
    REQUIRE(&value == &result);
}

TEST_CASE(
    "finally::returns creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    SECTION("When a value is returned.")
    {
        using CallInfoT = call::Info<int>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        const int value = GENERATE(range(0, 5));

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(value);
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), int()>);

            REQUIRE(value == policy.finalize_call(call));
            REQUIRE(value == policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), int()>);

            REQUIRE(value == policy.finalize_call(call));
            REQUIRE(value == policy.finalize_call(call));
        }
    }

    SECTION("When a lvalue ref is returned.")
    {
        using CallInfoT = call::Info<int&>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(42);
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&()>);

            int& result = policy.finalize_call(call);
            REQUIRE(42 == result);
            REQUIRE(&result == &policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            int value{42};
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&()>);

            REQUIRE(&value == &policy.finalize_call(call));
            REQUIRE(&value == &policy.finalize_call(call));
        }
    }

    SECTION("When a const lvalue ref is returned.")
    {
        using CallInfoT = call::Info<const int&>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(42);
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), const int&()>);

            const int& result = policy.finalize_call(call);
            REQUIRE(42 == result);
            REQUIRE(&result == &policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            constexpr int value{42};
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for<decltype(policy), const int&()>);

            REQUIRE(&value == &policy.finalize_call(call));
            REQUIRE(&value == &policy.finalize_call(call));
        }
    }

    SECTION("When a rvalue ref is returned.")
    {
        using CallInfoT = call::Info<int&&>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(42);
            STATIC_REQUIRE(finalize_policy_for < decltype(policy), int && () >);

            int&& result = policy.finalize_call(call);
            REQUIRE(42 == result);
            REQUIRE(42 == policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            int value{42};
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for < decltype(policy), int && () >);

            int&& result = policy.finalize_call(call);
            REQUIRE(&value == std::addressof(result));
            int&& secondResult = policy.finalize_call(call);
            REQUIRE(&value == std::addressof(secondResult));
        }
    }

    SECTION("When a const rvalue ref is returned.")
    {
        using CallInfoT = call::Info<const int&&>;
        const CallInfoT call{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        SECTION("And when value is stored.")
        {
            expectation_policies::ReturnsResultOf policy = finally::returns(42);
            STATIC_REQUIRE(finalize_policy_for < decltype(policy), const int && () >);

            const int&& result = policy.finalize_call(call);
            REQUIRE(42 == result);
            REQUIRE(42 == policy.finalize_call(call));
        }

        SECTION("And when std::reference_wrapper is stored.")
        {
            int value{42};
            expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
            STATIC_REQUIRE(finalize_policy_for < decltype(policy), const int && () >);

            const int&& result = policy.finalize_call(call);
            REQUIRE(&value == std::addressof(result));
            const int&& secondResult = policy.finalize_call(call);
            REQUIRE(&value == std::addressof(secondResult));
        }
    }
}

TEST_CASE(
    "finally::returns_apply_result_of creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    SECTION("When a single index is chosen.")
    {
        using SignatureT = std::string && (int, double, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;

        int param0{1337};
        double param1{4.2};
        std::string param2{"Hello, World!"};
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_apply_result_of<2>(
            [](std::string& str) noexcept -> std::string&& { return std::move(str); });
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        std::string&& result = policy.finalize_call(info);
        REQUIRE(&param2 == std::addressof(result));
    }

    SECTION("When an arbitrary index sequence is chosen")
    {
        using SignatureT = std::tuple<double, std::string&&, int, double&>(int, double, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;

        int param0{1337};
        double param1{4.2};
        std::string param2{"Hello, World!"};
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_apply_result_of<1, 2, 0, 1>(
            [](double& p0, std::string& p1, int& p2, double& p3) {
                return std::tuple<double, std::string&&, int, double&>{
                    p0,
                    std::move(p1),
                    p2,
                    p3};
            });
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        auto&& [r0, r1, r2, r3] = policy.finalize_call(info);
        REQUIRE(r0 == param1);
        REQUIRE(std::addressof(r1) == &param2);
        REQUIRE(r2 == param0);
        REQUIRE(&r3 == &param1);
    }
}

TEST_CASE(
    "finally::returns_apply_all_result_of creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    using trompeloeil::_;

    SECTION("When signature has zero params.")
    {
        const call::Info<int&> info{
            .args = {},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        InvocableMock<int&> action{};
        expectation_policies::ReturnsResultOf policy = finally::returns_apply_all_result_of(std::ref(action));
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&()>);

        int value{42};
        REQUIRE_CALL(action, Invoke())
            .LR_RETURN(std::ref(value));
        REQUIRE(&value == &policy.finalize_call(info));
    }

    SECTION("When signature has one param.")
    {
        int param0{1337};
        const call::Info<int&, int&> info{
            .args = {std::ref(param0)},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

        InvocableMock<int&, int&> action{};
        expectation_policies::ReturnsResultOf policy = finally::returns_apply_all_result_of(std::ref(action));
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&(int&)>);

        int value{42};
        REQUIRE_CALL(action, Invoke(_))
            .LR_WITH(&_1 == &param0)
            .LR_RETURN(std::ref(value));
        REQUIRE(&value == &policy.finalize_call(info));
    }

    SECTION("When signature has multiple params.")
    {
        int param0{1337};
        double param1{4.2};
        const call::Info<int&, int&, double&> info{
            .args = {param0, param1},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        InvocableMock<int&, int&, double&> action{};
        expectation_policies::ReturnsResultOf policy = finally::returns_apply_all_result_of(std::ref(action));
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&(int&, double&)>);

        int value{42};
        REQUIRE_CALL(action, Invoke(_, _))
            .LR_WITH(&_1 == &param0)
            .LR_WITH(&_2 == &param1)
            .LR_RETURN(std::ref(value));
        REQUIRE(&value == &policy.finalize_call(info));
    }
}

TEST_CASE(
    "finally::returns_arg creates expectation_policies::ReturnsResultOf.",
    "[expectation][expectation::factories]")
{
    int param0{1337};
    double param1{4.2};
    std::string param2{"Hello, World!"};

    SECTION("When value is selected.")
    {
        using SignatureT = int(int, double&, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<0>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(1337 == policy.finalize_call(info));
    }

    SECTION("When lvalue ref is selected.")
    {
        using SignatureT = double&(int, double&, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<1>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(&param1 == &policy.finalize_call(info));
    }

    SECTION("When const lvalue ref is selected.")
    {
        using SignatureT = const double&(int, const double&, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<1>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(&param1 == &policy.finalize_call(info));
    }

    SECTION("When rvalue ref is selected.")
    {
        using SignatureT = std::string && (int, double&, std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<2>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        std::string&& result = policy.finalize_call(info);
        REQUIRE(&param2 == std::addressof(result));
    }

    SECTION("When const rvalue ref is selected.")
    {
        using SignatureT = const std::string && (int, double&, const std::string&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
            .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<2>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        const std::string&& result = policy.finalize_call(info);
        REQUIRE(&param2 == std::addressof(result));
    }
}

TEST_CASE(
    "finally::throws creates expectation_policies::Throws.",
    "[expectation][expectation::factories]")
{
    using CallInfoT = call::Info<int>;

    const int value = GENERATE(range(0, 5));
    expectation_policies::Throws policy = finally::throws(value);

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
        .fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)};

    REQUIRE_THROWS_AS(
        policy.finalize_call(call),
        int);
    REQUIRE_THROWS_AS(
        policy.finalize_call(call),
        int);
}
