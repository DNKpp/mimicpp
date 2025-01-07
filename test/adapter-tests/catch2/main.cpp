//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "catch2/catch_test_macros.hpp"
#include "mimic++/Mock.hpp"
#include "mimic++/adapters/Catch2.hpp"

#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <atomic>

namespace
{
    inline std::atomic_int g_SuccessCounter{0};

    class SuccessListener final
        : public Catch::EventListenerBase
    {
        using SuperT = EventListenerBase;

    public:
        [[nodiscard]]
        explicit SuccessListener(const Catch::IConfig* config)
            : SuperT{config}
        {
            m_preferences.shouldReportAllAssertions = true;
        }

        void assertionEnded(const Catch::AssertionStats& assertionStats) override
        {
            if (assertionStats.assertionResult.succeeded())
            {
                ++g_SuccessCounter;
            }
        }
    };
}

CATCH_REGISTER_LISTENER(SuccessListener)

TEST_CASE(
    "catch2::send_success notifies Catch2 for success.",
    "[adapter][adapter::catch2]")
{
    g_SuccessCounter = 0;

    mimicpp::detail::catch2::send_success("Test");

    REQUIRE(g_SuccessCounter == 1);
}

TEST_CASE(
    "catch2::send_warning notifies Catch2.",
    "[adapter][adapter::catch2]")
{
    mimicpp::detail::catch2::send_warning("Test");

    // not testable
}

TEST_CASE(
    "catch2::send_fail notifies Catch2 for failures and aborts.",
    "[!shouldfail][adapter][adapter::catch2]")
{
    mimicpp::detail::catch2::send_fail("Test");
}

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_CATCH2_MATCHER_INTEGRATION

namespace
{
    // directly taken from the old-style example
    // see: https://github.com/catchorg/Catch2/blob/devel/docs/matchers.md#writing-custom-matchers-old-style
    template <typename T>
    class IsBetweenMatcher
        : public Catch::Matchers::MatcherBase<T>
    {
        T m_begin, m_end;

    public:
        IsBetweenMatcher(T begin, T end)
            : m_begin(begin),
              m_end(end)
        {
        }

        bool match(const T& in) const override
        {
            return in >= m_begin && in <= m_end;
        }

        std::string describe() const override
        {
            std::ostringstream ss;
            ss << "is between " << m_begin << " and " << m_end;
            return ss.str();
        }
    };

    template <typename T>
    IsBetweenMatcher<T> IsBetween(T begin, T end)
    {
        return {begin, end};
    }
}

TEST_CASE(
    "catch2 old-style matchers are fully supported.",
    "[adapter][adapter::catch2]")
{
    STATIC_REQUIRE(mimicpp::matcher_for<IsBetweenMatcher<int>, const int&>);

    mimicpp::Mock<void(int)> mock{};

    SCOPED_EXP mock.expect_call(IsBetween(42, 1337));
    mock(1337);

    // Don't do this. The inner matcher dangles.
    // see: https://github.com/catchorg/Catch2/blob/devel/docs/matchers.md#combining-operators-and-lifetimes
    //{
    //	SCOPED_EXP mock.expect_call(!IsBetween(42, 1337));
    //	mock(41);
    //}
}

    #include <catch2/matchers/catch_matchers_range_equals.hpp>

TEST_CASE(
    "catch2 new-style matchers are fully supported.",
    "[adapter][adapter::catch2]")
{
    STATIC_CHECK(
        std::derived_from<decltype(Catch::Matchers::RangeEquals(std::array{42, 1337})), Catch::Matchers::MatcherGenericBase>);

    STATIC_REQUIRE(
        mimicpp::matcher_for<
            decltype(Catch::Matchers::RangeEquals(std::array{42, 1337})),
            const std::vector<int>&>);

    mimicpp::Mock<void(std::vector<int>)> mock{};

    SCOPED_EXP mock.expect_call(Catch::Matchers::RangeEquals(std::array{42, 1337}));
    mock({42, 1337});

    // Don't do this. The inner matcher dangles.
    // see: https://github.com/catchorg/Catch2/blob/devel/docs/matchers.md#combining-operators-and-lifetimes
    //{
    //	SCOPED_EXP mock.expect_call(!Catch::Matchers::RangeEquals(std::array{42, 1337}));
    //	mock({1337});
    //}
}

#endif
