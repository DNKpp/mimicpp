// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_HPP
#define MIMICPP_EXPECTATION_HPP

#pragma once

#include "mimic++/Call.hpp"
#include "mimic++/ControlPolicy.hpp"
#include "mimic++/Reporter.hpp"
#include "mimic++/TypeTraits.hpp"

#include <cassert>
#include <concepts>
#include <memory>
#include <mutex>
#include <ranges>
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

	template <typename Signature>
	constexpr auto pick_best_match(std::vector<std::tuple<Expectation<Signature>&, MatchReport>>& matches)
	{
		constexpr auto ratings = [](const auto& el) noexcept -> const auto& {
			return std::get<state_applicable>(
					std::get<MatchReport>(el).controlReport)
				.sequenceRatings;
		};

		auto best = std::ranges::begin(matches);
		for (auto iter = best + 1;
			iter != std::ranges::end(matches);
			++iter)
		{
			if (!sequence::detail::has_better_rating(
				ratings(*best),
				ratings(*iter)))
			{
				best = iter;
			}
		}

		return best;
	}
}

namespace mimicpp
{
	/**
	 * \defgroup EXPECTATION expectation
	 * \brief Contains everything related to managing expectations.
	 * \details Expectations are one of the two core aspects of ``mimic++``. They define how a ``Mock`` is expected to be called.
	 * It is expected, that users store expectations in the ``ScopedExpectation`` RAII-object, which checks whether the expectation
	 * is satisfied. If not, an error is forwarded to the installed reporter.
	 *
	 * Once an expectation has been fully exhausted, it becomes inactive and can't be matched any further.
	 * If at any time multiple active expectations would be valid matches for an incoming call, ``mimic++`` has to make a choice.
	 * In such cases, when no ``Sequence`` has been applied, the latest created expectation will be selected. This may seem rather unintuitive at first,
	 * but as expectations are bound to their scope, usually expectations within the current scope should be preferred over the expectations from the
	 * outer scope.
	 *
	 * This is the technical explanation for the behavior, but users should not rely too much on that, because ``mimic++`` may change that anytime.
	 * In general one should think about all active expectations as equally applicable and treat such ambiguities as undeterministic outcomes.
	 *
	 * Nevertheless, ``mimic++`` provides a tool for such cases, which is designed to work in a deterministic manner:
	 * \ref EXPECTATION_SEQUENCE "Sequences".
	 *
	 * Expectations can be created anywhere in the program, you just need an appropriate ``Mock``. 
	 * \{
	 */

	/**
	 * \brief The base interface for expectations.
	 * \tparam Signature The decayed signature.
	 */
	template <typename Signature>
		requires std::same_as<Signature, signature_decay_t<Signature>>
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

		[[nodiscard]]
		virtual constexpr const std::source_location& from() const noexcept = 0;
	};

	template <typename Signature>
		requires std::same_as<Signature, signature_decay_t<Signature>>
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
			std::vector<std::tuple<ExpectationT&, MatchReport>> matches{};
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
						matches.emplace_back(*exp, *std::move(matchReport));
						break;
						// GCOVR_EXCL_START
					default:
						unreachable();
						// GCOVR_EXCL_STOP
					}
				}
			}

			if (!std::ranges::empty(matches))
			{
				auto&& [exp, report] = *detail::pick_best_match(matches);
				detail::report_full_match(
					make_call_report(call),
					std::move(report));
				exp.consume(call);
				return exp.finalize_call(call);
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
	concept control_policy = std::is_move_constructible_v<T>
							&& std::is_destructible_v<T>
							&& std::same_as<T, std::remove_cvref_t<T>>
							&& requires(T& policy)
							{
								{ std::as_const(policy).is_satisfied() } noexcept -> std::convertible_to<bool>;
								{ std::as_const(policy).state() } -> std::convertible_to<control_state_t>;
								policy.consume();
							};

	template <
		typename Signature,
		control_policy ControlPolicy,
		finalize_policy_for<Signature> FinalizePolicy,
		expectation_policy_for<Signature>... Policies
	>
	class BasicExpectation final
		: public Expectation<Signature>
	{
	public:
		using ControlPolicyT = ControlPolicy;
		using FinalizerT = FinalizePolicy;
		using PolicyListT = std::tuple<Policies...>;
		using CallInfoT = call::info_for_signature_t<Signature>;
		using ReturnT = typename Expectation<Signature>::ReturnT;

		template <typename ControlPolicyArg, typename FinalizerArg, typename... PolicyArgs>
			requires std::constructible_from<ControlPolicyT, ControlPolicyArg>
					&& std::constructible_from<FinalizerT, FinalizerArg>
					&& std::constructible_from<PolicyListT, PolicyArgs...>
		constexpr explicit BasicExpectation(
			const std::source_location& sourceLocation,
			ControlPolicyArg&& controlArg,
			FinalizerArg&& finalizerArg,
			PolicyArgs&&... args
		) noexcept(
			std::is_nothrow_constructible_v<ControlPolicyT, ControlPolicyArg>
			&& std::is_nothrow_constructible_v<FinalizerT, FinalizerArg>
			&& (std::is_nothrow_constructible_v<Policies, PolicyArgs> && ...))
			: m_SourceLocation{sourceLocation},
			m_ControlPolicy{std::forward<ControlPolicyArg>(controlArg)},
			m_Policies{std::forward<PolicyArgs>(args)...},
			m_Finalizer{std::forward<FinalizerArg>(finalizerArg)}
		{
		}

		[[nodiscard]]
		ExpectationReport report() const override
		{
			return ExpectationReport{
				.sourceLocation = m_SourceLocation,
				.finalizerDescription = std::nullopt,
				.timesDescription = std::invoke(
					[this]
					{
						StringStreamT ss{};
						std::visit(
							std::bind_front(
								detail::control_state_printer{},
								std::ostreambuf_iterator{ss}),
							m_ControlPolicy.state());
						return std::move(ss).str();
					}),
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
			return m_ControlPolicy.is_satisfied()
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
				.controlReport = m_ControlPolicy.state(),
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
			m_ControlPolicy.consume();
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
		constexpr const std::source_location& from() const noexcept override
		{
			return m_SourceLocation;
		}

	private:
		std::source_location m_SourceLocation;
		ControlPolicyT m_ControlPolicy;
		PolicyListT m_Policies;
		[[no_unique_address]] FinalizerT m_Finalizer{};
	};

	/**
	 * \brief Takes the ownership of an expectation and check whether it's satisfied during destruction.
	 * \details The owned Expectation is type-erased. This comes in handy, when users want to store ScopedExpectations
	 * in a single container.
	 */
	class ScopedExpectation
	{
	private:
		class Concept
		{
		public:
			virtual ~Concept() noexcept(false)
			{
			}

			Concept(const Concept&) = delete;
			Concept& operator =(const Concept&) = delete;
			Concept(Concept&&) = delete;
			Concept& operator =(Concept&&) = delete;

			[[nodiscard]]
			virtual bool is_satisfied() const = 0;

			[[nodiscard]]
			virtual const std::source_location& from() const noexcept = 0;

		protected:
			Concept() = default;
		};

		template <typename Signature>
		class Model final
			: public Concept
		{
		public:
			using StorageT = ExpectationCollection<Signature>;
			using ExpectationT = Expectation<Signature>;

			~Model() noexcept(false) override
			{
				m_Storage->remove(m_Expectation);
			}

			[[nodiscard]]
			explicit Model(
				std::shared_ptr<StorageT>&& storage,
				std::shared_ptr<ExpectationT>&& expectation
			) noexcept
				: m_Storage{std::move(storage)},
				m_Expectation{std::move(expectation)}
			{
				assert(m_Storage && "Storage is nullptr.");
				assert(m_Expectation && "Expectation is nullptr.");

				m_Storage->push(m_Expectation);
			}

			[[nodiscard]]
			bool is_satisfied() const override
			{
				return m_Expectation->is_satisfied();
			}

			[[nodiscard]]
			const std::source_location& from() const noexcept override
			{
				return m_Expectation->from();
			}

		private:
			std::shared_ptr<StorageT> m_Storage;
			std::shared_ptr<ExpectationT> m_Expectation;
		};

	public:
		/**
		 * \brief Removes the owned expectation from the ExpectationCollection and checks, whether it's satisfied.
		 * \throws In cases of an unsatisfied expectation, the destructor is expected to throw of terminate otherwise.
		 */
		~ScopedExpectation() noexcept(false)  // NOLINT(modernize-use-equals-default)
		{
			// we must call the dtor manually here, because std::unique_ptr's dtor mustn't throw.
			delete m_Inner.release();
		}

		/**
		 * \brief Constructor, which generates the type-erase storage.
		 * \tparam Signature The signature.
		 * \param collection The expectation collection, the expectation will be attached to.
		 * \param expectation The expectation.
		 */
		template <typename Signature>
		[[nodiscard]]
		explicit ScopedExpectation(
			std::shared_ptr<ExpectationCollection<Signature>> collection,
			std::shared_ptr<typename ExpectationCollection<Signature>::ExpectationT> expectation
		) noexcept
			: m_Inner{
				std::make_unique<Model<Signature>>(
					std::move(collection),
					std::move(expectation))
			}
		{
		}

		/**
		 * \brief A constructor, which accepts objects, which can be finalized (e.g. ExpectationBuilder).
		 * \tparam T The object type.
		 * \param object The object to be finalized.
		 * \param loc The source-location.
		 */
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

		/**
		 * \brief Deleted copy-constructor.
		 */
		ScopedExpectation(const ScopedExpectation&) = delete;

		/**
		 * \brief Deleted copy-assignment-operator.
		 */
		ScopedExpectation& operator =(const ScopedExpectation&) = delete;

		/**
		 * \brief Defaulted move-constructor.
		 */
		[[nodiscard]]
		ScopedExpectation(ScopedExpectation&&) = default;

		/**
		 * \brief Defaulted move-assignment-operator.
		 */
		ScopedExpectation& operator =(ScopedExpectation&&) = default;

		/**
		 * \brief Queries the stored expectation, whether it's satisfied.
		 * \return True, if satisfied.
		 */
		[[nodiscard]]
		bool is_satisfied() const
		{
			return m_Inner->is_satisfied();
		}

		/**
		 * \brief Queries the stored expectation for it's stored source-location.
		 * \return The stored source-location.
		 */
		[[nodiscard]]
		const std::source_location& from() const noexcept
		{
			return m_Inner->from();
		}

	private:
		std::unique_ptr<Concept> m_Inner;
	};

	/**
	 * \}
	 */
}

#endif
