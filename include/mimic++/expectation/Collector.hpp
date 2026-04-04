//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_COLLECTOR_HPP
#define MIMICPP_EXPECTATION_COLLECTOR_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/expectation/Owner.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <deque>
    #include <utility>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::expectation
{
    /**
     * \brief Collects scoped expectations and verifies them during destruction.
     * \ingroup EXPECTATION
     *
     * \details
     * A `Collector` owns explicitly attached expectations and keeps them alive until it is destroyed.
     * When destruction happens, all owned expectations are checked in order of construction.
     */
    class Collector
    {
    public:
        /**
         * \brief Deleted copy-constructor.
         */
        Collector(Collector const&) = delete;

        /**
         * \brief Deleted copy-assignment operator.
         */
        Collector& operator=(Collector const&) = delete;

        /**
         * \brief Possibly throwing destructor, checking the owned expectations in order of construction.
         */
        ~Collector() noexcept(false)
        {
            auto expectations = std::exchange(m_Expectations, {});
            while (!expectations.empty())
            {
                // Prevent the `pop_front() noexcept` from raising an exception and thus terminating.
                auto const expectation = std::move(expectations.front());
                expectations.pop_front();
            }
        }

        /**
         * \brief Defaulted default-constructor.
         */
        [[nodiscard]]
        Collector() = default;

        /**
         * \brief Defaulted move-constructor.
         */
        [[nodiscard]]
        Collector(Collector&&) = default;

        /**
         * \brief Defaulted move-assignment operator.
         * \return A mutable reference to the current instance.
         */
        Collector& operator=(Collector&&) = default;

        /**
         * \brief Attaches a newly constructed expectation.
         * \param builder The expectation-builder that will be finalized.
         * \return A mutable reference to the current instance.
         * \details This collector becomes the owner of the created expectation and attaches it to the internal expectation-collection.
         * Even though this function formally requires a `ScopedExpectation` as an argument, it's expected that users pass an expectation-builder.
         * Doing so will then correctly deduce the source-location.
         */
        Collector& attach(ScopedExpectation&& builder) &
        {
            m_Expectations.emplace_back(std::move(builder));

            return *this;
        }

        /**
         * \copydoc Collector::attach
         *
         * \note In CLion, expressions with this operator may incorrectly report that the variable is never used after being assigned.
         * This is a known bug that unfortunately cannot be worked around on the framework level.
         * If you find yourself stuck to the Resharper DFA, you can use \ref Collector::attach instead.
         * \see https://youtrack.jetbrains.com/issue/CPP-49527/
         * \see https://youtrack.jetbrains.com/issue/CPP-38216
         */
        Collector& operator+=(ScopedExpectation&& builder) &
        {
            return attach(std::move(builder));
        }

        /**
         * \brief Retrieves the collection of explicitly owned expectations.
         * \return A constant reference to the collection of attached expectations.
         */
        [[nodiscard]]
        constexpr std::deque<ScopedExpectation> const& expectations() const noexcept
        {
            return m_Expectations;
        }

    private:
        std::deque<ScopedExpectation> m_Expectations{};
    };
}

#endif
