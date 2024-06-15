// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/adapters/gtest.hpp"

#include "gtest/gtest-spi.h"

namespace
{
	inline int g_SuccessCounter{0};

	class SuccessListener final
		: public testing::EmptyTestEventListener
	{
	public:
		void OnTestPartResult(const testing::TestPartResult& result) override
		{
			if (result.passed())
			{
				++g_SuccessCounter;
			}
		}
	};

	template <typename Listener>
	class ScopedListener
	{
	public:
		~ScopedListener() noexcept
		{
			delete testing::UnitTest::GetInstance()
					->listeners()
					.Release(m_Ptr);
		}

		[[nodiscard]]
		ScopedListener()
			: m_Ptr{new Listener{}}
		{
			testing::UnitTest::GetInstance()
				->listeners()
				.Append(m_Ptr);
		}

	private:
		Listener* m_Ptr{};
	};
}

TEST(
	GTestReporter,
	MatchReport
)
{
	const ScopedListener<SuccessListener> listener{};

	g_SuccessCounter = 0;
	mimicpp::Mock<void(int)> mock{};

	SCOPED_EXP mock.expect_call(42);

	ASSERT_EQ(g_SuccessCounter, 0);
	ASSERT_EQ(g_SuccessCounter, 0);	// check, that ASSERT_EQ isn't reported
	mock(42);
	EXPECT_EQ(g_SuccessCounter, 1);
}

TEST(
	GTestReporter,
	NoMatchReport
)
{
	EXPECT_FATAL_FAILURE(
		mimicpp::Mock<void(int)> mock{};
		EXPECT_ANY_THROW(mock(1337)),
		"No match for");
}

namespace
{
	class ScopedDisableSuccessReporting
	{
	public:
		~ScopedDisableSuccessReporting() noexcept
		{
			mimicpp::detail::gtest::g_EnableSuccessReporting = true;
		}

		[[nodiscard]]
		ScopedDisableSuccessReporting() noexcept
		{
			mimicpp::detail::gtest::g_EnableSuccessReporting = false;
		}
	};
}

TEST(
	GTestReporter,
	InapplicableMatchReport
)
{
	const ScopedDisableSuccessReporting disableSuccessReporting{};

	EXPECT_FATAL_FAILURE(
		mimicpp::Mock<void(int)> mock{};

		SCOPED_EXP mock.expect_call(42);

		mock(42);
		EXPECT_ANY_THROW(mock(42)),
		"No applicable match for");
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
		static bool matches([[maybe_unused]] const auto& info)
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

TEST(
	GTestReporter,
	UnhandledExceptionReport
)
{
	mimicpp::Mock<void()> mock{};

	SCOPED_EXP mock.expect_call();

	SCOPED_EXP mock.expect_call()
				| mimicpp::expect::times<0, 1>()
				| ThrowOnMatches{};

	EXPECT_NO_THROW(mock());
}

TEST(
	GTestReporter,
	UnfulfilledExpectationReport
)
{
	EXPECT_ANY_THROW(
		EXPECT_FATAL_FAILURE(
			mimicpp::Mock<void()> mock{};
			SCOPED_EXP mock.expect_call();,
			"Unfulfilled expectation"));
}

TEST(
	GTestReporter,
	UnfulfilledExpectationNoReport
)
{
	mimicpp::Mock<void()> mock{};

	const auto runTest = [&]
	{
		SCOPED_EXP mock.expect_call();
		throw 42;
	};

	EXPECT_THROW(
		runTest(),
		int);
}

TEST(
	GTestReporter,
	GenericErrorReport
)
{
	EXPECT_FATAL_FAILURE(
		EXPECT_ANY_THROW(mimicpp::detail::report_error("Hello, World!")),
		"Hello, World!");
}

TEST(
	GTestReporter,
	GenericErrorNoReport
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

	EXPECT_THROW(
		runTest(),
		int);
}
