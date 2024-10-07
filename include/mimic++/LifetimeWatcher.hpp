// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_LIFETIME_WATCHER_HPP
#define MIMICPP_LIFETIME_WATCHER_HPP

#pragma once

#include "mimic++/Mock.hpp"

#include <stdexcept>
#include <utility>

namespace mimicpp
{
	class LifetimeWatcher
	{
	public:
		~LifetimeWatcher() noexcept(false)
		{
			if (const auto destruction = std::exchange(
				m_DestructionMock,
				nullptr))
			{
				(*destruction)();
			}
		}

		[[nodiscard]]
		LifetimeWatcher() = default;

		[[nodiscard]]
		LifetimeWatcher([[maybe_unused]] const LifetimeWatcher& other)
			: LifetimeWatcher{}
		{
		}

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

		[[nodiscard]]
		LifetimeWatcher(LifetimeWatcher&&) = default;
		LifetimeWatcher& operator =(LifetimeWatcher&&) = default;

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
			~BasicWatched() = default;

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
			~BasicWatched() noexcept(std::is_nothrow_destructible_v<Base>) override  // NOLINT(modernize-use-equals-default)
			{
			}

			using Base::Base;

			BasicWatched(const BasicWatched&) = default;
			BasicWatched& operator =(const BasicWatched&) = default;
			BasicWatched(BasicWatched&&) = default;
			BasicWatched& operator =(BasicWatched&&) = default;
		};
	}

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
}

#endif
