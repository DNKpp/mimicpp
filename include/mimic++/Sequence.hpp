// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_SEQUENCE_HPP
#define MIMICPP_SEQUENCE_HPP

#pragma once

#include "mimic++/Reporter.hpp"
#include "mimic++/Utility.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>

namespace mimicpp::expectation_policies
{
	class Sequence;
}

namespace mimicpp
{
	/**
	 * \defgroup EXPECTATION_SEQUENCE sequence
	 * \ingroup EXPECTATION
	 * \ingroup EXPECTATION_TIMES
	 * \brief Sequences enable deterministic ordering between multiple expectations.
	 * \details Their aim is to provide a convenient way for users, to circumvent the rather loosely ordering of expectations,
	 * which is by design. By default, if two or more expectations would match a call, the last created one is used.
	 * If multiple expectations are built one after another, that may be rather unintuitive design, but when it comes to
	 * scoping, it's usually preferable to match the expectations from within the current scope, even if there exist
	 * another similar expectation from the outside scope.
	 *
	 * As I have no idea how to use one strategy for one case and the other for the other case, I decided to take the same
	 * route as ``trompeloeil``.
	 * \see https://github.com/rollbear/trompeloeil/blob/main/docs/CookBook.md#sequences
	 *
	 * Either way, this is sometimes not enough, when we want to enforce deterministic ordering between two or more expectations.
	 * That's where sequences come into play. The expectation, which gets attached first must match before all subsequent
	 * expectations. If that one is fulfilled, the next one in the row must be matched; and so on.
	 * \snippet Sequences.cpp sequence
	 *
	 * Sequences can also enforce orders on expectations, which refer to different mocks.
	 * \snippet Sequences.cpp sequence multiple mocks
	 *
	 * Sequenced and non-sequenced expectations may be arbitrarily mixed; even if this can be very difficult to trace, by
	 * simply reviewing the code.
	 * \snippet Sequences.cpp sequence mixed
	 * 
	 * It's totally fine to attach expectations to sequences, which are already queried for matches. Sequences do not have
	 * to be setup in one go.
	 *
	 * # Thread-Safety
	 * Sequences are not thread-safe and are never intended to be. If one attempts to enforce a strong ordering between
	 * multiple threads without any explicit synchronisation, that attempt is doomed to fail.
	 *
	 * # Afterthoughts
	 * Sequence policies are treated as ``times_policy`` and therefore can not be mixed with e.g. ``Times``.
	 * At a first glance this seems rather unintuitive, as a sequence does not actually mind how often an expectation
	 * is matched, but rather if one expectation has been matched before another. This being true, there is still a
	 * huge intersection between those cases, because a sequence still must know how often an expectation is expected
	 * to match. This would lead to higher coupling between two rather unrelated domains, but isn't the actual deal-breaker.
	 * The actual reason is, that both, ``Times`` and ``RuntimeTimes`` are rather permissive designed and thus allow for
	 * a wide range of valid configurations. Especially the cases, where a range of possible calls is configured (like
	 * ``at_least``), makes it very hard for a sequence to reliably determine, whether an expectation shall match or not.
	 * When users define expectations not precise enough, this quickly leads to ambiguities between multiple expectations
	 * which may and will result in surprising outcomes.
	 *\{
	 */

	/**
	 * \brief Strong type for internally used sequence ids.
	 */
	enum class SequenceId
		: std::size_t
	{
	};

	/**
	 * \}
	 */

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

	/**
	 * \brief The user level sequence object.
	 * \ingroup EXPECTATION_SEQUENCE
	 * \details This class is just a very thin wrapper and does nothing by its own. It just exists, so that users
	 * have something they can attach expectations to. In fact, objects of this type may even go out of scope before
	 * the attached expectations are destroyed.
	 */
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
		explicit Sequence(
			const std::span<const std::reference_wrapper<mimicpp::Sequence>> sequences,
			const std::size_t times
		)
		{
			m_SequenceInfos.reserve(sequences.size());
			// ReSharper disable once CppRangeBasedForIncompatibleReference
			for (mimicpp::Sequence& sequence : sequences)
			{
				m_SequenceInfos.emplace_back(
					sequence.m_Sequence,
					sequence.m_Sequence->add(times));
			}
		}

		Sequence(const Sequence&) = delete;
		Sequence& operator =(const Sequence&) = delete;
		Sequence(Sequence&&) = default;
		Sequence& operator =(Sequence&&) = default;

		[[nodiscard]]
		constexpr bool is_satisfied() const noexcept
		{
			return std::ranges::all_of(
				m_SequenceInfos,
				[](const entry& info){ return info.sequence->is_saturated(info.id); });
		}

		[[nodiscard]]
		constexpr bool is_applicable() const noexcept
		{
			return std::ranges::all_of(
				m_SequenceInfos,
				[](const entry& info){ return info.sequence->is_consumable(info.id); });
		}

		// ReSharper disable once CppMemberFunctionMayBeConst
		constexpr void consume() noexcept
		{
			assert(is_applicable());

			for (auto& [sequence, id] : m_SequenceInfos)
			{
				sequence->consume(id);
			}
		}

	private:
		struct entry
		{
			std::shared_ptr<mimicpp::detail::Sequence> sequence;
			SequenceId id;
		};
		std::vector<entry> m_SequenceInfos;
	};
}

namespace mimicpp::expect
{
	/**
	 * \brief Attaches the expectation onto a sequence.
	 * \ingroup EXPECTATION_REQUIREMENT_TIMES
	 * \ingroup EXPECTATION_SEQUENCE
	 * \param sequence The sequence to be attached to.
	 * \param times The expected times.
	 * \snippet Sequences.cpp sequence
	 * \snippet Sequences.cpp sequence mixed
	 */
	[[nodiscard]]
	inline auto in_sequence(Sequence& sequence, const std::size_t times = 1u)
	{
		const std::array collection{std::ref(sequence)};
		return expectation_policies::Sequence{
			collection,
			times
		};
	}

	/**
	 * \brief Attaches the expectation onto the listed sequences.
	 * \ingroup EXPECTATION_REQUIREMENT_TIMES
	 * \ingroup EXPECTATION_SEQUENCE
	 * \param sequences The sequences to be attached to.
	 * \param times The expected times.
	 * \snippet Sequences.cpp sequence multiple sequences
	 */
	template <std::size_t size>
	[[nodiscard]]
	auto in_sequences(
		const std::reference_wrapper<Sequence> (&sequences)[size],
		const std::size_t times = 1u
	)
	{
		static_assert(
			0u < size,
			"Zero sequences are not allowed. Use times instead.");

		return expectation_policies::Sequence{
			sequences,
			times
		};
	}
}

#endif
