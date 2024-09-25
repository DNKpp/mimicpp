// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"

#include "mimic++/Mock.hpp"

#include "TestTypes.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

using namespace mimicpp;

TEMPLATE_TEST_CASE(
	"Mock is a non-copyable, but movable and default-constructible type.",
	"[mock]",
	void(),
	void() const,
	void() &,
	void() const &,
	void() &&,
	void() const &&,
	void() noexcept,
	void() const noexcept,
	void() & noexcept,
	void() const & noexcept,
	void() && noexcept,
	void() const && noexcept
)
{
	using MockT = Mock<TestType>;

	STATIC_REQUIRE(!std::is_copy_constructible_v<MockT>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<MockT>);

	STATIC_REQUIRE(std::is_move_constructible_v<MockT>);
	STATIC_REQUIRE(std::is_move_assignable_v<MockT>);
	STATIC_REQUIRE(std::is_default_constructible_v<MockT>);

	STATIC_REQUIRE(std::is_nothrow_move_constructible_v<MockT>);
	STATIC_REQUIRE(std::is_nothrow_move_assignable_v<MockT>);
}

TEMPLATE_TEST_CASE_SIG(
	"Mutable Mock specialization is an invocable type.",
	"[mock]",
	((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
	(true, void()),
	(true, int()),
	(true, void(int&), int&),
	(true, float&&(int&), int&),
	(true, float&&(std::tuple<int&>&&), std::tuple<int&>&&),
	(true, float&&(std::tuple<int&>&&, const std::tuple<double&&>&), std::tuple<int&>&&, const std::tuple<double&&>&)
)
{
	using ReturnT = signature_return_type_t<Sig>;
	using MockT = Mock<Sig>;
	using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT, Args...>);
	STATIC_REQUIRE(std::invocable<MockT, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&&, Args...>);
}

TEMPLATE_TEST_CASE_SIG(
	"Mutable Mock specialization is an invocable type.",
	"[mock]",
	((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
	(true, void() const),
	(true, int() const),
	(true, void(int&) const, int&),
	(true, float&&(int&) const, int&),
	(true, float&&(std::tuple<int&>&&) const, std::tuple<int&>&&),
	(true, float&&(std::tuple<int&>&&, const std::tuple<double&&>&) const, std::tuple<int&>&&, const std::tuple<double&&>&)
)
{
	using ReturnT = signature_return_type_t<Sig>;
	using MockT = Mock<Sig>;
	using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT, Args...>);
	STATIC_REQUIRE(std::invocable<MockT, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const MockT&, Args...>);
	STATIC_REQUIRE(std::invocable<const MockT&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const NothrowMockT&, Args...>);
	STATIC_REQUIRE(std::invocable<const NothrowMockT&, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&&, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const MockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<const MockT&&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const NothrowMockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<const NothrowMockT&&, Args...>);
}

TEMPLATE_TEST_CASE_SIG(
	"Lvalue Mock specialization is an invocable type.",
	"[mock]",
	((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
	(true, void() &),
	(true, int() &),
	(true, void(int&) &, int&),
	(true, float&&(int&) &, int&),
	(true, float&&(std::tuple<int&>&&) &, std::tuple<int&>&&),
	(true, float&&(std::tuple<int&>&&, const std::tuple<double&&>&) &, std::tuple<int&>&&, const std::tuple<double&&>&)
)
{
	using ReturnT = signature_return_type_t<Sig>;
	using MockT = Mock<Sig>;
	using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&, Args...>);
}

TEMPLATE_TEST_CASE_SIG(
	"Const lvalue Mock specialization is an invocable type.",
	"[mock]",
	((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
	(true, void() const &),
	(true, int() const &),
	(true, void(int&) const &, int&),
	(true, float&&(int&) const &, int&),
	(true, float&&(std::tuple<int&>&&) const &, std::tuple<int&>&&),
	(true, float&&(std::tuple<int&>&&, const std::tuple<double&&>&) const &, std::tuple<int&>&&, const std::tuple<double&&>&)
)
{
	using ReturnT = signature_return_type_t<Sig>;
	using MockT = Mock<Sig>;
	using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT, Args...>);
	STATIC_REQUIRE(std::invocable<MockT, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const MockT&, Args...>);
	STATIC_REQUIRE(std::invocable<const MockT&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const NothrowMockT&, Args...>);
	STATIC_REQUIRE(std::invocable<const NothrowMockT&, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&&, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const MockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<const MockT&&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const NothrowMockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<const NothrowMockT&&, Args...>);
}

TEMPLATE_TEST_CASE_SIG(
	"Rvalue Mock specialization is an invocable type.",
	"[mock]",
	((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
	(true, void() &&),
	(true, int() &&),
	(true, void(int&) &&, int&),
	(true, float&&(int&) &&, int&),
	(true, float&&(std::tuple<int&>&&) &&, std::tuple<int&>&&),
	(true, float&&(std::tuple<int&>&&, const std::tuple<double&&>&) &&, std::tuple<int&>&&, const std::tuple<double&&>&)
)
{
	using ReturnT = signature_return_type_t<Sig>;
	using MockT = Mock<Sig>;
	using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT, Args...>);
	STATIC_REQUIRE(std::invocable<MockT, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&&, Args...>);
}

TEMPLATE_TEST_CASE_SIG(
	"Const rvalue Mock specialization is an invocable type.",
	"[mock]",
	((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
	(true, void() const &&),
	(true, int() const &&),
	(true, void(int&) const &&, int&),
	(true, float&&(int&) const &&, int&),
	(true, float&&(std::tuple<int&>&&) const &&, std::tuple<int&>&&),
	(true, float&&(std::tuple<int&>&&, const std::tuple<double&&>&) const &&, std::tuple<int&>&&, const std::tuple<double&&>&)
)
{
	using ReturnT = signature_return_type_t<Sig>;
	using MockT = Mock<Sig>;
	using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT, Args...>);
	STATIC_REQUIRE(std::invocable<MockT, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<MockT&&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<NothrowMockT&&, Args...>);

	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const MockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<const MockT&&, Args...>);
	STATIC_REQUIRE(std::is_invocable_r_v<ReturnT, const NothrowMockT&&, Args...>);
	STATIC_REQUIRE(std::invocable<const NothrowMockT&&, Args...>);
}

TEST_CASE(
	"Mutable Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void()> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		CHECK(!expectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int()> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Const Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() const> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		CHECK(!expectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() const> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Lvalue Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() &> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		CHECK(!expectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() &> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Const lvalue Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() const &> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		CHECK(!expectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() const &> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Rvalue Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() &&> mock{};

		const ScopedExpectation expectation = std::move(mock).expect_call();
		CHECK(!expectation.is_satisfied());

		std::move(mock)();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() &&> mock{};

		const ScopedExpectation expectation = std::move(mock).expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == std::move(mock)());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Const rvalue Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() const &&> mock{};

		const ScopedExpectation expectation = std::move(mock).expect_call();
		CHECK(!expectation.is_satisfied());

		std::move(mock)();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() const &&> mock{};

		const ScopedExpectation expectation = std::move(mock).expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == std::move(mock)());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Mutable noexcept Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() noexcept> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		CHECK(!expectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() noexcept> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Const noexcept Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() const noexcept> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		CHECK(!expectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() const noexcept> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Lvalue noexcept Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() & noexcept> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		CHECK(!expectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() & noexcept> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Const lvalue noexcept Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() const & noexcept> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		CHECK(!expectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() const & noexcept> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Rvalue noexcept Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() && noexcept> mock{};

		const ScopedExpectation expectation = std::move(mock).expect_call();
		CHECK(!expectation.is_satisfied());

		std::move(mock)();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() && noexcept> mock{};

		const ScopedExpectation expectation = std::move(mock).expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == std::move(mock)());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Const rvalue noexcept Mock specialization supports expectations.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<void() const && noexcept> mock{};

		const ScopedExpectation expectation = std::move(mock).expect_call();
		CHECK(!expectation.is_satisfied());

		std::move(mock)();

		REQUIRE(expectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<int() const && noexcept> mock{};

		const ScopedExpectation expectation = std::move(mock).expect_call()
											&& finally::returns(42);
		CHECK(!expectation.is_satisfied());

		REQUIRE(42 == std::move(mock)());
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"Mock supports overloading with similar signature.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	SECTION("With void() signature.")
	{
		Mock<
			void(),
			void() const
		> mock{};

		const ScopedExpectation expectation = mock.expect_call();
		const ScopedExpectation constExpectation = std::as_const(mock).expect_call();
		CHECK(!expectation.is_satisfied());
		CHECK(!constExpectation.is_satisfied());

		mock();

		REQUIRE(expectation.is_satisfied());
		REQUIRE(!constExpectation.is_satisfied());

		std::as_const(mock)();

		REQUIRE(expectation.is_satisfied());
		REQUIRE(constExpectation.is_satisfied());
	}

	SECTION("With int() signature.")
	{
		Mock<
			int(),
			int() const
		> mock{};

		const ScopedExpectation expectation = mock.expect_call()
											&& finally::returns(42);
		const ScopedExpectation constExpectation = std::as_const(mock).expect_call()
													&& finally::returns(1337);
		CHECK(!expectation.is_satisfied());
		CHECK(!constExpectation.is_satisfied());

		REQUIRE(42 == mock());
		REQUIRE(expectation.is_satisfied());
		REQUIRE(!constExpectation.is_satisfied());

		REQUIRE(1337 == std::as_const(mock)());
		REQUIRE(expectation.is_satisfied());
		REQUIRE(constExpectation.is_satisfied());
	}
}

TEST_CASE(
	"Mock supports arbitrary overload sets.",
	"[mock]"
)
{
	ScopedReporter reporter{};

	Mock<
		void(),
		double(int) const
	> mock{};

	const ScopedExpectation firstExpectation = mock.expect_call();
	const ScopedExpectation secondExpectation = mock.expect_call(1337)
												&& finally::returns(4.2);

	CHECK(!firstExpectation.is_satisfied());
	CHECK(!secondExpectation.is_satisfied());

	mock();
	REQUIRE(firstExpectation.is_satisfied());
	REQUIRE(!secondExpectation.is_satisfied());

	REQUIRE(4.2 == mock(1337));
	REQUIRE(firstExpectation.is_satisfied());
	REQUIRE(secondExpectation.is_satisfied());
}
