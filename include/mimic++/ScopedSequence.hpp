//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_SCOPED_SEQUENCE_HPP
#define MIMICPP_SCOPED_SEQUENCE_HPP

#pragma once

#include "mimic++/Expectation.hpp"
#include "mimic++/ExpectationBuilder.hpp"
#include "mimic++/Sequence.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

#include <deque>
#include <functional>
#include <utility>

namespace mimicpp::sequence::detail
{
    template <typename Sequence>
    class ExpectationBuilderFinalizer
    {
    public:
        ExpectationBuilderFinalizer(ExpectationBuilderFinalizer const&) = delete;
        ExpectationBuilderFinalizer& operator=(ExpectationBuilderFinalizer const&) = delete;
        ExpectationBuilderFinalizer(ExpectationBuilderFinalizer&&) = delete;
        ExpectationBuilderFinalizer& operator=(ExpectationBuilderFinalizer&&) = delete;

        ~ExpectationBuilderFinalizer() = default;

        template <bool timesConfigured, typename... Args>
        [[nodiscard]]
        explicit(false) constexpr ExpectationBuilderFinalizer(
            BasicExpectationBuilder<timesConfigured, Args...>&& builder,
            util::SourceLocation loc = {})
            : m_Builder{&builder},
              m_SourceLocation{std::move(loc)},
              m_FinalizeFn{
                  +[](void* storage, util::SourceLocation const& loc, Sequence& sequence) {
                      auto* builderPtr = static_cast<BasicExpectationBuilder<timesConfigured, Args...>*>(storage);
                      return ScopedExpectation{
                          std::move(*builderPtr) and expect::in_sequence(sequence),
                          *loc};
                  }}
        {
        }

        [[nodiscard]]
        ScopedExpectation finalize(Sequence& sequence)
        {
            MIMICPP_ASSERT(m_Builder, "Builder is nullptr.");
            MIMICPP_ASSERT(m_FinalizeFn, "FinalizeFn is nullptr.");

            return std::invoke(
                std::exchange(m_FinalizeFn, nullptr),
                std::exchange(m_Builder, nullptr),
                m_SourceLocation,
                sequence);
        }

    private:
        void* m_Builder;
        util::SourceLocation m_SourceLocation;

        using FinalizeFn = ScopedExpectation (*)(void*, util::SourceLocation const&, Sequence&);
        FinalizeFn m_FinalizeFn;
    };
}

namespace mimicpp
{
    /**
     * \brief A sequence type that verifies its owned expectations during destruction.
     * \ingroup EXPECTATION_SEQUENCE
     * \tparam Strategy The sequence strategy employed.
     *
     * \details This type offers a convenient mechanism for chaining multiple expectations into a sequence,
     * and it monitors whether they are fulfilled upon destruction.
     *
     * In essence, it behaves similarly to manually attaching expectations via `expect::in_sequence` to a sequence.
     * The primary distinction is that if an expectation is not satisfied,
     * this type will report the first unfulfilled expectation.
     * In contrast, when expectations are manually constructed as `ScopedExpectation`,
     * the last created expectation (due to the stack’s order) would typically be reported.
     *
     * Nevertheless, all standard sequence rules also apply.
     *
     * \snippet Sequences.cpp scoped
     */
    template <auto Strategy>
    class BasicScopedSequence
        : public sequence::detail::BasicSequenceInterface<sequence::Id, Strategy>
    {
    public:
        /**
         * \brief Deleted copy-constructor.
         */
        [[nodiscard]]
        BasicScopedSequence(BasicScopedSequence const&) = delete;

        /**
         * \brief Deleted copy-assignment operator.
         */
        BasicScopedSequence& operator=(BasicScopedSequence const&) = delete;

        /**
         * \brief Possibly throwing destructor, checking the owned expectations in order of construction.
         */
        ~BasicScopedSequence() noexcept(false)
        {
            auto expectations = std::exchange(m_Expectations, {});
            while (!expectations.empty())
            {
                expectations.pop_front();
            }
        }

        /**
         * \brief Default default-constructor.
         */
        [[nodiscard]]
        BasicScopedSequence() = default;

        /**
         * \brief Defaulted move-constructor.
         */
        [[nodiscard]]
        BasicScopedSequence(BasicScopedSequence&&) = default;

        /**
         * \brief Defaulted move-assignment operator.
         * \return A mutable reference to the current instance.
         */
        BasicScopedSequence& operator=(BasicScopedSequence&&) = default;

        /**
         * \brief Attaches a newly constructed expectation.
         * \param builder The expectation-builder that will be finalized.
         * \return A mutable reference to the current instance.
         * \details This function augments the provided expectation-builder with an additional sequence-policy for this
         * sequence and finalizes its construction.
         * Additionally, the sequence takes over the ownership of the constructed `ScopedExpectation`.
         */
        BasicScopedSequence& operator+=(sequence::detail::ExpectationBuilderFinalizer<BasicScopedSequence>&& builder)
        {
            m_Expectations.emplace_back(builder.finalize(*this));

            return *this;
        }

        /**
         * \brief Retrieves the collection of explicitly owned expectations.
         * \return A constant reference to the collection of attached expectations.
         * \note This function returns only those expectations that were explicitly attached using the `operator +=`,
         * and not every expectation that is currently queued in the sequence (i.e. by manually using `expect::in_sequence`).
         */
        [[nodiscard]]
        std::deque<ScopedExpectation> const& expectations() const noexcept
        {
            return m_Expectations;
        }

    private:
        std::deque<ScopedExpectation> m_Expectations{};
    };

    /**
     * \brief The scoped-sequence type with greedy strategy.
     * \ingroup EXPECTATION_SEQUENCE
     */
    using GreedyScopedSequence = BasicScopedSequence<sequence::detail::GreedyStrategy{}>;

    /**
     * \brief The scoped-sequence type with lazy strategy.
     * \ingroup EXPECTATION_SEQUENCE
     */
    using LazyScopedSequence = BasicScopedSequence<sequence::detail::LazyStrategy{}>;

    /**
     * \brief The default scoped-sequence type (lazy strategy).
     * \ingroup EXPECTATION_SEQUENCE
     */
    using ScopedSequence = LazyScopedSequence;
}

#endif
