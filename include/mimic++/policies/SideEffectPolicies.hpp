// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_POLICIES_SIDE_EFFECT_POLICIES_HPP
#define MIMICPP_POLICIES_SIDE_EFFECT_POLICIES_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/policies/ArgumentList.hpp"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace mimicpp::expectation_policies
{
    template <typename Action>
    class SideEffectAction
    {
    public:
        ~SideEffectAction() = default;

        [[nodiscard]]
        explicit constexpr SideEffectAction(
            Action&& action) noexcept(std::is_nothrow_move_constructible_v<Action>)
            : m_Action{std::move(action)}
        {
        }

        SideEffectAction(const SideEffectAction&) = delete;
        SideEffectAction& operator=(const SideEffectAction&) = delete;

        [[nodiscard]]
        SideEffectAction(SideEffectAction&&) = default;
        SideEffectAction& operator=(SideEffectAction&&) = default;

        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        template <typename Return, typename... Args>
        [[nodiscard]]
        static constexpr bool matches(const call::Info<Return, Args...>&) noexcept
        {
            return true;
        }

        [[nodiscard]]
        static std::nullopt_t describe() noexcept
        {
            return std::nullopt;
        }

        template <typename Return, typename... Args>
            requires std::invocable<Action&, const call::Info<Return, Args...>&>
        constexpr void consume(const call::Info<Return, Args...>& info)
            noexcept(std::is_nothrow_invocable_v<Action&, const call::Info<Return, Args...>&>)
        {
            std::invoke(m_Action, info);
        }

    private:
        Action m_Action;
    };
}

namespace mimicpp::then
{
    /**
     * \defgroup EXPECTATION_SIDE_EFFECTS side effects
     * \ingroup EXPECTATION
     * \brief Side effects are a convenient way to apply actions on matched expectations.
     * \details After a match has been created, side effects will be applied during the ``consume`` step.
     * They may alter the call arguments and capture any variable from the outside scope. Beware that those captured variables
     * must outlive the expectation they are attached on.
     *
     * Side effects will be executed in order of their construction and later side effects will observe any changes applied on
     * arguments by prior side effects.
     *
     * \attention Side effects should never throw. If it is actually intended to throw an exception as a result, use
     * ``finally::throws`` instead.
     *
     * As side effects actually are ``expectation policies``, they may execute special behavior during ``is_satisfied`` and
     * ``matches`` steps. That being said, the provided side effects are actually no-ops on these functions, but custom side
     * effects may behave differently.
     *
     *\{
     */

    /**
     * \brief Applies the argument at the specified index on the given action.
     * \tparam index The argument index.
     * \tparam Action The action type.
     * \param action The action to be applied.
     * \return Newly created side effect action.
     */
    template <std::size_t index, typename Action>
    [[nodiscard]]
    constexpr auto apply_arg(Action&& action)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
    {
        using arg_selector_t = detail::args_selector_fn<
            std::add_lvalue_reference_t,
            std::index_sequence<index>>;
        using apply_strategy_t = detail::arg_list_forward_apply_fn;

        return expectation_policies::SideEffectAction{
            std::bind_front(
                detail::apply_args_fn{
                    arg_selector_t{},
                    apply_strategy_t{}},
                std::forward<Action>(action))};
    }

    /**
     * \brief Applies the arguments at the specified index and in that order on the given action.
     * \details This functions creates a side effect policy and applies the selected arguments in the specified order.
     * The indices can be in any order and may also contain duplicates.
     * \tparam index The first argument index.
     * \tparam additionalIndices Additional argument indices.
     * \tparam Action The action type.
     * \param action The action to be applied.
     * \return Newly created side effect action.
     */
    template <std::size_t index, std::size_t... additionalIndices, typename Action>
    [[nodiscard]]
    constexpr auto apply_args(Action&& action)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
    {
        using arg_selector_t = detail::args_selector_fn<
            std::add_lvalue_reference_t,
            std::index_sequence<index, additionalIndices...>>;
        using apply_strategy_t = detail::arg_list_forward_apply_fn;

        return expectation_policies::SideEffectAction{
            std::bind_front(
                detail::apply_args_fn{
                    arg_selector_t{},
                    apply_strategy_t{}},
                std::forward<Action>(action))};
    }

    /**
     * \brief Applies all arguments on the given action.
     * \tparam Action The action type.
     * \param action The action to be applied.
     * \return Newly created side effect action.
     */
    template <typename Action>
    [[nodiscard]]
    constexpr auto apply_all(Action&& action)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
    {
        using arg_selector_t = detail::all_args_selector_fn<std::add_lvalue_reference_t>;
        using apply_strategy_t = detail::arg_list_forward_apply_fn;

        return expectation_policies::SideEffectAction{
            std::bind_front(
                detail::apply_args_fn{
                    arg_selector_t{},
                    apply_strategy_t{}},
                std::forward<Action>(action))};
    }

    /**
     * \brief Invokes the given function.
     * \tparam Action  The action type.
     * \param action The action to be invoked.
     * \return Newly created side effect action.
     */
    template <std::invocable Action>
    [[nodiscard]]
    constexpr auto invoke(Action&& action)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
    {
        return expectation_policies::SideEffectAction{
            [fn = std::forward<Action>(action)]([[maybe_unused]] const auto& call) mutable noexcept(
                std::is_nothrow_invocable_v<Action&>) {
                std::invoke(fn);
            }};
    }

    /**
     * \}
     */
}

#endif
