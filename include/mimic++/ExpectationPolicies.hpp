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

	template <typename Action, std::size_t... indices>
	class ParamsSideEffect
	{
	public:
		~ParamsSideEffect() = default;

		[[nodiscard]]
		explicit constexpr ParamsSideEffect(
			Action&& action
		) noexcept(std::is_nothrow_move_constructible_v<Action>)
			: m_Action{std::move(action)}
		{
		}

		ParamsSideEffect(const ParamsSideEffect&) = delete;
		ParamsSideEffect& operator =(const ParamsSideEffect&) = delete;

		[[nodiscard]]
		ParamsSideEffect(ParamsSideEffect&&) = default;
		ParamsSideEffect& operator =(ParamsSideEffect&&) = default;

		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		template <typename Return, typename... Params>
			requires (... && (indices < sizeof...(Params)))
		[[nodiscard]]
		static constexpr call::SubMatchResult matches(const call::Info<Return, Params...>&) noexcept
		{
			return {true};
		}

		template <typename Return, typename... Params>
			requires (... && (indices < sizeof...(Params)))
					&& std::invocable<Action&, std::tuple_element_t<indices, std::tuple<Params...>>&...>
		constexpr void consume(
			const call::Info<Return, Params...>& info
		) noexcept(std::is_nothrow_invocable_v<Action&, std::tuple_element_t<indices, std::tuple<Params...>>&...>)
		{
			std::invoke(
				m_Action,
				std::get<indices>(info.params).get()...);
		}

	private:
		Action m_Action;
	};

	template <std::size_t... indices, typename Action>
	[[nodiscard]]
	constexpr ParamsSideEffect<std::remove_cvref_t<Action>, indices...> make_param_side_effect(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return ParamsSideEffect<std::remove_cvref_t<Action>, indices...>{
			std::forward<Action>(action)
		};
	}

	template <typename Action>
	class AllParamsSideEffect
	{
	public:
		~AllParamsSideEffect() = default;

		[[nodiscard]]
		explicit constexpr AllParamsSideEffect(
			Action&& action
		) noexcept(std::is_nothrow_move_constructible_v<Action>)
			: m_Action{std::move(action)}
		{
		}

		AllParamsSideEffect(const AllParamsSideEffect&) = delete;
		AllParamsSideEffect& operator =(const AllParamsSideEffect&) = delete;

		[[nodiscard]]
		AllParamsSideEffect(AllParamsSideEffect&&) = default;
		AllParamsSideEffect& operator =(AllParamsSideEffect&&) = default;

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
			requires std::invocable<Action&, Params&...>
		constexpr void consume(
			const call::Info<Return, Params...>& info
		) noexcept(std::is_nothrow_invocable_v<Action&, Params&...>)
		{
			return std::apply(
				[this](auto&... params) { return std::invoke(m_Action, params.get()...); },
				info.params);
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

namespace mimicpp::finally
{
	template <typename Action>
		requires std::invocable<std::remove_cvref_t<Action>&>
				&& (!std::is_void_v<std::invoke_result_t<std::remove_cvref_t<Action>&>>)
	[[nodiscard]]
	constexpr auto returns_result_of(
		Action&& action  // NOLINT(cppcoreguidelines-missing-std-forward)
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::ReturnsResultOf{
			[
				action = std::forward<Action>(action)
			]([[maybe_unused]] const auto& call) mutable noexcept(std::is_nothrow_invocable_v<decltype(action)>) -> decltype(auto)
			{
				return std::invoke(action);
			}
		};
	}

	template <typename T>
		requires std::copyable<std::remove_cvref_t<T>>
	[[nodiscard]]
	constexpr auto returns(
		T&& value  // NOLINT(cppcoreguidelines-missing-std-forward)
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<T>, T>)
	{
		return expectation_policies::ReturnsResultOf{
			[v = std::forward<T>(value)]([[maybe_unused]] const auto& call) mutable noexcept -> auto&
			{
				return static_cast<std::unwrap_reference_t<decltype(v)>&>(v);
			}			
		};
	}

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
	constexpr expectation_policies::ParamsSideEffect<std::remove_cvref_t<Action>, index> apply_param(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::make_param_side_effect<index>(std::forward<Action>(action));
	}

	/**
	 * \brief Applies ``param[indices]...`` in the specified order on the given action.
	 * \details This functions creates a side effect policy and applies the desired params in the specified order.
	 * The indices can be in any order and may also contain duplicates.
	 * \tparam indices The param indices.
	 * \tparam Action The action type.
	 * \param action The action to be applied.
	 * \return Newly created side effect action.
	 */
	template <std::size_t... indices, typename Action>
	[[nodiscard]]
	constexpr expectation_policies::ParamsSideEffect<std::remove_cvref_t<Action>, indices...> apply_params(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::make_param_side_effect<indices...>(std::forward<Action>(action));
	}

	/**
	 * \brief Applies all available params on the given action.
	 * \tparam Action The action type.
	 * \param action The action to be applied.
	 * \return Newly created side effect action.
	 */
	template <typename Action>
	[[nodiscard]]
	constexpr expectation_policies::AllParamsSideEffect<std::remove_cvref_t<Action>> apply_all_params(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::AllParamsSideEffect{std::forward<Action>(action)};
	}

	/**
	 * \brief Invokes the given function.
	 * \tparam Action  The action type.
	 * \param action The action to be invoked.
	 * \return Newly created side effect action.
	 */
	template <std::invocable Action>
	[[nodiscard]]
	constexpr expectation_policies::ParamsSideEffect<std::remove_cvref_t<Action>> apply(
		Action&& action
	) noexcept(std::is_nothrow_constructible_v<std::remove_cvref_t<Action>, Action>)
	{
		return expectation_policies::ParamsSideEffect<std::remove_cvref_t<Action>>{
			std::forward<Action>(action)
		};
	}

	/**
	 * \}
	 */
}

#endif
