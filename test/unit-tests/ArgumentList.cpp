// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ArgumentList.hpp"

#include "TestTypes.hpp"

using namespace mimicpp;

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
    "expectation_policies::all_args_selector_fn returns all arg-references as tuple.",
    "[detail][expectation][expectation::policy]",
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

    const detail::all_args_selector_fn<Projection> selector{};
    const std::tuple result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<const Expected, decltype(result)>);
    const auto same_addresses = [&]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>) {
        return (... && (&std::get<indices>(values) == &std::get<indices>(result)));
    };
    REQUIRE(same_addresses(std::index_sequence_for<Args...>{}));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::args_selector_fn returns selected arg-references as tuple.",
    "[detail][expectation][expectation::policy]",
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

    const detail::args_selector_fn<Projection, std::index_sequence<index>> selector{};
    const std::tuple result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<const Expected, decltype(result)>);
    REQUIRE(&std::get<0>(result) == &std::get<index>(values));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::args_selector_fn supports arbitrary selection-sequence.",
    "[detail][expectation][expectation::policy]",
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

    const detail::args_selector_fn<std::add_lvalue_reference_t, std::index_sequence<indices...>> selector{};
    const std::tuple result = selector(callInfo);
    STATIC_REQUIRE(std::same_as<const Expected, decltype(result)>);
    REQUIRE(
        std::apply(
            [&](auto&... elements) { return (... && (&elements == &std::get<indices>(values))); },
            result));
}

TEMPLATE_TEST_CASE_SIG(
    "expectation_policies::arg_list_forward_apply_fn forwards all elements to the given function.",
    "[detail][expectation][expectation::policy]",
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

    constexpr detail::arg_list_forward_apply_fn forwarder{};
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
    "[detail][expectation][expectation::policy]",
    int,
    // const int, never return a const value
    int&,
    const int&,
    int&&,
    const int&&)
{
    int value{1337};
    constexpr detail::arg_list_forward_apply_fn forwarder{};

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
    "[detail][expectation][expectation::policy]",
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
            return detail::arg_list_indirect_apply_fn{
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
    "[detail][expectation][expectation::policy]",
    int,
    // const int, never return a const value
    int&,
    const int&,
    int&&,
    const int&&)
{
    int value{1337};
    constexpr detail::arg_list_indirect_apply_fn<std::identity> forwarder{};

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
    "[detail][expectation][expectation::policy]")
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
            const detail::apply_args_fn fun{
                std::ref(argSelector),
                detail::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    const detail::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        detail::arg_list_forward_apply_fn>,
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
            const detail::apply_args_fn fun{
                std::ref(argSelector),
                detail::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    const detail::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        detail::arg_list_forward_apply_fn>,
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
            const detail::apply_args_fn fun{
                std::ref(argSelector),
                detail::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    const detail::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        detail::arg_list_forward_apply_fn>,
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
            const detail::apply_args_fn fun{
                std::ref(argSelector),
                detail::arg_list_forward_apply_fn{}};
            STATIC_REQUIRE(
                std::same_as<
                    const detail::apply_args_fn<
                        std::reference_wrapper<ArgSelectorT>,
                        detail::arg_list_forward_apply_fn>,
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
    constexpr detail::apply_args_fn fun{
        detail::all_args_selector_fn<std::add_rvalue_reference_t>{},
        detail::arg_list_forward_apply_fn{}};

    REQUIRE_CALL(action, Invoke(param0))
        .LR_WITH(!expectSameAddress || (&_1 == &param0));
    REQUIRE_NOTHROW(fun(action, info));
}

TEMPLATE_TEST_CASE(
    "expectation_policies::apply_args_fn forwards the returned value.",
    "[detail][expectation][expectation::policy]",
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

    constexpr detail::apply_args_fn fun{
        detail::all_args_selector_fn<std::add_rvalue_reference_t>{},
        detail::arg_list_forward_apply_fn{}};

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
