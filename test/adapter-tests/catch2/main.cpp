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
	"Catch2Reporter reports matches as succeeded statements.",
	"[adapter][adapter::catch2]"
)
{
	g_SuccessCounter = 0;
	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(42);

	CHECK(g_SuccessCounter == 0);
	mock(42);
	REQUIRE(g_SuccessCounter == 2);	// the CHECK and SUCCEED
}

TEST_CASE(
	"Catch2Reporter reports failure, when no match can be found.",
	"[!shouldfail][adapter][adapter::catch2]"
)
{
	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(42);
	SCOPED_EXP mock.expect_call(-42);

	mock(1337);
}

TEST_CASE(
	"Catch2Reporter reports failure, when no applicable match can be found.",
	"[!shouldfail][adapter][adapter::catch2]"
)
{
	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(42);

	mock(42);
	mock(42);
}

namespace
{
	class TestException
	{
	};

	class ThrowOnMatches
	{
	public:
		[[maybe_unused]]
		static constexpr bool is_satisfied() noexcept
		{
			return true;
		}

		[[maybe_unused]]
		static constexpr bool matches([[maybe_unused]] const auto& info)
		{
			throw TestException{};
		}

		[[maybe_unused]]
		static constexpr std::nullopt_t describe() noexcept
		{
			return std::nullopt;
		}

		[[maybe_unused]]
		static constexpr void consume([[maybe_unused]] const auto& info) noexcept
		{
		}
	};
}

TEST_CASE(
	"Catch2Reporter::report_unhandled_exception just creates a warning.",
	"[adapter][adapter::catch2]"
)
{
	mimicpp::Mock<void()> mock{};

	SCOPED_EXP mock.expect_call();

	SCOPED_EXP mock.expect_call()
				| mimicpp::expect::times<0, 1>()
				| ThrowOnMatches{};

	REQUIRE_NOTHROW(mock());
}

TEST_CASE(
	"Catch2Reporter::report_unfulfilled_expectation cancels the test, when no other exception exists.",
	"[!shouldfail][adapter][adapter::catch2]"
)
{
	mimicpp::Mock<void()> mock{};

	SCOPED_EXP mock.expect_call();
}

TEST_CASE(
	"Catch2Reporter::report_unfulfilled_expectation does nothing, when already an uncaught exception exists.",
	"[adapter][adapter::catch2]"
)
{
	mimicpp::Mock<void()> mock{};

	const auto runTest = [&]
	{
		SCOPED_EXP mock.expect_call();
		throw 42;
	};

	REQUIRE_THROWS_AS(
		runTest(),
		int);
}

TEST_CASE(
	"Catch2Reporter::report_error cancels the test, when no other exception exists.",
	"[!shouldfail][adapter][adapter::catch2]"
)
{
	mimicpp::detail::report_error("Hello, World!");
}

TEST_CASE(
	"Catch2Reporter::report_error does nothing, when already an uncaught exception exists.",
	"[adapter][adapter::catch2]"
)
{
	struct helper
	{
		~helper()
		{
			mimicpp::detail::report_error("Hello, World!");
		}
	};

	const auto runTest = []
	{
		helper h{};
		throw 42;
	};

	REQUIRE_THROWS_AS(
		runTest(),
		int);
}
