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
	template <typename Return, typename... Params, typename Signature>
	std::optional<call::MatchResult> make_match_result(
		const call::Info<Return, Params...>& call,
		const std::shared_ptr<Expectation<Signature>>& expectation
	) noexcept
	{
		try
		{
			return expectation->matches(call);
		}
		catch (...)
		{
			report_unhandled_exception(
				call,
				expectation,
				std::current_exception());
		}

		return std::nullopt;
	}
}

namespace mimicpp
{
	template <typename Signature>
	class Expectation
	{
	public:
		using CallInfoT = call::info_for_signature_t<Signature>;
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
		virtual call::MatchResult matches(const CallInfoT& call) const = 0;
		virtual void consume(const CallInfoT& call) = 0;

		[[nodiscard]]
		virtual constexpr ReturnT finalize_call(const CallInfoT& call) = 0;
	};

	template <typename Signature>
	class ExpectationCollection
	{
	public:
		using CallInfoT = call::info_for_signature_t<Signature>;
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
				detail::report_unfulfilled_expectation(
					std::move(expectation));
			}
		}

		[[nodiscard]]
		ReturnT handle_call(const CallInfoT& call)
		{
			std::vector<std::shared_ptr<ExpectationT>> noMatches{};
			std::vector<std::shared_ptr<ExpectationT>> inapplicableMatches{};

			for (const std::scoped_lock lock{m_ExpectationsMx};
				auto& exp : m_Expectations | std::views::reverse)
			{
				if (std::optional matchResult = detail::make_match_result(call, exp))
				{
					switch (*matchResult)
					{
						using enum call::MatchResult;
					case none:
						noMatches.emplace_back(exp);
						break;
					case inapplicable:
						inapplicableMatches.emplace_back(exp);
						break;
					case full:
						detail::report_full_match(
							call,
							exp);
						exp->consume(call);
						return exp->finalize_call(call);
					default:
						unreachable();
					}
				}
			}

			if (!std::ranges::empty(inapplicableMatches))
			{
				detail::report_inapplicable_matches(
					call,
					std::move(inapplicableMatches));
			}

			detail::report_no_matches(
				call,
				std::move(noMatches));
		}

	private:
		std::vector<std::shared_ptr<ExpectationT>> m_Expectations{};
		std::mutex m_ExpectationsMx{};
	};

	template <typename T, typename Signature>
	concept expectation_policy_for = std::is_move_constructible_v<T>
									&& std::is_destructible_v<T>
									&& std::same_as<T, std::remove_cvref_t<T>>
									&& requires(T& policy, const call::info_for_signature_t<Signature>& info)
									{
										{ std::as_const(policy).is_satisfied() } noexcept -> std::convertible_to<bool>;
										{ std::as_const(policy).matches(info) } -> std::convertible_to<bool>;
										{ std::as_const(policy).describe() } -> std::convertible_to<std::optional<StringT>>;
										{ policy.consume(info) };
									};

	template <typename T, typename Signature>
	concept finalize_policy_for = std::is_move_constructible_v<T>
								&& std::is_destructible_v<T>
								&& std::same_as<T, std::remove_cvref_t<T>>
								&& requires(T& policy, const call::info_for_signature_t<Signature>& info)
								{
									{ policy.finalize_call(info) } -> std::convertible_to<signature_return_type_t<Signature>>;
								};

	template <typename T>
	concept times_policy = std::is_move_constructible_v<T>
							&& std::is_destructible_v<T>
							&& std::same_as<T, std::remove_cvref_t<T>>
							&& requires(T& policy)
							{
								{ std::as_const(policy).is_satisfied() } noexcept -> std::convertible_to<bool>;
								{ std::as_const(policy).is_applicable() } noexcept -> std::convertible_to<bool>;
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
		using CallInfoT = call::info_for_signature_t<Signature>;
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
		call::MatchResult matches(const CallInfoT& call) const override
		{
			if (!std::apply(
				[&](const auto&... policies)
				{
					return (... && policies.matches(call));
				},
				m_Policies))
			{
				return call::MatchResult::none;
			}

			if (!m_Times.is_applicable())
			{
				return call::MatchResult::inapplicable;
			}

			return call::MatchResult::full;
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

		[[nodiscard]]
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

		template <typename T>
			requires requires
			{
				{ std::declval<T&&>().finalize() } -> std::convertible_to<ScopedExpectation>;
			}
		[[nodiscard]]
		explicit(false) constexpr ScopedExpectation(T&& object)
			: ScopedExpectation{std::forward<T>(object).finalize()}
		{
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
}

#endif
