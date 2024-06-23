// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_BUILDER_HPP
#define MIMICPP_EXPECTATION_BUILDER_HPP

#pragma once

#include "mimic++/ControlPolicy.hpp"
#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationPolicies.hpp"
#include "mimic++/Sequence.hpp"

namespace mimicpp
{
	template <
		bool timesConfigured,
		typename SequenceConfig,
		typename Signature,
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

		template <typename FinalizePolicyArg, typename PolicyListArg>
			requires std::constructible_from<FinalizePolicy, FinalizePolicyArg>
					&& std::constructible_from<PolicyListT, PolicyListArg>
		[[nodiscard]]
		explicit constexpr BasicExpectationBuilder(
			std::shared_ptr<StorageT> storage,
			TimesConfig timesConfig,
			SequenceConfig sequenceConfig,
			FinalizePolicyArg&& finalizePolicyArg,
			PolicyListArg&& policyListArg
		) noexcept
			: m_Storage{std::move(storage)},
			m_TimesConfig{std::move(timesConfig)},
			m_SequenceConfig{std::move(sequenceConfig)},
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
			requires std::same_as<expectation_policies::InitFinalize, FinalizePolicy>
					&& (!std::same_as<expectation_policies::InitFinalize, std::remove_cvref_t<Policy>>)
					&& finalize_policy_for<std::remove_cvref_t<Policy>, Signature>
		[[nodiscard]]
		constexpr auto operator |(Policy&& policy) &&
		{
			using ExtendedExpectationBuilderT = BasicExpectationBuilder<
				timesConfigured,
				SequenceConfig,
				Signature,
				std::remove_cvref_t<Policy>,
				Policies...>;

			return ExtendedExpectationBuilderT{
				std::move(m_Storage),
				std::move(m_TimesConfig),
				std::move(m_SequenceConfig),
				std::forward<Policy>(policy),
				std::move(m_ExpectationPolicies)
			};
		}

		template <typename Policy>
			requires expectation_policy_for<std::remove_cvref_t<Policy>, Signature>
		[[nodiscard]]
		constexpr auto operator |(Policy&& policy) &&  // NOLINT(cppcoreguidelines-missing-std-forward)
		{
			using ExtendedExpectationBuilderT = BasicExpectationBuilder<
				timesConfigured,
				SequenceConfig,
				Signature,
				FinalizePolicy,
				Policies...,
				std::remove_cvref_t<Policy>>;

			return ExtendedExpectationBuilderT{
				std::move(m_Storage),
				std::move(m_TimesConfig),
				std::move(m_SequenceConfig),
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
		constexpr auto operator |(const TimesConfig config) &&
			requires (!timesConfigured)
		{
			using ExtendedExpectationBuilderT = BasicExpectationBuilder<
				true,
				SequenceConfig,
				Signature,
				FinalizePolicy,
				Policies...>;

			m_TimesConfig.set_limits(
				config.min(),
				config.max());
			return ExtendedExpectationBuilderT{
				std::move(m_Storage),
				std::move(m_TimesConfig),
				std::move(m_SequenceConfig),
				std::move(m_FinalizePolicy),
				std::move(m_ExpectationPolicies)
			};
		}

		template <typename... Sequences>
		[[nodiscard]]
		constexpr auto operator |(detail::SequenceConfig<Sequences...>&& config) &&
		{
			detail::SequenceConfig newConfig = m_SequenceConfig.concat(std::move(config));

			using ExtendedExpectationBuilderT = BasicExpectationBuilder<
				true,
				decltype(newConfig),
				Signature,
				FinalizePolicy,
				Policies...>;

			return ExtendedExpectationBuilderT{
				std::move(m_Storage),
				std::move(m_TimesConfig),
				std::move(newConfig),
				std::move(m_FinalizePolicy),
				std::move(m_ExpectationPolicies)
			};
		}

		[[nodiscard]]
		constexpr ScopedExpectationT finalize(const std::source_location& sourceLocation) &&
		{
			static_assert(
				finalize_policy_for<FinalizePolicy, Signature>,
				"For non-void return types, a finalize policy must be set.");

			return ScopedExpectationT{
				std::move(m_Storage),
				std::apply(
					[&](auto&... policies)
					{
						ControlPolicy controlPolicy{
								std::move(m_TimesConfig),
								std::move(m_SequenceConfig)
							};

						using ExpectationT = BasicExpectation<
							Signature,
							decltype(controlPolicy),
							FinalizePolicy,
							Policies...>;

						return std::make_unique<ExpectationT>(
							sourceLocation,
							std::move(controlPolicy),
							std::move(m_FinalizePolicy),
							std::move(policies)...);
					},
					m_ExpectationPolicies)
			};
		}

	private:
		std::shared_ptr<StorageT> m_Storage;
		TimesConfig m_TimesConfig{};
		SequenceConfig m_SequenceConfig{};
		FinalizePolicy m_FinalizePolicy{};
		PolicyListT m_ExpectationPolicies{};
	};

	template <bool timesConfigured, typename SequenceConfig, typename Signature, typename... Policies>
	ScopedExpectation(BasicExpectationBuilder<timesConfigured, SequenceConfig, Signature, Policies...>&&) -> ScopedExpectation<Signature>;
}

namespace mimicpp::detail
{
	template <typename Signature, std::size_t index, typename Arg>
		requires matcher_for<
			std::remove_cvref_t<Arg>,
			signature_param_type_t<index, Signature>>
	[[nodiscard]]
	constexpr auto make_arg_policy(Arg&& arg, [[maybe_unused]] const priority_tag<2>)
	{
		return expect::arg<index>(std::forward<Arg>(arg));
	}

	template <typename Signature, std::size_t index, std::equality_comparable_with<signature_param_type_t<index, Signature>> Arg>
	[[nodiscard]]
	constexpr auto make_arg_policy(Arg&& arg, [[maybe_unused]] const priority_tag<1>)
	{
		return expect::arg<index>(
			matches::eq(std::forward<Arg>(arg)));
	}

	template <typename Signature, std::size_t index, typename Arg>
	constexpr void make_arg_policy([[maybe_unused]] Arg&& arg, [[maybe_unused]] const priority_tag<0>) noexcept  // NOLINT(cppcoreguidelines-missing-std-forward)
	{
		static_assert(
			always_false<Arg>{},
			"The provided argument is neither a matcher, nor is it equality comparable with the selected param.");
	}

	template <typename Signature, typename Builder, std::size_t... indices, typename... Args>
	[[nodiscard]]
	constexpr auto extend_builder_with_arg_policies(
		Builder&& builder,
		const std::index_sequence<indices...>,
		Args&&... args
	)
	{
		return (
			std::forward<Builder>(builder)
			| ...
			| detail::make_arg_policy<Signature, indices>(
					std::forward<Args>(args),
					priority_tag<2>{}));
	}

	template <typename Signature, typename... Args>
	constexpr auto make_expectation_builder(
		std::shared_ptr<ExpectationCollection<Signature>> expectations,
		Args&&... args
	)
	{
		using BaseBuilderT = BasicExpectationBuilder<
			false,
			SequenceConfig<>,
			Signature,
			expectation_policies::InitFinalize
		>;

		return detail::extend_builder_with_arg_policies<Signature>(
			BaseBuilderT{
				std::move(expectations),
				TimesConfig{},
				SequenceConfig<>{},
				expectation_policies::InitFinalize{},
				std::tuple{}
			},
			std::index_sequence_for<Args...>{},
			std::forward<Args>(args)...);
	}
}

#define MIMICPP_UNIQUE_NAME(prefix, counter) prefix##counter
#define MIMICPP_SCOPED_EXPECTATION_IMPL(counter)													\
	[[maybe_unused]]																				\
	const ::mimicpp::ScopedExpectation MIMICPP_UNIQUE_NAME(_mimicpp_expectation_, counter) = 

#define MIMICPP_SCOPED_EXPECTATION MIMICPP_SCOPED_EXPECTATION_IMPL(__COUNTER__)
#define SCOPED_EXP MIMICPP_SCOPED_EXPECTATION

#endif
