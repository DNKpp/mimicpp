// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_LIFETIME_WATCHER_HPP
#define MIMICPP_LIFETIME_WATCHER_HPP

#pragma once

#include "mimic++/Mock.hpp"

namespace mimicpp
{
	class LifetimeWatcher
	{
	public:
		~LifetimeWatcher() noexcept(false)
		{
			m_DestructionMock();
		}

		[[nodiscard]]
		auto expect_destruct()
		{
			return m_DestructionMock.expect_call()
					and expect::once();	// prevent further times specifications
		}

	private:
		Mock<void()> m_DestructionMock{};
	};
}

#endif
