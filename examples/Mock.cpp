// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"

#include <catch2/catch_test_macros.hpp>

namespace
{
	template <typename T, typename Value>
	concept stack_backend_for = requires(T& container, Value v)
	{
		{ container.back() } -> std::convertible_to<Value&>;
		{ std::as_const(container).back() } -> std::convertible_to<const Value&>;
		container.push_back(std::move(v));
		{ container.pop_back() } noexcept;
	};

	template <typename T, stack_backend_for<T> Container>
	class MyStack
	{
	public:
		[[nodiscard]]
		explicit MyStack(Container&& container)
			: m_Container{std::move(container)}
		{
		}

		[[nodiscard]]
		auto& top()
		{
			return m_Container.back();
		}

		[[nodiscard]]
		const auto& top() const
		{
			return m_Container.back();
		}

		void push(T obj)
		{
			m_Container.push_back(std::move(obj));
		}

		void pop() noexcept
		{
			m_Container.pop_back();
		}

	private:
		Container m_Container;
	};
}

TEST_CASE(
	"Mocks can be members and thus fulfill any interface requirement.",
	"[example][example::mock]"
)
{
	namespace matches = mimicpp::matches;
	namespace finally = mimicpp::finally;

	class ContainerMock
	{
	public:
		mimicpp::Mock<
			int&(),
			const int&() const> back{};

		mimicpp::Mock<void(int)> push_back{};

		mimicpp::Mock<void() noexcept> pop_back{};
	};

	STATIC_REQUIRE(stack_backend_for<ContainerMock, int>);

	SECTION("When element is added.")
	{
		ContainerMock innerContainer{};
		SCOPED_EXP innerContainer.push_back.expect_call(42);

		MyStack<int, ContainerMock> container{std::move(innerContainer)};
		container.push(42);
	}

	SECTION("When top element is accessed.")
	{
		ContainerMock innerContainer{};
		SCOPED_EXP innerContainer.push_back.expect_call(42);
		SCOPED_EXP innerContainer.back.expect_call()
					| finally::returns(42);
		SCOPED_EXP std::as_const(innerContainer).back.expect_call()
					| finally::returns(42);

		MyStack<int, ContainerMock> container{std::move(innerContainer)};
		container.push(42);

		REQUIRE(42 == container.top());
		REQUIRE(42 == std::as_const(container).top());
	}

	SECTION("When top element is removed.")
	{
		ContainerMock innerContainer{};
		SCOPED_EXP innerContainer.push_back.expect_call(42);
		SCOPED_EXP innerContainer.pop_back.expect_call();

		MyStack<int, ContainerMock> container{std::move(innerContainer)};
		container.push(42);
		container.pop();
	}
}
