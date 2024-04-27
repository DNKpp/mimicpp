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
#include <functional>

namespace mimicpp::expectation_policies
{
	class InitFinalize
	{
	public:
		template <typename Return, typename... Params>
		static constexpr void finalize_call(const call::Info<Return, Params...>&) noexcept
		{
		}
	};

	template <std::size_t min, std::size_t max>
		requires (min <= max)
	class Times
	{
	public:
		[[nodiscard]]
		constexpr bool is_satisfied() const noexcept
		{
			return min <= m_Count
					&& m_Count <= max;
		}

		[[nodiscard]]
		constexpr bool is_saturated() const noexcept
		{
			return m_Count == max;
		}

		constexpr void consume() noexcept
		{
			++m_Count;
		}

	private:
		std::size_t m_Count{};
	};

	class InitTimes
		: public Times<1u, 1u>
	{
	};

	class RuntimeTimes
	{
	public:
		[[nodiscard]]
		constexpr explicit RuntimeTimes(const std::size_t min, const std::size_t max)
			: m_Min{min},
			m_Max{max}
		{
			if (m_Max < m_Min)
			{
				throw std::runtime_error{"min must be less or equal to max."};
			}
		}

		[[nodiscard]]
		constexpr bool is_satisfied() const noexcept
		{
			return m_Min <= m_Count
					&& m_Count <= m_Max;
		}

		[[nodiscard]]
		constexpr bool is_saturated() const noexcept
		{
			return m_Count == m_Max;
		}

		constexpr void consume() noexcept
		{
			++m_Count;
		}

	private:
		std::size_t m_Min;
		std::size_t m_Max;
		std::size_t m_Count{};
	};

	template <ValueCategory expected>
	class Category
	{
	public:
		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		template <typename Return, typename... Params>
		static call::SubMatchResult matches(const call::Info<Return, Params...>& info)
		{
			if (mimicpp::is_matching(info.fromCategory, expected))
			{
				return {
					.matched = true,
					.msg = format::format(" matches Category {}", expected)
				};
			}

			return {
				.matched = false,
				.msg = format::format(" does not match Category {}", expected)
			};
		}

		template <typename Return, typename... Params>
		static constexpr void consume(const call::Info<Return, Params...>& info) noexcept
		{
			assert(mimicpp::is_matching(info.fromCategory, expected) && "Call does not match.");
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

		template <typename Return, typename... Params>
		static constexpr call::SubMatchResult matches(const call::Info<Return, Params...>& info) noexcept
		{
			if (mimicpp::is_matching(info.fromConstness, constness))
			{
				return {
					.matched = true,
					.msg = format::format(" matches Constness {}", constness)
				};
			}

			return {
				.matched = false,
				.msg = format::format(" does not match Constness {}", constness)
			};
		}

		template <typename Return, typename... Params>
		static constexpr void consume(const call::Info<Return, Params...>& info) noexcept
		{
			assert(mimicpp::is_matching(info.fromConstness, constness) && "Call does not match.");
		}
	};

	template <typename Action>
		requires std::same_as<Action, std::remove_cvref_t<Action>>
				&& std::is_move_constructible_v<Action>
	class ReturnsResultOf
	{
	public:
		[[nodiscard]]
		explicit constexpr ReturnsResultOf(
			Action&& action
		) noexcept(std::is_nothrow_move_constructible_v<Action>)
			: m_Action{std::move(action)}
		{
		}

		template <typename Return, typename... Params>
			requires std::invocable<Action&, const call::Info<Return, Params...>&>
					&& explicitly_convertible_to<
						std::invoke_result_t<Action&, const call::Info<Return, Params...>&>,
						Return>
		[[nodiscard]]
		constexpr Return finalize_call(
			[[maybe_unused]] const call::Info<Return, Params...>& call
		) noexcept(
			std::is_nothrow_invocable_v<Action&, const call::Info<Return, Params...>&>
			&& nothrow_explicitly_convertible_to<
				std::invoke_result_t<Action&, const call::Info<Return, Params...>&>,
				Return>)
		{
			return static_cast<Return>(
				std::invoke(m_Action, call));
		}

	private:
		Action m_Action;
	};

	template <typename Exception>
		requires (!std::is_reference_v<Exception>)
				&& std::copyable<Exception>
	class Throws
	{
	public:
		[[nodiscard]]
		explicit constexpr Throws(Exception exception) noexcept(std::is_nothrow_move_constructible_v<Exception>)
			: m_Exception{std::move(exception)}
		{
		}

		template <typename Return, typename... Params>
		constexpr Return finalize_call(
			[[maybe_unused]] const call::Info<Return, Params...>& call
		)
		{
			throw m_Exception;  // NOLINT(hicpp-exception-baseclass)
		}

	private:
		Exception m_Exception;
	};

	template <
		typename Signature,
		std::size_t index,
		matcher_for<signature_param_type_t<index, Signature>> Matcher>
	class ArgumentMatcher
	{
	public:
		using ParamT = signature_param_type_t<index, Signature>;
		using CallInfoT = call::info_for_signature_t<Signature>;

		~ArgumentMatcher() = default;

		[[nodiscard]]
		explicit constexpr ArgumentMatcher(
			Matcher&& matcher
		) noexcept(std::is_nothrow_move_constructible_v<Matcher>)
			: m_Matcher{std::move(matcher)}
		{
		}

		ArgumentMatcher(const ArgumentMatcher&) = delete;
		ArgumentMatcher& operator =(const ArgumentMatcher&) = delete;

		[[nodiscard]]
		ArgumentMatcher(ArgumentMatcher&&) = default;
		ArgumentMatcher& operator =(ArgumentMatcher&&) = default;

		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		[[nodiscard]]
		constexpr call::SubMatchResult matches(const CallInfoT& info) const noexcept
		{
			const std::reference_wrapper param = std::get<index>(info.params);
			if (m_Matcher.matches(param.get()))
			{
				return {
					.matched = true,
					.msg = format::format(
						"param[{}] matches {}",
						index,
						m_Matcher.describe(param.get()))
				};
			}

			return {
				.matched = false,
				.msg = format::format(
					"param[{}] does not match {}",
					index,
					m_Matcher.describe(param.get()))
			};
		}

		static constexpr void consume(const CallInfoT&) noexcept
		{
		}

	private:
		[[no_unique_address]] Matcher m_Matcher;
	};

	template <typename Signature, std::size_t index, typename Matcher>
		requires matcher_for<std::remove_cvref_t<Matcher>, signature_param_type_t<index, Signature>>
	[[nodiscard]]
	constexpr ArgumentMatcher<Signature, index, std::remove_cvref_t<Matcher>> make_argument_matcher(
		Matcher&& matcher
	) noexcept(std::is_nothrow_move_constructible_v<std::remove_cvref_t<Matcher>>)
	{
		return ArgumentMatcher<Signature, index, std::remove_cvref_t<Matcher>>{
			std::forward<Matcher>(matcher)
		};
	}

	template <typename Signature, std::size_t index, std::equality_comparable_with<signature_param_type_t<index, Signature>> Value>
	[[nodiscard]]
	constexpr ArgumentMatcher<Signature, index, decltype(matches::eq(std::declval<Value>()))> make_argument_matcher(
		Value&& value
	)
	{
		return ArgumentMatcher<Signature, index, decltype(matches::eq(std::declval<Value>()))>{
			matches::eq(std::forward<Value>(value))
		};
	}

	template <typename Action>
	class SideEffectAction
	{
	public:
		~SideEffectAction() = default;

		[[nodiscard]]
		explicit constexpr SideEffectAction(
			Action&& action
		) noexcept(std::is_nothrow_move_constructible_v<Action>)
			: m_Action{std::move(action)}
		{
		}

		SideEffectAction(const SideEffectAction&) = delete;
		SideEffectAction& operator =(const SideEffectAction&) = delete;

		[[nodiscard]]
		SideEffectAction(SideEffectAction&&) = default;
		SideEffectAction& operator =(SideEffectAction&&) = default;

		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		template <typename Return, typename... Params>
		[[nodiscard]]
		static constexpr call::SubMatchResult matches(const call::Info<Return, Params...>&) noexcept
		{
			return {true};
		}

		template <typename Return, typename... Params>
			requires std::invocable<Action&, const call::Info<Return, Params...>&>
		constexpr void consume(
			const call::Info<Return, Params...>& info
		) noexcept(std::is_nothrow_invocable_v<Action&, const call::Info<Return, Params...>&>)
		{
			std::invoke(m_Action, info);
		}

	private:
		Action m_Action;
	};

	template <typename Action, template <typename> typename Projection>
		requires std::same_as<Action, std::remove_cvref_t<Action>>
	class ApplyAllParamsAction
	{
	public:
		[[nodiscard]]
		explicit constexpr ApplyAllParamsAction(
			Action&& action
		) noexcept(std::is_nothrow_move_constructible_v<Action>)
			: m_Action{std::move(action)}
		{
		}

		template <typename Param>
		using ProjectedParamElementT = Projection<Param>;

		template <typename Return, typename... Params>
			requires std::invocable<
				Action&,
				ProjectedParamElementT<Params>...>
		constexpr decltype(auto) operator ()(
			const call::Info<Return, Params...>& callInfo
		) noexcept(
			std::is_nothrow_invocable_v<
				Action&,
				ProjectedParamElementT<Params>...>)
		{
			static_assert(
				(... && explicitly_convertible_to<Params&, ProjectedParamElementT<Params>>),
				"Projection can not be applied.");

			return std::apply(
				[this](auto&... params) -> decltype(auto)
				{
					return std::invoke(
						m_Action,
						static_cast<ProjectedParamElementT<Params>>(
							params.get())...);
				},
				callInfo.params);
		}

	private:
		Action m_Action;
	};

	template <typename Action, template <typename> typename Projection, std::size_t... indices>
		requires std::same_as<Action, std::remove_cvref_t<Action>>
	class ApplyParamsAction
	{
	public:
		explicit constexpr ApplyParamsAction(
			Action&& action
		) noexcept(std::is_nothrow_move_constructible_v<Action>)
			: m_Action{std::move(action)}
		{
		}

		template <std::size_t index, typename... Params>
		using ParamElementT = std::tuple_element_t<index, std::tuple<Params...>>;

		template <std::size_t index, typename... Params>
		using ProjectedParamElementT = Projection<ParamElementT<index, Params...>>;

		template <typename Return, typename... Params>
			requires (... && (indices < sizeof...(Params)))
					&& std::invocable<
						Action,
						ProjectedParamElementT<indices, Params...>...>
		constexpr decltype(auto) operator ()(
			const call::Info<Return, Params...>& callInfo
		) noexcept(
			std::is_nothrow_invocable_v<
				Action&,
				ProjectedParamElementT<indices, Params...>...>)
		{
			static_assert(
				(explicitly_convertible_to<
						ParamElementT<indices, Params...>&,
						ProjectedParamElementT<indices, Params...>>
					&& ...),
				"Projection can not be applied.");

			return std::invoke(
				m_Action,
				static_cast<ProjectedParamElementT<indices, Params...>>(
					std::get<indices>(callInfo.params).get())...);
		}

	private:
		Action m_Action;
	};
}

namespace mimicpp::expect
{
	template <std::size_t min, std::size_t max = min>
	[[nodiscard]]
	consteval expectation_policies::Times<min, max> times() noexcept
	{
		return {};
	}

	template <std::size_t min>
	[[nodiscard]]
	consteval expectation_policies::Times<min, std::numeric_limits<std::size_t>::max()> at_least() noexcept
	{
		return {};
	}

	template <std::size_t max>
	[[nodiscard]]
	consteval expectation_policies::Times<0u, max> at_most() noexcept
	{
		return {};
	}

	[[nodiscard]]
	consteval expectation_policies::Times<1u, 1u> once() noexcept
	{
		return {};
	}

	[[nodiscard]]
	consteval expectation_policies::Times<2u, 2u> twice() noexcept
	{
		return {};
	}

	[[nodiscard]]
	constexpr expectation_policies::RuntimeTimes times(const std::size_t min, const std::size_t max)
	{
		return expectation_policies::RuntimeTimes{min, max};
	}

	[[nodiscard]]
	constexpr expectation_policies::RuntimeTimes times(const std::size_t exactly) noexcept
	{
		return times(exactly, exactly);
	}

	[[nodiscard]]
	constexpr expectation_policies::RuntimeTimes at_least(const std::size_t min) noexcept
	{
		return expectation_policies::RuntimeTimes{min, std::numeric_limits<std::size_t>::max()};
	}

	[[nodiscard]]
	constexpr expectation_policies::RuntimeTimes at_most(const std::size_t max) noexcept
	{
		return expectation_policies::RuntimeTimes{0u, max};
	}
}

namespace mimicpp::finally::detail
{
	struct forward_fn
	{
		template <typename T>
		[[nodiscard]]
		constexpr T&& operator ()(T&& obj) const noexcept
		{
			return std::forward<T>(obj);
		}
	};
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
	constexpr auto returns_result_of(
		Fun&& fun  // NOLINT(cppcoreguidelines-missing-std-forward)
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Fun>, Fun>)
	{
		return expectation_policies::ReturnsResultOf{
			[
				fun = std::forward<Fun>(fun)
			]([[maybe_unused]] const auto& call) mutable noexcept(std::is_nothrow_invocable_v<decltype(fun)>) -> decltype(auto)
			{
				return std::invoke(fun);
			}
		};
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
	constexpr auto returns(
		T&& value  // NOLINT(cppcoreguidelines-missing-std-forward)
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<T>, T>)
	{
		return expectation_policies::ReturnsResultOf{
			[v = std::forward<T>(value)]([[maybe_unused]] const auto& call) mutable noexcept -> auto& {
				return static_cast<std::unwrap_reference_t<decltype(v)>&>(v);
			}
		};
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
	 * as desired, but be aware that this may actually affect objects outside the call (e.g. if call params are lvalues.).
	 * \snippet Finalizers.cpp finally::returns_apply_result_of
	 */
	template <std::size_t index, std::size_t... otherIndices, typename Action>
	[[nodiscard]]
	constexpr auto returns_apply_result_of(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::ReturnsResultOf{
			expectation_policies::ApplyParamsAction<
				std::remove_cvref_t<Action>,
				std::add_lvalue_reference_t,
				index,
				otherIndices...>{
				std::forward<Action>(action)
			}
		};
	}

	/**
	 * \brief During the finalization step, the all call arguments are applied on the given action.
	 * \tparam Action The action type.
	 * \param action The action to be applied to.
	 * \return Returns the invocation result of the given action.
	 *
	 * \details All call arguments are applied as (possibly const qualified) lvalue-references. The action may proceed with them
	 * as desired, but be aware that this may actually affect objects outside the call (e.g. if call params are lvalues.).
	 * \snippet Finalizers.cpp finally::returns_apply_all_result_of
	 */
	template <typename Action>
	[[nodiscard]]
	constexpr auto returns_apply_all_result_of(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::ReturnsResultOf{
			expectation_policies::ApplyAllParamsAction<
				std::remove_cvref_t<Action>,
				std::add_lvalue_reference_t>{
				std::forward<Action>(action)
			}
		};
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
	constexpr auto returns_param() noexcept
	{
		return expectation_policies::ReturnsResultOf{
			expectation_policies::ApplyParamsAction<
				detail::forward_fn,
				std::add_rvalue_reference_t,
				index>{
				detail::forward_fn{}
			}
		};
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
	constexpr expectation_policies::Throws<std::remove_cvref_t<T>> throws(
		T&& exception
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<T>, T>)
	{
		return expectation_policies::Throws<std::remove_cvref_t<T>>{
			std::forward<T>(exception)
		};
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
	 * \brief Side effects are an easy way to apply actions on matched expectations.
	 * \details After an expectation match has been found, side effects will be applied during the ``consume`` step.
	 * They may alter the call params and capture any variable from the outside scope. Beware that those captured variables
	 * must outlive the expectation they are attached on.
	 *
	 * Side effects will be executed in their construction order, but they should actually never throw. If it is intended to
	 * actually throw an exception as a result, use ``expect::throws`` instead.
	 *
	 * As side effects actually are ``expectation policies``, they are in fact treated as such and may carry out special
	 * behavior during ``is_satisfied`` and ``matches`` calls. That being said, the provided side effects are actually no-ops
	 * on these functions, but custom side effects may behave differently.
	 *\{
	 */

	/**
	 * \brief Applies the ``param[index]`` on the given action.
	 * \tparam index The param index.
	 * \tparam Action The action type.
	 * \param action The action to be applied.
	 * \return Newly created side effect action.
	 */
	template <std::size_t index, typename Action>
	[[nodiscard]]
	constexpr auto apply_param(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::SideEffectAction{
			expectation_policies::ApplyParamsAction<
				std::remove_cvref_t<Action>,
				std::add_lvalue_reference_t,
				index>{
				std::forward<Action>(action)
			}
		};
	}

	/**
	 * \brief Applies ``param[indices]...`` in the specified order on the given action.
	 * \details This functions creates a side effect policy and applies the desired params in the specified order.
	 * The indices can be in any order and may also contain duplicates.
	 * \tparam index The first param index.
	 * \tparam additionalIndices Additional param indices.
	 * \tparam Action The action type.
	 * \param action The action to be applied.
	 * \return Newly created side effect action.
	 */
	template <std::size_t index, std::size_t... additionalIndices, typename Action>
	[[nodiscard]]
	constexpr auto apply_params(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::SideEffectAction{
			expectation_policies::ApplyParamsAction<
				std::remove_cvref_t<Action>,
				std::add_lvalue_reference_t,
				index,
				additionalIndices...>{
				std::forward<Action>(action)
			}
		};
	}

	/**
	 * \brief Applies all available params on the given action.
	 * \tparam Action The action type.
	 * \param action The action to be applied.
	 * \return Newly created side effect action.
	 */
	template <typename Action>
	[[nodiscard]]
	constexpr auto apply_all_params(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::SideEffectAction{
			expectation_policies::ApplyAllParamsAction<
				std::remove_cvref_t<Action>,
				std::add_lvalue_reference_t>{
				std::forward<Action>(action)
			}
		};
	}

	/**
	 * \brief Invokes the given function.
	 * \tparam Action  The action type.
	 * \param action The action to be invoked.
	 * \return Newly created side effect action.
	 */
	template <std::invocable Action>
	[[nodiscard]]
	constexpr auto apply(
		Action&& action  // NOLINT(cppcoreguidelines-missing-std-forward)
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::SideEffectAction{
			[
				action = std::forward<Action>(action)
			]([[maybe_unused]] const auto& call) mutable noexcept(std::is_nothrow_invocable_v<Action&>)
			{
				std::invoke(action);
			}
		};
	}

	/**
	 * \}
	 */
}

#endif
