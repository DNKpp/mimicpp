// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include <catch2/catch_test_macros.hpp>

namespace
{
    //! [stack concept]
    template <typename T, typename Value>
    concept stack_backend_for = requires(T& container, Value v) {
        { std::as_const(container).empty() } -> std::convertible_to<bool>;
        { container.back() } -> std::convertible_to<Value&>;
        { std::as_const(container).back() } -> std::convertible_to<const Value&>;
        container.push_back(std::move(v));
        { container.pop_back() };
    };

    //! [stack concept]

    //! [stack adapter]
    template <typename T, stack_backend_for<T> Container>
    class MyStack
    {
    public:
        [[nodiscard]]
        explicit MyStack(Container container)
            : m_Container{std::move(container)}
        {
        }

        [[nodiscard]]
        auto& top()
        {
            if (m_Container.empty())
            {
                throw std::runtime_error{"Container is empty."};
            }
            return m_Container.back();
        }

        [[nodiscard]]
        const auto& top() const
        {
            if (m_Container.empty())
            {
                throw std::runtime_error{"Container is empty."};
            }
            return m_Container.back();
        }

        void push(T obj)
        {
            m_Container.push_back(std::move(obj));
        }

        void pop()
        {
            if (m_Container.empty())
            {
                throw std::runtime_error{"Container is empty."};
            }
            m_Container.pop_back();
        }

    private:
        Container m_Container;
    };

    //! [stack adapter]
}

TEST_CASE(
    "Mocks can be members and thus fulfill any interface requirement.",
    "[example][example::mock]")
{
    //! [container mock]
    class ContainerMock
    {
    public:
        mimicpp::Mock<bool() const> empty{};

        mimicpp::Mock<
            int&(),
            const int&() const>
            back{};

        mimicpp::Mock<void(int)> push_back{};

        mimicpp::Mock<void()> pop_back{};
    };

    //! [container mock]

    STATIC_REQUIRE(stack_backend_for<ContainerMock, int>);

    namespace matches = mimicpp::matches;
    namespace expect = mimicpp::expect;
    namespace finally = mimicpp::finally;

    //! [test push]
    SECTION("When element is added.")
    {
        ContainerMock innerContainer{};
        SCOPED_EXP innerContainer.push_back.expect_call(42);

        MyStack<int, ContainerMock> stack{std::move(innerContainer)};
        stack.push(42);
    }
    //! [test push]

    //! [test top]
    SECTION("When top element is accessed.")
    {
        ContainerMock innerContainer{};

        SECTION("Throws, when inner container is empty.")
        {
            SCOPED_EXP innerContainer.empty.expect_call()
                and expect::times(2) // we test both, the const and non-const top() overload
                and finally::returns(true);

            MyStack<int, ContainerMock> stack{std::move(innerContainer)};

            REQUIRE_THROWS_AS(stack.top(), std::runtime_error);
            REQUIRE_THROWS_AS(std::as_const(stack).top(), std::runtime_error);
        }

        SECTION("Returns a reference to the top element otherwise.")
        {
            SCOPED_EXP innerContainer.empty.expect_call()
                and finally::returns(false);

            SECTION("A const-ref, when accessed via const.")
            {
                SCOPED_EXP std::as_const(innerContainer).back.expect_call()
                    and finally::returns(42);

                MyStack<int, ContainerMock> stack{std::move(innerContainer)};

                REQUIRE(42 == std::as_const(stack).top());
            }

            SECTION("A mutable ref, when accessed via non-const.")
            {
                SCOPED_EXP innerContainer.back.expect_call()
                    and finally::returns(42);

                MyStack<int, ContainerMock> stack{std::move(innerContainer)};

                REQUIRE(42 == stack.top());
            }
        }
    }
    //! [test top]

    //! [test pop]
    SECTION("When top element is removed.")
    {
        SECTION("Throws, when inner container is empty.")
        {
            ContainerMock innerContainer{};
            SCOPED_EXP innerContainer.empty.expect_call()
                and finally::returns(true);

            MyStack<int, ContainerMock> stack{std::move(innerContainer)};
            REQUIRE_THROWS_AS(stack.pop(), std::runtime_error);
        }

        SECTION("Succeeds silently, when at least one element exists.")
        {
            ContainerMock innerContainer{};
            SCOPED_EXP innerContainer.empty.expect_call()
                and finally::returns(false);
            SCOPED_EXP innerContainer.pop_back.expect_call();

            MyStack<int, ContainerMock> container{std::move(innerContainer)};
            container.pop();
        }
    }
    //! [test pop]
}
