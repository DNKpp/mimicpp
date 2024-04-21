// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>

using namespace mimicpp;

TEMPLATE_TEST_CASE(
	"Mocks are non-copyable but movable and can be default constructed.",
	"[mock]",
	void(),
	void(int),
	void(int, double),
	int(),
	int(float),
	int(float, double)
)
{
	using MockT = Mock<TestType>;

	STATIC_REQUIRE(!std::is_copy_constructible_v<MockT>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<MockT>);

	STATIC_REQUIRE(std::is_move_constructible_v<MockT>);
	STATIC_REQUIRE(std::is_move_assignable_v<MockT>);
	STATIC_REQUIRE(std::is_default_constructible_v<MockT>);
}

TEMPLATE_TEST_CASE_SIG(
	"Mocks satisfy std::invocable concept and std::is_invocable type-trait.",
	"[mock]",
	((bool dummy, typename Return, typename... Params), dummy, Return, Params...),
	(true, void),
	(true, void, int),
	(true, void, int, double),
	(true, int),
	(true, int, float),
	(true, int, float, double)
)
{
	using MockT = Mock<Return(Params...)>;

	STATIC_REQUIRE(std::invocable<const MockT&, Params...>);
	STATIC_REQUIRE(std::is_invocable_r_v<Return, const MockT&, Params...>);

	STATIC_REQUIRE(std::invocable<MockT&, Params...>);
	STATIC_REQUIRE(std::is_invocable_r_v<Return, MockT&, Params...>);

	STATIC_REQUIRE(std::invocable<const MockT&&, Params...>);
	STATIC_REQUIRE(std::is_invocable_r_v<Return, const MockT&&, Params...>);

	STATIC_REQUIRE(std::invocable<MockT&&, Params...>);
	STATIC_REQUIRE(std::is_invocable_r_v<Return, MockT&&, Params...>);
}

TEST_CASE("Mock<void()>::expect_call expectes a call with the same category and constness.", "[mock]")
{
	using ExpectationT = ScopedExpectation<void()>;
	ScopedReporter reporter{};
	Mock<void()> mock{};

	SECTION("Expect const lvalue call.")
	{
		SECTION("As const lvalue succeeds.")
		{
			{
				ExpectationT expectation = std::as_const(mock).expect_call();
				REQUIRE_NOTHROW(std::as_const(mock)());
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::SizeIs(1));
		}

		SECTION("Any other fails.")
		{
			{
				ExpectationT expectation = std::as_const(mock).expect_call();

				SECTION("As lvalue.")
				{
					REQUIRE_THROWS_AS(
						mock(),
						TestExpectationError);
				}

				SECTION("As const rvalue.")
				{
					REQUIRE_THROWS_AS(
						std::move(std::as_const(mock))(),
						TestExpectationError);
				}

				SECTION("As rvalue.")
				{
					REQUIRE_THROWS_AS(
						std::move(mock)(),
						TestExpectationError);
				}

				REQUIRE_THAT(
					reporter.no_match_reports(),
					Catch::Matchers::SizeIs(1));
				REQUIRE_THAT(
					reporter.exhausted_match_reports(),
					Catch::Matchers::IsEmpty());
				REQUIRE_THAT(
					reporter.ok_match_reports(),
					Catch::Matchers::IsEmpty());
				REQUIRE_THAT(
					reporter.unsatisfied_expectations(),
					Catch::Matchers::IsEmpty());
			}

			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::SizeIs(1));
		}
	}

	SECTION("Expect lvalue call.")
	{
		SECTION("As lvalue succeeds.")
		{
			{
				ExpectationT expectation = mock.expect_call();
				REQUIRE_NOTHROW(mock());
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::SizeIs(1));
		}

		SECTION("Any other fails.")
		{
			{
				ExpectationT expectation = mock.expect_call();

				SECTION("As const lvalue.")
				{
					REQUIRE_THROWS_AS(
						std::as_const(mock)(),
						TestExpectationError);
				}

				SECTION("As const rvalue.")
				{
					REQUIRE_THROWS_AS(
						std::move(std::as_const(mock))(),
						TestExpectationError);
				}

				SECTION("As rvalue.")
				{
					REQUIRE_THROWS_AS(
						std::move(mock)(),
						TestExpectationError);
				}

				REQUIRE_THAT(
					reporter.no_match_reports(),
					Catch::Matchers::SizeIs(1));
				REQUIRE_THAT(
					reporter.exhausted_match_reports(),
					Catch::Matchers::IsEmpty());
				REQUIRE_THAT(
					reporter.ok_match_reports(),
					Catch::Matchers::IsEmpty());
				REQUIRE_THAT(
					reporter.unsatisfied_expectations(),
					Catch::Matchers::IsEmpty());
			}

			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::SizeIs(1));
		}
	}

	SECTION("Expect const rvalue call.")
	{
		SECTION("As const rvalue succeeds.")
		{
			{
				ExpectationT expectation = std::move(std::as_const(mock)).expect_call();
				REQUIRE_NOTHROW(std::move(std::as_const(mock))());
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::SizeIs(1));
		}

		SECTION("Any other fails.")
		{
			{
				ExpectationT expectation = std::move(std::as_const(mock)).expect_call();

				SECTION("As const lvalue.")
				{
					REQUIRE_THROWS_AS(
						std::as_const(mock)(),
						TestExpectationError);
				}

				SECTION("As lvalue.")
				{
					REQUIRE_THROWS_AS(
						mock(),
						TestExpectationError);
				}

				SECTION("As rvalue.")
				{
					REQUIRE_THROWS_AS(
						std::move(mock)(),
						TestExpectationError);
				}

				REQUIRE_THAT(
					reporter.no_match_reports(),
					Catch::Matchers::SizeIs(1));
				REQUIRE_THAT(
					reporter.exhausted_match_reports(),
					Catch::Matchers::IsEmpty());
				REQUIRE_THAT(
					reporter.ok_match_reports(),
					Catch::Matchers::IsEmpty());
				REQUIRE_THAT(
					reporter.unsatisfied_expectations(),
					Catch::Matchers::IsEmpty());
			}

			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::SizeIs(1));
		}
	}

	SECTION("Expect rvalue call.")
	{
		SECTION("As rvalue succeeds.")
		{
			{
				ExpectationT expectation = std::move(mock).expect_call();
				REQUIRE_NOTHROW(std::move(mock)());
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::SizeIs(1));
		}

		SECTION("Any other fails.")
		{
			{
				ExpectationT expectation = std::move(mock).expect_call();

				SECTION("As const lvalue.")
				{
					REQUIRE_THROWS_AS(
						std::as_const(mock)(),
						TestExpectationError);
				}

				SECTION("As lvalue.")
				{
					REQUIRE_THROWS_AS(
						mock(),
						TestExpectationError);
				}

				SECTION("As const rvalue.")
				{
					REQUIRE_THROWS_AS(
						std::move(std::as_const(mock))(),
						TestExpectationError);
				}

				REQUIRE_THAT(
					reporter.no_match_reports(),
					Catch::Matchers::SizeIs(1));
				REQUIRE_THAT(
					reporter.exhausted_match_reports(),
					Catch::Matchers::IsEmpty());
				REQUIRE_THAT(
					reporter.ok_match_reports(),
					Catch::Matchers::IsEmpty());
				REQUIRE_THAT(
					reporter.unsatisfied_expectations(),
					Catch::Matchers::IsEmpty());
			}

			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::SizeIs(1));
		}
	}
}

TEST_CASE("Mock<void()>::expect_lvalue_call expectes a call with lvalue category.", "[mock]")
{
	using ExpectationT = ScopedExpectation<void()>;
	ScopedReporter reporter{};
	Mock<void()> mock{};

	SECTION("Succeeds as any lvalue.")
	{
		{
			ExpectationT expectation = std::as_const(mock).expect_lvalue_call();
			SECTION("As const lvalue.")
			{
				REQUIRE_NOTHROW(std::as_const(mock)());
			}

			SECTION("As lvalue.")
			{
				REQUIRE_NOTHROW(mock());
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::SizeIs(1));
		}

		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::IsEmpty());
	}

	SECTION("Fails as any rvalue.")
	{
		{
			ExpectationT expectation = std::as_const(mock).expect_lvalue_call();

			SECTION("As const rvalue.")
			{
				REQUIRE_THROWS_AS(
					std::move(std::as_const(mock))(),
					TestExpectationError);
			}

			SECTION("As rvalue.")
			{
				REQUIRE_THROWS_AS(
					std::move(mock)(),
					TestExpectationError);
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::SizeIs(1));
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::IsEmpty());
		}

		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::SizeIs(1));
	}
}

TEST_CASE("Mock<void()>::expect_rvalue_call expectes a call with rvalue category.", "[mock]")
{
	using ExpectationT = ScopedExpectation<void()>;
	ScopedReporter reporter{};
	Mock<void()> mock{};

	SECTION("Succeeds as any rvalue.")
	{
		{
			ExpectationT expectation = std::as_const(mock).expect_rvalue_call();
			SECTION("As const rvalue.")
			{
				REQUIRE_NOTHROW(std::move(std::as_const(mock))());
			}

			SECTION("As rvalue.")
			{
				REQUIRE_NOTHROW(std::move(mock)());
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::SizeIs(1));
		}

		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::IsEmpty());
	}

	SECTION("Fails as any lvalue.")
	{
		{
			ExpectationT expectation = std::as_const(mock).expect_rvalue_call();

			SECTION("As const lvalue.")
			{
				REQUIRE_THROWS_AS(
					std::as_const(mock)(),
					TestExpectationError);
			}

			SECTION("As lvalue.")
			{
				REQUIRE_THROWS_AS(
					mock(),
					TestExpectationError);
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::SizeIs(1));
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::IsEmpty());
		}

		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::SizeIs(1));
	}
}

TEST_CASE("Mock<void()>::expect_const_call expectes a const call.", "[mock]")
{
	using ExpectationT = ScopedExpectation<void()>;
	ScopedReporter reporter{};
	Mock<void()> mock{};

	SECTION("Succeeds as any const.")
	{
		{
			ExpectationT expectation = std::as_const(mock).expect_const_call();
			SECTION("As const lvalue.")
			{
				REQUIRE_NOTHROW(std::as_const(mock)());
			}

			SECTION("As const rvalue.")
			{
				REQUIRE_NOTHROW(std::move(std::as_const(mock))());
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::SizeIs(1));
		}

		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::IsEmpty());
	}

	SECTION("Fails as any mutable.")
	{
		{
			ExpectationT expectation = std::as_const(mock).expect_const_call();

			SECTION("As lvalue.")
			{
				REQUIRE_THROWS_AS(
					mock(),
					TestExpectationError);
			}

			SECTION("As rvalue.")
			{
				REQUIRE_THROWS_AS(
					std::move(mock)(),
					TestExpectationError);
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::SizeIs(1));
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::IsEmpty());
		}

		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::SizeIs(1));
	}
}

TEST_CASE("Mock<void()>::expect_mutable_call expectes a mutable call.", "[mock]")
{
	using ExpectationT = ScopedExpectation<void()>;
	ScopedReporter reporter{};
	Mock<void()> mock{};

	SECTION("Succeeds as any mutable.")
	{
		{
			ExpectationT expectation = std::as_const(mock).expect_mutable_call();
			SECTION("As lvalue.")
			{
				REQUIRE_NOTHROW(mock());
			}

			SECTION("As rvalue.")
			{
				REQUIRE_NOTHROW(std::move(mock)());
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::SizeIs(1));
		}

		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::IsEmpty());
	}

	SECTION("Fails as any const.")
	{
		{
			ExpectationT expectation = std::as_const(mock).expect_mutable_call();

			SECTION("As const lvalue.")
			{
				REQUIRE_THROWS_AS(
					std::as_const(mock)(),
					TestExpectationError);
			}

			SECTION("As const rvalue.")
			{
				REQUIRE_THROWS_AS(
					std::move(std::as_const(mock))(),
					TestExpectationError);
			}

			REQUIRE_THAT(
				reporter.no_match_reports(),
				Catch::Matchers::SizeIs(1));
			REQUIRE_THAT(
				reporter.exhausted_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.ok_match_reports(),
				Catch::Matchers::IsEmpty());
			REQUIRE_THAT(
				reporter.unsatisfied_expectations(),
				Catch::Matchers::IsEmpty());
		}

		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::SizeIs(1));
	}
}

TEST_CASE("Mock<void()>::expect_any_call expects call from any category and constness.", "[mock]")
{
	using ExpectationT = ScopedExpectation<void()>;
	ScopedReporter reporter{};
	Mock<void()> mock{};

	{
		ExpectationT expectation = std::as_const(mock).expect_any_call();

		SECTION("As const lvalue.")
		{
			REQUIRE_NOTHROW(std::as_const(mock)());
		}

		SECTION("As lvalue.")
		{
			REQUIRE_NOTHROW(mock());
		}

		SECTION("As const rvalue.")
		{
			REQUIRE_NOTHROW(std::move(std::as_const(mock))());
		}

		SECTION("As rvalue.")
		{
			REQUIRE_NOTHROW(std::move(mock)());
		}

		REQUIRE_THAT(
			reporter.no_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.exhausted_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.ok_match_reports(),
			Catch::Matchers::SizeIs(1));
	}

	REQUIRE_THAT(
		reporter.unsatisfied_expectations(),
		Catch::Matchers::IsEmpty());
}

TEST_CASE(
	"Mock supports arbitrary signatures.",
	"[mock]"
)
{
	const ScopedReporter reporter{};

	SECTION("With value params.")
	{
		using SignatureT = void(int);
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation = mock.expect_any_call(42);

		constexpr int value{42};
		mock(value);

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With lvalue ref params.")
	{
		using SignatureT = void(int&);
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation = mock.expect_any_call(42);

		int value{42};
		mock(value);

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With const lvalue ref params.")
	{
		using SignatureT = void(const int&);
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation = mock.expect_any_call(42);

		constexpr int value{42};
		mock(value);

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With rvalue ref params.")
	{
		using SignatureT = void(int&&);
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation = mock.expect_any_call(42);

		int value{42};
		mock(std::move(value));

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With const rvalue ref params.")
	{
		using SignatureT = void(const int&&);
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation = mock.expect_any_call(42);

		constexpr int value{42};
		mock(std::move(value));

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With value return type.")
	{
		using SignatureT = int();
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation =
			mock.expect_any_call()
			| expectation_policies::Returns{42};

		REQUIRE(42 == mock());

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With lvalue ref return type.")
	{
		using SignatureT = int&();
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation =
			mock.expect_any_call()
			| expectation_policies::Returns{42};

		int& ret = mock();

		REQUIRE(42 == ret);
		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With const lvalue ref return type.")
	{
		using SignatureT = const int&();
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation =
			mock.expect_any_call()
			| expectation_policies::Returns{42};

		const int& ret = mock();

		REQUIRE(42 == ret);
		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With const lvalue ref return type.")
	{
		using SignatureT = int&&();
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation =
			mock.expect_any_call()
			| expectation_policies::Returns{42};

		int&& ret = mock();

		REQUIRE(42 == ret);
		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With const lvalue ref return type.")
	{
		using SignatureT = const int&&();
		using MockT = Mock<SignatureT>;

		MockT mock{};
		const ScopedExpectation<SignatureT> expectation =
			mock.expect_any_call()
			| expectation_policies::Returns{42};

		const int&& ret = mock();

		REQUIRE(42 == ret);
		REQUIRE(expectation.is_satisfied());
	}
}
