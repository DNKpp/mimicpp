// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CONTROL_POLICY_HPP
#define MIMICPP_CONTROL_POLICY_HPP

#pragma once

#include "mimic++/Printer.hpp"
#include "mimic++/Reporter.hpp"
#include "mimic++/Sequence.hpp"
#include "mimic++/Utility.hpp"

#include <array>
#include <cassert>
#include <limits>
#include <stdexcept>

namespace mimicpp
{
	class TimesConfig
	{
	public:
		constexpr void set_limits(const int min, const int max)
		{
			if (min < 0
				|| max < 0
				|| max < min)
			{
				throw std::invalid_argument{
					"min must be less or equal to max and both must not be less than zero."
				};
			}

			m_Min = min;
			m_Max = max;
		}

		[[nodiscard]]
		constexpr int min() const noexcept
		{
			return m_Min;
		}

		[[nodiscard]]
		constexpr int max() const noexcept
		{
			return m_Max;
		}

	private:
		int m_Min{1};
		int m_Max{1};
	};

	template <typename... Sequences>
	class ControlPolicy
	{
	public:
		static constexpr std::size_t sequenceCount{sizeof...(Sequences)};

		[[nodiscard]]
		explicit constexpr ControlPolicy(
			const TimesConfig& timesConfig,
			const detail::SequenceConfig<Sequences...> sequenceConfig
		) noexcept
			: m_Min{timesConfig.min()},
			m_Max{timesConfig.max()},
			m_Sequences{
				std::apply(
				[](auto... sequences) noexcept
				{
					return std::tuple{
						std::tuple{sequences, sequences->add()}...
					};
				},
				sequenceConfig.sequences())
			}
		{
			update_sequence_states();
		}

		[[nodiscard]]
		constexpr bool is_satisfied() const noexcept
		{
			return m_Min <= m_Count
					&& m_Count <= m_Max;
		}

		[[nodiscard]]
		constexpr bool is_applicable() const noexcept
		{
			return m_Count < m_Max
					&& std::apply(
						[](const auto&... entries) noexcept
						{
							return (... && std::get<0>(entries)->is_consumable(std::get<1>(entries)));
						},
						m_Sequences);
		}

		constexpr void consume() noexcept
		{
			assert(m_Count < m_Max);

			std::apply(
				[](auto&... entries) noexcept
				{
					(..., std::get<0>(entries)->consume(std::get<1>(entries)));
				},
				m_Sequences);

			++m_Count;

			update_sequence_states();
		}

		[[nodiscard]]
		constexpr std::array<detail::sequence_rating, sequenceCount> priorities() const noexcept
		{
			assert(is_applicable());

			constexpr auto makePriority = [](const auto& entry) noexcept
			{
				const auto& [seq, id] = entry;
				const std::optional priority = seq->priority_of(id);
				assert(priority);

				return detail::sequence_rating{
					.priority = *priority,
					.tag = seq->tag()
				};
			};

			return std::apply(
				[](const auto&... entries) noexcept
				{
					return std::array<detail::sequence_rating, sequenceCount>{
						makePriority(entries)...
					};
				},
				m_Sequences);
		}

		[[nodiscard]]
		StringT describe_state() const
		{
			return {};
		}

	private:
		int m_Min;
		int m_Max;
		int m_Count{};
		std::tuple<
			std::tuple<std::shared_ptr<Sequences>, detail::SequenceId>...> m_Sequences{};

		constexpr void update_sequence_states() noexcept
		{
			if (m_Count == m_Min)
			{
				std::apply(
					[](auto&... entries) noexcept
					{
						(..., std::get<0>(entries)->set_satisfied(std::get<1>(entries)));
					},
					m_Sequences);
			}
			else if (m_Count == m_Max)
			{
				std::apply(
					[](auto&... entries) noexcept
					{
						(..., std::get<0>(entries)->set_saturated(std::get<1>(entries)));
					},
					m_Sequences);
			}
		}
	};
}

namespace mimicpp::expect
{
	/**
	 * \defgroup EXPECTATION_TIMES times
	 * \ingroup EXPECTATION
	 * \brief Times indicates, how often an expectation must be matched.
	 * \details During each expectation building users can specify a times policy once. If not specified, that policy defaults to ``once``.
	 * If users attempt to specify a times policy more than once for a single expectation, a compile-error will occur.
	 *
	 * Times in general have both, a lower and an upper limit. Both limits are treated as inclusive.
	 *
	 *\{
	 */

	/**
	 * \brief Specifies a times policy with a limit range.
	 * \param min The lower limit.
	 * \param max The upper limit.
	 * \return The newly created policy.
	 *
	 * \snippet Times.cpp times rt
	 */
	[[nodiscard]]
	constexpr auto times(const int min, const int max)
	{
		TimesConfig config{};
		config.set_limits(min, max);
		return config;
	}

	/**
	 * \brief Specifies a times policy with an exact limit.
	 * \param exactly The limit.
	 * \return The newly created policy.
	 * \details This requires the expectation to be matched exactly the specified times.
	 * \snippet Times.cpp times single rt
	 */
	[[nodiscard]]
	constexpr auto times(const int exactly) noexcept
	{
		return times(exactly, exactly);
	}

	/**
	 * \brief Specifies a times policy with just a lower limit.
	 * \tparam min The lower limit.
	 * \return The newly created policy.
	 * \details This requires the expectation to be matched at least ``min`` times or more.
	 * \snippet Times.cpp at_least compile-time
	 */
	[[nodiscard]]
	constexpr auto at_least(const int min) noexcept
	{
		return times(
			min,
			std::numeric_limits<int>::max());
	}

	/**
	 * \brief Specifies a times policy with just an upper limit.
	 * \param max The upper limit.
	 * \return The newly created policy.
	 * \details This requires the expectation to be matched up to ``max`` times.
	 */
	[[nodiscard]]
	constexpr auto at_most(const int max) noexcept
	{
		return times(
			0,
			max);
	}

	/**
	 * \brief Specifies a times policy with both limits set to 1.
	 * \return The newly created policy.
	 * \details This requires the expectation to be matched exactly once.
	 * \snippet Times.cpp once
	 */
	[[nodiscard]]
	consteval auto once() noexcept
	{
		return times(1);
	}

	/**
	 * \brief Specifies a times policy with both limits set to 2.
	 * \return The newly created policy.
	 * \details This requires the expectation to be matched exactly twice.
	 * \snippet Times.cpp twice
	 */
	[[nodiscard]]
	consteval auto twice() noexcept
	{
		return times(2);
	}

	/**
	 * \}
	 */

}

#endif
