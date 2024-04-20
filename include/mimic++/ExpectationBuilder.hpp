// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_BUILDER_HPP
#define MIMICPP_EXPECTATION_BUILDER_HPP

#pragma once

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationPolicies.hpp"

namespace mimicpp
{
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
			requires std::same_as<expectation_policies::InitTimes, TimesPolicy>
					&& (!std::same_as<expectation_policies::InitTimes, std::remove_cvref_t<Policy>>)
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
			requires std::same_as<expectation_policies::InitFinalize, FinalizePolicy>
					&& (!std::same_as<expectation_policies::InitFinalize, std::remove_cvref_t<Policy>>)
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
		constexpr ScopedExpectationT finalize() &&
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

namespace mimicpp::detail
{
	class BuildFinalizer
	{
	public:
		template <
			typename Signature,
			times_policy TimesPolicy,
			finalize_policy_for<Signature> FinalizePolicy,
			expectation_policy_for<Signature>... Policies>
		[[nodiscard]]
		friend constexpr ScopedExpectation<Signature> operator <<=(
			const BuildFinalizer&&,
			BasicExpectationBuilder<Signature, TimesPolicy, FinalizePolicy, Policies...>&& builder
			)
		{
			return std::move(builder).finalize();
		}
	};
}

#define MIMICPP_UNIQUE_NAME(prefix, counter) prefix##counter
#define MIMICPP_SCOPED_EXPECTATION_IMPL(counter) \
	[[maybe_unused]] const ::mimicpp::ScopedExpectation MIMICPP_UNIQUE_NAME(_mimicpp_expectation_, counter) = \
	mimicpp::detail::BuildFinalizer{} <<= 

#define MIMICPP_SCOPED_EXPECTATION MIMICPP_SCOPED_EXPECTATION_IMPL(__COUNTER__)

#endif
