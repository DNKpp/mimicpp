//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_POLICIES_CONTROL_POLICY_HPP
#define MIMICPP_POLICIES_CONTROL_POLICY_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Sequence.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/reporting/ExpectationReport.hpp"
#include "mimic++/reporting/SequenceReport.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <limits>
    #include <memory>
    #include <optional>
    #include <stdexcept>
    #include <tuple>
    #include <utility>
    #include <vector>
#endif

namespace mimicpp::detail
{
    template <typename... Sequences>
    [[nodiscard]]
    constexpr std::tuple<std::tuple<std::shared_ptr<Sequences>, sequence::Id>...> make_sequence_entries(
        util::SourceLocation loc,
        std::tuple<std::shared_ptr<Sequences>...> const& sequences) noexcept
    {
        // This is a workaround due to some issues with clang-17 with c++23 and libstdc++
        // That configuration prevents the direct initialization, thus we have to default construct first and
        // setup afterwards. Compilers will probably detect that and optimize accordingly.
        std::tuple<std::tuple<std::shared_ptr<Sequences>, sequence::Id>...> result{};
        std::invoke(
            [&]<std::size_t... indices>([[maybe_unused]] std::index_sequence<indices...> const) noexcept {
                (..., (std::get<indices>(result) = std::tuple{
                           std::get<indices>(sequences),
                           std::get<indices>(sequences)->add(loc),
                       }));
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
        constexpr TimesConfig(int const min, int const max)
        {
            if (min < 0
                || max < 0
                || max < min)
            {
                throw std::invalid_argument{
                    "min must be less or equal to max and both must not be less than zero."};
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
        constexpr std::tuple<std::vector<sequence::rating>, std::vector<reporting::SequenceReport>> gather_sequence_reports(auto const& sequenceEntries)
        {
            std::vector<reporting::SequenceReport> inapplicable{};
            std::vector<sequence::rating> ratings{};

            auto const handleSequence = [&](auto& seq, sequence::Id const id) {
                if (std::optional const priority = seq->priority_of(id))
                {
                    ratings.emplace_back(*priority, seq->tag());
                }
                else
                {
                    inapplicable.emplace_back(reporting::make_sequence_report(*seq));
                }
            };

            std::apply(
                [&](auto const&... entries) {
                    (..., handleSequence(std::get<0>(entries), std::get<1>(entries)));
                },
                sequenceEntries);

            return {std::move(ratings), std::move(inapplicable)};
        }

        [[nodiscard]]
        reporting::control_state_t make_control_state(
            int const min,
            int const max,
            int const count,
            auto const& sequenceEntries)
        {
            if (count == max)
            {
                return reporting::state_saturated{
                    .min = min,
                    .max = max,
                    .count = count,
                    .sequences = std::apply(
                        [](auto const&... entries) {
                            return std::vector<reporting::SequenceReport>{reporting::make_sequence_report(*std::get<0>(entries))...};
                        },
                        sequenceEntries)};
            }

            auto&& [ratings, inapplicable] = gather_sequence_reports(sequenceEntries);
            if (!std::ranges::empty(inapplicable))
            {
                return reporting::state_inapplicable{
                    .min = min,
                    .max = max,
                    .count = count,
                    .sequences = std::move(ratings),
                    .inapplicableSequences = std::move(inapplicable)};
            }

            return reporting::state_applicable{
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
            util::SourceLocation loc,
            detail::TimesConfig const& timesConfig,
            sequence::detail::Config<Sequences...> const& sequenceConfig) noexcept
            : m_Min{timesConfig.min()},
              m_Max{timesConfig.max()},
              m_Sequences{detail::make_sequence_entries(std::move(loc), sequenceConfig.sequences())}
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
                       [](auto const&... entries) noexcept {
                           return (... && std::get<0>(entries)->is_consumable(std::get<1>(entries)));
                       },
                       m_Sequences);
        }

        constexpr void consume() noexcept
        {
            MIMICPP_ASSERT(is_applicable(), "Policy is inapplicable.");

            std::apply(
                [](auto&... entries) noexcept {
                    (..., std::get<0>(entries)->consume(std::get<1>(entries)));
                },
                m_Sequences);

            ++m_Count;
            update_sequence_states();
        }

        [[nodiscard]]
        reporting::control_state_t state() const
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
            std::tuple<std::shared_ptr<Sequences>, sequence::Id>...>
            m_Sequences{};

        constexpr void update_sequence_states() noexcept
        {
            if (m_Count == m_Max)
            {
                constexpr auto set_saturated = [](auto& sequence, auto const& id) { sequence->set_saturated(id); };
                std::apply(
                    [&](auto&... entries) noexcept { (..., std::apply(set_saturated, entries)); },
                    m_Sequences);
            }
            else if (m_Count == m_Min)
            {
                constexpr auto set_satisfied = [](auto& sequence, auto const& id) { sequence->set_satisfied(id); };
                std::apply(
                    [&](auto&... entries) noexcept { (..., std::apply(set_satisfied, entries)); },
                    m_Sequences);
            }
        }
    };
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::expect
{
    /**
     * \defgroup EXPECTATION_TIMES times
     * \ingroup EXPECTATION
     * \brief Specifies how many times an expectation must be matched.
     * \details
     * When defining an expectation, users may specify a times-policy exactly once.
     * If no policy is provided, it defaults to `expect::once()`.
     * Attempting to set more than one times-policy for the same expectation results in a compile-time error.
     *
     * A times-policy always defines both a lower and an upper bound on the number of allowed matches.
     * Both bounds are inclusive.
     *
     *\{
     */

    /**
     * \brief Specifies a times policy with a limit range.
     * \param min The lower limit.
     * \param max The upper limit.
     * \return The newly created policy.
     * \throws std::invalid_argument if
     * - ``min < 0``,
     * - ``max < 0`` or
     * - ``max < min``.
     *
     * \snippet Times.cpp times
     */
    [[nodiscard]]
    constexpr auto times(int const min, int const max)
    {
        return mimicpp::detail::TimesConfig{min, max};
    }

    /**
     * \brief Specifies a times policy with an exact limit.
     * \param exactly The limit.
     * \return The newly created policy.
     * \details This requires the expectation to be matched exactly the specified times.
     * \throws std::invalid_argument if ``exactly < 0``.
     *
     * \snippet Times.cpp times single
     */
    [[nodiscard]]
    constexpr auto times(int const exactly)
    {
        return mimicpp::detail::TimesConfig(exactly, exactly);
    }

    /**
     * \brief Specifies a times policy with just a lower limit.
     * \param min The lower limit.
     * \return The newly created policy.
     * \details This requires the expectation to be matched at least ``min`` times or more.
     * \throws std::invalid_argument if ``min < 0``.
     *
     * \snippet Times.cpp at_least
     */
    [[nodiscard]]
    constexpr auto at_least(int const min)
    {
        return mimicpp::detail::TimesConfig{min, std::numeric_limits<int>::max()};
    }

    /**
     * \brief Specifies a times policy with just an upper limit.
     * \param max The upper limit.
     * \return The newly created policy.
     * \details This requires the expectation to be matched up to ``max`` times.
     * \throws std::invalid_argument if ``max < 0``.
     *
     * \snippet Times.cpp at_most
     */
    [[nodiscard]]
    constexpr auto at_most(int const max)
    {
        return mimicpp::detail::TimesConfig{0, max};
    }

    /**
     * \brief Specifies a times policy with both limits set to 0.
     * \return The newly created policy.
     * \details This requires the expectation to be never matched.
     * Useful for explicitly forbidding certain calls.
     */
    [[nodiscard]]
    consteval auto never() noexcept
    {
        constexpr mimicpp::detail::TimesConfig config{0, 0};

        return config;
    }

    /**
     * \brief Specifies a times policy with both limits set to 1.
     * \return The newly created policy.
     * \details This requires the expectation to be matched exactly once.
     *
     * \snippet Times.cpp once
     */
    [[nodiscard]]
    consteval auto once() noexcept
    {
        constexpr mimicpp::detail::TimesConfig config{1, 1};

        return config;
    }

    /**
     * \brief Specifies a times policy with both limits set to 2.
     * \return The newly created policy.
     * \details This requires the expectation to be matched exactly twice.
     *
     * \snippet Times.cpp twice
     */
    [[nodiscard]]
    consteval auto twice() noexcept
    {
        constexpr mimicpp::detail::TimesConfig config{2, 2};

        return config;
    }

    /**
     * \brief Specifies a times-policy with no constraints on how many times an expectation may match.
     * \return The newly created policy.
     */
    [[nodiscard]]
    consteval auto any_times() noexcept
    {
        constexpr mimicpp::detail::TimesConfig config{0, std::numeric_limits<int>::max()};

        return config;
    }

    /**
     * \}
     */
}

#endif
