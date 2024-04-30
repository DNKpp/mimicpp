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
		: int
	{
	};

	namespace detail
	{
	class Sequence
	{
	public:
		~Sequence() = default;

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

			return to_underlying(m_Current) <= to_underlying(id);
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
		SequenceId m_Current{};
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
	};
}

#endif
