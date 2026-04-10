//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/expectation/policies/ArgumentList.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;
namespace policies = expectation::policies;

namespace
{
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
    "all_args_selector_fn returns all arg-references as tuple.",
    "[detail][expectation][expectation::policy]",
    ((bool dummy, typename Expected, template <typename> typename Projection, typename... Args), dummy, Expected, Projection, Args...),
    (true, std::tuple<int&>, std::add_lvalue_reference_t, int),
    (true, std::tuple<int&>, std::add_lvalue_reference_t, int&),
    (true, std::tuple<int const&>, std::add_lvalue_reference_t, int const&),
    (true, std::tuple<int&>, std::add_lvalue_reference_t, int&&),
    (true, std::tuple<int const&>, std::add_lvalue_reference_t, int const&&),

    (true, std::tuple<int&&>, std::add_rvalue_reference_t, int),
    (true, std::tuple<int&>, std::add_rvalue_reference_t, int&),
    (true, std::tuple<int const&>, std::add_rvalue_reference_t, int const&),
    (true, std::tuple<int&&>, std::add_rvalue_reference_t, int&&),
    (true, std::tuple<int const&&>, std::add_rvalue_reference_t, int const&&),

    (true, std::tuple<float&, int&, double&>, std::add_lvalue_reference_t, float, int, double),
    (true, std::tuple<float&, int&, double&>, std::add_lvalue_reference_t, float&, int&, double&),
    (true, std::tuple<float const&, int const&, double const&>, std::add_lvalue_reference_t, float const&, int const&, double const&),
    (true, std::tuple<float&, int&, double&>, std::add_lvalue_reference_t, float&&, int&&, double&&),
    (true, std::tuple<float const&, int const&, double const&>, std::add_lvalue_reference_t, float const&&, int const&&, double const&&),

    (true, std::tuple<float&&, int&&, double&&>, std::add_rvalue_reference_t, float, int, double),
    (true, std::tuple<float&, int&, double&>, std::add_rvalue_reference_t, float&, int&, double&),
    (true, std::tuple<float const&, int const&, double const&>, std::add_rvalue_reference_t, float const&, int const&, double const&),
    (true, std::tuple<float&&, int&&, double&&>, std::add_rvalue_reference_t, float&&, int&&, double&&),
    (true, std::tuple<float const&&, int const&&, double const&&>, std::add_rvalue_reference_t, float const&&, int const&&, double const&&))
{
    std::tuple<std::remove_cvref_t<Args>...> values{};
    using CallInfoT = call::Info<void, Args...>;
    CallInfoT const callInfo{
        .args = make_ref_tuple(values),
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    policies::detail::all_args_selector_fn<Projection> const selector{};
    std::tuple const result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<Expected const, decltype(result)>);
    auto const same_addresses = [&]<std::size_t... indices>([[maybe_unused]] std::index_sequence<indices...> const) {
        return (... && (&std::get<indices>(values) == &std::get<indices>(result)));
    };
    REQUIRE(same_addresses(std::index_sequence_for<Args...>{}));
}

TEMPLATE_TEST_CASE_SIG(
    "args_selector_fn returns selected arg-references as tuple.",
    "[detail][expectation][expectation::policy]",
    ((typename Expected, template <typename> typename Projection, std::size_t index, typename... Args), Expected, Projection, index, Args...),
    (std::tuple<int&>, std::add_lvalue_reference_t, 0u, int),
    (std::tuple<int&>, std::add_lvalue_reference_t, 0u, int&),
    (std::tuple<int const&>, std::add_lvalue_reference_t, 0u, int const&),
    (std::tuple<int&>, std::add_lvalue_reference_t, 0u, int&&),
    (std::tuple<int const&>, std::add_lvalue_reference_t, 0u, int const&&),

    (std::tuple<int&&>, std::add_rvalue_reference_t, 0u, int),
    (std::tuple<int&>, std::add_rvalue_reference_t, 0u, int&),
    (std::tuple<int const&>, std::add_rvalue_reference_t, 0u, int const&),
    (std::tuple<int&&>, std::add_rvalue_reference_t, 0u, int&&),
    (std::tuple<int const&&>, std::add_rvalue_reference_t, 0u, int const&&),

    (std::tuple<int&>, std::add_lvalue_reference_t, 1u, float, int, double),
    (std::tuple<int&>, std::add_lvalue_reference_t, 1u, float&, int&, double&),
    (std::tuple<int const&>, std::add_lvalue_reference_t, 1u, float const&, int const&, double const&),
    (std::tuple<int&>, std::add_lvalue_reference_t, 1u, float&&, int&&, double&&),
    (std::tuple<int const&>, std::add_lvalue_reference_t, 1u, float const&&, int const&&, double const&&),

    (std::tuple<int&&>, std::add_rvalue_reference_t, 1u, float, int, double),
    (std::tuple<int&>, std::add_rvalue_reference_t, 1u, float&, int&, double&),
    (std::tuple<int const&>, std::add_rvalue_reference_t, 1u, float const&, int const&, double const&),
    (std::tuple<int&&>, std::add_rvalue_reference_t, 1u, float&&, int&&, double&&),
    (std::tuple<int const&&>, std::add_rvalue_reference_t, 1u, float const&&, int const&&, double const&&))
{
    std::tuple<std::remove_cvref_t<Args>...> values{};
    using CallInfoT = call::Info<void, Args...>;
    CallInfoT const callInfo{
        .args = make_ref_tuple(values),
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    policies::detail::args_selector_fn<Projection, std::index_sequence<index>> const selector{};
    std::tuple const result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<Expected const, decltype(result)>);
    REQUIRE(&std::get<0>(result) == &std::get<index>(values));
}

TEMPLATE_TEST_CASE_SIG(
    "args_selector_fn supports arbitrary selection-sequence.",
    "[detail][expectation][expectation::policy]",
    ((typename Expected, std::size_t... indices), Expected, indices...),
    (std::tuple<float&>, 0u),
    (std::tuple<float&, float&>, 0u, 0u),
    (std::tuple<double&, int&, double&>, 2u, 1u, 2u),
    (std::tuple<int&, float&>, 1u, 0u))
{
    std::tuple<float, int, double> values{};
    using CallInfoT = call::Info<void, float&, int&, double&>;
    CallInfoT const callInfo{
        .args = make_ref_tuple(values),
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    policies::detail::args_selector_fn<std::add_lvalue_reference_t, std::index_sequence<indices...>> const selector{};
    std::tuple const result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<Expected const, decltype(result)>);
    REQUIRE(
        std::apply(
            [&](auto&... elements) { return (... && (&elements == &std::get<indices>(values))); },
            result));
}

TEMPLATE_TEST_CASE_SIG(
    "arg_list_forward_apply_fn forwards all elements to the given function.",
    "[detail][expectation][expectation::policy]",
    ((bool dummy, typename... Args), dummy, Args...),
    (true, int&),
    (true, int const&),
    (true, int&&),
    (true, int const&&),

    (true, float&, int&, double&),
    (true, float const&, int const&, double const&),
    (true, float&&, int&&, double&&),
    (true, float const&&, int const&&, double const&&))
{
    std::tuple<std::remove_cvref_t<Args>...> values{};
    auto const action = [&]<typename... Ts>(Ts&&... elements) {
        STATIC_REQUIRE((... && std::same_as<Ts&&, Args>));

        auto const same_addresses = [&]<std::size_t... indices>([[maybe_unused]] std::index_sequence<indices...> const) {
            return (... && (&elements == &std::get<indices>(values)));
        };
        REQUIRE(same_addresses(std::index_sequence_for<Args...>{}));
    };

    constexpr policies::detail::arg_list_forward_apply_fn forwarder{};
    std::tuple refs = std::apply(
        [](auto&... elements) { return std::tuple<Args...>{static_cast<Args>(elements)...}; },
        values);
    std::invoke(
        forwarder,
        action,
        std::move(refs));
}

TEMPLATE_TEST_CASE(
    "arg_list_forward_apply_fn forwards the returned value.",
    "[detail][expectation][expectation::policy]",
    int,
    // int const, never return an int const value
    int&,
    int const&,
    int&&,
    int const&&)
{
    int value{1337};
    constexpr policies::detail::arg_list_forward_apply_fn forwarder{};

    auto const action = [](int& v) -> TestType {
        return static_cast<TestType>(v);
    };

    auto const make_refs = [&] { return std::make_tuple(std::ref(value)); };

    STATIC_REQUIRE(std::same_as<TestType, decltype(forwarder(action, make_refs()))>);
    REQUIRE(1337 == forwarder(action, make_refs()));
    CHECKED_IF(std::is_reference_v<TestType>)
    {
        auto&& r = forwarder(action, make_refs());
        REQUIRE(&value == &r);
    }
}

TEMPLATE_TEST_CASE_SIG(
    "arg_list_indirect_apply_fn forwards all projected elements to the given function.",
    "[detail][expectation][expectation::policy]",
    ((bool dummy, typename... Args), dummy, Args...),
    (true, int&),
    (true, int const&),
    (true, int&&),
    (true, int const&&),

    (true, float&, int&, double&),
    (true, float const&, int const&, double const&),
    (true, float&&, int&&, double&&),
    (true, float const&&, int const&&, double const&&))
{
    std::tuple<std::remove_cvref_t<Args>...> values{};
    int invokeCount{};
    auto const projection = [&](auto&& value) {
        ++invokeCount;
        return std::ref(value);
    };
    auto const action = [&]<typename... Ts>(Ts... elements) {
        STATIC_REQUIRE((... && std::same_as<Ts, std::reference_wrapper<std::remove_reference_t<Args>>>));

        auto const same_addresses = [&]<std::size_t... indices>([[maybe_unused]] std::index_sequence<indices...> const) {
            return (... && (&elements.get() == &std::get<indices>(values)));
        };
        REQUIRE(same_addresses(std::index_sequence_for<Args...>{}));
    };

    auto const apply = std::invoke(
        [&]<std::size_t... indices>([[maybe_unused]] std::index_sequence<indices...> const) {
            return policies::detail::arg_list_indirect_apply_fn{
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
    "arg_list_indirect_apply_fn forwards the returned value.",
    "[detail][expectation][expectation::policy]",
    int,
    // int const, never return an int const value
    int&,
    int const&,
    int&&,
    int const&&)
{
    int value{1337};
    constexpr policies::detail::arg_list_indirect_apply_fn<std::identity> forwarder{};

    auto const action = [](int& v) -> TestType {
        return static_cast<TestType>(v);
    };

    auto const make_refs = [&] { return std::make_tuple(std::ref(value)); };

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
    "[detail][expectation][expectation::policy]")
{
    using trompeloeil::_;

    SECTION("When just single param exists.")
    {
        int arg0{1337};
        using CallInfoT = call::Info<void, int&>;
        CallInfoT const callInfo{
            .args = {arg0},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))};

        SECTION("When single param is selected.")
        {
            using ArgSelectorT = InvocableMock<std::tuple<int&>, CallInfoT const&>;
            using ActionT = InvocableMock<void, int&>;

            ArgSelectorT argSelector{};
            ActionT action{};
            policies::detail::apply_args_fn const fun{
                std::ref(argSelector),
                policies::detail::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    policies::detail::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        policies::detail::arg_list_forward_apply_fn> const,
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
            using ArgSelectorT = InvocableMock<std::tuple<int&, int&, int&>, CallInfoT const&>;
            using ActionT = InvocableMock<void, int&, int&, int&>;

            ArgSelectorT argSelector{};
            ActionT action{};
            policies::detail::apply_args_fn const fun{
                std::ref(argSelector),
                policies::detail::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    policies::detail::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        policies::detail::arg_list_forward_apply_fn> const,
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
        CallInfoT const callInfo{
            .args = {arg0, arg1, arg2},
            .fromCategory = GENERATE(from_range(refQualifiers)),
            .fromConstness = GENERATE(from_range(constQualifiers))
        };

        SECTION("When single param is selected.")
        {
            using ArgSelectorT = InvocableMock<std::tuple<std::string&>, CallInfoT const&>;
            using ActionT = InvocableMock<void, std::string&>;

            ArgSelectorT argSelector{};
            ActionT action{};
            policies::detail::apply_args_fn const fun{
                std::ref(argSelector),
                policies::detail::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    policies::detail::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        policies::detail::arg_list_forward_apply_fn> const,
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
            using ArgSelectorT = InvocableMock<std::tuple<double&, std::string&, double&, int&>, CallInfoT const&>;
            using ActionT = InvocableMock<void, double&, std::string&, double&, int&>;

            ArgSelectorT argSelector{};
            ActionT action{};
            policies::detail::apply_args_fn const fun{
                std::ref(argSelector),
                policies::detail::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    policies::detail::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        policies::detail::arg_list_forward_apply_fn> const,
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
    "[detail][expectation][expectation::policy]",
    ((bool expectSameAddress, typename ActionParam, typename SigParam), expectSameAddress, ActionParam, SigParam),
    (false, int, int),
    (false, int, int&),
    (false, int, int const&),
    (false, int, int&&),
    (false, int, int const&&),

    (true, int&, int&),

    (false, int const&, int),
    (true, int const&, int&),
    (true, int const&, int const&),
    (true, int const&, int&&),
    (true, int const&, int const&&))
{
    using trompeloeil::_;

    int param0{1337};
    call::Info<void, SigParam> const info{
        .args = {param0},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    InvocableMock<void, ActionParam> action{};
    constexpr policies::detail::apply_args_fn fun{
        policies::detail::all_args_selector_fn<std::add_rvalue_reference_t>{},
        policies::detail::arg_list_forward_apply_fn{}};

    REQUIRE_CALL(action, Invoke(param0))
        .LR_WITH(!expectSameAddress || (&_1 == &param0));
    REQUIRE_NOTHROW(fun(action, info));
}

TEMPLATE_TEST_CASE(
    "expectation_policies::apply_args_fn forwards the returned value.",
    "[detail][expectation][expectation::policy]",
    int,
    // int const, never return an int const value
    int&,
    int const&,
    int&&,
    int const&&)
{
    using trompeloeil::_;

    int param0{1337};
    call::Info<void, int> const info{
        .args = {param0},
        .fromCategory = GENERATE(from_range(refQualifiers)),
        .fromConstness = GENERATE(from_range(constQualifiers))};

    constexpr policies::detail::apply_args_fn fun{
        policies::detail::all_args_selector_fn<std::add_rvalue_reference_t>{},
        policies::detail::arg_list_forward_apply_fn{}};

    int value{42};
    auto const action = [&]([[maybe_unused]] int p) -> TestType {
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
