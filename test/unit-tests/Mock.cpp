//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "TestReporter.hpp"
#include "TestTypes.hpp"

using namespace mimicpp;

// This disables the std::invocable checks for Mocks, due to an issue on clang 18.1
// see: https://github.com/llvm/llvm-project/issues/106428
#if MIMICPP_DETAIL_IS_CLANG  \
    && __clang_major__ == 18 \
    && __clang_minor__ == 1  \
    && MIMICPP_DETAIL_USES_LIBCXX

    #define CLANG_18_STD_INVOCABLE_REGRESSION

// Should fail, when the regression has been resolved.
// If so, enable the tests and remove the issue from the ``Known Issues`` section in the readme.
static_assert(!std::invocable<Mock<void()>>);

#endif

TEMPLATE_TEST_CASE(
    "Mock is a non-copyable, but movable and default-constructible type.",
    "[mock]",
    void(),
    void() const,
    void() &,
    void() const&,
    void() &&,
    void() const&&,
    void() noexcept,
    void() const noexcept,
    void() & noexcept,
    void() const& noexcept,
    void() && noexcept,
    void() const&& noexcept)
{
    using MockT = Mock<TestType>;

    STATIC_CHECK(!std::is_copy_constructible_v<MockT>);
    STATIC_CHECK(!std::is_copy_assignable_v<MockT>);

    STATIC_CHECK(std::is_move_constructible_v<MockT>);
    STATIC_CHECK(std::is_move_assignable_v<MockT>);
    STATIC_CHECK(std::is_default_constructible_v<MockT>);

    STATIC_CHECK(std::is_nothrow_move_constructible_v<MockT>);
    STATIC_CHECK(std::is_nothrow_move_assignable_v<MockT>);
}

TEMPLATE_TEST_CASE_SIG(
    "Mutable Mock specialization is an invocable type.",
    "[mock]",
    ((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
    (true, void()),
    (true, int()),
    (true, void(int&), int&),
    (true, float && (int&), int&),
    (true, float && (std::tuple<int&>&&), std::tuple<int&>&&),
    (true, float && (std::tuple<int&>&&, std::tuple<double&&> const&), std::tuple<int&>&&, std::tuple<double&&> const&))
{
    using ReturnT = signature_return_type_t<Sig>;
    using MockT = Mock<Sig>;
    using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);

    // negative checks

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const&, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const&&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const&&, Args...>);

#ifndef CLANG_18_STD_INVOCABLE_REGRESSION

    STATIC_CHECK(std::invocable<MockT, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT, Args...>);

    STATIC_CHECK(std::invocable<MockT&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&, Args...>);

    STATIC_CHECK(std::invocable<MockT&&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&&, Args...>);

    // negative checks

    STATIC_CHECK_FALSE(std::invocable<MockT const, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT const&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const&, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT const&&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const&&, Args...>);

#endif
}

TEMPLATE_TEST_CASE_SIG(
    "Const Mock specialization is an invocable type.",
    "[mock]",
    ((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
    (true, void() const),
    (true, int() const),
    (true, void(int&) const, int&),
    (true, float && (int&) const, int&),
    (true, float && (std::tuple<int&>&&) const, std::tuple<int&>&&),
    (true, float && (std::tuple<int&>&&, std::tuple<double&&> const&) const, std::tuple<int&>&&, std::tuple<double&&> const&))
{
    using ReturnT = signature_return_type_t<Sig>;
    using MockT = Mock<Sig>;
    using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT const, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT const, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT const&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT const&, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT const&&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT const&&, Args...>);

#ifndef CLANG_18_STD_INVOCABLE_REGRESSION

    STATIC_CHECK(std::invocable<MockT, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT, Args...>);

    STATIC_CHECK(std::invocable<MockT const, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT const, Args...>);

    STATIC_CHECK(std::invocable<MockT&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&, Args...>);

    STATIC_CHECK(std::invocable<MockT const&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT const&, Args...>);

    STATIC_CHECK(std::invocable<MockT&&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&&, Args...>);

    STATIC_CHECK(std::invocable<MockT const&&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT const&&, Args...>);

#endif
}

TEMPLATE_TEST_CASE_SIG(
    "Lvalue Mock specialization is an invocable type.",
    "[mock]",
    ((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
    (true, void() &),
    (true, int() &),
    (true, void(int&) &, int&),
    (true, float && (int&)&, int&),
    (true, float && (std::tuple<int&>&&)&, std::tuple<int&>&&),
    (true, float && (std::tuple<int&>&&, std::tuple<double&&> const&)&, std::tuple<int&>&&, std::tuple<double&&> const&))
{
    using ReturnT = signature_return_type_t<Sig>;
    using MockT = Mock<Sig>;
    using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);

    // negative checks

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const&, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const&&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const&&, Args...>);

#ifndef CLANG_18_STD_INVOCABLE_REGRESSION

    STATIC_CHECK(std::invocable<MockT&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&, Args...>);

    // negative checks

    STATIC_CHECK_FALSE(std::invocable<MockT, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT const, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT const&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const&, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT&&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT&&, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT const&&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const&&, Args...>);

#endif
}

TEMPLATE_TEST_CASE_SIG(
    "Const lvalue Mock specialization is an invocable type.",
    "[mock]",
    ((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
    (true, void() const&),
    (true, int() const&),
    (true, void(int&) const&, int&),
    (true, float && (int&) const&, int&),
    (true, float && (std::tuple<int&>&&) const&, std::tuple<int&>&&),
    (true, float && (std::tuple<int&>&&, std::tuple<double&&> const&) const&, std::tuple<int&>&&, std::tuple<double&&> const&))
{
    using ReturnT = signature_return_type_t<Sig>;
    using MockT = Mock<Sig>;
    using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT const, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT const, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT const&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT const&, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT const&&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT const&&, Args...>);

#ifndef CLANG_18_STD_INVOCABLE_REGRESSION

    STATIC_CHECK(std::invocable<MockT, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT, Args...>);

    STATIC_CHECK(std::invocable<MockT const, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT const, Args...>);

    STATIC_CHECK(std::invocable<MockT&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&, Args...>);

    STATIC_CHECK(std::invocable<MockT const&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT const&, Args...>);

    STATIC_CHECK(std::invocable<MockT&&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&&, Args...>);

    STATIC_CHECK(std::invocable<MockT const&&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT const&&, Args...>);

#endif
}

TEMPLATE_TEST_CASE_SIG(
    "Rvalue Mock specialization is an invocable type.",
    "[mock]",
    ((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
    (true, void() &&),
    (true, int() &&),
    (true, void(int&) &&, int&),
    (true, float && (int&) &&, int&),
    (true, float && (std::tuple<int&>&&) &&, std::tuple<int&>&&),
    (true, float && (std::tuple<int&>&&, std::tuple<double&&> const&) &&, std::tuple<int&>&&, std::tuple<double&&> const&))
{
    using ReturnT = signature_return_type_t<Sig>;
    using MockT = Mock<Sig>;
    using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);

    // negative checks

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const&, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const&&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const&&, Args...>);

#ifndef CLANG_18_STD_INVOCABLE_REGRESSION

    STATIC_CHECK(std::invocable<MockT, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT, Args...>);

    STATIC_CHECK(std::invocable<MockT&&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&&, Args...>);

    // negative checks

    STATIC_CHECK_FALSE(std::invocable<MockT const, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT&, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT const&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const&, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT const&&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const&&, Args...>);

#endif
}

TEMPLATE_TEST_CASE_SIG(
    "Const rvalue Mock specialization is an invocable type.",
    "[mock]",
    ((bool dummy, typename Sig, typename... Args), dummy, Sig, Args...),
    (true, void() const&&),
    (true, int() const&&),
    (true, void(int&) const&&, int&),
    (true, float && (int&) const&&, int&),
    (true, float && (std::tuple<int&>&&) const&&, std::tuple<int&>&&),
    (true, float && (std::tuple<int&>&&, std::tuple<double&&> const&) const&&, std::tuple<int&>&&, std::tuple<double&&> const&))
{
    using ReturnT = signature_return_type_t<Sig>;
    using MockT = Mock<Sig>;
    using NothrowMockT = Mock<signature_add_noexcept_t<Sig>>;

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT const, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT const, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT&&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT&&, Args...>);

    STATIC_CHECK(std::is_invocable_r_v<ReturnT, MockT const&&, Args...>);
    STATIC_CHECK(std::is_invocable_r_v<ReturnT, NothrowMockT const&&, Args...>);

    // negative checks

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT&, Args...>);

    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, MockT const&, Args...>);
    STATIC_CHECK_FALSE(std::is_invocable_r_v<ReturnT, NothrowMockT const&, Args...>);

#ifndef CLANG_18_STD_INVOCABLE_REGRESSION

    STATIC_CHECK(std::invocable<MockT, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT, Args...>);

    STATIC_CHECK(std::invocable<MockT const, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT const, Args...>);

    STATIC_CHECK(std::invocable<MockT&&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT&&, Args...>);

    STATIC_CHECK(std::invocable<MockT const&&, Args...>);
    STATIC_CHECK(std::invocable<NothrowMockT const&&, Args...>);

    // negative checks

    STATIC_CHECK_FALSE(std::invocable<MockT&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT&, Args...>);

    STATIC_CHECK_FALSE(std::invocable<MockT const&, Args...>);
    STATIC_CHECK_FALSE(std::invocable<NothrowMockT const&, Args...>);

#endif
}

namespace
{
    class OverloadTargetMatcher
        : public Catch::Matchers::MatcherGenericBase
    {
    public:
        [[nodiscard]]
        OverloadTargetMatcher(reporting::TypeReport&& report)
            : m_SignatureReport{std::move(report)}
        {
        }

        [[nodiscard, maybe_unused]]
        bool match(std::tuple<reporting::CallReport, reporting::ExpectationReport> const& entry) const
        {
            auto const& [callReport, expectationReport] = entry;
            return callReport.target.overloadReport == m_SignatureReport
                && expectationReport.target.overloadReport == m_SignatureReport;
        }

        [[nodiscard]]
        std::string describe() const override
        {
            return "Overload signature is: " + m_SignatureReport.name();
        }

    private:
        reporting::TypeReport m_SignatureReport;
    };

    template <util::satisfies<std::is_function> Signature>
    auto MatchesOverloadTarget()
    {
        return OverloadTargetMatcher{reporting::TypeReport::make<Signature>()};
    }
}

TEST_CASE(
    "Mutable Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void()> mock{};

        ScopedExpectation const expectation = mock.expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void()>()));
    }

    SECTION("With int() signature.")
    {
        Mock<int()> mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int()>()));
    }
}

TEST_CASE(
    "Const Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() const> mock{};

        ScopedExpectation const expectation = mock.expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void() const>()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() const> mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int() const>()));
    }
}

TEST_CASE(
    "Lvalue Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void()&> mock{};

        ScopedExpectation const expectation = mock.expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void()&>()));
    }

    SECTION("With int() signature.")
    {
        Mock<int()&> mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int()&>()));
    }
}

TEST_CASE(
    "Const lvalue Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() const&> mock{};

        ScopedExpectation const expectation = mock.expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void() const&>()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() const&> mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int() const&>()));
    }
}

TEST_CASE(
    "Rvalue Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() &&> mock{};

        ScopedExpectation const expectation = std::move(mock).expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        std::move(mock)();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void() &&>()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() &&> mock{};

        ScopedExpectation const expectation = std::move(mock).expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == std::move(mock)());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int() &&>()));
    }
}

TEST_CASE(
    "Const rvalue Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() const&&> mock{};

        ScopedExpectation const expectation = std::move(mock).expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        std::move(mock)();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void() const&&>()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() const&&> mock{};

        ScopedExpectation const expectation = std::move(mock).expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == std::move(mock)());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int() const&&>()));
    }
}

TEST_CASE(
    "Mutable noexcept Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() noexcept> mock{};

        ScopedExpectation const expectation = mock.expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void() noexcept>()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() noexcept> mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int() noexcept>()));
    }
}

TEST_CASE(
    "Const noexcept Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() const noexcept> mock{};

        ScopedExpectation const expectation = mock.expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void() const noexcept>()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() const noexcept> mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int() const noexcept>()));
    }
}

TEST_CASE(
    "Lvalue noexcept Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() & noexcept> mock{};

        ScopedExpectation const expectation = mock.expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget < void() & noexcept > ()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() & noexcept> mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget < int() & noexcept > ()));
    }
}

TEST_CASE(
    "Const lvalue noexcept Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() const & noexcept> mock{};

        ScopedExpectation const expectation = mock.expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget < void() const& noexcept > ()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() const & noexcept> mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget < int() const& noexcept > ()));
    }
}

TEST_CASE(
    "Rvalue noexcept Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() && noexcept> mock{};

        ScopedExpectation const expectation = std::move(mock).expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        std::move(mock)();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget < void() && noexcept > ()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() && noexcept> mock{};

        ScopedExpectation const expectation = std::move(mock).expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == std::move(mock)());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget < int() && noexcept > ()));
    }
}

TEST_CASE(
    "Const rvalue noexcept Mock specialization supports expectations.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<void() const && noexcept> mock{};

        ScopedExpectation const expectation = std::move(mock).expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());

        std::move(mock)();

        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget < void() const&& noexcept > ()));
    }

    SECTION("With int() signature.")
    {
        Mock<int() const && noexcept> mock{};

        ScopedExpectation const expectation = std::move(mock).expect_call()
                                           && finally::returns(42);
        REQUIRE_FALSE(expectation.is_satisfied());

        CHECK(42 == std::move(mock)());
        CHECK(expectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget < int() const&& noexcept > ()));
    }
}

TEST_CASE(
    "Mock supports overloading with similar signature.",
    "[mock]")
{
    ScopedReporter reporter{};

    SECTION("With void() signature.")
    {
        Mock<
            void(),
            void() const>
            mock{};

        ScopedExpectation const expectation = mock.expect_call();
        ScopedExpectation const constExpectation = std::as_const(mock).expect_call();
        REQUIRE_FALSE(expectation.is_satisfied());
        REQUIRE_FALSE(constExpectation.is_satisfied());

        mock();

        CHECK(expectation.is_satisfied());
        CHECK_FALSE(constExpectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<void()>()));

        std::as_const(mock)();

        CHECK(expectation.is_satisfied());
        CHECK(constExpectation.is_satisfied());
        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(2u));
        CHECK_THAT(
            reporter.full_match_reports().back(),
            MatchesOverloadTarget<void() const>());
    }

    SECTION("With int() signature.")
    {
        Mock<
            int(),
            int() const>
            mock{};

        ScopedExpectation const expectation = mock.expect_call()
                                           && finally::returns(42);
        ScopedExpectation const constExpectation = std::as_const(mock).expect_call()
                                                && finally::returns(1337);
        REQUIRE_FALSE(expectation.is_satisfied());
        REQUIRE_FALSE(constExpectation.is_satisfied());

        CHECK(42 == mock());
        CHECK(expectation.is_satisfied());
        CHECK_FALSE(constExpectation.is_satisfied());
        CHECK_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u)
                && Catch::Matchers::AllMatch(MatchesOverloadTarget<int()>()));

        CHECK(1337 == std::as_const(mock)());
        CHECK(expectation.is_satisfied());
        CHECK(constExpectation.is_satisfied());
        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(2u));
        CHECK_THAT(
            reporter.full_match_reports().back(),
            MatchesOverloadTarget<int() const>());
    }
}

TEST_CASE(
    "Mocks report a stacktrace.",
    "[mock]")
{
    ScopedReporter reporter{};

    Mock<void()> mock{};

    util::Stacktrace const here = util::stacktrace::current();
    auto const check = [&](reporting::CallReport const& call) {
#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND
        REQUIRE(!here.empty());
        REQUIRE(call.stacktrace.size() == here.size());
        std::invoke(
            [&] {
                auto const hereSrc = here.source_file(0u);
                auto const callSrc = call.stacktrace.source_file(0u);

                // On msvc, there may be additional noise after the actual function name (e.g. `foo+1337`).
                CHECKED_ELSE(hereSrc == callSrc)
                {
                    auto const hereIter = std::ranges::find(hereSrc, '+');
                    auto const callIter = std::ranges::find(callSrc, '+');
                    CHECK_THAT(
                        (std::string{callSrc.cbegin(), callIter}),
                        Catch::Matchers::Equals(std::string{hereSrc.cbegin(), hereIter}));
                    CHECK_THAT(
                        (std::string{hereIter, hereSrc.cend()}),
                        Catch::Matchers::Matches(R"(\+\d+)"));
                    CHECK_THAT(
                        (std::string{callIter, callSrc.cend()}),
                        Catch::Matchers::Matches(R"(\+\d+)"));
                }
            });
        CHECK(call.stacktrace.source_file(0u) == here.source_file(0u));
        CHECK(call.stacktrace.source_line(0u) > here.source_line(0u));
        CHECK(
            std::ranges::all_of(
                std::views::iota(1u, here.size()),
                [&](std::size_t const i) {
                    return call.stacktrace.source_file(i) == here.source_file(i)
                        && call.stacktrace.source_line(i) == here.source_line(i)
                        && call.stacktrace.description(i) == here.description(i);
                }));
#else
        REQUIRE(call.stacktrace.empty());
#endif
    };

    SECTION("When matched call is given.")
    {
        ScopedExpectation exp = mock.expect_call();

        mock();

        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u));

        check(std::get<reporting::CallReport>(reporter.full_match_reports().front()));
    }

    SECTION("When no-match call is given.")
    {
        REQUIRE_THROWS(mock());

        REQUIRE_THAT(
            reporter.no_match_reports(),
            Catch::Matchers::SizeIs(1u));

        check(std::get<reporting::CallReport>(reporter.no_match_reports().front()));
    }

    SECTION("When inapplicable call is given.")
    {
        ScopedExpectation exp = mock.expect_call();

        mock();
        REQUIRE_THROWS(mock());

        REQUIRE_THAT(
            reporter.full_match_reports(),
            Catch::Matchers::SizeIs(1u));
        REQUIRE_THAT(
            reporter.inapplicable_match_reports(),
            Catch::Matchers::SizeIs(1u));

        check(std::get<reporting::CallReport>(reporter.inapplicable_match_reports().front()));
    }
}

TEST_CASE(
    "Mocks stacktrace-skip value can be adjusted.",
    "[mock]")
{
    ScopedReporter reporter{};

    Mock<void()> mock{};
    ScopedExpectation exp = mock.expect_call();
    mock();
    std::size_t const skip = GENERATE(1u, 2u, 3u);
    mock = Mock<void()>{MockSettings{.stacktraceSkip = skip}};
    exp = mock.expect_call();
    mock();

    auto const& [first, _1] = reporter.full_match_reports().front();
    auto const& [second, _2] = reporter.full_match_reports().at(1u);

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND
    REQUIRE(first.stacktrace.size() == second.stacktrace.size() + skip);

    CHECK(
        std::ranges::all_of(
            std::views::iota(0u, second.stacktrace.size()),
            [&](std::size_t const i) {
                return first.stacktrace.source_file(i + skip) == second.stacktrace.source_file(i)
                    && first.stacktrace.source_line(i + skip) == second.stacktrace.source_line(i)
                    && first.stacktrace.description(i + skip) == second.stacktrace.description(i);
            }));
#else
    REQUIRE(first.stacktrace.empty());
    REQUIRE(second.stacktrace.empty());
#endif
}

TEST_CASE(
    "Mock supports arbitrary overload sets.",
    "[mock]")
{
    ScopedReporter reporter{};

    Mock<
        void(),
        double(int) const>
        mock{};

    ScopedExpectation const firstExpectation = mock.expect_call();
    ScopedExpectation const secondExpectation = mock.expect_call(1337)
                                             && finally::returns(4.2);

    REQUIRE_FALSE(firstExpectation.is_satisfied());
    REQUIRE_FALSE(secondExpectation.is_satisfied());

    mock();
    CHECK(firstExpectation.is_satisfied());
    CHECK(!secondExpectation.is_satisfied());

    CHECK(4.2 == mock(1337));
    CHECK(firstExpectation.is_satisfied());
    CHECK(secondExpectation.is_satisfied());
}

TEST_CASE(
    "Mocks support direct argument matchers.",
    "[mock]")
{
    SECTION("For arguments, which support operator ==")
    {
        Mock<void(int)> mock{};
        SCOPED_EXP mock.expect_call(1337);
        mock(1337);
    }

    SECTION("For arguments, which support operator== but do not satisfy std::equality_comparable_with.")
    {
        Mock<void(std::optional<int>)> mock{};
        SCOPED_EXP mock.expect_call(std::nullopt);
        mock({});
    }

    SECTION("For strings.")
    {
        Mock<void(const char*)> mock{};
        SCOPED_EXP mock.expect_call("Hello, World!");
        mock("Hello, World!");

        SECTION("If both, param and argument, are character pointers, they are not treated as strings.")
        {
            constexpr std::array<char, 2> value1{42, 0};
            constexpr std::array<char, 2> value2{42, 0};

            // If the second expectation is matched, then we know, that the argument has been treated as a string.
            // Therefore, the first expectation must be chosen, which compares the actual pointers.
            SCOPED_EXP mock.expect_call(matches::eq(value2.data()));
            SCOPED_EXP mock.expect_call(value1.data())
                and expect::at_least(0u);

            mock(value2.data());
        }
    }

    SECTION("With explicit matchers.")
    {
        Mock<void(int)> mock{};
        SCOPED_EXP mock.expect_call(matches::ne(42));
        mock(1337);
    }
}

TEST_CASE(
    "Mocks have names.",
    "[mock]")
{
    SECTION("When name is set.")
    {
        Mock<void()> mock{
            MockSettings{.name = "MyMock"}};
        ScopedExpectation const expectation = mock.expect_call()
                                          and expect::never();

        CHECK_THAT(
            expectation.mock_name(),
            Catch::Matchers::Equals("MyMock"));
    }

    SECTION("When not specified, it will be automatically generated.")
    {
        SECTION("When single signature is given.")
        {
            Mock<void()> mock{};
            ScopedExpectation const expectation = mock.expect_call()
                                              and expect::never();

            CHECK_THAT(
                expectation.mock_name(),
                Catch::Matchers::Equals("Mock<void()>"));
        }

        SECTION("When multiple signatures are given.")
        {
#ifdef MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES
            Mock<void(), void(std::vector<int>*) const, float(std::string const&&, int) & noexcept> mock{};
            ScopedExpectation const expectation = mock.expect_call()
                                              and expect::never();

            CHECK_THAT(
                expectation.mock_name(),
                Catch::Matchers::Equals("Mock<void(), void(std::vector<int>*) const, float(std::string const&&, int) & noexcept>"));
#else
            Mock<void(), void(std::string*) const, float(std::string const&&, int) & noexcept> mock{};
            const ScopedExpectation expectation = mock.expect_call()
                                              and expect::never();

            REQUIRE_THAT(
                expectation.mock_name(),
                Catch::Matchers::Equals("Mock<void(), void(std::string*) const, float(std::string const&&, int) & noexcept>"));
#endif
        }
    }
}
