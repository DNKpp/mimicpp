// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "../test/unit-tests/SuppressionMacros.hpp"	// needs to disable some warnings on gcc
#include "mimic++/ObjectWatcher.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE(
	"Watched<T, LifetimeWatcher> reports violations during destruction.",
	"[example][example::watched][example::watched::life-time]"
)
{
	//! [watched lifetime-watcher violation]
	constexpr auto action = []
	{
		struct not_nothrow_destructible
		{
			// explicitly make the destructor throwable, this isn't necessarily required in real tests
			~not_nothrow_destructible() noexcept(false)
			{
			}
		};

		const mimicpp::Watched<
			not_nothrow_destructible,
			mimicpp::LifetimeWatcher> watched{};	// let's create a watched instance

		// We purposely forget to define a destruction expectation.
		// Due to this, a no-match will be reported during destruction.
	};

	// Depending on the active reporter, this may either raise an exception or terminate the program.
	// The default reporter simply throws an exception.
	REQUIRE_THROWS(action());
	//! [watched lifetime-watcher violation]
}

TEST_CASE(
	"Watched<T, LifetimeWatcher> reports violations during copy-construction.",
	"[example][example::watched][example::watched::life-time]"
)
{
	//! [watched lifetime-watcher copy-construction violation]
	constexpr auto action = []
	{
		struct my_copyable	// Mock isn't a copyable type, thus use a custom one for this example
		{
			// explicitly make the destructor throwable, this isn't necessarily required in real tests
			~my_copyable() noexcept(false)
			{
			}
		};

		mimicpp::Watched<
			my_copyable,
			mimicpp::LifetimeWatcher> source{};

		// setting up a destruction-expectation is mandatory to prevent violation reports during scope leave
		SCOPED_EXP source.expect_destruct();
		// This is just a little trick for this small example. In real code,
		// one would usually pass the ownership of the watched object somewhere else,
		// but in this example we fake that by simply moving it below our expectation.
		auto moved = std::move(source); // note: source is now a "moved-from"-object, which doesn't report any violations

		mimicpp::Watched other{source};			// now create a "copy" of source

		// other is a new instance without an existing destruction-expectation,
		// because the copy-constructor doesn't semantically copy anything.
		// As other goes out of scope without a destruction-expectation, a no-match is reported.
	};

	// Depending on the active reporter, this may either raise an exception or terminate the program.
	// The default reporter simply throws an exception.
	REQUIRE_THROWS(action());
	//! [watched lifetime-watcher copy-construction violation]
}

TEST_CASE(
	"Watched<T, LifetimeWatcher> reports violations during copy-assignment.",
	"[example][example::watched][example::watched::life-time]"
)
{
	SECTION("Copy-assignment to an watched object, which is not yet ready.")
	{
		//! [watched lifetime-watcher copy-assignment violation]
		constexpr auto action = []
		{
			struct my_copyable	// Mock isn't a copyable type, thus use a custom one for this example
			{
				// explicitly make the destructor throwable, this isn't necessarily required in real tests
				~my_copyable() noexcept(false)
				{
				}
			};

			mimicpp::Watched<
				my_copyable,
				mimicpp::LifetimeWatcher> source{};

			// setting up a destruction-expectation is mandatory to prevent violation reports during scope leave
			SCOPED_EXP source.expect_destruct();
			// This is just a little trick for this small example. In real code,
			// one would usually pass the ownership of the watched object somewhere else,
			// but in this example we fake that by simply moving it below our expectation.
			auto moved = std::move(source); // note: source is now a "moved-from"-object, which doesn't report any violations

			mimicpp::Watched<
				my_copyable,
				mimicpp::LifetimeWatcher> other{};	// now create another watched object

			// other will be destructed, but there is no existing destruction-expectation
			other = source;	// a no-match will be reported immediately
		};

		// Depending on the active reporter, this may either raise an exception or terminate the program.
		// The default reporter simply throws an exception.
		REQUIRE_THROWS(action());
		//! [watched lifetime-watcher copy-assignment violation]
	}

	SECTION("Copy-assignment without setting up an destruction-expectation.")
	{
		//! [watched lifetime-watcher copy-assignment violation2]
		constexpr auto action = []
		{
			struct my_copyable	// Mock isn't a copyable type, thus use a custom one for this example
			{
				// explicitly make the destructor throwable, this isn't necessarily required in real tests
				~my_copyable() noexcept(false)
				{
				}
			};

			mimicpp::Watched<
				my_copyable,
				mimicpp::LifetimeWatcher> source{};

			// setting up a destruction-expectation is mandatory to prevent violation reports during scope leave
			SCOPED_EXP source.expect_destruct();
			// This is just a little trick for this small example. In real code,
			// one would usually pass the ownership of the watched object somewhere else,
			// but in this example we fake that by simply moving it below our expectation.
			auto moved = std::move(source); // note: source is now a "moved-from"-object, which doesn't report any violations

			mimicpp::Watched<
				my_copyable,
				mimicpp::LifetimeWatcher> other{};	// now create another watched object
			SCOPED_EXP other.expect_destruct();		// setting it up accordingly
			other = source;							// other will be destructed and the expectation from the line above fulfilled

			// other is now a new instance without an existing destruction-expectation,
			// because the copy-operator doesn't semantically copy anything.
			// As other goes out of scope without a destruction-expectation, a no-match is reported.
		};

		// Depending on the active reporter, this may either raise an exception or terminate the program.
		// The default reporter simply throws an exception.
		REQUIRE_THROWS(action());
		//! [watched lifetime-watcher copy-assignment violation2]
	}
}

TEST_CASE(
	"Watched<T, LifetimeWatcher> is satisfied, if destruction actually happens.",
	"[example][example::watched][example::watched::life-time]"
)
{
	//! [watched lifetime-watcher]
	namespace expect = mimicpp::expect;

	// imagine this to be a function, we wanted to test
	constexpr auto some_function = [](auto fun)
	{
		fun(); // let's just invoke the given fun.
	};

	mimicpp::Watched<
		mimicpp::Mock<void()>,
		mimicpp::LifetimeWatcher> watched{};	// let's create a watched mock

	// Let's say, we are very suspicious and want to get sure, that ``some_function``
	// invokes the provided functional, before its getting destroyed.
	mimicpp::SequenceT sequence{};
	SCOPED_EXP watched.expect_call()
				and expect::in_sequence(sequence);
	SCOPED_EXP watched.expect_destruct()
				and expect::in_sequence(sequence);

	// pass the mock to ``some_function`` and track from the outside, whether the expectations hold
	some_function(std::move(watched));

	// nothing to do here. Violations will be reported automatically (as usual).
	//! [watched lifetime-watcher]
}

// gcc constantly complaints about the optionals as "maybe-uninitialized"
START_WARNING_SUPPRESSION
SUPPRESS_MAYBE_UNINITIALIZED

TEST_CASE(
	"LifetimeWatcher and RelocationWatcher can trace object instances.",
	"[example][example::watched]"
)
{
	//! [watched lifetime relocation]
	namespace expect = mimicpp::expect;
	namespace then = mimicpp::then;

	mimicpp::Watched<
		mimicpp::Mock<void()>,
		mimicpp::LifetimeWatcher,
		mimicpp::RelocationWatcher> watched{};

	SCOPED_EXP watched.expect_destruct();
	int relocationCounter{};
	SCOPED_EXP watched.expect_relocate()
				and then::invoke([&] { ++relocationCounter; })
				and expect::at_least(1);

	std::optional wrapped{std::move(watched)};	// satisfies one relocate-expectation
	std::optional other{std::move(wrapped)};		// satisfies a second relocate-expectation
	wrapped.reset();								// won't require a destruct-expectation, as moved-from objects are considered dead
	other.reset();									// fulfills the destruct-expectation
	REQUIRE(2 == relocationCounter);				// let's see, how often the instance has been relocated
	//! [watched lifetime relocation]
}
STOP_WARNING_SUPPRESSION
