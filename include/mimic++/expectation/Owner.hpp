//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_EXPECTATION_OWNER_HPP
#define MIMICPP_EXPECTATION_OWNER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/config/Config.hpp"
#include "mimic++/expectation/Expectation.hpp"
#include "mimic++/expectation/Registry.hpp"

#ifndef MIMICPP_DETAIL_IS_MODULE
    #include <concepts>
    #include <utility>
#endif

MIMICPP_DETAIL_MODULE_EXPORT namespace mimicpp::expectation
{
    namespace detail
    {
        template <typename T>
        concept finalizable = requires(util::SourceLocation loc) {
            { std::declval<T&&>().finalize(loc) } -> std::convertible_to<Owner>;
        };
    }

    /**
     * \brief Takes the ownership of an expectation and checks whether it's satisfied during destruction.
     * \ingroup EXPECTATION
     * \details
     * Instances of this type have exclusive ownership of their attached expectations, which will be checked during destruction.
     * \snippet ExpectationHandling.cpp expectation basic
     *
     * They can be freely passed around and may even outlive their target-mock.
     * \snippet ExpectationHandling.cpp expectation outlive
     */
    class Owner
    {
    public:
        /**
         * \brief Deleted copy-constructor.
         */
        Owner(Owner const&) = delete;

        /**
         * \brief Deleted copy-assignment-operator.
         */
        Owner& operator=(Owner const&) = delete;

        /**
         * \brief Removes the owned expectation from the `Registry` and checks whether it's satisfied.
         * \attention In case of an unsatisfied expectation, the destructor is expected to throw or terminate otherwise.
         */
        ~Owner() noexcept(false)
        {
            if (m_Registry)
            {
                m_Registry->remove(m_Expectation);
            }
        }

        /**
         * \brief Constructor, which generates the type-erase storage.
         * \param registry The target expectation registry.
         * \param expectation The target expectation.
         */
        [[nodiscard]]
        explicit Owner(Registry::Ptr registry, Expectation expectation)
            : m_Registry{std::move(registry)},
              m_Expectation{std::move(expectation)}
        {
        }

        /**
         * \brief A constructor accepting objects, which can be finalized (i.e., `expectation::Builder`).
         * \tparam T The object type.
         * \param object The object to be finalized.
         * \param loc The source-location.
         */
        template <detail::finalizable T>
        [[nodiscard]]
        explicit(false) constexpr Owner(T&& object, util::SourceLocation loc = {})
            : Owner{std::forward<T>(object).finalize(std::move(loc))}
        {
        }

        /**
         * \brief Defaulted move-constructor.
         */
        [[nodiscard]]
        Owner(Owner&&) = default;

        /**
         * \brief Defaulted move-assignment-operator.
         */
        Owner& operator=(Owner&&) = default;

        /**
         * \brief Queries the stored expectation, whether it's satisfied.
         * \return True, if satisfied.
         */
        [[nodiscard]]
        bool is_satisfied() const
        {
            return m_Expectation.is_satisfied();
        }

        /**
         * \brief Queries the stored expectation, whether it's applicable.
         * \return True, if applicable.
         */
        [[nodiscard]]
        bool is_applicable() const
        {
            return m_Expectation.is_applicable();
        }

        /**
         * \brief Queries the stored expectation for it's stored source-location.
         * \return The stored source-location.
         */
        [[nodiscard]]
        util::SourceLocation const& from() const noexcept
        {
            return m_Expectation.from();
        }

        /**
         * \brief Queries the stored expectation for the name of the related mock.
         * \return The stored mock-name.
         */
        [[nodiscard]]
        StringT const& mock_name() const noexcept
        {
            return m_Expectation.mock_name();
        }

    private:
        Registry::Ptr m_Registry;
        Expectation m_Expectation;
    };
}

#endif
