// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_HPP
#define MIMICPP_EXPECTATION_HPP

#pragma once

#include "Call.hpp"
#include "Reporter.hpp"
#include "TypeTraits.hpp"

#include <cassert>
#include <concepts>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace mimicpp::detail
{
	template <typename Signature>
	[[noreturn]]
	void handle_call_match_fail(
		call::Info<Signature> call,
		std::vector<call::MatchResult_NoT> noMatches,
		std::vector<call::MatchResult_ExhaustedT> partialMatches
	)
	{
		if (!std::ranges::empty(partialMatches))
		{
			report_fail(
				std::move(call),
				std::move(partialMatches));
		}
		else
		{
			report_fail(
				std::move(call),
				std::move(noMatches));
		}
	}
}

namespace mimicpp
{
	template <typename Signature>
	class Expectation
	{
	public:
		using CallInfoT = call::Info<Signature>;
		using ReturnT = signature_return_type_t<Signature>;

		virtual ~Expectation() = default;

		[[nodiscard]]
		Expectation() = default;

		Expectation(const Expectation&) = delete;
		Expectation& operator =(const Expectation&) = delete;
		Expectation(Expectation&&) = delete;
		Expectation& operator =(Expectation&&) = delete;

		[[nodiscard]]
		virtual bool is_satisfied() const noexcept = 0;

		[[nodiscard]]
		virtual call::MatchResultT matches(const CallInfoT& call) const = 0;
		virtual void consume(const CallInfoT& call) = 0;

		[[nodiscard]]
		virtual constexpr ReturnT finalize_call(const CallInfoT& call) = 0;
	};

	template <typename Signature>
	class ExpectationCollection
	{
	public:
		using CallInfoT = call::Info<Signature>;
		using ExpectationT = Expectation<Signature>;
		using ReturnT = signature_return_type_t<Signature>;

		~ExpectationCollection() = default;

		[[nodiscard]]
		ExpectationCollection() = default;

		ExpectationCollection(const ExpectationCollection&) = delete;
		ExpectationCollection& operator =(const ExpectationCollection&) = delete;

		[[nodiscard]]
		ExpectationCollection(ExpectationCollection&&) = default;
		ExpectationCollection& operator =(ExpectationCollection&&) = default;

		void push(std::shared_ptr<ExpectationT> expectation)
		{
			const std::scoped_lock lock{m_ExpectationsMx};

			assert(
				std::ranges::find(m_Expectations, expectation) == std::ranges::end(m_Expectations)
				&& "Expectation already belongs to this storage.");

			m_Expectations.emplace_back(std::move(expectation));
		}

		void remove(std::shared_ptr<ExpectationT> expectation) noexcept
		{
			const std::scoped_lock lock{m_ExpectationsMx};

			auto iter = std::ranges::find(m_Expectations, expectation);
			assert(iter != std::ranges::end(m_Expectations) && "Expectation does not belong to this storage.");
			m_Expectations.erase(iter);

			if (!expectation->is_satisfied())
			{
				report_unsatisfied_expectation(std::move(expectation));
			}
		}

		[[nodiscard]]
		constexpr ReturnT handle_call(const CallInfoT& call)
		{
			static_assert(3 == std::variant_size_v<call::MatchResultT>, "Unexpected MatchResult alternative count.");

			const std::scoped_lock lock{m_ExpectationsMx};

			std::vector<call::MatchResult_NoT> noMatches{};
			std::vector<call::MatchResult_ExhaustedT> exhaustedMatches{};

			for (auto& exp : m_Expectations)
			{
				auto matchResult = exp->matches(call);
				if (auto* match = std::get_if<call::MatchResult_OkT>(&matchResult))
				{
					report_ok(
						call,
						std::move(*match));
					exp->consume(call);
					return exp->finalize_call(call);
				}

				if (auto* match = std::get_if<call::MatchResult_NoT>(&matchResult))
				{
					noMatches.emplace_back(std::move(*match));
				}
				else
				{
					exhaustedMatches.emplace_back(std::get<call::MatchResult_ExhaustedT>(std::move(matchResult)));
				}
			}

			detail::handle_call_match_fail(
				call,
				std::move(noMatches),
				std::move(exhaustedMatches));
		}

	private:
		std::vector<std::shared_ptr<ExpectationT>> m_Expectations{};
		std::mutex m_ExpectationsMx{};
	};

	template <typename T, typename Signature>
	concept expectation_policy_for = std::movable<T>
									&& std::is_destructible_v<T>
									&& std::same_as<T, std::remove_cvref_t<T>>
									&& requires(T& policy, const call::Info<Signature>& call)
									{
										{ std::as_const(policy).is_satisfied() } noexcept -> std::convertible_to<bool>;
										{ std::as_const(policy).matches(call) } -> std::convertible_to<call::SubMatchResult>;
										{ policy.consume(call) };
									};

	template <typename T, typename Signature>
	concept finalize_policy_for = std::movable<T>
								&& std::is_destructible_v<T>
								&& std::same_as<T, std::remove_cvref_t<T>>
								&& requires(T& policy, const call::Info<Signature>& call)
								{
									{ policy.finalize_call(call) } -> std::convertible_to<signature_return_type_t<Signature>>;
								};

	template <typename T>
	concept times_policy = std::movable<T>
							&& std::is_destructible_v<T>
							&& std::same_as<T, std::remove_cvref_t<T>>
							&& requires(T& policy)
							{
								{ std::as_const(policy).is_satisfied() } noexcept -> std::convertible_to<bool>;
								{ std::as_const(policy).is_saturated() } noexcept -> std::convertible_to<bool>;
								policy.consume();
							};

	template <
		typename Signature,
		times_policy TimesPolicy,
		finalize_policy_for<Signature> FinalizePolicy,
		expectation_policy_for<Signature>... Policies
	>
	class BasicExpectation final
		: public Expectation<Signature>
	{
	public:
		using TimesT = TimesPolicy;
		using FinalizerT = FinalizePolicy;
		using PolicyListT = std::tuple<Policies...>;
		using CallInfoT = call::Info<Signature>;
		using ReturnT = typename Expectation<Signature>::ReturnT;

		template <typename TimesArg, typename FinalizerArg, typename... PolicyArgs>
			requires std::constructible_from<TimesT, TimesArg>
					&& std::constructible_from<FinalizerT, FinalizerArg>
					&& std::constructible_from<PolicyListT, PolicyArgs...>
		constexpr explicit BasicExpectation(
			TimesArg&& timesArg,
			FinalizerArg&& finalizerArg,
			PolicyArgs&&... args
		) noexcept(
			std::is_nothrow_constructible_v<TimesT, TimesArg>
			&& std::is_nothrow_constructible_v<FinalizerT, FinalizerArg>
			&& (std::is_nothrow_constructible_v<Policies, PolicyArgs> && ...))
			: m_Times{std::forward<TimesArg>(timesArg)},
			m_Finalizer{std::forward<FinalizerArg>(finalizerArg)},
			m_Policies{std::forward<PolicyArgs>(args)...}
		{
		}

		[[nodiscard]]
		constexpr bool is_satisfied() const noexcept override
		{
			return m_Times.is_satisfied()
					&& std::apply(
						[](const auto&... policies) noexcept
						{
							return (... && policies.is_satisfied());
						},
						m_Policies);
		}

		[[nodiscard]]
		constexpr call::MatchResultT matches(const CallInfoT& call) const override
		{
			return call::detail::evaluate_sub_match_results(
				m_Times.is_saturated(),
				std::apply(
					[&](const auto&... policies)
					{
						return std::vector<call::SubMatchResult>{
							policies.matches(call)...
						};
					},
					m_Policies));
		}

		constexpr void consume(const CallInfoT& call) override
		{
			m_Times.consume();
			std::apply(
				[&](auto&... policies) noexcept
				{
					(..., policies.consume(call));
				},
				m_Policies);
		}

		[[nodiscard]]
		constexpr ReturnT finalize_call(const CallInfoT& call) override
		{
			return m_Finalizer.finalize_call(call);
		}

	private:
		[[no_unique_address]] TimesT m_Times{};
		[[no_unique_address]] FinalizerT m_Finalizer{};
		PolicyListT m_Policies;
	};

	template <typename Signature>
	class ScopedExpectation
	{
	public:
		using StorageT = ExpectationCollection<Signature>;
		using ExpectationT = Expectation<Signature>;

		~ScopedExpectation()
		{
			if (m_Storage
				&& m_Expectation)
			{
				m_Storage->remove(m_Expectation);
			}
		}

		explicit ScopedExpectation(
			std::shared_ptr<StorageT> storage,
			std::shared_ptr<ExpectationT> expectation
		) noexcept
			: m_Storage{std::move(storage)},
			m_Expectation{std::move(expectation)}
		{
			assert(m_Storage && "Storage is nullptr.");
			assert(m_Expectation && "Expectation is nullptr.");

			m_Storage->push(m_Expectation);
		}

		ScopedExpectation(const ScopedExpectation&) = delete;
		ScopedExpectation& operator =(const ScopedExpectation&) = delete;

		[[nodiscard]]
		ScopedExpectation(ScopedExpectation&&) = default;
		ScopedExpectation& operator =(ScopedExpectation&&) = default;

		[[nodiscard]]
		bool is_satisfied() const
		{
			if (m_Expectation)
			{
				return m_Expectation->is_satisfied();
			}
			throw std::runtime_error{"Expired expectation."};
		}

	private:
		std::shared_ptr<StorageT> m_Storage{};
		std::shared_ptr<ExpectationT> m_Expectation{};
	};

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
