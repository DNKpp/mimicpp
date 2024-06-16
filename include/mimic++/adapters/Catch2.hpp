// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_ADAPTERS_CATCH2_HPP
#define MIMICPP_ADAPTERS_CATCH2_HPP

#pragma once

#include "mimic++/Reporter.hpp"

#include <iterator>

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
	#error "Unable to find catch2 includes."
#endif

namespace mimicpp::detail::catch2
{
	[[noreturn]]
	inline void send_fail(const StringViewT msg)
	{
#ifdef CATCH_CONFIG_PREFIX_ALL
		CATCH_FAIL(msg);
#else
		FAIL(msg);
#endif

		unreachable();
	}

	inline void send_success(const StringViewT msg)
	{
#ifdef CATCH_CONFIG_PREFIX_ALL
		CATCH_SUCCEED(msg);
#else
		SUCCEED(msg);
#endif
	}

	inline void send_warning(const StringViewT msg)
	{
#ifdef CATCH_CONFIG_PREFIX_MESSAGES
		CATCH_WARN(msg);
#else
		WARN(msg);
#endif
	}
}

namespace mimicpp
{
	/**
	 * \brief Reporter for the integration into Catch2.
	 * \ingroup REPORTING_ADAPTERS
	 * \details This reporter enables the integration of ``mimic++`` into ``Catch2`` and prefixes the headers
	 * of ``Catch2`` with ``catch2/``.
	 *
	 * This reporter installs itself by simply including this header file into any source file of the test executable.
	 */
	using Catch2ReporterT = BasicReporter<
		&detail::catch2::send_success,
		&detail::catch2::send_warning,
		&detail::catch2::send_fail
	>;
}

namespace mimicpp::detail::catch2
{
	inline const ReporterInstaller<Catch2ReporterT> installer{};
}

#endif
