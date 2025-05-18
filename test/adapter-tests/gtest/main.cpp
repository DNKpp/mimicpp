//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <TestAssert.hpp>

#include "mimic++/Mock.hpp"
#include "mimic++/adapters/gtest.hpp"

#include <gtest/gtest-spi.h>

namespace
{
    inline int g_SuccessCounter{0};

    class SuccessListener final
        : public testing::EmptyTestEventListener
    {
    public:
        void OnTestPartResult(const testing::TestPartResult& result) override
        {
            if (result.passed())
            {
                ++g_SuccessCounter;
            }
        }
    };

    template <typename Listener>
    class ScopedListener
    {
    public:
        ~ScopedListener() noexcept
        {
            mimicpp::settings::report_success().store(false);
            delete testing::UnitTest::GetInstance()
                ->listeners()
                .Release(m_Ptr);
        }

        [[nodiscard]]
        ScopedListener()
            : m_Ptr{new Listener{}}
        {
            mimicpp::settings::report_success().store(true);
            testing::UnitTest::GetInstance()
                ->listeners()
                .Append(m_Ptr);
        }

    private:
        Listener* m_Ptr{};
    };
}

TEST(
    GTestReporter,
    MatchReport)
{
    const ScopedListener<SuccessListener> listener{};

    g_SuccessCounter = 0;
    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(42);

    ASSERT_EQ(g_SuccessCounter, 0);
    ASSERT_EQ(g_SuccessCounter, 0); // check, that ASSERT_EQ isn't reported
    mock(42);
    EXPECT_EQ(g_SuccessCounter, 1);
}

TEST(
    GTestReporter,
    NoMatchReport)
{
    EXPECT_FATAL_FAILURE(
        mimicpp::Mock<void(int)> mock{};
        EXPECT_ANY_THROW(mock(1337)),
        "Unmatched Call originated from ");
}

TEST(
    GTestReporter,
    ReportSuccess)
{
    const ScopedListener<SuccessListener> listener{};

    g_SuccessCounter = 0;

    mimicpp::reporting::detail::gtest::send_success("Test");

    EXPECT_EQ(g_SuccessCounter, 1);
}

TEST(
    GTestReporter,
    ReportWarning)
{
    mimicpp::reporting::detail::gtest::send_warning("Test");

    // nothing to do
}

TEST(
    GTestReporter,
    ReportFail)
{
    EXPECT_FATAL_FAILURE(
        EXPECT_ANY_THROW(mimicpp::reporting::detail::gtest::send_fail("Test")),
        "Test");
}
