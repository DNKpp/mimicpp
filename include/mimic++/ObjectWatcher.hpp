// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_OBJECT_WATCHER_HPP
#define MIMICPP_OBJECT_WATCHER_HPP

#pragma once

#include "mimic++/Mock.hpp"

#include <stdexcept>
#include <utility>

namespace mimicpp
{
	/**
	 * \defgroup OBJECT_WATCHING object-watching
	 * \brief Contains utility for explicit tracking of special object functionalities.
	 * \details 
	 *
	 *\{
	 */

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
	public:
		/**
		 * \brief Destructor, which reports the call.
		 * \note A no-match error may occur, if no destruction-expectation has been defined.
		 * \snippet Watcher.cpp watched lifetime-watcher violation
		 */
		~LifetimeWatcher() noexcept(false)
		{
			if (const auto destruction = std::exchange(
				m_DestructionMock,
				nullptr))
			{
				(*destruction)();
			}
		}

		/**
		 * \brief Defaulted default constructor.
		 */
		[[nodiscard]]
		LifetimeWatcher() = default;

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
		LifetimeWatcher([[maybe_unused]] const LifetimeWatcher& other)
			: LifetimeWatcher{}
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
		LifetimeWatcher& operator =([[maybe_unused]] const LifetimeWatcher& other)
		{
			// let's make this a two-step.
			// First destroy the previous instance, which may already report a violation.
			// If we would already have the new instance created, this would lead also to
			// a violation report, which actually might break everything.
			{
				LifetimeWatcher temp{std::move(*this)};
			}

			*this =	LifetimeWatcher{};

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
		LifetimeWatcher& operator =(LifetimeWatcher&&) = default;

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
					"LifetimeWatcher: A destruct expectation can not be created more than once for a single instance."
				};
			}

			return m_DestructionMock->expect_call()
					and expect::once();	// prevent further times specifications
		}

	private:
		bool m_HasDestructExpectation{};
		std::unique_ptr<Mock<void()>> m_DestructionMock{
			std::make_unique<Mock<void()>>()
		};
	};

	template <typename T>
	concept object_watcher = std::is_default_constructible_v<T>
							&& std::is_copy_constructible_v<T>
							&& std::is_copy_assignable_v<T>
							&& std::is_move_constructible_v<T>
							&& std::is_move_assignable_v<T>
							&& std::is_destructible_v<T>;

	namespace detail
	{
		template <typename Base, typename... Watchers>
		class BasicWatched
			: public Base,
			public Watchers...
		{
		public:
			~BasicWatched() noexcept(std::is_nothrow_destructible_v<Base>) // NOLINT(modernize-use-equals-default)
			{
			}

			using Base::Base;

			BasicWatched(const BasicWatched&) = default;
			BasicWatched& operator =(const BasicWatched&) = default;
			BasicWatched(BasicWatched&&) = default;
			BasicWatched& operator =(BasicWatched&&) = default;
		};

		template <satisfies<std::has_virtual_destructor> Base, typename... Watchers>
		class BasicWatched<Base, Watchers...>
			: public Base,
			public Watchers...
		{
		public:
			~BasicWatched() noexcept(std::is_nothrow_destructible_v<Base>) override // NOLINT(modernize-use-equals-default)
			{
			}

			using Base::Base;

			BasicWatched(const BasicWatched&) = default;
			BasicWatched& operator =(const BasicWatched&) = default;
			BasicWatched(BasicWatched&&) = default;
			BasicWatched& operator =(BasicWatched&&) = default;
		};
	}

	/**
	 * \brief CRTP-type, inheriting first from ``Base`` and then all ``Watchers``, thus effectively couple them all together.
	 * \tparam Base The main type.
	 * \tparam Watchers All utilized watcher types.
	 * \details
	 * ## Destructors
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
	template <typename Base, object_watcher... Watchers>
		requires std::same_as<Base, std::remove_cvref_t<Base>>
	class Watched
		: public detail::BasicWatched<Base, Watchers...>
	{
		using SuperT = detail::BasicWatched<Base, Watchers...>;

	public:
		~Watched() = default;

		using SuperT::SuperT;

		Watched(const Watched&) = default;
		Watched& operator =(const Watched&) = default;
		Watched(Watched&&) = default;
		Watched& operator =(Watched&&) = default;
	};

	/**
	 * \}
	 */
}

#endif
