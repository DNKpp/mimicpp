// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_POLICIES_FINALIZER_POLICIES_HPP
#define MIMICPP_POLICIES_FINALIZER_POLICIES_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Utility.hpp"
#include "mimic++/policies/ArgumentList.hpp"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace mimicpp::expectation_policies
{
    template <typename Action>
        requires std::same_as<Action, std::remove_cvref_t<Action>>
              && std::is_move_constructible_v<Action>
    class ReturnsResultOf
    {
    public:
        [[nodiscard]]
        explicit constexpr ReturnsResultOf(Action&& action)
            noexcept(std::is_nothrow_move_constructible_v<Action>)
            : m_Action{std::move(action)}
        {
        }

        template <typename Return, typename... Args>
            requires std::invocable<Action&, const call::Info<Return, Args...>&>
                  && explicitly_convertible_to<
                         std::invoke_result_t<Action&, const call::Info<Return, Args...>&>,
                         Return>
        [[nodiscard]]
        constexpr Return finalize_call([[maybe_unused]] const call::Info<Return, Args...>& call)
            noexcept(
                std::is_nothrow_invocable_v<Action&, const call::Info<Return, Args...>&>
                && nothrow_explicitly_convertible_to<std::invoke_result_t<Action&, const call::Info<Return, Args...>&>, Return>)
        {
            return static_cast<Return>(
                std::invoke(m_Action, call));
        }

    private:
        Action m_Action;
    };

    template <typename Exception>
        requires(!std::is_reference_v<Exception>)
             && std::copyable<Exception>
    class Throws
    {
    public:
        [[nodiscard]]
        explicit constexpr Throws(Exception exception) noexcept(std::is_nothrow_move_constructible_v<Exception>)
            : m_Exception{std::move(exception)}
        {
        }

        template <typename Return, typename... Args>
        constexpr Return finalize_call(
            [[maybe_unused]] const call::Info<Return, Args...>& call)
        {
            throw m_Exception; // NOLINT(hicpp-exception-baseclass)
        }

    private:
        Exception m_Exception;
    };
}

namespace mimicpp::finally
{
    /**
     * \defgroup EXPECTATION_FINALIZER finalizer
     * \ingroup EXPECTATION
     * \brief Finalizers are the last step of a matching expectation.
     * \details Finalizers are executed for calls, which have a matching expectation. They are responsible for either returning an
     * appropriate return value or leaving the call by throwing an exception.
     *
     * An expectation must have exactly one finalizer applied. For mocks returning void, an appropriate finalizer is attached by default.
     * This default finalizer can be exchanged once. If an expectation setup contains multiple finalize statements, a compile error is
     * triggered.
     *
     *# Custom Finalizers
     * There are several provided finalizers, but user may create their own as desired.
     * A valid finalizer has to satisfy the ``finalize_policy_for`` concept, which in fact requires the existence of a ``finalize_call``
     * member function.
     * \snippet Finalizers.cpp custom finalizer
     *\{
     */

    /**
     * \brief During the finalization step, the invocation result of the given function is returned.
     * \snippet Finalizers.cpp finally::returns_result_of
     * \tparam Fun The function type.
     * \param fun The function to be invoked.
     * \return Forward returns the invocation result of ``fun``.
     *
     * \details The provided functions must be invocable without arguments, but may return any type, which is explicitly convertible to
     * the mocks return type.
     */
    template <typename Fun>
        requires std::invocable<std::remove_cvref_t<Fun>&>
              && (!std::is_void_v<std::invoke_result_t<std::remove_cvref_t<Fun>&>>)
    [[nodiscard]]
    constexpr auto returns_result_of(Fun&& fun)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Fun>, Fun>)
    {
        return expectation_policies::ReturnsResultOf{
            [fn = std::forward<Fun>(fun)]([[maybe_unused]] const auto& call) mutable noexcept(
                std::is_nothrow_invocable_v<decltype(fun)>) -> decltype(auto) {
                return std::invoke(fn);
            }};
    }

    /**
     * \brief During the finalization step, the stored value is returned.
     * \tparam T The value type.
     * \param value The value to be returned.
     * \return Returns a copy of ``value``.
     *
     * \details The provided value must be copyable, because multiple invocations must be possible.
     * \snippet Finalizers.cpp finally::returns
     *
     * This finalizer is aware of ``std::reference_wrapper``, which can be used to return values from the outer scope. Such values are
     * explicitly unwrapped, before they are returned.
     * \snippet Finalizers.cpp finally::returns std::ref
     *
     * In fact any value category can be returned; it is the users responsibility to make sure, not to use any dangling references.
     * If an actual value is stored and a reference is returned, this is fine until the expectation goes out of scope.
     * \snippet Finalizers.cpp finally::returns ref
     */
    template <typename T>
        requires std::copyable<std::remove_cvref_t<T>>
    [[nodiscard]]
    constexpr auto returns(T&& value)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<T>, T>)
    {
        return expectation_policies::ReturnsResultOf{
            [v = std::forward<T>(value)]([[maybe_unused]] const auto& call) mutable noexcept -> auto& {
                return static_cast<std::unwrap_reference_t<decltype(v)>&>(v);
            }};
    }

    /**
     * \brief During the finalization step, the selected call arguments are applied on the given action.
     * \tparam index The first selected argument index.
     * \tparam otherIndices Addition selected arguments.
     * \tparam Action The action type.
     * \param action The action to be applied to.
     * \return Returns the invocation result of the given action.
     *
     * \details The selected call arguments are applied as (possibly const qualified) lvalue-references. The action may proceed with them
     * as desired, but be aware that this may actually affect objects outside the call (e.g. if call arguments are lvalues.).
     * \snippet Finalizers.cpp finally::returns_apply_result_of
     */
    template <std::size_t index, std::size_t... otherIndices, typename Action>
    [[nodiscard]]
    constexpr auto returns_apply_result_of(Action&& action)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
    {
        using arg_selector_t = detail::args_selector_fn<
            std::add_lvalue_reference_t,
            std::index_sequence<index, otherIndices...>>;
        using apply_strategy_t = detail::arg_list_forward_apply_fn;

        return expectation_policies::ReturnsResultOf{
            std::bind_front(
                detail::apply_args_fn{
                    arg_selector_t{},
                    apply_strategy_t{}},
                std::forward<Action>(action))};
    }

    /**
     * \brief During the finalization step, all call arguments are applied on the given action.
     * \tparam Action The action type.
     * \param action The action to be applied to.
     * \return Returns the invocation result of the given action.
     *
     * \details All call arguments are applied as (possibly const qualified) lvalue-references. The action may proceed with them
     * as desired, but be aware that this may actually affect objects outside the call (e.g. if call arguments are lvalues.).
     * \snippet Finalizers.cpp finally::returns_apply_all_result_of
     */
    template <typename Action>
    [[nodiscard]]
    constexpr auto returns_apply_all_result_of(Action&& action)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
    {
        using arg_selector_t = detail::all_args_selector_fn<std::add_lvalue_reference_t>;
        using apply_strategy_t = detail::arg_list_forward_apply_fn;

        return expectation_policies::ReturnsResultOf{
            std::bind_front(
                detail::apply_args_fn{
                    arg_selector_t{},
                    apply_strategy_t{}},
                std::forward<Action>(action))};
    }

    /**
     * \brief During the finalization step, the selected call argument is returned.
     * \return Returns the forwarded and explicitly converted argument.
     *
     * \details The selected call argument is forwarded and explicitly converted to the mocks return type.
     * \snippet Finalizers.cpp finally::returns_param
     */
    template <std::size_t index>
    [[nodiscard]]
    consteval auto returns_arg() noexcept
    {
        using arg_selector_t = detail::args_selector_fn<
            std::add_rvalue_reference_t,
            std::index_sequence<index>>;
        using apply_strategy_t = detail::arg_list_forward_apply_fn;

        return expectation_policies::ReturnsResultOf{
            std::bind_front(
                detail::apply_args_fn{
                    arg_selector_t{},
                    apply_strategy_t{}},
                std::identity{})};
    }

    /**
     * \brief During the finalization step, the given exception is thrown.
     * \tparam T The exception type.
     * \param exception The exception to be thrown.
     * \throws A copy of ``exception``.
     *
     * \details The provided exception must be copyable, because multiple invocations must be possible.
     * \snippet Finalizers.cpp finally::throws
     */
    template <typename T>
        requires std::copyable<std::remove_cvref_t<T>>
    [[nodiscard]]
    constexpr expectation_policies::Throws<std::remove_cvref_t<T>> throws(T&& exception)
        noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<T>, T>)
    {
        return expectation_policies::Throws<std::remove_cvref_t<T>>{
            std::forward<T>(exception)};
    }

    /**
     * \}
     */
}

#endif
