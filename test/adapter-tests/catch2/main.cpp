// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch_test_macros.hpp"
#include "mimic++/Mock.hpp"
#include "mimic++/adapters/Catch2.hpp"

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <atomic>

namespace
{
	inline std::atomic_int g_SuccessCounter{0};

	class SuccessListener final
		: public Catch::EventListenerBase
	{
		using SuperT = EventListenerBase;

	public:
		[[nodiscard]]
		explicit SuccessListener(const Catch::IConfig* config)
			: SuperT{config}
		{
			m_preferences.shouldReportAllAssertions = true;
		}

		void assertionEnded(const Catch::AssertionStats& assertionStats) override
		{
			if (assertionStats.assertionResult.succeeded())
			{
				++g_SuccessCounter;
			}
		}
	};
}

CATCH_REGISTER_LISTENER(SuccessListener)

TEST_CASE(
	"catch2::send_success notifies Catch2 for success.",
	"[adapter][adapter::catch2]"
)
{
	g_SuccessCounter = 0;

	mimicpp::detail::catch2::send_success("Test");

	REQUIRE(g_SuccessCounter == 1);
}

TEST_CASE(
	"catch2::send_warning notifies Catch2.",
	"[adapter][adapter::catch2]"
)
{
	mimicpp::detail::catch2::send_warning("Test");

	// not testable
}

TEST_CASE(
	"catch2::send_fail notifies Catch2 for failures and aborts.",
	"[!shouldfail][adapter][adapter::catch2]"
)
{
	mimicpp::detail::catch2::send_fail("Test");
}
