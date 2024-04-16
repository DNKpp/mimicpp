// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_HPP
#define MIMICPP_EXPECTATION_HPP

#pragma once

#include "Call.hpp"
#include "TypeTraits.hpp"

#include <cassert>
#include <concepts>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

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
		virtual bool is_saturated() const noexcept = 0;

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
			assert((*iter)->is_satisfied() && "Expectation is unsatisfied.");
			m_Expectations.erase(iter);
		}

		[[nodiscard]]
		bool consume(const CallInfoT& call)
		{
			const std::scoped_lock lock{m_ExpectationsMx};

			for (auto& exp : m_Expectations)
			{
				if (!exp->is_saturated()
					&& std::holds_alternative<call::MatchResult_OkT>(exp->matches(call)))
				{
					exp->consume(call);
					return true;
				}
			}

			return false;
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
										{ std::as_const(policy).is_saturated() } noexcept -> std::convertible_to<bool>;
										{ std::as_const(policy).matches(call) } noexcept -> std::convertible_to<call::SubMatchResultT>;
										{ policy.consume(call) } noexcept;
									};

	template <typename T, typename Signature>
	concept finalize_policy_for = std::movable<T>
								&& std::is_destructible_v<T>
								&& std::same_as<T, std::remove_cvref_t<T>>
								&& requires(T& policy, const call::Info<Signature>& call)
								{
									{ policy.finalize_call(call) } -> std::convertible_to<signature_return_type_t<Signature>>;
								};

	template <
		typename Signature,
		finalize_policy_for<Signature> FinalizePolicy,
		expectation_policy_for<Signature>... Policies
	>
	class BasicExpectation final
		: public Expectation<Signature>
	{
	public:
		using FinalizerT = FinalizePolicy;
		using PolicyListT = std::tuple<Policies...>;
		using CallInfoT = call::Info<Signature>;
		using ReturnT = typename Expectation<Signature>::ReturnT;

		template <typename FinalizerArg, typename... PolicyArgs>
			requires std::constructible_from<FinalizerT, FinalizerArg>
					&& std::constructible_from<PolicyListT, PolicyArgs...>
		constexpr explicit BasicExpectation(
			FinalizerArg&& finalizerArg,
			PolicyArgs&&... args
		) noexcept(
			std::is_nothrow_constructible_v<FinalizerT, PolicyArgs>
			&& (std::is_nothrow_constructible_v<Policies, PolicyArgs> && ...))
			: m_Finalizer{std::forward<FinalizerArg>(finalizerArg)},
			m_Policies{std::forward<PolicyArgs>(args)...}
		{
		}

		[[nodiscard]]
		constexpr bool is_satisfied() const noexcept override
		{
			return std::apply(
				[](const auto&... policies) noexcept
				{
					return (... && policies.is_satisfied());
				},
				m_Policies);
		}

		[[nodiscard]]
		constexpr bool is_saturated() const noexcept override
		{
			return std::apply(
				[](const auto&... policies) noexcept
				{
					return (... || policies.is_saturated());
				},
				m_Policies);
		}

		[[nodiscard]]
		constexpr call::MatchResultT matches(const CallInfoT& call) const override
		{
			return call::detail::evaluate_sub_match_results(
				std::apply(
					[&](const auto&... policies)
					{
						return std::vector<call::SubMatchResultT>{
							policies.matches(call)...
						};
					},
					m_Policies));
		}

		constexpr void consume(const CallInfoT& call) override
		{
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

		[[nodiscard]]
		bool is_saturated() const
		{
			if (m_Expectation)
			{
				return m_Expectation->is_saturated();
			}
			throw std::runtime_error{"Expired expectation."};
		}

	private:
		std::shared_ptr<StorageT> m_Storage{};
		std::shared_ptr<ExpectationT> m_Expectation{};
	};
}

#endif
