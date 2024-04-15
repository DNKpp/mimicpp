// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_HPP
#define MIMICPP_EXPECTATION_HPP

#pragma once

#include "Call.hpp"

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
		using CallT = Call<Signature>;

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
		virtual bool matches(const CallT& call) const noexcept = 0;
		virtual void consume(const CallT& call) noexcept = 0;
	};

	template <typename Signature>
	class ExpectationCollection
	{
	public:
		using CallT = Call<Signature>;
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
		bool consume(const CallT& call) noexcept
		{
			const std::scoped_lock lock{m_ExpectationsMx};

			if (const auto iter = std::ranges::find_if(
				m_Expectations,
				[&](const auto& exp)
				{
					return !exp->is_saturated()
							&& exp->matches(call);
				});
				iter != std::ranges::end(m_Expectations))
			{
				(*iter)->consume(call);
				return true;
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
									&& requires(T& policy, const Call<Signature>& call)
									{
										{ policy.is_satisfied() } noexcept -> std::convertible_to<bool>;
										{ policy.is_saturated() } noexcept -> std::convertible_to<bool>;
										{ policy.matches(call) } noexcept -> std::convertible_to<bool>;
										{ policy.consume(call) } noexcept;
									};
}

#endif
