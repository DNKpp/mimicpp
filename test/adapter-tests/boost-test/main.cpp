// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"

#define BOOST_TEST_MODULE BoostAdapterTest
#include "mimic++/adapters/BoostTest.hpp"

BOOST_AUTO_TEST_SUITE(SuccessSuite)

	BOOST_AUTO_TEST_CASE(ReportSuccess)
	{
		mimicpp::detail::boost_test::send_success("Report Success");
	}

	BOOST_AUTO_TEST_CASE(ReportWarning)
	{
		mimicpp::detail::boost_test::send_warning("Report Warning");
	}

BOOST_AUTO_TEST_SUITE_END()

// This tests will fail. ctest has appropriate properties set, thus should be reported as success
BOOST_AUTO_TEST_SUITE(FailureSuite)

	BOOST_AUTO_TEST_CASE(ReportFail)
	{
		mimicpp::detail::boost_test::send_fail("Report fail");
	}

BOOST_AUTO_TEST_SUITE_END()
