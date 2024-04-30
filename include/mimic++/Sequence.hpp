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

#include "Sequence.hpp"

namespace mimicpp::expectation_policies
{
	class Sequence;
}

namespace mimicpp
{
	enum class SequenceId
		: int
	{
	};

	class Sequence
	{
		friend class expectation_policies::Sequence;

	public:
		~Sequence() noexcept(false)
		{
			if (0 <= m_MaxId
				&& (!m_Current
					|| to_underlying(*m_Current) != m_MaxId))
			{
				const int consumedExpectations = 1 + to_underlying(m_Current.value_or(SequenceId{-1}));

				report_error(
					format::format(
						"Unfulfilled sequence. {} out of {} expectation(s) where consumed.",
						consumedExpectations,
						m_MaxId + 1));
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
			assert(0 <= to_underlying(id) && to_underlying(id) <= m_MaxId);

			if (m_Current)
			{
				return to_underlying(id) == to_underlying(*m_Current)
						|| to_underlying(id) == to_underlying(*m_Current) + 1;
			}

			return id == SequenceId{0};
		}

		constexpr void consume(const SequenceId id) noexcept
		{
			assert(is_consumable(id));

			m_Current = id;
		}

		[[nodiscard]]
		constexpr SequenceId add() noexcept
		{
			return SequenceId{++m_MaxId};
		}

	private:
		std::underlying_type_t<SequenceId> m_MaxId{-1};
		std::optional<SequenceId> m_Current{};
	};
}

#endif
