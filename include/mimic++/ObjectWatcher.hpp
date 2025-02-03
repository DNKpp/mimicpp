//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_OBJECT_WATCHER_HPP
#define MIMICPP_OBJECT_WATCHER_HPP

#pragma once

#include "mimic++/Fwd.hpp"
#include "mimic++/Mock.hpp"
#include "mimic++/printing/TypePrinter.hpp"

#include <concepts>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace mimicpp::detail
{
    template <typename Base>
    [[nodiscard]]
    static StringT generate_lifetime_watcher_mock_name()
    {
        StringStreamT out{};
        out << "LifetimeWatcher for " << print_type<Base>();
        return std::move(out).str();
    }

    template <typename Base>
    [[nodiscard]]
    static StringT generate_relocation_watcher_mock_name()
    {
        StringStreamT out{};
        out << "RelocationWatcher for " << print_type<Base>();
        return std::move(out).str();
    }
}

namespace mimicpp
{
    /**
     * \defgroup OBJECT_WATCHING object-watching
     * \brief Contains utility for explicit tracking of special object functionalities.
     * \details
     * \snippet Watcher.cpp watched lifetime relocation
     *
     *\{
     */

    template <typename Base>
    struct for_base_tag
    {
        using type = Base;
    };

    template <typename Base>
    constexpr for_base_tag<Base> for_base_v{};

    /**
     * \brief A watcher type, which reports it's destructor calls.
     * \details This watcher is designed to track, whether the destructor has been called.
     * During its destructor call, it reports the destruction to the framework, which can be tracked
     * by a previously created destruction-expectation.
     *
     * \snippet Watcher.cpp watched lifetime-watcher
     *
     * ## Moving
     *
     * This watcher can be freely moved around.
     *
     * ## Copying
     *
     * This watcher is copyable, but with very special behaviour.
     *
     * As this watcher is generally designed to be part of a bigger object, it would be very limiting not supporting
     * copy-operations at all. The question is, how should a copy look like?
     *
     * In general a copy should be a logical duplicate of its source and the general expectation is:
     * if ``B`` is a copy of ``A``, then ``A == B`` should yield true.
     * \note This doesn't say, that if ``B`` is *not* a copy of ``A``, then ``A == B`` has to yield false!
     *
     * \details This won't be the case for ``LifetimeWatcher``s, as active destruction-expectations won't be copied over
     * to the target. In general, if a LifetimeWatcher is used, we want to be very precise with our object-lifetime,
     * thus an implicit expectation copy would be against the purpose of this helper.
     * Due to this, each ``LifetimeWatcher`` will be created as a fresh instance, when copy-construction is used.
     * The same logic also applies to copy-assignment.
     */
    class LifetimeWatcher
    {
        // omits the following entries:
        // - ~LifetimeWatcher
        // - ~CombinedWatchers
        // - ~BasicWatched
        // - ~Watched
        static constexpr std::size_t stacktraceSkip = 4u;

    public:
        /**
         * \brief Destructor, which reports the call.
         * \note A no-match error may occur, if no destruction-expectation has been defined.
         * \snippet Watcher.cpp watched lifetime-watcher violation
         */
        ~LifetimeWatcher() noexcept(false)
        {
            if (const std::unique_ptr destruction = std::exchange(
                    m_DestructionMock,
                    nullptr))
            {
                (*destruction)();
            }
        }

        /**
         * \brief Default constructor.
         */
        [[nodiscard]]
        LifetimeWatcher() = default;

        template <typename Base>
        [[nodiscard]] explicit LifetimeWatcher([[maybe_unused]] const for_base_tag<Base>)
            : LifetimeWatcher{
                  MockSettings{
                               .name = detail::generate_lifetime_watcher_mock_name<Base>(),
                               .stacktraceSkip = stacktraceSkip}
        }
        {
        }

        /**
         * \brief Copy-constructor.
         * \param other The other object.
         * \details This copy-constructor's purpose is to provide syntactically correct copy operations,
         * but semantically this does not copy anything.
         * In fact, it simply default-constructs the new instance, without even touching the ``other``.
         * \note It is mandatory setting up a new destruction-expectation. Otherwise, a no-match will be reported
         * during destruction.
         * \snippet Watcher.cpp watched lifetime-watcher copy-construction violation
         */
        [[nodiscard]]
        LifetimeWatcher(const LifetimeWatcher& other)
            : LifetimeWatcher{other.m_MockSettings}
        {
        }

        /**
         * \brief Copy-assignment-operator.
         * \param other The other object.
         * \details This copy-assignment-operator's purpose is to provide syntactically correct copy operations,
         * but semantically this does not copy anything.
         * In fact, it simply deletes the previous content of this instance, default-constructs a new instance
         * and move-assigns it to this instance, without even touching the ``other``.
         * \note It is mandatory setting up a new destruction-expectation. Otherwise, a no-match will be reported
         * during destruction.
         * \snippet Watcher.cpp watched lifetime-watcher copy-assignment violation
         *
         * \note As this actually destructs a ``LifetimeWatcher``, violations will be reported, if the previous
         * instance didn't have a valid destruction-expectation.
         * \snippet Watcher.cpp watched lifetime-watcher copy-assignment violation2
         */
        LifetimeWatcher& operator=(const LifetimeWatcher& other)
        {
            // let's make this a two-step.
            // First destroy the previous instance, which may already report a violation.
            // If we would already have the new instance created, this would lead also to
            // a violation report, which actually might break everything.
            {
                LifetimeWatcher temp{std::move(*this)};
            }

            *this = LifetimeWatcher{other.m_MockSettings};

            return *this;
        }

        /**
         * \brief Defaulted move-constructor.
         * \details This move-constructor simply transfers everything from the source to the destination object.
         * As source is then a "moved-from"-object, it doesn't require any destruction-expectations.
         */
        [[nodiscard]]
        LifetimeWatcher(LifetimeWatcher&&) = default;

        /**
         * \brief Defaulted move-assignment-operator.
         * \details This move-assignment-operator simply transfers everything from the source to the destination object.
         * As source is then a "moved-from"-object, it doesn't require any destruction-expectations.
         */
        LifetimeWatcher& operator=(LifetimeWatcher&&) = default;

        /**
         * \brief Begins a destruction-expectation construction.
         * \return A newly created expectation-builder-instance.
         * \note This function creates a new expectation-builder-instance, which isn't an expectation yet.
         * User must convert this to an actual expectation, by handing it over to a new ``ScopedExpectation`` instance.
         * This can either be done manually or via \ref MIMICPP_SCOPED_EXPECTATION (or the shorthand version \ref SCOPED_EXP).
         * \throws std::logic_error if a destruction-expectation has already been created for this instance.
         */
        [[nodiscard]]
        auto expect_destruct()
        {
            if (std::exchange(m_HasDestructExpectation, true))
            {
                throw std::logic_error{
                    "LifetimeWatcher: A destruct expectation can not be created more than once for a single instance."};
            }

            return m_DestructionMock->expect_call()
               and expect::once(); // prevent further times specifications
        }

    private:
        bool m_HasDestructExpectation{};
        MockSettings m_MockSettings{};
        std::unique_ptr<Mock<void()>> m_DestructionMock{
            std::make_unique<Mock<void()>>(m_MockSettings)};

        [[nodiscard]]
        explicit LifetimeWatcher(MockSettings settings)
            : m_MockSettings{std::move(settings)}
        {
        }
    };

    /**
     * \brief A watcher type, which reports it's move-constructor and -assignment calls.
     * \details This watcher is designed to track, whether a move has been performed.
     * During a move, it reports the relocation to the framework, which can be tracked
     * by a previously created relocation-expectation.
     *
     * \snippet Watcher.cpp watched lifetime relocation
     *
     * ## Moving
     *
     * This watcher can be freely moved around, but any relocation events must match with a previously created
     * relocation-expectation.
     *
     * ## Copying
     *
     * This watcher is copyable, but with very special behaviour.
     *
     * As this watcher is generally designed to be part of a bigger object, it would be very limiting not supporting
     * copy-operations at all. The question is, how should a copy look like?
     *
     * In general a copy should be a logical duplicate of its source and the general expectation is:
     * if ``B`` is a copy of ``A``, then ``A == B`` should yield true.
     * \note This doesn't say, that if ``B`` is *not* a copy of ``A``, then ``A == B`` has to yield false!
     *
     * \details This won't be the case for ``RelocationWatcher``s, as active relocation-expectations won't be copied over
     * to the target. In general, if a ``RelocationWatcher`` is used, we want to be very precise with our move,
     * thus an implicit expectation copy would be against the purpose of this helper.
     * Due to this, each ``RelocationWatcher`` will be created as a fresh instance, when copy-construction is used.
     * The same logic also applies to copy-assignment.
     */
    class RelocationWatcher
    {
        // omits the following entries:
        // - RelocationWatcher::handle_move
        // - RelocationWatcher move ctor/assignment
        // - CombinedWatchers move ctor/assignment
        // - BasicWatched move ctor/assignment
        // - Watched move ctor/assignment
        static constexpr std::size_t stacktraceSkip = 5u;

    public:
        /**
         * \brief Defaulted destructor.
         */
        ~RelocationWatcher() = default;

        /**
         * \brief Defaulted default constructor.
         */
        [[nodiscard]]
        RelocationWatcher() = default;

        template <typename Base>
        [[nodiscard]] explicit RelocationWatcher([[maybe_unused]] const for_base_tag<Base>)
            : RelocationWatcher{
                  MockSettings{
                               .name = detail::generate_relocation_watcher_mock_name<Base>(),
                               .stacktraceSkip = stacktraceSkip}
        }
        {
        }

        /**
         * \brief Copy-constructor.
         * \param other The other object.
         * \details This copy-constructor's purpose is to provide syntactically correct copy operations,
         * but semantically this does not copy anything.
         * In fact, it simply default-constructs the new instance, without even touching the ``other``.
         */
        [[nodiscard]]
        RelocationWatcher(const RelocationWatcher& other)
            : RelocationWatcher{other.m_MockSettings}
        {
        }

        /**
         * \brief Copy-assignment-operator.
         * \param other The other object.
         * \details This copy-assignment-operator's purpose is to provide syntactically correct copy operations,
         * but semantically this does not copy anything.
         * In fact, it simply overrides its internals with a fresh instance, without even touching the ``other``.
         */
        RelocationWatcher& operator=(const RelocationWatcher& other)
        {
            m_MockSettings = other.m_MockSettings;

            // explicitly circumvent default construct and assign, because otherwise that would
            // involve the move-assignment.
            m_RelocationMock = Mock<void()>{m_MockSettings};

            return *this;
        }

        /**
         * \brief Move-constructor, which reports a relocation.
         * \note A no-match error may occur, if no relocation-expectation has been defined.
         * \param other The other object.
         */
        [[nodiscard]]
        RelocationWatcher(RelocationWatcher&& other) noexcept(false)
        {
            handle_move(std::move(other));
        }

        /**
         * \brief Move-assignment-operator, which reports a relocation.
         * \note A no-match error may occur, if no relocation-expectation has been defined.
         * \param other The other object.
         */
        RelocationWatcher& operator=(RelocationWatcher&& other) noexcept(false)
        {
            handle_move(std::move(other));

            return *this;
        }

        /**
         * \brief Begins a relocation-expectation construction.
         * \return A newly created expectation-builder-instance.
         * \note This function creates a new expectation-builder-instance, which isn't an expectation yet.
         * User must convert this to an actual expectation, by handing it over to a new ``ScopedExpectation`` instance.
         * This can either be done manually or via \ref MIMICPP_SCOPED_EXPECTATION (or the shorthand version \ref SCOPED_EXP).
         */
        [[nodiscard]]
        auto expect_relocate()
        {
            return m_RelocationMock.expect_call();
        }

    private:
        MockSettings m_MockSettings{};
        Mock<void()> m_RelocationMock{m_MockSettings};

        [[nodiscard]]
        explicit RelocationWatcher(MockSettings settings)
            : m_MockSettings{std::move(settings)}
        {
        }

        void handle_move(RelocationWatcher&& other)
        {
            other.m_RelocationMock();

            std::ranges::swap(m_MockSettings, other.m_MockSettings);
            // do not swap here, because we want the target mock to be destroyed NOW
            m_RelocationMock = std::move(other).m_RelocationMock;
        }
    };

    template <typename T, typename Base>
    concept object_watcher_for = std::is_constructible_v<T, for_base_tag<Base>>
                              && std::is_copy_constructible_v<T>
                              && std::is_copy_assignable_v<T>
                              && std::is_move_constructible_v<T>
                              && std::is_move_assignable_v<T>
                              && std::is_destructible_v<T>;

    namespace detail
    {
        template <typename Base, typename... Watchers>
        class CombinedWatchers
            : public Watchers...
        {
        public:
            ~CombinedWatchers() noexcept(std::is_nothrow_destructible_v<Base>) = default;

            CombinedWatchers()
                : Watchers{for_base_v<Base>}...
            {
            }

            CombinedWatchers(const CombinedWatchers&) = default;
            CombinedWatchers& operator=(const CombinedWatchers&) = default;

            CombinedWatchers(CombinedWatchers&& other) noexcept(std::is_nothrow_move_constructible_v<Base>) = default;
            CombinedWatchers& operator=(CombinedWatchers&& other) noexcept(std::is_nothrow_move_assignable_v<Base>) = default;
        };

        template <typename Base, typename... Watchers>
        class BasicWatched
            : public CombinedWatchers<Base, Watchers...>,
              public Base
        {
        public:
            ~BasicWatched() = default;

            using Base::Base;

            BasicWatched(const BasicWatched&) = default;
            BasicWatched& operator=(const BasicWatched&) = default;
            BasicWatched(BasicWatched&&) = default;
            BasicWatched& operator=(BasicWatched&&) = default;
        };

        template <satisfies<std::has_virtual_destructor> Base, typename... Watchers>
        class BasicWatched<Base, Watchers...>
            : public CombinedWatchers<Base, Watchers...>,
              public Base
        {
        public:
            ~BasicWatched() override = default;

            using Base::Base;

            BasicWatched(const BasicWatched&) = default;
            BasicWatched& operator=(const BasicWatched&) = default;
            BasicWatched(BasicWatched&&) = default;
            BasicWatched& operator=(BasicWatched&&) = default;
        };
    }

    /**
     * \brief CRTP-type, inheriting first from all ``Watchers`` and then ``Base``, thus effectively couple them all together.
     * \tparam Base The main type.
     * \tparam Watchers All utilized watcher types.
     * \details
     * ## Move-constructor and -assignment-operator
     *
     * ``Watched`` automatically detects the specifications of the ``Base`` move-constructor and -assignment-operator,
     * regardless of the ``Watchers`` specifications. This is done, so that the ``Watched`` instance does mimic the interface
     * of ``Base`` as closely as possible.
     *
     * This is important to note, as this has implications when a ``RelocationWatcher`` is utilized.
     * ``RelocationWatcher`` may, during either move-construction or -assignment, report violations to the currently active reporter.
     * This reporter has to act accordingly, by either throwing an exception or terminating the program.
     *
     * So, if reporter throws due to a detected violation and the move-operation is declared ``noexcept``, this will inevitable lead
     * to a ``std::terminate``.
     * \see https://en.cppreference.com/w/cpp/error/terminate
     * Nevertheless, this is usually fine, as watchers are merely used under controlled circumstances and to guarantee the expected
     * behavior. If a violation is reported, an appropriate output will be generated, which should be enough of a hint to track
     * down the bug.
     *
     * ## Destructor
     *
     * ``Watched`` automatically detects, whether ``Base`` has a virtual destructor and applies ``override`` if that's the case.
     * It also forces the same ``noexcept``-ness for the destruct: If ``Base`` is nothrow destructible, ``Watched`` is it, too.
     *
     * This is important to note, as this has implications when a ``LifetimeWatcher`` is utilized.
     * ``LifetimeWatcher`` may, during destruction, report violations to the currently active reporter. This reporter has to
     * act accordingly, by either throwing an exception or terminating the program.
     *
     * As the destructor of the ``LifetimeWatcher`` will effectively be called from ``~Watched``, this will lead to a call to
     * ``std::terminate``, if ``Base`` has a ``noexcept`` destructor (which is very likely, as it's a very strong default) and
     * the reporter propagates the violation via an exception.
     * \see https://en.cppreference.com/w/cpp/language/noexcept_spec
     * \see https://en.cppreference.com/w/cpp/error/terminate
     *
     * There is no real way around that, beside explicitly ``~Watched`` as ``noexcept(false)``. Unfortunately, this would
     * lead to inconsistencies with ``noexcept`` declared ``virtual`` destructors, because this requires all subclasses to match
     * that specification.
     * Besides that, there is an even stronger argument to strictly follow what ``Base`` offers:
     * A ``Watched`` object should be as close to the original ``Base`` type as possible.
     * If one wants to store a ``Watched<Base>`` inside e.g. ``std::vector`` and ``~Watched`` would have a different ``noexcept``
     * specification than ``Base``, that would lead to behavior changes. This should never be the case,
     * as mocks are expected to behave like an actual implementation-object.
     *
     * So, what does all of this mean?
     *
     * Actually, there are no implications to working tests. If they satisfy the expectations, no one will notice anything different.
     * When it comes to a violation, which is detected by the destructor of a ``LifetimeWatcher``, the reporter will be notified and
     * should print the no-match report to the console. After that, the program will than probably terminate (or halt, if a debugger
     * is attached), but you should at least have an idea, which test is affected.
     */
    template <typename Base, object_watcher_for<Base>... Watchers>
        requires std::same_as<Base, std::remove_cvref_t<Base>>
    class Watched
        : public detail::BasicWatched<Base, Watchers...>
    {
        using SuperT = detail::BasicWatched<Base, Watchers...>;

    public:
        ~Watched() = default;

        using SuperT::SuperT;

        Watched(const Watched&) = default;
        Watched& operator=(const Watched&) = default;
        Watched(Watched&&) = default;
        Watched& operator=(Watched&&) = default;
    };

    /**
     * \}
     */
}

#endif
