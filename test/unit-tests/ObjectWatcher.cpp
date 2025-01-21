//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ObjectWatcher.hpp"
#include "mimic++/InterfaceMock.hpp"
#include "mimic++/policies/FinalizerPolicies.hpp"

#include "SuppressionMacros.hpp"
#include "TestReporter.hpp"

using namespace mimicpp;

TEST_CASE(
    "LifetimeWatcher tracks destruction",
    "[object-watcher][object-watcher::lifetime]")
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
            std::invoke([&] { delete watcher.release(); }),
            NoMatchError);
    }

    SECTION("Reports a full-match, if destruction occurs with an active expectation.")
    {
        SECTION("From an lvalue.")
        {
            auto expectation = std::invoke(
                []() -> ScopedExpectation {
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
            []() -> ScopedExpectation {
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
                    [&] {
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
                    [&]() -> ScopedExpectation {
                        LifetimeWatcher target{*std::move(source)};
                        return target.expect_destruct();
                    });
            }

            SECTION("When move assigned.")
            {
                auto innerExp = std::invoke(
                    [&]() -> ScopedExpectation {
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
                    [&]() -> ScopedExpectation {
                        LifetimeWatcher target{*source};
                        return target.expect_destruct();
                    });

                // need to manually destroy the object, to prevent the expectation outliving the lifetime-watcher
                source.reset();
            }

            SECTION("When copy-assigning.")
            {
                auto expectation = std::invoke(
                    [&]() -> ScopedExpectation {
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
                    [&]() -> ScopedExpectation {
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
                    [&]() -> ScopedExpectation {
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
    "LifetimeWatcher generates appropriate mock name.",
    "[object-watcher][object-watcher::lifetime]")
{
    struct my_base
    {
    };

    SECTION("When directly used.")
    {
        volatile const auto expectation = std::invoke(
            []() -> ScopedExpectation {
                LifetimeWatcher watcher{for_base_v<my_base>};
                ScopedExpectation exp = watcher.expect_destruct();

                REQUIRE_THAT(
                    exp.mock_name(),
                    Catch::Matchers::StartsWith("LifetimeWatcher for ")
                        && Catch::Matchers::EndsWith("my_base"));

                return exp;
            });
    }

    SECTION("When copied.")
    {
        std::optional<LifetimeWatcher> watcher{for_base_v<my_base>};
        MIMICPP_SCOPED_EXPECTATION watcher->expect_destruct();

        SECTION("Copy constructed.")
        {
            volatile const auto expectation = std::invoke(
                [&]() -> ScopedExpectation {
                    LifetimeWatcher inner{*watcher};
                    ScopedExpectation exp = inner.expect_destruct();

                    REQUIRE_THAT(
                        exp.mock_name(),
                        Catch::Matchers::StartsWith("LifetimeWatcher for ")
                            && Catch::Matchers::EndsWith("my_base"));

                    return exp;
                });
        }

        SECTION("Copy assigned.")
        {
            volatile const auto expectation = std::invoke(
                [&]() -> ScopedExpectation {
                    LifetimeWatcher inner{};
                    MIMICPP_SCOPED_EXPECTATION inner.expect_destruct();

                    inner = *watcher;
                    ScopedExpectation exp = inner.expect_destruct();

                    REQUIRE_THAT(
                        exp.mock_name(),
                        Catch::Matchers::StartsWith("LifetimeWatcher for ")
                            && Catch::Matchers::EndsWith("my_base"));

                    return exp;
                });
        }

        watcher.reset();
    }
}

TEST_CASE(
    "LifetimeWatcher supports finally::throws policy.",
    "[object-watcher][object-watcher::lifetime]")
{
    struct my_exception
    {
    };

    const auto action = [] {
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
    "LifetimeWatcher watched can wrap the actual type to be watched with the utilized watcher types.",
    "[object-watcher][object-watcher::lifetime]")
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
            LifetimeWatcher>
            watched{};

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
            LifetimeWatcher>
            watched{};

        SequenceT sequence{};
        {
            Watched<
                Mock<void()>,
                LifetimeWatcher>
                other{};

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
    "LifetimeWatcher watched can be used on interface-mocks.",
    "[object-watcher][object-watcher::lifetime]")
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
    "Violations of LifetimeWatcher watched interface-implementations will be detected.",
    "[object-watcher][object-watcher::lifetime]")
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

TEST_CASE(
    "RelocationWatcher tracks object move-constructions and -assignments.",
    "[object-watcher][object-watcher::relocation]")
{
    namespace Matches = Catch::Matchers;

    ScopedReporter reporter{};

    SECTION("Reports a no-match, if move occurs without an expectation.")
    {
        RelocationWatcher watcher{};

        SECTION("When move constructing.")
        {
            REQUIRE_THROWS_AS(
                RelocationWatcher{std::move(watcher)},
                NoMatchError);
        }

        SECTION("When move assigning.")
        {
            const auto action = [&] {
                RelocationWatcher target{};
                target = std::move(watcher);
            };

            REQUIRE_THROWS_AS(
                action(),
                NoMatchError);
        }

        SECTION("When self assigning.")
        {
            START_WARNING_SUPPRESSION
            SUPPRESS_SELF_MOVE
            REQUIRE_THROWS_AS(
                watcher = std::move(watcher),
                NoMatchError);
            STOP_WARNING_SUPPRESSION
        }

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

    SECTION("Reports an unfulfilled expectation, if the expectation expires before relocation occurs.")
    {
        RelocationWatcher watcher{};
        std::optional<ScopedExpectation> expectation = watcher.expect_relocate();
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
    }

    SECTION("Is satisfied, if a relocation occurs with an existing expectation.")
    {
        RelocationWatcher watcher{};
        SCOPED_EXP watcher.expect_relocate();

        SECTION("When move-constructing.")
        {
            const RelocationWatcher target{std::move(watcher)};
        }

        SECTION("When move-assigning.")
        {
            RelocationWatcher target{};
            target = std::move(watcher);
        }

        SECTION("When self move-assigning.")
        {
            START_WARNING_SUPPRESSION
            SUPPRESS_SELF_MOVE
            watcher = std::move(watcher);
            STOP_WARNING_SUPPRESSION
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
}

TEST_CASE(
    "Copying RelocationWatcher doesn't satisfy it.",
    "[object-watcher][object-watcher::relocation]")
{
    namespace Matches = Catch::Matchers;

    ScopedReporter reporter{};

    RelocationWatcher watcher{};
    std::optional<ScopedExpectation> expectation = watcher.expect_relocate();

    SECTION("When copy-constructing.")
    {
        RelocationWatcher other{watcher};
    }

    SECTION("When copy-assigning.")
    {
        RelocationWatcher other{};

        other = watcher;
    }

    SECTION("When self copy-assigning.")
    {
        START_WARNING_SUPPRESSION
        SUPPRESS_SELF_ASSIGN
        watcher = watcher;
        STOP_WARNING_SUPPRESSION

        SECTION("And it does not accept the expectation from the previous instance.")
        {
            START_WARNING_SUPPRESSION
            SUPPRESS_SELF_MOVE
            REQUIRE_THROWS_AS(
                watcher = std::move(watcher),
                NoMatchError);
            STOP_WARNING_SUPPRESSION
        }
    }

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
}

TEST_CASE(
    "RelocationWatcher generates appropriate mock name.",
    "[object-watcher][object-watcher::relocation]")
{
    struct my_base
    {
    };

    std::optional<RelocationWatcher> watcher{for_base_v<my_base>};
    ScopedExpectation exp = watcher->expect_relocate();

    REQUIRE_THAT(
        exp.mock_name(),
        Catch::Matchers::StartsWith("RelocationWatcher for ")
            && Catch::Matchers::EndsWith("my_base"));

    RelocationWatcher other = std::move(*watcher);
    watcher.reset();
    exp = other.expect_relocate();

    REQUIRE_THAT(
        exp.mock_name(),
        Catch::Matchers::StartsWith("RelocationWatcher for ")
            && Catch::Matchers::EndsWith("my_base"));

    watcher = std::move(other);
    exp = watcher->expect_relocate()
      and expect::never();

    REQUIRE_THAT(
        exp.mock_name(),
        Catch::Matchers::StartsWith("RelocationWatcher for ")
            && Catch::Matchers::EndsWith("my_base"));

    SECTION("Copy constructed.")
    {
        RelocationWatcher inner{watcher.value()};
        ScopedExpectation innerExp = inner.expect_relocate()
                                 and expect::never();

        REQUIRE_THAT(
            innerExp.mock_name(),
            Catch::Matchers::StartsWith("RelocationWatcher for ")
                && Catch::Matchers::EndsWith("my_base"));
    }

    SECTION("Copy constructed.")
    {
        RelocationWatcher inner{};
        inner = watcher.value();
        ScopedExpectation innerExp = inner.expect_relocate()
                                 and expect::never();

        REQUIRE_THAT(
            innerExp.mock_name(),
            Catch::Matchers::StartsWith("RelocationWatcher for ")
                && Catch::Matchers::EndsWith("my_base"));
    }
}

TEST_CASE(
    "RelocationWatcher supports finally::throws policy.",
    "[object-watcher][object-watcher::relocation]")
{
    namespace Matches = Catch::Matchers;

    ScopedReporter reporter{};

    struct my_exception
    {
    };

    RelocationWatcher watcher{};
    MIMICPP_SCOPED_EXPECTATION watcher.expect_relocate()
        and finally::throws(my_exception{});

    SECTION("When move-constructing.")
    {
        REQUIRE_THROWS_AS(
            RelocationWatcher{std::move(watcher)},
            my_exception);
    }

    SECTION("When move-assigning.")
    {
        RelocationWatcher target{};

        REQUIRE_THROWS_AS(
            target = std::move(watcher),
            my_exception);
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

TEST_CASE(
    "RelocationWatcher watched can wrap the actual type to be watched with the utilized watcher types.",
    "[object-watcher][object-watcher::relocation]")
{
    STATIC_REQUIRE(std::is_nothrow_destructible_v<Watched<Mock<void(int)>, LifetimeWatcher>>);

    SECTION("Detects violations.")
    {
        ScopedReporter reporter{};

        struct not_nothrow_movable
        {
            ~not_nothrow_movable() = default;
            not_nothrow_movable() = default;

            not_nothrow_movable(const not_nothrow_movable&) = delete;
            not_nothrow_movable& operator=(const not_nothrow_movable&) = delete;

            not_nothrow_movable(not_nothrow_movable&&) noexcept(false)
            {
            }

            not_nothrow_movable& operator=(not_nothrow_movable&&) noexcept(false)
            {
                return *this;
            }
        };

        Watched<
            not_nothrow_movable,
            RelocationWatcher>
            watched{};
        STATIC_REQUIRE(!std::is_nothrow_move_constructible_v<decltype(watched)>);
        STATIC_REQUIRE(!std::is_nothrow_move_assignable_v<decltype(watched)>);

        REQUIRE_THROWS_AS(
            Watched{std::move(watched)},
            NoMatchError);
    }

    SECTION("Just plain usage.")
    {
        Watched<
            Mock<void(int)>,
            RelocationWatcher>
            watched{};
        STATIC_REQUIRE(std::is_nothrow_move_constructible_v<decltype(watched)>);
        STATIC_REQUIRE(std::is_nothrow_move_assignable_v<decltype(watched)>);

        MIMICPP_SCOPED_EXPECTATION watched.expect_call(1337);
        MIMICPP_SCOPED_EXPECTATION watched.expect_relocate()
            and expect::twice();
        MIMICPP_SCOPED_EXPECTATION watched.expect_call(42);

        watched(42);
        Watched other{std::move(watched)};
        other(1337);
        watched = std::move(other);
    }

    SECTION("With explicit sequence.")
    {
        Watched<
            Mock<void(int)>,
            RelocationWatcher>
            watched{};
        STATIC_REQUIRE(std::is_nothrow_move_constructible_v<decltype(watched)>);
        STATIC_REQUIRE(std::is_nothrow_move_assignable_v<decltype(watched)>);

        SequenceT sequence{};

        MIMICPP_SCOPED_EXPECTATION watched.expect_call(42)
            and expect::in_sequence(sequence);
        MIMICPP_SCOPED_EXPECTATION watched.expect_relocate()
            and expect::in_sequence(sequence);
        MIMICPP_SCOPED_EXPECTATION watched.expect_call(1337)
            and expect::in_sequence(sequence);
        MIMICPP_SCOPED_EXPECTATION watched.expect_relocate()
            and expect::in_sequence(sequence);

        watched(42);
        Watched other{std::move(watched)};
        other(1337);
        watched = std::move(other);
    }
}

TEST_CASE(
    "RelocationWatcher watched can be used on interface-mocks.",
    "[object-watcher][object-watcher::relocation]")
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

    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<Watched<Derived, RelocationWatcher>>);
    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<Watched<Derived, RelocationWatcher>>);

    Watched<Derived, LifetimeWatcher> watched{};

    MIMICPP_SCOPED_EXPECTATION watched.expect_destruct();
    MIMICPP_SCOPED_EXPECTATION watched.foo_.expect_call();

    watched.foo();
    Watched other{std::move(watched)};
}

TEST_CASE(
    "Violations of RelocationWatcher watched interface-implementations will be detected.",
    "[object-watcher][object-watcher::lifetime]")
{
    class Interface
    {
    public:
        ~Interface() = default;
        Interface() = default;

        Interface(const Interface&) = delete;
        Interface& operator=(const Interface&) = delete;

        Interface(Interface&&) noexcept(false)
        {
        }

        Interface& operator=(Interface&&) noexcept(false)
        {
            return *this;
        }
    };

    class Derived
        : public Interface
    {
    };

    Watched<Derived, RelocationWatcher> watched{};
    STATIC_REQUIRE(!std::is_nothrow_move_constructible_v<decltype(watched)>);
    STATIC_REQUIRE(!std::is_nothrow_move_assignable_v<decltype(watched)>);

    ScopedReporter reporter{};

    REQUIRE_THROWS_AS(
        Watched{std::move(watched)},
        NoMatchError);
}

TEST_CASE(
    "Watched sets up an appropriate name for each watcher Mock.",
    "[object-watcher]")
{
    struct my_base
    {
    };

    volatile const auto expectation = std::invoke(
        []() -> ScopedExpectation {
            Watched<
                my_base,
                LifetimeWatcher,
                RelocationWatcher>
                watched{};

            ScopedExpectation exp = watched.expect_relocate()
                                and expect::never();
            REQUIRE_THAT(
                exp.mock_name(),
                Catch::Matchers::StartsWith("RelocationWatcher for ")
                    && Catch::Matchers::EndsWith("my_base"));

            exp = watched.expect_destruct();
            REQUIRE_THAT(
                exp.mock_name(),
                Catch::Matchers::StartsWith("LifetimeWatcher for ")
                    && Catch::Matchers::EndsWith("my_base"));

            return exp;
        });
}

#ifdef __APPLE__
TEST_CASE(
    "Watched omits the forwarding functions stacktrace entry.",
    "[!mayfail][mock][mock::interface]")
#else
TEST_CASE(
    "Watched omits the forwarding functions stacktrace entry.",
    "[mock][mock::interface]")
#endif
{
    struct my_base
    {
    };

    ScopedReporter reporter{};

    const auto check_stacktrace = [](const Stacktrace& stacktrace, const std::source_location& before, const std::source_location& after) {
        CHECKED_IF(!stacktrace.empty())
        {
            INFO("stacktrace:\n"
                 << print(stacktrace));

            REQUIRE_THAT(
                stacktrace.source_file(0u),
                Catch::Matchers::Equals(before.file_name()));
            // there is no straight-forward way to check the description
            REQUIRE(before.line() < stacktrace.source_line(0u));
            // strict < fails on some compilers
            REQUIRE(stacktrace.source_line(0u) <= after.line());
        }
    };

    SECTION("When LifetimeWatcher is used.")
    {
        using WatchedT = Watched<my_base, LifetimeWatcher>;
        WatchedT watched{};
        MIMICPP_SCOPED_EXPECTATION watched.expect_destruct();

        constexpr std::source_location before = std::source_location::current();
        {
            WatchedT other{std::move(watched)};
        }
        constexpr std::source_location after = std::source_location::current();

        const CallReport& report = std::get<0>(reporter.full_match_reports().front());
        check_stacktrace(report.stacktrace, before, after);
    }

    SECTION("When RelocationWatcher is used.")
    {
        using WatchedT = Watched<my_base, RelocationWatcher>;
        WatchedT watched{};
        MIMICPP_SCOPED_EXPECTATION watched.expect_relocate();

        SECTION("When move constructing.")
        {
            constexpr auto before = std::source_location::current();
            WatchedT other{std::move(watched)};
            constexpr auto after = std::source_location::current();

            const CallReport& report = std::get<0>(reporter.full_match_reports().front());
            check_stacktrace(report.stacktrace, before, after);
        }

        SECTION("When move assigning.")
        {
            WatchedT other{};

            constexpr auto before = std::source_location::current();
            other = std::move(watched);
            constexpr auto after = std::source_location::current();

            const CallReport& report = std::get<0>(reporter.full_match_reports().front());
            check_stacktrace(report.stacktrace, before, after);
        }
    }
}
