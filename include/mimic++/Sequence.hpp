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

namespace mimicpp::expectation_policies
{
	class Sequence
	{
	public:
		~Sequence() = default;

		// ReSharper disable once CppParameterMayBeConstPtrOrRef
		explicit Sequence(mimicpp::Sequence& sequence) noexcept
			: m_Sequence{sequence.m_Sequence},
			m_SequenceId{m_Sequence->add()}
		{
		}

		Sequence(const Sequence&) = delete;
		Sequence& operator =(const Sequence&) = delete;
		Sequence(Sequence&&) = default;
		Sequence& operator =(Sequence&&) = default;

		[[nodiscard]]
		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		template <typename Return, typename... Args>
		[[nodiscard]]
		call::SubMatchResult matches([[maybe_unused]] const call::Info<Return, Args...>& info) const
		{
			const auto result = m_Sequence->is_consumable(m_SequenceId);
			return {
				.matched = result,
				.msg = result
							? " is in sequence"
							: " is not in sequence"
			};
		}

		template <typename Return, typename... Args>
		constexpr void consume([[maybe_unused]] const call::Info<Return, Args...>& info) noexcept
		{
			m_Sequence->consume(m_SequenceId);
		}

	private:
		std::shared_ptr<detail::Sequence> m_Sequence;
		SequenceId m_SequenceId;
	};
}

namespace mimicpp::expect
{
	[[nodiscard]]
	inline auto in_sequence(Sequence& sequence) noexcept
	{
		return expectation_policies::Sequence{
			sequence
		};
	}
}

#endif
