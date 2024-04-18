// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_BUILDER_HPP
#define MIMICPP_EXPECTATION_BUILDER_HPP

#pragma once

#include "mimic++/Expectation.hpp"

namespace mimicpp
{
	class InitFinalizePolicy
	{
	public:
		template <typename Signature>
		static constexpr void finalize_call(const call::Info<Signature>&) noexcept
		{
		}
	};

	static_assert(finalize_policy_for<InitFinalizePolicy, void()>);

	class InitTimesPolicy
	{
	public:
		[[nodiscard]]
		constexpr bool is_satisfied() const noexcept
		{
			return m_Called;
		}

		[[nodiscard]]
		constexpr bool is_saturated() const noexcept
		{
			return m_Called;
		}

		constexpr void consume() noexcept
		{
			assert(!m_Called && "Times policy is already saturated.");
			m_Called = true;
		}

	private:
		bool m_Called{};
	};

	static_assert(times_policy<InitTimesPolicy>);

	template <
		typename Signature,
		times_policy TimesPolicy,
		typename FinalizePolicy,
		expectation_policy_for<Signature>... Policies>
	class BasicExpectationBuilder
	{
	public:
		using StorageT = ExpectationCollection<Signature>;
		using ScopedExpectationT = ScopedExpectation<Signature>;
		using PolicyListT = std::tuple<Policies...>;
		using ReturnT = typename Expectation<Signature>::ReturnT;

		~BasicExpectationBuilder() = default;

		template <typename TimesPolicyArg, typename FinalizePolicyArg, typename PolicyListArg>
			requires std::constructible_from<TimesPolicy, TimesPolicyArg>
					&& std::constructible_from<FinalizePolicy, FinalizePolicyArg>
					&& std::constructible_from<PolicyListT, PolicyListArg>
		[[nodiscard]]
		explicit constexpr BasicExpectationBuilder(
			std::shared_ptr<StorageT> storage,
			TimesPolicyArg&& timesPolicyArg,
			FinalizePolicyArg&& finalizePolicyArg,
			PolicyListArg&& policyListArg
		) noexcept
			: m_Storage{std::move(storage)},
			m_TimesPolicy{std::forward<TimesPolicyArg>(timesPolicyArg)},
			m_FinalizePolicy{std::forward<FinalizePolicyArg>(finalizePolicyArg)},
			m_ExpectationPolicies{std::forward<PolicyListArg>(policyListArg)}
		{
			assert(m_Storage && "Storage is nullptr.");
		}

		BasicExpectationBuilder(const BasicExpectationBuilder&) = delete;
		BasicExpectationBuilder& operator =(const BasicExpectationBuilder&) = delete;

		[[nodiscard]]
		BasicExpectationBuilder(BasicExpectationBuilder&&) = default;
		BasicExpectationBuilder& operator =(BasicExpectationBuilder&&) = default;

		template <typename Policy>
			requires std::same_as<InitTimesPolicy, TimesPolicy>
					&& (!std::same_as<InitTimesPolicy, std::remove_cvref_t<Policy>>)
					&& times_policy<std::remove_cvref_t<Policy>>
		[[nodiscard]]
		constexpr auto operator |(Policy&& policy) &&
		{
			using ExtendedExpectationBuilderT = BasicExpectationBuilder<
				Signature,
				std::remove_cvref_t<Policy>,
				FinalizePolicy,
				Policies...>;

			return ExtendedExpectationBuilderT{
				std::move(m_Storage),
				std::forward<Policy>(policy),
				std::move(m_FinalizePolicy),
				std::move(m_ExpectationPolicies)
			};
		}

		template <typename Policy>
			requires std::same_as<InitFinalizePolicy, FinalizePolicy>
					&& (!std::same_as<InitFinalizePolicy, std::remove_cvref_t<Policy>>)
					&& finalize_policy_for<std::remove_cvref_t<Policy>, Signature>
		[[nodiscard]]
		constexpr auto operator |(Policy&& policy) &&
		{
			using ExtendedExpectationBuilderT = BasicExpectationBuilder<
				Signature,
				TimesPolicy,
				std::remove_cvref_t<Policy>,
				Policies...>;

			return ExtendedExpectationBuilderT{
				std::move(m_Storage),
				std::move(m_TimesPolicy),
				std::forward<Policy>(policy),
				std::move(m_ExpectationPolicies)
			};
		}

		template <typename Policy>
			requires expectation_policy_for<std::remove_cvref_t<Policy>, Signature>
		[[nodiscard]]
		constexpr auto operator |(Policy&& policy) &&
		{
			using ExtendedExpectationBuilderT = BasicExpectationBuilder<
				Signature,
				TimesPolicy,
				FinalizePolicy,
				Policies...,
				std::remove_cvref_t<Policy>>;

			return ExtendedExpectationBuilderT{
				std::move(m_Storage),
				std::move(m_TimesPolicy),
				std::move(m_FinalizePolicy),
				std::apply(
					[&](auto&... policies) noexcept
					{
						return std::forward_as_tuple(
							std::move(policies)...,
							std::forward<Policy>(policy));
					},
					m_ExpectationPolicies)
			};
		}

		[[nodiscard]]
		explicit(false) constexpr operator ScopedExpectationT() &&
		{
			static_assert(
				finalize_policy_for<FinalizePolicy, Signature>,
				"For non-void return types, a finalize policy must be set.");

			using ExpectationT = BasicExpectation<Signature, TimesPolicy, FinalizePolicy, Policies...>;

			return ScopedExpectationT{
				std::move(m_Storage),
				std::apply(
					[&](auto&... policies)
					{
						return std::make_unique<ExpectationT>(
							std::move(m_TimesPolicy),
							std::move(m_FinalizePolicy),
							std::move(policies)...);
					},
					m_ExpectationPolicies)
			};
		}

	private:
		std::shared_ptr<StorageT> m_Storage;
		TimesPolicy m_TimesPolicy{};
		FinalizePolicy m_FinalizePolicy{};
		PolicyListT m_ExpectationPolicies{};
	};
}

#endif
