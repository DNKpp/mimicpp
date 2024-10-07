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

		LifetimeWatcher([[maybe_unused]] const LifetimeWatcher& other)
			: LifetimeWatcher{}
		{
		}

		LifetimeWatcher& operator =([[maybe_unused]] const LifetimeWatcher& other)
		{
			std::exchange(*this, LifetimeWatcher{});

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

	template <typename Base, object_watcher... Watchers>
		requires std::same_as<Base, std::remove_cvref_t<Base>>
				&& (... && std::same_as<Watchers, std::remove_cvref_t<Watchers>>)
	class Watched
		: public Base,
		public Watchers...
	{
	public:
		~Watched() noexcept(std::is_nothrow_destructible_v<Base>)
		{
		}

		using Base::Base;

		Watched(const Watched&) = default;
		Watched& operator =(const Watched&) = default;
		Watched(Watched&&) = default;
		Watched& operator =(Watched&&) = default;
	};
}

#endif
