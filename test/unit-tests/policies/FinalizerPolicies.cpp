//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/policies/FinalizerPolicies.hpp"
#include "mimic++/Expectation.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

TEST_CASE(
    "expectation_policies::ReturnsResultOf forwards the invocation result during finalize_call().",
    "[expectation][expectation::policy]")
{
    using trompeloeil::_;

    using SignatureT = int&();
    using CallInfoT = call::info_for_signature_t<SignatureT>;

    const CallInfoT call{
        .args = {},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

        PolicyT policy{test_exception{}};

        REQUIRE_THROWS_AS(
            policy.finalize_call(call),
            test_exception);
    }
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
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
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
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
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
    std::unique_ptr param2 = std::make_unique<int>(-42);

    SECTION("When value is selected.")
    {
        using SignatureT = std::unique_ptr<int>(int, double&, std::unique_ptr<int>&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
        };

        const int* ptr = param2.get();
        expectation_policies::ReturnsResultOf policy = finally::returns_arg<2>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(ptr == policy.finalize_call(info).get());
    }

    SECTION("When lvalue ref is selected.")
    {
        using SignatureT = double&(int, double&, std::unique_ptr<int>&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<1>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(&param1 == &policy.finalize_call(info));
    }

    SECTION("When const lvalue ref is selected.")
    {
        using SignatureT = const double&(int, const double&, std::unique_ptr<int>&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<1>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        REQUIRE(&param1 == &policy.finalize_call(info));
    }

    SECTION("When rvalue ref is selected.")
    {
        using SignatureT = std::unique_ptr<int> && (int, double&, std::unique_ptr<int>&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<2>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        std::unique_ptr<int>&& result = policy.finalize_call(info);
        REQUIRE(&param2 == std::addressof(result));
    }

    SECTION("When const rvalue ref is selected.")
    {
        using SignatureT = const std::unique_ptr<int> && (int, double&, const std::unique_ptr<int>&&);
        using CallInfoT = call::info_for_signature_t<SignatureT>;
        const CallInfoT info{
            .args = {param0, param1, param2},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
        };

        expectation_policies::ReturnsResultOf policy = finally::returns_arg<2>();
        STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

        const std::unique_ptr<int>&& result = policy.finalize_call(info);
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
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    REQUIRE_THROWS_AS(
        policy.finalize_call(call),
        int);
    REQUIRE_THROWS_AS(
        policy.finalize_call(call),
        int);
}
