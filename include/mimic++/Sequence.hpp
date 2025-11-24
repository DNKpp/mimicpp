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
#include "mimic++/utilities/C++20Compatibility.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/Concepts.hpp"
#include "mimic++/utilities/PassKey.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <algorithm>
    #include <array>
    #include <functional>
    #include <span>
    #include <tuple>
#endif

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
     * Sequenced and non-sequenced expectations may be arbitrarily mixed; even if this can be very difficult to trace,
     * by simply reviewing the code.
     * \snippet Sequences.cpp sequence mixed
     *
     * `ScopedSequence` simplifies the setup of sequences. This is actually equivalent to the previously demonstrated
     * `LazySequence` example.
     * \snippet Sequences.cpp scoped
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
}

namespace mimicpp::sequence::detail
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
        BasicSequence(BasicSequence const&) = delete;
        BasicSequence& operator=(BasicSequence const&) = delete;
        BasicSequence(BasicSequence&&) = delete;
        BasicSequence& operator=(BasicSequence&&) = delete;

        ~BasicSequence() noexcept(false)
        {
            auto const iter = std::ranges::find_if_not(m_Entries, &Entry::is_fulfilled);
            auto const satisfiedCount = std::ranges::distance(m_Entries.cbegin(), iter);
            MIMICPP_ASSERT(m_Cursor <= satisfiedCount, "Cursor skipped unsatisfied entries.");

            if (iter != m_Entries.cend())
            {
                reporting::detail::report_error(
                    format::format(
                        "Unfulfilled sequence. {} out of {} expectation(s) are satisfied.",
                        satisfiedCount,
                        m_Entries.size()));
            }
        }

        [[nodiscard]]
        explicit constexpr BasicSequence(util::SourceLocation loc = {}) noexcept
            : m_Loc{std::move(loc)}
        {
        }

        [[nodiscard]]
        constexpr util::SourceLocation const& from() const noexcept
        {
            return m_Loc;
        }

        [[nodiscard]]
        constexpr std::optional<int> priority_of(Id const id) const noexcept
        {
            MIMICPP_ASSERT(is_valid(id), "Invalid id given.");

            if (is_consumable(id))
            {
                return std::invoke(priorityStrategy, id, m_Cursor);
            }

            return std::nullopt;
        }

        constexpr void set_satisfied(Id const id) noexcept
        {
            MIMICPP_ASSERT(is_valid(id), "Invalid id given.");
            MIMICPP_ASSERT(m_Cursor <= util::to_underlying(id), "Invalid state.");

            auto& element = m_Entries[util::to_underlying(id)];
            MIMICPP_ASSERT(element.is_unsatisfied(), "Element is in unexpected state.");
            element.state = State::satisfied;
        }

        constexpr void set_saturated(Id const id) noexcept
        {
            MIMICPP_ASSERT(is_valid(id), "Invalid id given.");
            int const index = util::to_underlying(id);
            MIMICPP_ASSERT(m_Cursor <= index, "Invalid state.");

            auto& element = m_Entries[index];
            MIMICPP_ASSERT(element.is_active(), "Element is in unexpected state.");
            element.state = State::saturated;
        }

        [[nodiscard]]
        constexpr bool is_consumable(Id const id) const noexcept
        {
            MIMICPP_ASSERT(is_valid(id), "Invalid id given.");

            std::span const pending = std::span{m_Entries}.subspan(m_Cursor);
            auto const index = util::to_underlying(id) - m_Cursor;

            return 0 <= index
                && std::ranges::all_of(pending.first(index), &Entry::is_fulfilled)
                && pending[index].is_active();
        }

        constexpr void consume(Id const id) noexcept
        {
            MIMICPP_ASSERT(is_consumable(id), "Sequence is not in consumable state.");

            m_Cursor = util::to_underlying(id);
        }

        [[nodiscard]]
        constexpr Id add(util::SourceLocation info)
        {
            if (!std::in_range<std::underlying_type_t<Id>>(m_Entries.size()))
                [[unlikely]]
            {
                throw std::runtime_error{"Sequence already holds maximum amount of elements."};
            }

            m_Entries.emplace_back(State::unsatisfied, std::move(info));

            return static_cast<Id>(m_Entries.size() - 1);
        }

        [[nodiscard]]
        constexpr Tag tag() const noexcept
        {
            return Tag{util::bit_cast<std::ptrdiff_t>(this)};
        }

    private:
        util::SourceLocation m_Loc;

        enum class State
        {
            unsatisfied,
            satisfied,
            saturated
        };

        struct Entry
        {
            State state{State::unsatisfied};
            util::SourceLocation loc{};

            [[nodiscard]]
            constexpr bool is_fulfilled() const noexcept
            {
                return state == State::satisfied
                    || state == State::saturated;
            }

            [[nodiscard]]
            constexpr bool is_active() const noexcept
            {
                return state == State::unsatisfied
                    || state == State::satisfied;
            }

            [[nodiscard]]
            constexpr bool is_unsatisfied() const noexcept
            {
                return state == State::unsatisfied;
            }
        };

        std::vector<Entry> m_Entries{};
        int m_Cursor{};

        [[nodiscard]]
        constexpr bool is_valid(Id const id) const noexcept
        {
            auto const index = util::to_underlying(id);

            return 0 <= index
                && index < std::ssize(m_Entries);
        }
    };

    class LazyStrategy
    {
    public:
        [[nodiscard]]
        constexpr int operator()(auto const id, int const cursor) const noexcept
        {
            int const index = util::to_underlying(id);
            MIMICPP_ASSERT(std::cmp_less_equal(cursor, index), "Invalid state.");

            return std::numeric_limits<int>::max()
                 - (index - cursor);
        }
    };

    class GreedyStrategy
    {
    public:
        [[nodiscard]]
        constexpr int operator()(auto const id, int const cursor) const noexcept
        {
            int const index = util::to_underlying(id);
            MIMICPP_ASSERT(std::cmp_less_equal(cursor, index), "Invalid state.");

            return index - cursor;
        }
    };

    template <typename... Sequences>
    class Config;

    template <typename Id, auto priorityStrategy>
    class BasicSequenceInterface
    {
        using Sequence = BasicSequence<Id, priorityStrategy>;

    public:
        BasicSequenceInterface(const BasicSequenceInterface&) = delete;
        BasicSequenceInterface& operator=(const BasicSequenceInterface&) = delete;

        ~BasicSequenceInterface() = default;

        [[nodiscard]]
        explicit BasicSequenceInterface(util::SourceLocation loc = {})
            : m_Sequence{std::make_shared<Sequence>(std::move(loc))}
        {
        }

        [[nodiscard]]
        BasicSequenceInterface(BasicSequenceInterface&&) = default;
        BasicSequenceInterface& operator=(BasicSequenceInterface&&) = default;

        [[nodiscard]]
        constexpr Tag tag() const noexcept
        {
            return m_Sequence->tag();
        }

        [[nodiscard]]
        util::SourceLocation const& from() const noexcept
        {
            return m_Sequence->from();
        }

        template <typename... Sequences>
            requires util::same_as_any<Sequence, Sequences...>
        constexpr std::shared_ptr<Sequence> sequence([[maybe_unused]] util::pass_key<Config<Sequences...>> key) const
        {
            return m_Sequence;
        }

    private:
        std::shared_ptr<Sequence> m_Sequence;
    };

    template <typename... Sequences>
    class Config
    {
        static constexpr util::pass_key<Config> selfKey{};

    public:
        static constexpr std::size_t sequenceCount = sizeof...(Sequences);

        [[nodiscard]]
        Config()
            requires(0u == sequenceCount)
        = default;

        template <typename First, typename... Others>
            requires(1u + sizeof...(Others) == sequenceCount)
                 && (!std::same_as<util::pass_key<Config>, std::remove_cvref_t<First>>)
                 && (!std::same_as<Config, std::remove_cvref_t<First>>)
        [[nodiscard]] //
        explicit constexpr Config(First& firstInterface, Others&... interfaces) noexcept(1u == sequenceCount)
            : Config{
                  selfKey,
                  firstInterface.sequence(selfKey),
                  interfaces.sequence(selfKey)...}
        {
        }

        template <typename... OtherSequences>
        [[nodiscard]] //
        explicit constexpr Config(
            [[maybe_unused]] util::pass_key<Config<OtherSequences...>> const key,
            std::shared_ptr<Sequences>... sequences)
            noexcept(1u == sequenceCount)
        {
            if constexpr (1u < sequenceCount)
            {
                std::array tags{sequences->tag()...};

                std::ranges::sort(tags);
                if (!std::ranges::empty(std::ranges::unique(tags)))
                {
                    throw std::invalid_argument{
                        "Expectations can not be assigned to the same sequence multiple times."};
                }
            }

            m_Sequences = std::tuple{std::move(sequences)...};
        }

        [[nodiscard]]
        constexpr auto& sequences() const noexcept
        {
            return m_Sequences;
        }

        template <typename... OtherSequences>
        [[nodiscard]]
        constexpr Config<Sequences..., OtherSequences...> concat(Config<OtherSequences...> const& other) const
        {
            return std::make_from_tuple<Config<Sequences..., OtherSequences...>>(
                std::tuple_cat(std::make_tuple(selfKey), m_Sequences, other.sequences()));
        }

    private:
        std::tuple<std::shared_ptr<Sequences>...> m_Sequences;
    };
}

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp
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
    public:
        [[nodiscard]]
        explicit LazySequence([[maybe_unused]] auto&&... canary, util::SourceLocation loc = {})
            : BasicSequenceInterface{std::move(loc)}
        {
            static_assert(0u == sizeof...(canary), "LazySequence does not accept constructor arguments.");
        }
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
    public:
        [[nodiscard]]
        explicit GreedySequence([[maybe_unused]] auto&&... canary, util::SourceLocation loc = {})
            : BasicSequenceInterface{std::move(loc)}
        {
            static_assert(0u == sizeof...(canary), "GreedySequence does not accept constructor arguments.");
        }
    };

    /**
     * \brief The default sequence type (LazySequence).
     * \ingroup EXPECTATION_SEQUENCE
     * \deprecated Please use `mimicpp::Sequence` instead.
     */
    using SequenceT [[deprecated("Please use mimicpp::Sequence instead.")]] = LazySequence;

    /**
     * \brief The default sequence type (LazySequence).
     * \ingroup EXPECTATION_SEQUENCE
     */
    using Sequence = LazySequence;
}

namespace mimicpp::sequence::detail
{
    [[nodiscard]]
    constexpr bool has_better_rating(
        std::span<rating const> const lhs,
        std::span<rating const> const rhs) noexcept
    {
        int rating{};
        for (auto const& [lhsPriority, lhsTag] : lhs)
        {
            if (auto const iter = std::ranges::find(rhs, lhsTag, &rating::tag);
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

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::expect
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
    constexpr auto in_sequence(sequence::detail::BasicSequenceInterface<Id, priorityStrategy>& sequence)
    {
        using Config = sequence::detail::Config<
            sequence::detail::BasicSequence<Id, priorityStrategy>>;

        return Config{sequence};
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
        using Config = sequence::detail::Config<
            sequence::detail::BasicSequence<FirstId, firstPriorityStrategy>,
            sequence::detail::BasicSequence<SecondId, secondPriorityStrategy>,
            sequence::detail::BasicSequence<OtherIds, otherPriorityStrategies>...>;

        return Config{firstSequence, secondSequence, otherSequences...};
    }
}

#endif
