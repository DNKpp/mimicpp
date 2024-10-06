// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/LifetimeWatcher.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>

#include "TestReporter.hpp"

using namespace mimicpp;

TEST_CASE(
	"LifetimeWatcher tracks destruction",
	"[lifetime-watcher]"
)
{
	namespace Matches = Catch::Matchers;

	ScopedReporter reporter{};

	SECTION("Reports a no-match-error, when destruction occurs without an expectation.")
	{
		const auto action = []() { LifetimeWatcher watcher{}; };

		REQUIRE_THROWS_AS(
			action(),
			NoMatchError);

		REQUIRE_THAT(
			reporter.full_match_reports(),
			Matches::IsEmpty());
		REQUIRE_THAT(
			reporter.inapplicable_match_reports(),
			Matches::IsEmpty());
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Matches::IsEmpty());
		REQUIRE_THAT(
			reporter.unfulfilled_expectations(),
			Matches::IsEmpty());
	}

	SECTION("Reports an unfulfilled expectation, if the expectation expires before destruction occurs.")
	{
		auto watcher = std::make_unique<LifetimeWatcher>();
		std::optional<ScopedExpectation> expectation = watcher->expect_destruct();
		expectation.reset();

		REQUIRE_THAT(
			reporter.full_match_reports(),
			Matches::IsEmpty());
		REQUIRE_THAT(
			reporter.inapplicable_match_reports(),
			Matches::IsEmpty());
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Matches::IsEmpty());
		REQUIRE_THAT(
			reporter.unfulfilled_expectations(),
			Matches::SizeIs(1));

		// we need to safely tear-down the watcher
		CHECK_THROWS_AS(
			std::invoke([&]{ delete watcher.release(); }),
			NoMatchError);
	}

	SECTION("Reports a full-match, if destruction occurs with an active expectation.")
	{
		auto expectation = std::invoke(
			[]() -> ScopedExpectation
			{
				LifetimeWatcher watcher{};
				return watcher.expect_destruct();
			});

		REQUIRE_THAT(
			reporter.full_match_reports(),
			Matches::SizeIs(1));
		REQUIRE_THAT(
			reporter.inapplicable_match_reports(),
			Matches::IsEmpty());
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Matches::IsEmpty());
		REQUIRE_THAT(
			reporter.unfulfilled_expectations(),
			Matches::IsEmpty());
	}
}
