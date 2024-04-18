// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_POLICIES_CALL_PROPERTIES_HPP
#define MIMICPP_EXPECTATION_POLICIES_CALL_PROPERTIES_HPP

#pragma once

#include <cassert>
#include <functional>

#include "mimic++/Expectation.hpp"
#include "mimic++/Printer.hpp"

namespace mimicpp::expectation_policies
{
	class InitFinalize
	{
	public:
		template <typename Signature>
		static constexpr void finalize_call(const call::Info<Signature>&) noexcept
		{
		}
	};

	template <std::size_t min, std::size_t max = min>
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
		: public Times<1u>
	{
	};

	class RuntimeTimes
	{
	public:
		constexpr explicit RuntimeTimes(const std::size_t min, const std::size_t max) noexcept
			: m_Min{min},
			m_Max{max}
		{
		}

		constexpr explicit RuntimeTimes(const std::size_t min) noexcept
			: m_Min{min},
			m_Max{min}
		{
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

		template <typename Signature>
		static constexpr call::SubMatchResult matches(const call::Info<Signature>& info)
		{
			if (info.fromCategory == expected)
			{
				return {
					.matched = true,
					.msg = std::format(" matches Category {}", expected)
				};
			}

			return {
				.matched = false,
				.msg = std::format(" does not match Category {}", expected)
			};
		}

		template <typename Signature>
		static constexpr void consume(const call::Info<Signature>& call) noexcept
		{
			assert(call.fromCategory == expected && "Call does not match.");
		}
	};

	template <bool expectsConst>
	class Constness
	{
	public:
		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		template <typename Signature>
		static constexpr call::SubMatchResult matches(const call::Info<Signature>& info) noexcept
		{
			if (info.fromConst == expectsConst)
			{
				return {
					.matched = true,
					.msg = std::format(" matches Constness {}", expectsConst)
				};
			}

			return {
				.matched = false,
				.msg = std::format(" does not match Constness {}", expectsConst)
			};
		}

		template <typename Signature>
		static constexpr void consume(const call::Info<Signature>& call) noexcept
		{
			assert(call.fromConst == expectsConst && "Call does not match.");
		}
	};

	class NullDescriber
	{
	public:
		constexpr std::optional<StringT> operator ()(const bool, const auto&) const noexcept
		{
			return std::nullopt;
		}
	};

	template <typename T, typename Param>
	concept describer = std::regular_invocable<T, bool, Param>
						&& std::convertible_to<
							std::invoke_result_t<T, bool, Param>,
							std::optional<StringT>>;

	template <
		typename Signature,
		std::size_t index,
		std::predicate<signature_param_type_t<index, Signature>> Predicate,
		describer<signature_param_type_t<index, Signature>> Describer = NullDescriber>
	class ArgumentMatcher
	{
	public:
		using ParamT = signature_param_type_t<index, Signature>;
		using CallInfoT = call::Info<Signature>;

		~ArgumentMatcher() = default;

		[[nodiscard]]
		explicit constexpr ArgumentMatcher(
			Predicate predicate = {},
			Describer describer = {}
		) noexcept(
			std::is_nothrow_move_constructible_v<Predicate>
			&& std::is_nothrow_move_constructible_v<Describer>)
			: m_Predicate{std::move(predicate)},
			m_Describer{std::move(describer)}
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
			const bool matched = std::invoke(m_Predicate, param.get());
			return {
				.matched = matched,
				.msg = std::invoke(m_Describer, matched, param.get())
			};
		}

		static constexpr void consume(const CallInfoT&) noexcept
		{
		}

	private:
		[[no_unique_address]] Predicate m_Predicate;
		[[no_unique_address]] Describer m_Describer;
	};

	template <typename Signature, std::size_t index, typename Predicate, typename Describer = NullDescriber>
	[[nodiscard]]
	constexpr ArgumentMatcher<Signature, index, Predicate, Describer> make_argument_matcher(
		Predicate predicate = {},
		Describer describer = {}
	) noexcept(
		std::is_nothrow_move_constructible_v<Predicate>
		&& std::is_nothrow_move_constructible_v<Describer>)
	{
		return ArgumentMatcher<Signature, index, Predicate, Describer>{
			std::move(predicate),
			std::move(describer)
		};
	}
}

#endif