// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ObjectWatcher.hpp"
#include "mimic++/InterfaceMock.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>

#include "TestReporter.hpp"
#include "TestTypes.hpp"

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
		SECTION("From an lvalue.")
		{
			auto expectation = std::invoke(
				[]() -> ScopedExpectation
				{
					LifetimeWatcher watcher{};
					return watcher.expect_destruct();
				});
		}

		SECTION("From an rvalue.")
		{
			ScopedExpectation expectation = LifetimeWatcher{}.expect_destruct();
		}

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

	SECTION("An exception is thrown, if a destruction expectation is set more than once.")
	{
		auto expectation = std::invoke(
			[]() -> ScopedExpectation
			{
				LifetimeWatcher watcher{};
				ScopedExpectation exp1 = watcher.expect_destruct();

				REQUIRE_THROWS_AS(
					watcher.expect_destruct(),
					std::logic_error);

				REQUIRE_THROWS_AS(
					watcher.expect_destruct(),
					std::logic_error);

				return exp1;
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

	SECTION("LifetimeWatcher can be moved.")
	{
		std::optional<LifetimeWatcher> source{std::in_place};

		SECTION("With an already active destruct-expectation")
		{
			ScopedExpectation firstExpectation = source->expect_destruct();

			SECTION("When move constructed.")
			{
				LifetimeWatcher target{*std::move(source)};
			}

			SECTION("When move assigned.")
			{
				auto innerExp = std::invoke(
					[&]
					{
						LifetimeWatcher target{};
						ScopedExpectation secondExpectation = target.expect_destruct();

						target = *std::move(source);

						// let's also swap the expectations, so the tracking becomes easier.
						using std::swap;
						swap(firstExpectation, secondExpectation);
						return secondExpectation;
					});
			}

			SECTION("When self-move assigned.")
			{
				START_WARNING_SUPPRESSION
				SUPPRESS_SELF_MOVE
				*source = *std::move(source);
				STOP_WARNING_SUPPRESSION

				// need to manually destroy the object, to prevent the expectation outliving the lifetime-watcher
				source.reset();
			}
		}

		SECTION("Without an active destruct-expectation.")
		{
			SECTION("When move constructed.")
			{
				auto expectation = std::invoke(
					[&]() -> ScopedExpectation
					{
						LifetimeWatcher target{*std::move(source)};
						return target.expect_destruct();
					});
			}

			SECTION("When move assigned.")
			{
				auto innerExp = std::invoke(
					[&]() -> ScopedExpectation
					{
						LifetimeWatcher target{};
						// note: The target must have an active expectation, as it's considered dead after the move happened.
						ScopedExpectation targetExp = target.expect_destruct();

						target = *std::move(source);

						return target.expect_destruct();
					});
			}
		}
	}

	SECTION("LifetimeWatcher can be copied.")
	{
		std::optional<LifetimeWatcher> source{std::in_place};

		SECTION("With an already active destruct-expectation")
		{
			MIMICPP_SCOPED_EXPECTATION source->expect_destruct();

			SECTION("When copy-constructing.")
			{
				auto expectation = std::invoke(
					[&]() -> ScopedExpectation
					{
						LifetimeWatcher target{*source};
						return target.expect_destruct();
					});

				// need to manually destroy the object, to prevent the expectation outliving the lifetime-watcher
				source.reset();
			}

			SECTION("When copy-assigning.")
			{
				auto expectation = std::invoke(
					[&]() -> ScopedExpectation
					{
						LifetimeWatcher target{};
						MIMICPP_SCOPED_EXPECTATION target.expect_destruct();

						target = *source;
						return target.expect_destruct();
					});

				// need to manually destroy the object, to prevent the expectation outliving the lifetime-watcher
				source.reset();
			}

			SECTION("When self-copy-assigning.")
			{
				source = *source;
				MIMICPP_SCOPED_EXPECTATION source->expect_destruct();

				// need to manually destroy the object, to prevent the expectation outliving the lifetime-watcher
				source.reset();
			}
		}

		SECTION("Without an active destruct-expectation")
		{
			SECTION("When copy-constructing.")
			{
				auto expectation = std::invoke(
					[&]() -> ScopedExpectation
					{
						LifetimeWatcher target{*source};
						return target.expect_destruct();
					});

				MIMICPP_SCOPED_EXPECTATION source->expect_destruct();
				// need to manually destroy the object, to prevent the expectation outliving the lifetime-watcher
				source.reset();
			}

			SECTION("When copy-assigning.")
			{
				auto expectation = std::invoke(
					[&]() -> ScopedExpectation
					{
						LifetimeWatcher target{};
						MIMICPP_SCOPED_EXPECTATION target.expect_destruct();

						target = *source;
						return target.expect_destruct();
					});

				MIMICPP_SCOPED_EXPECTATION source->expect_destruct();
				// need to manually destroy the object, to prevent the expectation outliving the lifetime-watcher
				source.reset();
			}
		}
	}
}

TEST_CASE(
	"LifetimeWatcher supports finally::throws policy.",
	"[lifetime-watcher]"
)
{
	struct my_exception
	{
	};

	const auto action = []
	{
		LifetimeWatcher watcher{};
		MIMICPP_SCOPED_EXPECTATION watcher.expect_destruct()
									and finally::throws(my_exception{});

		// it's very important, making sure, that the expectation outlives the LifetimeWatcher
		LifetimeWatcher other{std::move(watcher)};
	};

	REQUIRE_THROWS_AS(
		action(),
		my_exception);
}

TEST_CASE(
	"Watched can wrap the actual type to be watched with the utilized watcher types.",
	"[lifetime-watcher]")
{
	STATIC_REQUIRE(std::is_nothrow_destructible_v<Watched<Mock<void(int)>, LifetimeWatcher>>);

	SECTION("Detects violations.")
	{
		ScopedReporter reporter{};

		struct not_nothrow_destructible
		{
			~not_nothrow_destructible() noexcept(false)
			{
			}
		};

		using WatcherT = Watched<not_nothrow_destructible, LifetimeWatcher>;
		STATIC_REQUIRE(!std::is_nothrow_destructible_v<WatcherT>);

		REQUIRE_THROWS_AS(
			WatcherT{},
			NoMatchError);
	}

	SECTION("Just plain usage.")
	{
		Watched<
			Mock<void(int)>,
			LifetimeWatcher> watched{};

		MIMICPP_SCOPED_EXPECTATION watched.expect_destruct();
		MIMICPP_SCOPED_EXPECTATION watched.expect_call(42);

		watched(42);

		// extend lifetime, to outlive all expectations
		auto temp{std::move(watched)};
	}

	SECTION("With explicit sequence.")
	{
		Watched<
			Mock<void(int)>,
			LifetimeWatcher> watched{};

		SequenceT sequence{};
		{
			Watched<
				Mock<void()>,
				LifetimeWatcher> other{};

			MIMICPP_SCOPED_EXPECTATION other.expect_destruct()
										and expect::in_sequence(sequence);

			// extend lifetime, to outlive its expectations
			auto temp{std::move(other)};
		}

		MIMICPP_SCOPED_EXPECTATION watched.expect_call(42)
									and expect::in_sequence(sequence);
		MIMICPP_SCOPED_EXPECTATION watched.expect_destruct()
									and expect::in_sequence(sequence);

		watched(42);

		// extend lifetime, to outlive its expectations
		auto temp{std::move(watched)};
	}
}

TEST_CASE(
	"Watched can be used on interface-mocks.",
	"[lifetime-watcher]")
{
	class Interface
	{
	public:
		virtual ~Interface() = default;
		virtual void foo() = 0;
	};

	class Derived
		: public Interface
	{
	public:
		MIMICPP_MOCK_METHOD(foo, void, ());
	};

	STATIC_REQUIRE(std::is_nothrow_destructible_v<Watched<Derived, LifetimeWatcher>>);

	auto watched = std::make_unique<Watched<Derived, LifetimeWatcher>>();

	MIMICPP_SCOPED_EXPECTATION watched->expect_destruct();
	MIMICPP_SCOPED_EXPECTATION watched->foo_.expect_call();

	watched->foo();

	// extend lifetime, to outlive its expectations
	const std::unique_ptr<Interface> temp{std::move(watched)};
}

TEST_CASE(
	"Violations of watched interface-implementations will be detected.",
	"[lifetime-watcher]")
{
	class Interface
	{
	public:
		// must not be noexcept, due to the installed reporter
		virtual ~Interface() noexcept(false)
		{
		}
	};

	class Derived
		: public Interface
	{
	};

	STATIC_REQUIRE(!std::is_nothrow_destructible_v<Watched<Derived, LifetimeWatcher>>);

	ScopedReporter reporter{};

	REQUIRE_THROWS_AS(
		(Watched<Derived, LifetimeWatcher>{}),
		NoMatchError);
}
