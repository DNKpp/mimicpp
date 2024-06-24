// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_CONTROL_POLICY_HPP
#define MIMICPP_CONTROL_POLICY_HPP

#pragma once

#include "mimic++/Printer.hpp"
#include "mimic++/Reports.hpp"
#include "mimic++/Sequence.hpp"
#include "mimic++/Utility.hpp"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>

namespace mimicpp::detail
{
	[[nodiscard]]
	inline StringT describe_times_state(const std::size_t current, const std::size_t min, const std::size_t max)
	{
		const auto verbalizeValue = [](const std::size_t value)-> StringT
		{
			switch (value)
			{
			case 0:
				return "never";
			case 1:
				return "once";
			case 2:
				return "twice";
			default:
				return format::format("{} times", value);
			}
		};

		if (current == max)
		{
			return format::format(
				"inapplicable: already saturated (matched {})",
				verbalizeValue(current));
		}

		if (min <= current)
		{
			return format::format(
				"applicable: accepts further matches (matched {} out of {} times)",
				current,
				max);
		}

		const auto verbalizeInterval = [verbalizeValue](const std::size_t start, const std::size_t end)
		{
			if (start < end)
			{
				return format::format(
					"between {} and {} times",
					start,
					end);
			}

			return format::format(
				"exactly {}",
				verbalizeValue(end));
		};

		return format::format(
			"unsatisfied: matched {} - {} is expected",
			verbalizeValue(current),
			verbalizeInterval(min, max));
	}

	template <typename... Sequences>
	[[nodiscard]]
	constexpr std::tuple<std::tuple<std::shared_ptr<Sequences>, sequence::Id>...> make_sequence_entries(
		const std::tuple<std::shared_ptr<Sequences>...>& sequences
	) noexcept
	{
		// This is a workaround due to some issues with clang-17 with c++23 and libstdc++
		// That configuration prevents the direct initialization, thus we have to default construct first and
		// setup afterwards. Compilers will probably detect that and optimize accordingly.
		std::tuple<std::tuple<std::shared_ptr<Sequences>, sequence::Id>...> result{};
		std::invoke(
			[&]<std::size_t... indices>([[maybe_unused]] const std::index_sequence<indices...>) noexcept
			{
				((std::get<indices>(result) =
				std::tuple{
					std::get<indices>(sequences),
					std::get<indices>(sequences)->add(),
				}), ...);
			},
			std::index_sequence_for<Sequences...>{});
		return result;
	}

	class TimesConfig
	{
	public:
		[[nodiscard]]
		TimesConfig() = default;

		[[nodiscard]]
		constexpr TimesConfig(const int min, const int max)
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
}

namespace mimicpp
{
	namespace detail
	{
		[[nodiscard]]
		control_state_t make_control_state(
			const int min,
			const int max,
			const int count,
			const auto& sequenceEntries
		)
		{
			if (count == max)
			{
				return state_saturated{
					.min = min,
					.max = max,
					.count = count,
					.sequences = std::apply(
						[](const auto&... entries)
						{
							return std::vector<sequence::Tag>{
								std::get<0>(entries)->tag()...
							};
						},
						sequenceEntries)
				};
			}

			std::vector<sequence::Tag> inapplicable{};
			std::vector<sequence::rating> ratings{};
			std::apply(
				[&](const auto&... entries)
				{
					const auto distribute = [&](auto& seq, const sequence::Id id)
					{
						if (const std::optional priority = seq->priority_of(id))
						{
							ratings.emplace_back(
								*priority,
								seq->tag());
						}
						else
						{
							inapplicable.emplace_back(seq->tag());
						}
					};

					(...,
						distribute(
							std::get<0>(entries),
							std::get<1>(entries)));
				},
				sequenceEntries);

			if (!std::ranges::empty(inapplicable))
			{
				return state_inapplicable{
					.min = min,
					.max = max,
					.count = count,
					.sequenceRatings = std::move(ratings),
					.inapplicableSequences = std::move(inapplicable)
				};
			}

			return state_applicable{
				.min = min,
				.max = max,
				.count = count,
				.sequenceRatings = std::move(ratings),
			};
		}
	}

	template <typename... Sequences>
	class ControlPolicy
	{
	public:
		static constexpr std::size_t sequenceCount{sizeof...(Sequences)};

		[[nodiscard]]
		explicit constexpr ControlPolicy(
			const detail::TimesConfig& timesConfig,
			const sequence::detail::Config<Sequences...>& sequenceConfig
		) noexcept
			: m_Min{timesConfig.min()},
			m_Max{timesConfig.max()},
			m_Sequences{
				detail::make_sequence_entries(sequenceConfig.sequences())
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
		constexpr bool is_saturated() const noexcept
		{
			return m_Count == m_Max;
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
			assert(is_applicable());

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
		StringT describe_state() const
		{
			StringT description = detail::describe_times_state(
				m_Count,
				m_Min,
				m_Max);
			if (m_Count < m_Max
				&& 0 < sequenceCount)
			{
				const int consumableCount =
					std::apply(
						[](auto&... entries) noexcept
						{
							return (0 + ... + int{std::get<0>(entries)->is_consumable(std::get<1>(entries))});
						},
						m_Sequences);

				// ReSharper disable once CppRedundantQualifier
				format::format_to(
					std::back_inserter(description),
					"\n\tIs head from {} out of {} sequences.",
					consumableCount,
					sequenceCount);
			}

			return description;
		}

		[[nodiscard]]
		constexpr control_state_t state() const
		{
			return detail::make_control_state(
				m_Min,
				m_Max,
				m_Count,
				m_Sequences);
		}

	private:
		int m_Min;
		int m_Max;
		int m_Count{};
		std::tuple<
			std::tuple<std::shared_ptr<Sequences>, sequence::Id>...> m_Sequences{};

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
		return detail::TimesConfig{min, max};
	}

	/**
	 * \brief Specifies a times policy with an exact limit.
	 * \param exactly The limit.
	 * \return The newly created policy.
	 * \details This requires the expectation to be matched exactly the specified times.
	 * \snippet Times.cpp times single rt
	 */
	[[nodiscard]]
	constexpr auto times(const int exactly)
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
	constexpr auto at_least(const int min)
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
	constexpr auto at_most(const int max)
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
