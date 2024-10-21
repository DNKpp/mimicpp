// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "mimic++/adapters/Doctest.hpp"

namespace
{
	inline std::atomic_int g_SuccessCounter{0};
	inline std::atomic_int g_WarningCounter{0};

	class SuccessListener final
		: public doctest::IReporter
	{
	public:
		explicit SuccessListener([[maybe_unused]] const doctest::ContextOptions& in)
		{
		}

		void report_query([[maybe_unused]] const doctest::QueryData& in) override
		{
		}

		void test_run_start() override
		{
		}

		void test_run_end([[maybe_unused]] const doctest::TestRunStats& in) override
		{
		}

		void test_case_start([[maybe_unused]] const doctest::TestCaseData& in) override
		{
		}

		void test_case_reenter([[maybe_unused]] const doctest::TestCaseData& in) override
		{
		}

		void test_case_end([[maybe_unused]] const doctest::CurrentTestCaseStats& in) override
		{
		}

		void test_case_exception([[maybe_unused]] const doctest::TestCaseException& in) override
		{
		}

		void subcase_start([[maybe_unused]] const doctest::SubcaseSignature& in) override
		{
		}

		void subcase_end() override
		{
		}

		void log_assert([[maybe_unused]] const doctest::AssertData& in) override
		{
			if (!in.m_failed)
			{
				++g_SuccessCounter;
			}
		}

		void log_message([[maybe_unused]] const doctest::MessageData& in) override
		{
			if (in.m_string == "Warning")
			{
				++g_WarningCounter;
			}
		}

		void test_case_skipped([[maybe_unused]] const doctest::TestCaseData& in) override
		{
		}
	};
}

REGISTER_LISTENER("SuccessListener", 1, SuccessListener);

TEST_SUITE("adapter::doctest")
{
	TEST_CASE("doctest::send_success notifies Doctest for success.")
	{
		g_SuccessCounter = 0;

		mimicpp::detail::doctest::send_success("Success");

		REQUIRE(g_SuccessCounter == 1);
	}

	TEST_CASE("doctest::send_warning notifies Doctest.")
	{
		mimicpp::detail::doctest::send_warning("Warning");

		REQUIRE(g_WarningCounter == 1);
	}

	TEST_CASE(
		"doctest::send_fail notifies Doctest for failures and aborts."
		* doctest::should_fail{})
	{
		mimicpp::detail::doctest::send_fail("Fail");
	}
}
