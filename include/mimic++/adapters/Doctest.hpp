// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_ADAPTERS_DOCTEST_HPP
#define MIMICPP_ADAPTERS_DOCTEST_HPP

#pragma once

#include "mimic++/Reporter.hpp"

#if __has_include(<doctest/doctest.h>)
#include <doctest/doctest.h>
#else
	#error "Unable to find Doctest includes."
#endif

namespace mimicpp::detail::doctest
{
	using namespace ::doctest;

	[[noreturn]]
	inline void send_fail([[maybe_unused]] const StringViewT msg)
	{
		DOCTEST_FAIL(msg);
		unreachable();
	}

	inline void send_success(const StringViewT msg)
	{
		DOCTEST_REQUIRE_MESSAGE(true, msg);
	}

	inline void send_warning(const StringViewT msg)
	{
		DOCTEST_MESSAGE(msg);
	}
}

namespace mimicpp
{
	/**
	 * \brief Reporter for the integration into Doctest.
	 * \ingroup REPORTING_ADAPTERS
	 * \details This reporter enables the integration of ``mimic++`` into ``Doctest`` and prefixes the headers
	 * of ``Doctest`` with ``doctest/``.
	 *
	 * This reporter installs itself by simply including this header file into any source file of the test executable.
	 */
	using DoctestReporterT = BasicReporter<
		&detail::doctest::send_success,
		&detail::doctest::send_warning,
		&detail::doctest::send_fail
	>;
}

namespace mimicpp::detail::doctest
{
	inline const ReporterInstaller<DoctestReporterT> installer{};
}

#endif
