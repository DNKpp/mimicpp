// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_POLICIES_HPP
#define MIMICPP_EXPECTATION_POLICIES_HPP

#pragma once

#include "mimic++/Expectation.hpp"
#include "mimic++/Matcher.hpp"
#include "mimic++/Printer.hpp"
#include "mimic++/Utility.hpp"

#include <cassert>
// ReSharper disable once CppUnusedIncludeDirective
#include <functional> // std::invoke

namespace mimicpp::expectation_policies
{
    class InitFinalize
    {
    public:
        template <typename Return, typename... Args>
        static constexpr void finalize_call(const call::Info<Return, Args...>&) noexcept
        {
        }
    };

    template <ValueCategory expected>
    class Category
    {
    public:
        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        template <typename Return, typename... Args>
        static constexpr bool matches(const call::Info<Return, Args...>& info) noexcept
        {
            return mimicpp::is_matching(info.fromCategory, expected);
        }

        template <typename Return, typename... Args>
        static constexpr void consume([[maybe_unused]] const call::Info<Return, Args...>& info) noexcept
        {
            assert(mimicpp::is_matching(info.fromCategory, expected) && "Call does not match.");
        }

        [[nodiscard]]
        static StringT describe()
        {
            return format::format(
                "expect: from {} category overload",
                expected);
        }
    };

    template <Constness constness>
    class Constness
    {
    public:
        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        template <typename Return, typename... Args>
        static constexpr bool matches(const call::Info<Return, Args...>& info) noexcept
        {
            return mimicpp::is_matching(info.fromConstness, constness);
        }

        template <typename Return, typename... Args>
        static constexpr void consume([[maybe_unused]] const call::Info<Return, Args...>& info) noexcept
        {
            assert(mimicpp::is_matching(info.fromConstness, constness) && "Call does not match.");
        }

        [[nodiscard]]
        static StringT describe()
        {
            return format::format(
                "expect: from {} qualified overload",
                constness);
        }
    };

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
        constexpr auto operator()(const call::Info<Return, Args...>& callInfo) const noexcept
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
        constexpr auto operator()(const call::Info<Return, Args...>& callInfo) const noexcept
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
            requires(... && std::invocable<const Projections&, Args>)
                 && std::invocable<Fun, std::invoke_result_t<const Projections&, Args>...>
        constexpr decltype(auto) operator()(Fun&& fun, std::tuple<Args...>&& argList) const
            noexcept(
                (... && std::is_nothrow_invocable_v<const Projections&, Args>)
                && std::is_nothrow_invocable_v<Fun, std::invoke_result_t<const Projections&, Args>...>)
        {
            return [&, this]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>)
                       -> decltype(auto) {
                return std::invoke(
                    std::forward<Fun>(fun),
                    std::invoke(
                        std::get<indices>(m_Projections),
                        std::forward<Args>(std::get<indices>(argList)))...);
            }(std::index_sequence_for<Projections...>{});
        }

    private:
        std::tuple<Projections...> m_Projections;
    };

    template <typename Matcher>
    struct matcher_matches_fn
    {
    public:
        const Matcher& matcher;

        template <typename... Args>
            requires std::invocable<
                decltype(detail::matches_hook::matches),
                const Matcher&,
                Args&...>
        [[nodiscard]]
        // projected arguments may come as value, so Args& won't work in all cases
        // just forward them as lvalue-ref
        constexpr bool operator()(Args&&... args) const
            noexcept(
                std::is_nothrow_invocable_v<
                    decltype(detail::matches_hook::matches),
                    const Matcher&,
                    Args&...>)
        {
            return detail::matches_hook::matches(matcher, args...);
        }
    };

    template <typename Matcher, typename MatchesStrategy, typename DescribeStrategy>
    class ArgsRequirement
    {
    public:
        [[nodiscard]]
        explicit constexpr ArgsRequirement(
            Matcher matcher,
            MatchesStrategy matchesStrategy,
            DescribeStrategy describeStrategy)
            noexcept(
                std::is_nothrow_move_constructible_v<Matcher>
                && std::is_nothrow_move_constructible_v<MatchesStrategy>
                && std::is_nothrow_move_constructible_v<DescribeStrategy>)
            : m_Matcher{std::move(matcher)},
              m_MatchesStrategy{std::move(matchesStrategy)},
              m_DescribeStrategy{std::move(describeStrategy)}
        {
        }

        [[nodiscard]]
        static constexpr bool is_satisfied() noexcept
        {
            return true;
        }

        template <typename Return, typename... Args>
            requires std::is_invocable_r_v<bool, const MatchesStrategy&, matcher_matches_fn<Matcher>, const call::Info<Return, Args...>&>
        [[nodiscard]]
        constexpr bool matches(const call::Info<Return, Args...>& info) const
            noexcept(std::is_nothrow_invocable_v<const MatchesStrategy&, matcher_matches_fn<Matcher>, const call::Info<Return, Args...>&>)
        {
            return std::invoke(
                m_MatchesStrategy,
                matcher_matches_fn<Matcher>{m_Matcher},
                info);
        }

        template <typename Return, typename... Args>
        static constexpr void consume([[maybe_unused]] const call::Info<Return, Args...>& info) noexcept
        {
        }

        [[nodiscard]]
        StringT describe() const
        {
            return std::invoke(
                m_DescribeStrategy,
                detail::describe_hook::describe(m_Matcher));
        }

    private:
        Matcher m_Matcher;
        [[no_unique_address]] MatchesStrategy m_MatchesStrategy;
        [[no_unique_address]] DescribeStrategy m_DescribeStrategy;
    };

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
        constexpr void consume(
            const call::Info<Return, Args...>& info) noexcept(std::is_nothrow_invocable_v<Action&, const call::Info<Return, Args...>&>)
        {
            std::invoke(m_Action, info);
        }

    private:
        Action m_Action;
    };

    template <typename Action, template <typename> typename Projection>
        requires std::same_as<Action, std::remove_cvref_t<Action>>
    class ApplyAllArgsAction
    {
    public:
        [[nodiscard]]
        explicit constexpr ApplyAllArgsAction(
            Action action = {}) noexcept(std::is_nothrow_move_constructible_v<Action>)
            : m_Action{std::move(action)}
        {
        }

        template <typename Arg>
        using ProjectedArgT = Projection<Arg>;

        template <typename Return, typename... Args>
            requires std::invocable<
                const Action&,
                ProjectedArgT<Args>...>
        constexpr decltype(auto) operator()(
            const call::Info<Return, Args...>& callInfo) const noexcept(std::is_nothrow_invocable_v<const Action&, ProjectedArgT<Args>...>)
        {
            static_assert(
                (... && explicitly_convertible_to<Args&, ProjectedArgT<Args>>),
                "Projection can not be applied.");

            return std::apply(
                [this](auto&... args) -> decltype(auto) {
                    return std::invoke(
                        m_Action,
                        static_cast<ProjectedArgT<Args>>(
                            args.get())...);
                },
                callInfo.args);
        }

    private:
        Action m_Action;
    };

    template <typename Action, template <typename> typename Projection, std::size_t... indices>
        requires std::same_as<Action, std::remove_cvref_t<Action>>
    class ApplyArgsAction
    {
    public:
        [[nodiscard]]
        explicit(false) constexpr ApplyArgsAction(Action action)
            noexcept(std::is_nothrow_move_constructible_v<Action>)
            : m_Action{std::move(action)}
        {
        }

        template <std::size_t index, typename... Args>
        using ArgListElementT = std::tuple_element_t<index, std::tuple<Args...>>;

        template <std::size_t index, typename... Args>
        using ProjectedArgListElementT = Projection<ArgListElementT<index, Args...>>;

        template <typename Return, typename... Args>
            requires(... && (indices < sizeof...(Args)))
                 && std::invocable<
                        const Action&,
                        ProjectedArgListElementT<indices, Args...>...>
        constexpr decltype(auto) operator()(
            const call::Info<Return, Args...>& callInfo) const
            noexcept(std::is_nothrow_invocable_v<const Action&, ProjectedArgListElementT<indices, Args...>...>)
        {
            static_assert(
                (explicitly_convertible_to<
                     ArgListElementT<indices, Args...>&,
                     ProjectedArgListElementT<indices, Args...>>
                 && ...),
                "Projection can not be applied.");

            return std::invoke(
                m_Action,
                static_cast<ProjectedArgListElementT<indices, Args...>>(
                    std::get<indices>(callInfo.args).get())...);
        }

    private:
        [[no_unique_address]] Action m_Action;
    };

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
            requires std::invocable<const ArgSelector&, const call::Info<Return, Args...>&>
                  && std::invocable<
                         const ApplyStrategy&,
                         Action,
                         std::invoke_result_t<const ArgSelector&, const call::Info<Return, Args...>&>>
        constexpr decltype(auto) operator()(Action&& action, const call::Info<Return, Args...>& info) const
            noexcept(
                std::is_nothrow_invocable_v<const ArgSelector&, const call::Info<Return, Args...>&>
                && std::is_nothrow_invocable_v<
                    const ApplyStrategy&,
                    Action,
                    std::invoke_result_t<const ArgSelector&, const call::Info<Return, Args...>&>>)
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
}

namespace mimicpp::expect
{
    namespace detail
    {
        template <std::size_t index, std::size_t... others>
        struct arg_requirement_describer
        {
            [[nodiscard]]
            StringT operator()(const StringViewT matcherDescription) const
            {
                StringStreamT out{};
                out << "expect: arg[" << index;
                ((out << ", " << others), ...);
                out << "] " << matcherDescription;
                return std::move(out).str();
            }
        };

        template <
            std::size_t... indices,
            typename Matcher,
            typename... Projections>
        [[nodiscard]]
        constexpr auto make_args_policy(
            Matcher&& matcher,
            std::tuple<Projections...>&& projections)
        {
            static_assert(
                sizeof...(indices) == sizeof...(Projections),
                "Indices and projections size mismatch.");

            using arg_selector_t = expectation_policies::args_selector_fn<
                std::add_lvalue_reference_t,
                std::index_sequence<indices...>>;
            using apply_strategy_t = expectation_policies::arg_list_indirect_apply_fn<std::remove_cvref_t<Projections>...>;
            using describe_strategy_t = arg_requirement_describer<indices...>;

            return expectation_policies::ArgsRequirement{
                std::forward<Matcher>(matcher),
                expectation_policies::apply_args_fn(
                    arg_selector_t{},
                    apply_strategy_t{std::move(projections)}),
                describe_strategy_t{}};
        }
    }

    /**
     * \defgroup EXPECTATION_REQUIREMENT requirement
     * \ingroup EXPECTATION
     * \brief Requirements determine, whether an expectation matches an incoming call.
     * \details Requirements are the building blocks, which determine whether a call satisfies the expectation. If any of the specified
     * requirements fail, there is no match.
     * \note An expectation without requirements matches any call.
     *
     *\{
     */

    /**
     * \brief Checks, whether the selected argument matches the given matcher.
     * \tparam index The index of the selected argument.
     * \tparam Matcher The matcher type.
     * \param matcher The matcher.
     * \param projection Projection to apply to the argument.
     *
     * \details This requirement checks, whether the selected argument matches the given matcher. One argument can be checked multiple times
     * in different requirements and all results will be combined as conjunction.
     *
     * For a list of built-in matchers, see \ref EXPECTATION_MATCHER "matcher" section.
     * \snippet Requirements.cpp expect::arg
     */
    template <std::size_t index, typename Matcher, typename Projection = std::identity>
    [[nodiscard]]
    constexpr auto arg(
        Matcher&& matcher,
        Projection&& projection = {})
        noexcept(
            std::is_nothrow_constructible_v<std::remove_cvref_t<Matcher>, Matcher>
            && std::is_nothrow_constructible_v<std::remove_cvref_t<Projection>, Projection>)
    {
        return detail::make_args_policy<index>(
            std::forward<Matcher>(matcher),
            std::forward_as_tuple(std::forward<Projection>(projection)));
    }

    /**
     * \brief Checks, whether the selected arguments match the given matcher.
     * \tparam first The index of the first selected argument.
     * \tparam others The indices of other selected arguments.
     * \tparam Matcher The matcher type.
     * \param matcher The matcher.
     * \param projections Projections, the arguments will be applied on.
     *
     * \details This requirement checks, whether the selected arguments match the given matcher.
     * It's useful, when multiple arguments must be checked together, because they have some kind of relationship.
     * When ``n`` indices are provided the matcher must accept ``n`` arguments.
     *
     * The projections will be applied from the beginning: E.g. if ``n`` arguments and ``p`` projections are given
     * (with ``0 <= p <= n``), then the first ``p`` arguments will be applied on these projections (the ``i``-th argument
     * on the ``i``-th projection).
     * \note ``std::identity`` can be used to skip arguments, which shall not be projected.
     * \see https://en.cppreference.com/w/cpp/utility/functional/identity
     *
     * \details For a list of built-in matchers, see \ref EXPECTATION_MATCHER "matcher" section.
     * \snippet Requirements.cpp expect::args
     */
    template <
        std::size_t first,
        std::size_t... others,
        typename Matcher,
        typename... Projections>
    [[nodiscard]]
    constexpr auto args(Matcher&& matcher, Projections&&... projections)
        noexcept(
            std::is_nothrow_constructible_v<std::remove_cvref_t<Matcher>, Matcher>
            && (... && std::is_nothrow_move_constructible_v<std::remove_cvref_t<Projections>>))
    {
        static_assert(
            sizeof...(projections) <= 1u + sizeof...(others),
            "The projection count exceeds the amount of indices.");

        return detail::make_args_policy<first, others...>(
            std::forward<Matcher>(matcher),
            mimicpp::detail::expand_tuple<std::identity, 1u + sizeof...(others)>(
                std::forward_as_tuple(std::forward<Projections>(projections)...)));
    }

    /**
     * \}
     */
}

namespace mimicpp::finally
{
    // ReSharper disable CppDoxygenUnresolvedReference

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
            [fn = std::forward<Fun>(fun)]([[maybe_unused]] const auto& call) mutable noexcept(std::is_nothrow_invocable_v<decltype(fun)>) -> decltype(auto) {
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
        using ActionT = expectation_policies::ApplyArgsAction<
            std::remove_cvref_t<Action>,
            std::add_lvalue_reference_t,
            index,
            otherIndices...>;

        return expectation_policies::ReturnsResultOf{
            ActionT{std::forward<Action>(action)}};
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
        using ActionT = expectation_policies::ApplyAllArgsAction<
            std::remove_cvref_t<Action>,
            std::add_lvalue_reference_t>;

        return expectation_policies::ReturnsResultOf{
            ActionT{std::forward<Action>(action)}};
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
    constexpr auto returns_arg() noexcept
    {
        using ActionT = expectation_policies::ApplyArgsAction<
            std::identity,
            std::add_rvalue_reference_t,
            index>;

        return expectation_policies::ReturnsResultOf{
            ActionT{{}}};
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

    // ReSharper restore CppDoxygenUnresolvedReference
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
        using arg_selector_t = expectation_policies::args_selector_fn<
            std::add_lvalue_reference_t,
            std::index_sequence<index>>;
        using apply_strategy_t = expectation_policies::arg_list_forward_apply_fn;

        return expectation_policies::SideEffectAction{
            std::bind_front(
                expectation_policies::apply_args_fn{
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
        using arg_selector_t = expectation_policies::args_selector_fn<
            std::add_lvalue_reference_t,
            std::index_sequence<index, additionalIndices...>>;
        using apply_strategy_t = expectation_policies::arg_list_forward_apply_fn;

        return expectation_policies::SideEffectAction{
            std::bind_front(
                expectation_policies::apply_args_fn{
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
        using arg_selector_t = expectation_policies::all_args_selector_fn<std::add_lvalue_reference_t>;
        using apply_strategy_t = expectation_policies::arg_list_forward_apply_fn;

        return expectation_policies::SideEffectAction{
            std::bind_front(
                expectation_policies::apply_args_fn{
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
