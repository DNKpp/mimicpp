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
	template <typename Signature, ValueCategory expected>
	class Category
	{
	public:
		using CallInfoT = call::Info<Signature>;

		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		static constexpr call::SubMatchResult matches(const CallInfoT& info)
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

		static constexpr void consume(const CallInfoT& call) noexcept
		{
			assert(call.fromCategory == expected && "Call does not match.");
		}
	};

	template <typename Signature, bool expectsConst>
	class Constness
	{
	public:
		using CallInfoT = call::Info<Signature>;

		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		static constexpr call::SubMatchResult matches(const CallInfoT& info) noexcept
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

		static constexpr void consume(const CallInfoT& call) noexcept
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
