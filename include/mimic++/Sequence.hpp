//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_SEQUENCE_HPP
#define MIMICPP_SEQUENCE_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/printing/Format.hpp"
#include "mimic++/reporting/GlobalReporter.hpp"
#include "mimic++/utilities/C++23Backports.hpp"

#include <algorithm>
#include <array>
#include <functional>
#include <span>
#include <tuple>

namespace mimicpp::sequence
{
    /**
     * \defgroup EXPECTATION_SEQUENCE sequence
     * \ingroup EXPECTATION
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
     * # A word on sequences with times
     * Sequences and times are fully compatible, but can quickly lead to very hard to understand flows.
     * In general, when mixing sequences with exact times (like expect::twice) it will just work. But when users are defining very
     * permissive expectations combined with ranging times (like binary expect::times), that may lead to surprising behaviour.
     *
     * Imagine a Sequence with two expectations; the first one with ``min = 0`` and ``max = 1`` and the second one with arbitrary limits.
     * When a new call matches both expectations, which one should be preferred?
     *
     * As a last resort, sequences come in two built-in flavors:
     * - LazySequence (default)
     * - GreedySequence
     *
     * LazySequence is designed to make the least possible progress for a match, thus prefers not to skip sequence elements:
     * \snippet Sequences.cpp lazy
     *
     * GreedySequence does make the maximal possible progress for a match, thus prefers to skip sequence elements:
     * \snippet Sequences.cpp greedy
     */

    namespace detail
    {
        template <typename Id, auto priorityStrategy>
            requires std::is_enum_v<Id>
                  && std::signed_integral<std::underlying_type_t<Id>>
                  && std::convertible_to<
                         std::invoke_result_t<decltype(priorityStrategy), Id, int>,
                         int>
        class BasicSequence
        {
        public:
            using IdT = Id;

            ~BasicSequence() noexcept(false)
            {
                const auto iter = std::ranges::find_if_not(
                    m_Entries.cbegin() + m_Cursor,
                    m_Entries.cend(),
                    [](const State s) noexcept {
                        return s == State::satisfied
                            || s == State::saturated;
                    });

                if (iter != m_Entries.cend())
                {
                    reporting::detail::report_error(
                        format::format(
                            "Unfulfilled sequence. {} out of {} expectation(s) are satisfied.",
                            std::ranges::distance(m_Entries.cbegin(), iter),
                            m_Entries.size()));
                }
            }

            [[nodiscard]]
            BasicSequence() = default;

            BasicSequence(const BasicSequence&) = delete;
            BasicSequence& operator=(const BasicSequence&) = delete;
            BasicSequence(BasicSequence&&) = delete;
            BasicSequence& operator=(BasicSequence&&) = delete;

            [[nodiscard]]
            constexpr std::optional<int> priority_of(const IdT id) const noexcept
            {
                MIMICPP_ASSERT(is_valid(id), "Invalid id given.");

                if (is_consumable(id))
                {
                    return std::invoke(
                        priorityStrategy,
                        id,
                        m_Cursor);
                }

                return std::nullopt;
            }

            constexpr void set_satisfied(const IdT id) noexcept
            {
                MIMICPP_ASSERT(is_valid(id), "Invalid id given.");
                MIMICPP_ASSERT(m_Cursor <= util::to_underlying(id), "Invalid state.");

                auto& element = m_Entries[util::to_underlying(id)];
                MIMICPP_ASSERT(element == State::unsatisfied, "Element is in unexpected state.");
                element = State::satisfied;
            }

            constexpr void set_saturated(const IdT id) noexcept
            {
                MIMICPP_ASSERT(is_valid(id), "Invalid id given.");
                const auto index = util::to_underlying(id);
                MIMICPP_ASSERT(m_Cursor <= index, "Invalid state.");

                auto& element = m_Entries[index];
                MIMICPP_ASSERT(element == State::unsatisfied || element == State::satisfied, "Element is in unexpected state.");
                element = State::saturated;
            }

            [[nodiscard]]
            constexpr bool is_consumable(const IdT id) const noexcept
            {
                MIMICPP_ASSERT(is_valid(id), "Invalid id given.");

                const int index = util::to_underlying(id);
                const auto state = m_Entries[index];
                return m_Cursor <= index
                    && std::ranges::all_of(
                           m_Entries.begin() + m_Cursor,
                           m_Entries.begin() + index,
                           [](const State s) noexcept {
                               return s == State::satisfied
                                   || s == State::saturated;
                           })
                    && (state == State::unsatisfied
                        || state == State::satisfied);
            }

            constexpr void consume(const IdT id) noexcept
            {
                MIMICPP_ASSERT(is_consumable(id), "Sequence is not in consumable state.");

                m_Cursor = util::to_underlying(id);
            }

            [[nodiscard]]
            constexpr IdT add()
            {
                if (!std::in_range<std::underlying_type_t<IdT>>(m_Entries.size()))
                    [[unlikely]]
                {
                    throw std::runtime_error{
                        "Sequence already holds maximum amount of elements."};
                }

                m_Entries.emplace_back(State::unsatisfied);
                return static_cast<IdT>(m_Entries.size() - 1);
            }

            [[nodiscard]]
            constexpr Tag tag() const noexcept
            {
                return Tag{
                    std::bit_cast<std::ptrdiff_t>(this)};
            }

        private:
            enum class State
            {
                unsatisfied,
                satisfied,
                saturated
            };

            std::vector<State> m_Entries{};
            int m_Cursor{};

            [[nodiscard]]
            constexpr bool is_valid(const IdT id) const noexcept
            {
                return 0 <= util::to_underlying(id)
                    && std::cmp_less(util::to_underlying(id), m_Entries.size());
            }
        };

        class LazyStrategy
        {
        public:
            [[nodiscard]]
            constexpr int operator()(const auto id, const int cursor) const noexcept
            {
                const auto index = util::to_underlying(id);
                MIMICPP_ASSERT(std::cmp_less_equal(cursor, index), "Invalid state.");

                return std::numeric_limits<int>::max()
                     - (static_cast<int>(index) - cursor);
            }
        };

        class GreedyStrategy
        {
        public:
            [[nodiscard]]
            constexpr int operator()(const auto id, const int cursor) const noexcept
            {
                const auto index = util::to_underlying(id);
                MIMICPP_ASSERT(std::cmp_less_equal(cursor, index), "Invalid state.");

                return static_cast<int>(index) - cursor;
            }
        };

        template <typename Id, auto priorityStrategy>
        class BasicSequenceInterface
        {
            template <typename... Sequences>
            friend class Config;

        public:
            using SequenceT = BasicSequence<Id, priorityStrategy>;

            ~BasicSequenceInterface() = default;

            [[nodiscard]]
            BasicSequenceInterface() = default;

            BasicSequenceInterface(const BasicSequenceInterface&) = delete;
            BasicSequenceInterface& operator=(const BasicSequenceInterface&) = delete;
            BasicSequenceInterface(BasicSequenceInterface&&) = delete;
            BasicSequenceInterface& operator=(BasicSequenceInterface&&) = delete;

            [[nodiscard]]
            constexpr Tag tag() const noexcept
            {
                return m_Sequence->tag();
            }

        private:
            std::shared_ptr<SequenceT> m_Sequence{
                std::make_shared<SequenceT>()};
        };

        template <typename... Sequences>
        class Config
        {
            template <typename... Ts>
            friend class Config;

        public:
            static constexpr std::size_t sequenceCount = sizeof...(Sequences);

            [[nodiscard]]
            Config()
                requires(0 == sizeof...(Sequences))
            = default;

            template <typename... Interfaces>
                requires(sizeof...(Interfaces) == sequenceCount)
            [[nodiscard]] explicit constexpr Config(Interfaces&... interfaces) noexcept(1 == sequenceCount)
                : Config{
                      interfaces.m_Sequence...}
            {
            }

            [[nodiscard]]
            constexpr auto& sequences() const noexcept
            {
                return m_Sequences;
            }

            template <typename... OtherSequences>
            [[nodiscard]]
            constexpr Config<Sequences..., OtherSequences...> concat(
                const Config<OtherSequences...>& other) const
            {
                return std::apply(
                    [](auto... sequences) {
                        return Config<Sequences..., OtherSequences...>{
                            std::move(sequences)...};
                    },
                    std::tuple_cat(m_Sequences, other.sequences()));
            }

        private:
            std::tuple<std::shared_ptr<Sequences>...> m_Sequences;

            [[nodiscard]]
            explicit constexpr Config(std::shared_ptr<Sequences>... sequences) noexcept(1 == sequenceCount)
                requires(0 < sequenceCount)
            {
                if constexpr (1 < sequenceCount)
                {
                    std::array tags{
                        sequences->tag()...};

                    std::ranges::sort(tags);
                    if (!std::ranges::empty(std::ranges::unique(tags)))
                    {
                        throw std::invalid_argument{
                            "Expectations can not be assigned to the same sequence multiple times."};
                    }
                }

                m_Sequences = std::tuple{
                    std::move(sequences)...};
            }
        };
    }
}

namespace mimicpp
{
    /**
     * \brief The lazy sequence interface.
     * \ingroup EXPECTATION_SEQUENCE
     * \details This sequence type prefers to make the least possible sequence progress.
     * \note This class is just a very thin wrapper and does nothing by its own. It just exists, so that users
     * have something they can attach expectations to. In fact, objects of this type may even go out of scope before
     * the attached expectations are destroyed.
     */
    class LazySequence
        : public sequence::detail::BasicSequenceInterface<
              sequence::Id,
              sequence::detail::LazyStrategy{}>
    {
    };

    /**
     * \brief The greedy sequence interface.
     * \ingroup EXPECTATION_SEQUENCE
     * \details This sequence type prefers to make the maximal possible sequence progress.
     * \note This class is just a very thin wrapper and does nothing by its own. It just exists, so that users
     * have something they can attach expectations to. In fact, objects of this type may even go out of scope before
     * the attached expectations are destroyed.
     */
    class GreedySequence
        : public sequence::detail::BasicSequenceInterface<
              sequence::Id,
              sequence::detail::GreedyStrategy{}>
    {
    };

    /**
     * \brief The default sequence type (LazySequence).
     * \ingroup EXPECTATION_SEQUENCE
     */
    using SequenceT = LazySequence;
}

namespace mimicpp::sequence::detail
{
    [[nodiscard]]
    constexpr bool has_better_rating(
        const std::span<const rating> lhs,
        const std::span<const rating> rhs) noexcept
    {
        int rating{};
        for (const auto& [lhsPriority, lhsTag] : lhs)
        {
            if (const auto iter = std::ranges::find(rhs, lhsTag, &rating::tag);
                iter != std::ranges::end(rhs))
            {
                rating += lhsPriority < iter->priority ? -1 : 1;
            }
            else
            {
                return true;
            }
        }

        return 0 <= rating;
    }
}

namespace mimicpp::expect
{
    /**
     * \brief Attaches the expectation onto a sequence.
     * \ingroup EXPECTATION_SEQUENCE
     * \param sequence The sequence to be attached to.
     * \throws std::invalid_argument if the expectation is already part of the given sequence.
     *
     * \snippet Sequences.cpp sequence
     * \snippet Sequences.cpp sequence mixed
     */
    template <typename Id, auto priorityStrategy>
    [[nodiscard]]
    constexpr auto in_sequence(sequence::detail::BasicSequenceInterface<Id, priorityStrategy>& sequence) noexcept
    {
        using ConfigT = sequence::detail::Config<
            sequence::detail::BasicSequence<Id, priorityStrategy>>;

        return ConfigT{
            sequence};
    }

    /**
     * \brief Attaches the expectation onto the listed sequences.
     * \ingroup EXPECTATION_SEQUENCE
     * \param firstSequence The first sequence to be attached to.
     * \param secondSequence The second sequence to be attached to.
     * \param otherSequences Other sequences to be attached to.
     * \throws std::invalid_argument if the expectation is already attached to any of the given sequences or the given sequences contain duplicates.
     *
     * \snippet Sequences.cpp sequence multiple sequences
     */
    template <
        typename FirstId,
        auto firstPriorityStrategy,
        typename SecondId,
        auto secondPriorityStrategy,
        typename... OtherIds,
        auto... otherPriorityStrategies>
    [[nodiscard]]
    constexpr auto in_sequences(
        sequence::detail::BasicSequenceInterface<FirstId, firstPriorityStrategy>& firstSequence,
        sequence::detail::BasicSequenceInterface<SecondId, secondPriorityStrategy>& secondSequence,
        sequence::detail::BasicSequenceInterface<OtherIds, otherPriorityStrategies>&... otherSequences)
    {
        using ConfigT = sequence::detail::Config<
            sequence::detail::BasicSequence<FirstId, firstPriorityStrategy>,
            sequence::detail::BasicSequence<SecondId, secondPriorityStrategy>,
            sequence::detail::BasicSequence<OtherIds, otherPriorityStrategies>...>;

        return ConfigT{
            firstSequence,
            secondSequence,
            otherSequences...};
    }
}

#endif
