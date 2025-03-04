//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef MIMICPP_ADAPTERS_GTEST_HPP
#define MIMICPP_ADAPTERS_GTEST_HPP

#pragma once

#if __has_include("mimic++/Reporting.hpp")
    #include "mimic++/Reporting.hpp"
#elif not defined(MIMICPP_VERSION)
    #error "It appears that the test-adapter is not included in the mimic++ project or package." \
        "If you plan to use it alongside the mimic++-amalgamated header, please ensure to include the adapter-header afterwards."
#endif

#if __has_include(<gtest/gtest.h>)
    #include <gtest/gtest.h>
#else
    #error "Unable to find gtest includes."
#endif

namespace mimicpp::reporting::detail::gtest
{
    struct failure
    {
    };

    [[noreturn]]
    inline void send_fail(StringViewT const msg)
    {
        // GTEST_FAIL has an immediate return
        std::invoke(
            [&] {
                GTEST_FAIL() << msg;
            });

        throw failure{};
    }

    inline void send_success(StringViewT const msg)
    {
        GTEST_SUCCEED() << msg;
    }

    inline void send_warning([[maybe_unused]] StringViewT const msg)
    {
        // seems unsupported
    }
}

namespace mimicpp::reporting
{
    // GCOVR_EXCL_START

    /**
     * \brief Reporter for the integration into gtest.
     * \ingroup REPORTING_ADAPTERS
     * \details This reporter enables the integration of ``mimic++`` into ``gtest`` and prefixes the headers
     * of ``gtest`` with ``gtest/``.
     *
     * This reporter installs itself by simply including this header file into any source file of the test executable.
     */
    using GTestReporterT = BasicReporter<
        &detail::gtest::send_success,
        &detail::gtest::send_warning,
        &detail::gtest::send_fail>;

    // GCOVR_EXCL_STOP
}

namespace mimicpp::reporting::detail::gtest
{
    [[maybe_unused]]
    inline ReporterInstaller<GTestReporterT> const installer{};
}

template <typename Matcher>
	requires requires
	{
		typename Matcher::is_gtest_matcher;
		requires std::is_void_v<typename Matcher::is_gtest_matcher>;
	}
struct mimicpp::custom::matcher_traits<Matcher>
{
	template <typename T>
	[[nodiscard]]
	static constexpr bool matches(const Matcher& matcher, const T& value)
		requires requires{ { matcher.MatchAndExplain(value, nullptr) } -> std::convertible_to<bool>; }
	{
		return matcher.MatchAndExplain(value, nullptr);
	}

	[[nodiscard]]
	static constexpr StringViewT describe(const Matcher& matcher)
		requires requires{ { matcher.Desc() } -> std::convertible_to<StringViewT>; }
	{
		return matcher.Desc();
	}
};


template <typename T>
struct mimicpp::custom::matcher_traits<::testing::PolymorphicMatcher<T>>
{
	using MatcherT = ::testing::PolymorphicMatcher<T>;

	template <typename Value>
	[[nodiscard]]
	static constexpr bool matches(const MatcherT& matcher, const Value& value)
	{
		return matcher
				.impl()
				.MatchAndExplain(value, nullptr);
	}

	[[nodiscard]]
	static constexpr StringT describe(const MatcherT& matcher)
	{
		StringStreamT out{};
		matcher
			.impl()
			.DescribeTo(&out);
		return std::move(out).str();
	}
};

#endif
