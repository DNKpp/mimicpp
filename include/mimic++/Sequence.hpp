// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_SEQUENCE_HPP
#define MIMICPP_SEQUENCE_HPP

#pragma once

#include "mimic++/Reporter.hpp"
#include "mimic++/Utility.hpp"

#include <cassert>

namespace mimicpp::expectation_policies
{
	class Sequence;
}

namespace mimicpp
{
	enum class SequenceId
		: std::size_t
	{
	};

	namespace detail
	{
		class Sequence
		{
		public:
			~Sequence() noexcept(false)
			{
				if (m_Current != m_Entries.size())
				{
					report_error(
						format::format(
							"Unfulfilled sequence. {} out of {} expectation(s) where fully consumed.",
							m_Current,
							m_Entries.size()));
				}
			}

			[[nodiscard]]
			Sequence() = default;

			Sequence(const Sequence&) = delete;
			Sequence& operator =(const Sequence&) = delete;
			Sequence(Sequence&&) = delete;
			Sequence& operator =(Sequence&&) = delete;

			[[nodiscard]]
			constexpr bool is_consumable(const SequenceId id) const noexcept
			{
				assert(to_underlying(id) < m_Entries.size());

				return m_Current == to_underlying(id);
			}

			[[nodiscard]]
			constexpr bool is_saturated(const SequenceId id) const noexcept
			{
				assert(to_underlying(id) < m_Entries.size());

				const auto [amount, counter] = m_Entries[to_underlying(id)];
				return amount == counter;
			}

			constexpr void consume(const SequenceId id) noexcept
			{
				assert(is_consumable(id));

				if (auto& [amount, counter] = m_Entries[m_Current];
					amount == ++counter)
				{
					++m_Current;
				}
			}

			[[nodiscard]]
			constexpr SequenceId add(const std::size_t count)
			{
				if (count == 0)
				{
					throw std::invalid_argument{"Count must be greater than 0."};
				}

				m_Entries.emplace_back(count, 0);
				return SequenceId{m_Entries.size() - 1};
			}

		private:
			struct entry
			{
				std::size_t amount{};
				std::size_t counter{};
			};

			std::vector<entry> m_Entries{};
			std::size_t m_Current{};
		};
	}

	class Sequence
	{
		friend class expectation_policies::Sequence;

	public:
		~Sequence() = default;

		[[nodiscard]]
		Sequence() = default;

		Sequence(const Sequence&) = delete;
		Sequence& operator =(const Sequence&) = delete;
		Sequence(Sequence&&) = delete;
		Sequence& operator =(Sequence&&) = delete;

	private:
		std::shared_ptr<detail::Sequence> m_Sequence{
			std::make_shared<detail::Sequence>()
		};
	};
}

namespace mimicpp::expectation_policies
{
	class Sequence
	{
	public:
		~Sequence() = default;

		// ReSharper disable once CppParameterMayBeConstPtrOrRef
		explicit Sequence(mimicpp::Sequence& sequence, const std::size_t times) noexcept
			: m_Sequence{sequence.m_Sequence},
			m_SequenceId{m_Sequence->add(times)}
		{
		}

		Sequence(const Sequence&) = delete;
		Sequence& operator =(const Sequence&) = delete;
		Sequence(Sequence&&) = default;
		Sequence& operator =(Sequence&&) = default;

		[[nodiscard]]
		constexpr bool is_satisfied() const noexcept
		{
			return m_Sequence->is_saturated(m_SequenceId);
		}

		[[nodiscard]]
		constexpr bool is_applicable() const noexcept
		{
			return m_Sequence->is_consumable(m_SequenceId);
		}

		// ReSharper disable once CppMemberFunctionMayBeConst
		constexpr void consume() noexcept
		{
			assert(is_applicable());

			m_Sequence->consume(m_SequenceId);
		}

	private:
		std::shared_ptr<mimicpp::detail::Sequence> m_Sequence;
		SequenceId m_SequenceId;
	};
}

namespace mimicpp::expect
{
	/**
	 * \brief 
	 * \param sequence 
	 * \param times 
	 * \return 
	 */
	[[nodiscard]]
	inline auto in_sequence(Sequence& sequence, const std::size_t times = 1u)
	{
		return expectation_policies::Sequence{
			sequence,
			times
		};
	}
}

#endif
