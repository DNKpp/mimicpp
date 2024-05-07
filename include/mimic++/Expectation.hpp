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
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace mimicpp::detail
{
	template <typename Return, typename... Params, typename Signature>
	std::optional<MatchReport> make_match_report(
		const call::Info<Return, Params...>& call,
		const Expectation<Signature>& expectation
	) noexcept
	{
		try
		{
			return expectation.matches(call);
		}
		catch (...)
		{
			report_unhandled_exception(
				make_call_report(call),
				expectation.report(),
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
		virtual ExpectationReport report() const = 0;

		[[nodiscard]]
		virtual bool is_satisfied() const noexcept = 0;

		[[nodiscard]]
		virtual MatchReport matches(const CallInfoT& call) const = 0;
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

		void remove(std::shared_ptr<ExpectationT> expectation)
		{
			const std::scoped_lock lock{m_ExpectationsMx};

			auto iter = std::ranges::find(m_Expectations, expectation);
			assert(iter != std::ranges::end(m_Expectations) && "Expectation does not belong to this storage.");
			m_Expectations.erase(iter);

			if (!expectation->is_satisfied())
			{
				detail::report_unfulfilled_expectation(
					expectation->report());
			}
		}

		[[nodiscard]]
		ReturnT handle_call(const CallInfoT& call)
		{
			std::vector<MatchReport> noMatches{};
			std::vector<MatchReport> inapplicableMatches{};

			for (const std::scoped_lock lock{m_ExpectationsMx};
				auto& exp : m_Expectations | std::views::reverse)
			{
				if (std::optional matchReport = detail::make_match_report(call, *exp))
				{
					switch (evaluate_match_report(*matchReport))
					{
						using enum MatchResult;
					case none:
						noMatches.emplace_back(*std::move(matchReport));
						break;
					case inapplicable:
						inapplicableMatches.emplace_back(*std::move(matchReport));
						break;
					case full:
						detail::report_full_match(
							make_call_report(call),
							*std::move(matchReport));
						exp->consume(call);
						return exp->finalize_call(call);

						// GCOVR_EXCL_START
					default:
						unreachable();
						// GCOVR_EXCL_STOP
					}
				}
			}

			if (!std::ranges::empty(inapplicableMatches))
			{
				detail::report_inapplicable_matches(
					make_call_report(call),
					std::move(inapplicableMatches));
			}

			detail::report_no_matches(
				make_call_report(call),
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
								{ std::as_const(policy).describe_state() } -> std::convertible_to<std::optional<StringT>>;
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
			const std::source_location& sourceLocation,
			TimesArg&& timesArg,
			FinalizerArg&& finalizerArg,
			PolicyArgs&&... args
		) noexcept(
			std::is_nothrow_constructible_v<TimesT, TimesArg>
			&& std::is_nothrow_constructible_v<FinalizerT, FinalizerArg>
			&& (std::is_nothrow_constructible_v<Policies, PolicyArgs> && ...))
			: m_SourceLocation{sourceLocation},
			m_Policies{std::forward<PolicyArgs>(args)...},
			m_Times{std::forward<TimesArg>(timesArg)},
			m_Finalizer{std::forward<FinalizerArg>(finalizerArg)}
		{
		}

		[[nodiscard]]
		ExpectationReport report() const override
		{
			return ExpectationReport{
				.sourceLocation = m_SourceLocation,
				.finalizerDescription = std::nullopt,
				.timesDescription = m_Times.describe_state(),
				.expectationDescriptions = std::apply(
					[&](const auto&... policies)
					{
						return std::vector<std::optional<StringT>>{
								policies.describe()...
						};
					},
					m_Policies)
			};
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
		MatchReport matches(const CallInfoT& call) const override
		{
			return MatchReport{
				.sourceLocation = m_SourceLocation,
				.finalizeReport = {std::nullopt},
				.timesReport = MatchReport::Times{
					.isApplicable = m_Times.is_applicable(),
					.description = m_Times.describe_state()
				},
				.expectationReports = std::apply(
					[&](const auto&... policies)
					{
						return std::vector<MatchReport::Expectation>{
							MatchReport::Expectation{
								.isMatching = policies.matches(call),
								.description = policies.describe()
							}...
						};
					},
					m_Policies)
			};
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

		[[nodiscard]]
		constexpr const std::source_location& from() const noexcept
		{
			return m_SourceLocation;
		}

	private:
		std::source_location m_SourceLocation;
		PolicyListT m_Policies;
		[[no_unique_address]] TimesT m_Times{};
		[[no_unique_address]] FinalizerT m_Finalizer{};
	};

	template <typename Signature>
	class ScopedExpectation
	{
	public:
		using StorageT = ExpectationCollection<Signature>;
		using ExpectationT = Expectation<Signature>;

		~ScopedExpectation() noexcept(false)
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
			requires requires(const std::source_location& loc)
			{
				{ std::declval<T&&>().finalize(loc) } -> std::convertible_to<ScopedExpectation>;
			}
		[[nodiscard]]
		explicit(false) constexpr ScopedExpectation(T&& object, const std::source_location& loc = std::source_location::current())
			: ScopedExpectation{std::forward<T>(object).finalize(loc)}
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

		[[nodiscard]]
		const ExpectationT& expectation() const noexcept
		{
			return *m_Expectation;
		}

	private:
		std::shared_ptr<StorageT> m_Storage{};
		std::shared_ptr<ExpectationT> m_Expectation{};
	};
}

#endif
