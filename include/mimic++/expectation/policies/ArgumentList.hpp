//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_POLICIES_ARGUMENT_LIST_HPP
#define MIMICPP_EXPECTATION_POLICIES_ARGUMENT_LIST_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/config/Config.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    // ReSharper disable once CppUnusedIncludeDirective
    #include <functional> // std::invoke
    #include <tuple>
    #include <type_traits>
    #include <utility>
#endif

namespace mimicpp::expectation::policies::detail
{
    template <typename ArgSelector, typename ApplyStrategy>
    struct apply_args_fn
    {
    public:
        [[nodiscard]]
        explicit constexpr apply_args_fn(ArgSelector argSelector, ApplyStrategy applyStrategy)
            noexcept(
                std::is_nothrow_move_constructible_v<ArgSelector>
                && std::is_nothrow_move_constructible_v<ApplyStrategy>)
            : m_ArgSelector{std::move(argSelector)},
              m_ApplyStrategy{std::move(applyStrategy)}
        {
        }

        template <typename Action, typename Return, typename... Args>
            requires std::invocable<ArgSelector const&, call::Info<Return, Args...> const&>
                  && std::invocable<
                         ApplyStrategy const&,
                         Action,
                         std::invoke_result_t<ArgSelector const&, call::Info<Return, Args...> const&>>
        constexpr decltype(auto) operator()(Action&& action, call::Info<Return, Args...> const& info) const
            noexcept(
                std::is_nothrow_invocable_v<ArgSelector const&, call::Info<Return, Args...> const&>
                && std::is_nothrow_invocable_v<
                    ApplyStrategy const&,
                    Action,
                    std::invoke_result_t<ArgSelector const&, call::Info<Return, Args...> const&>>)
        {
            return std::invoke(
                m_ApplyStrategy,
                std::forward<Action>(action),
                std::invoke(m_ArgSelector, info));
        }

    private:
        ArgSelector m_ArgSelector;
        ApplyStrategy m_ApplyStrategy;
    };

    template <template <typename> typename TypeProjection, typename IndexSequence>
    struct args_selector_fn;

    template <template <typename> typename TypeProjection, std::size_t... indices>
    struct args_selector_fn<TypeProjection, std::index_sequence<indices...>>
    {
    public:
        static_assert(
            std::is_reference_v<TypeProjection<int&>>,
            "Only use reference-projections.");

        template <std::size_t index, typename Signature>
        using projected_t = TypeProjection<signature_param_type_t<index, Signature>>;

        template <typename Return, typename... Args>
        constexpr auto operator()(call::Info<Return, Args...> const& callInfo) const noexcept
        {
            using signature_t = Return(Args...);

            return std::forward_as_tuple(
                static_cast<projected_t<indices, signature_t>>(
                    // all elements are std::reference_wrapper, so unwrap them first
                    std::get<indices>(callInfo.args).get())...);
        }
    };

    template <template <typename> typename TypeProjection>
    struct all_args_selector_fn
    {
    public:
        template <typename Return, typename... Args>
        constexpr auto operator()(call::Info<Return, Args...> const& callInfo) const noexcept
        {
            return std::invoke(
                args_selector_fn<TypeProjection, std::index_sequence_for<Args...>>{},
                callInfo);
        }
    };

    struct arg_list_forward_apply_fn
    {
    public:
        template <typename Fun, typename... Args>
            requires std::invocable<Fun, Args...>
        constexpr decltype(auto) operator()(Fun&& fun, std::tuple<Args...>&& argList) const
            noexcept(std::is_nothrow_invocable_v<Fun, Args...>)
        {
            return std::apply(
                std::forward<Fun>(fun),
                std::move(argList));
        }
    };

    template <typename... Projections>
    struct arg_list_indirect_apply_fn
    {
    public:
        [[nodiscard]]
        explicit constexpr arg_list_indirect_apply_fn(std::tuple<Projections...> projections = {})
            noexcept(std::is_nothrow_move_constructible_v<std::tuple<Projections...>>)
            : m_Projections{std::move(projections)}
        {
        }

        template <typename Fun, typename... Args>
            requires(... && std::invocable<Projections const&, Args>)
                 && std::invocable<Fun, std::invoke_result_t<Projections const&, Args>...>
        constexpr decltype(auto) operator()(Fun&& fun, std::tuple<Args...>&& argList) const
            noexcept(
                (... && std::is_nothrow_invocable_v<Projections const&, Args>)
                && std::is_nothrow_invocable_v<Fun, std::invoke_result_t<Projections const&, Args>...>)
        {
            return invoke_impl(
                std::forward<Fun>(fun),
                std::move(argList),
                std::index_sequence_for<Projections...>{});
        }

    private:
        std::tuple<Projections...> m_Projections;

        template <typename Fun, typename... Args, std::size_t... indices>
        constexpr decltype(auto) invoke_impl(
            Fun&& fun,
            std::tuple<Args...>&& argList,
            std::index_sequence<indices...> const /*sequence*/) const
        {
            return std::invoke(
                std::forward<Fun>(fun),
                std::invoke(
                    std::get<indices>(m_Projections),
                    std::forward<Args>(std::get<indices>(argList)))...);
        }
    };
}

#endif
