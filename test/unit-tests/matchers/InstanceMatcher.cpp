//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/matchers/GeneralMatchers.hpp"

#include <TestTypes.hpp>

using namespace mimicpp;

TEMPLATE_TEST_CASE(
    "matches::instance does only accept lvalues.",
    "[matcher]",
    int,
    const int,
    int&&,
    const int&&)
{
    STATIC_REQUIRE(!requires { matches::instance(std::declval<TestType>()); });
}

TEST_CASE(
    "matches::instance matches when the target is the expected instance.",
    "[matcher]")
{
    int instance{42};
    const auto matcher = matches::instance(instance);

    auto withDescription = [](StringT regex) {
        return [regex = std::move(regex)](auto const& value) {
            UNSCOPED_CAPTURE(value.description);
            REQUIRE(value.description);
            CHECK_THAT(*value.description, Catch::Matchers::Matches(regex));
            return true;
        };
    };

    SECTION("When target is the instance.")
    {
        CHECK_THAT(
            matcher.matches(instance),
            variant_matches<expectation::MatchSuccess>(withDescription("is instance at 0x[\\dAaBbCcDdEeFf]{1,16}")));
    }

    SECTION("When target is not the instance.")
    {
        constexpr int target{};
        CHECK_THAT(
            matcher.matches(target),
            variant_matches<expectation::MatchFailure>(
                withDescription("is instance at 0x[\\dAaBbCcDdEeFf]{1,16}, but actually 0x[\\dAaBbCcDdEeFf]{1,16}")));
    }

    SECTION("Matcher can be inverted.")
    {
        const auto invertedMatcher = !matches::instance(instance);

        SECTION("When target is not the instance.")
        {
            constexpr int target{};
            CHECK_THAT(
                invertedMatcher.matches(target),
                variant_matches<expectation::MatchSuccess>(
                    withDescription(R"(not \(is instance at 0x[\dAaBbCcDdEeFf]{1,16}\))")));
        }

        SECTION("When target is the instance.")
        {
            CHECK_THAT(
                invertedMatcher.matches(instance),
                variant_matches<expectation::MatchFailure>(
                    withDescription(R"(not \(is instance at 0x[\dAaBbCcDdEeFf]{1,16}\), but actually 0x[\dAaBbCcDdEeFf]{1,16})")));
        }
    }
}

TEST_CASE(
    "matches::instance supports type hierarchies.",
    "[matcher]")
{
    struct base
    {
    };

    struct derived
        : public base
    {
    };

    constexpr derived object{};
    auto const matcher = matches::instance(object);

    SECTION("Matches, when same instance is provided.")
    {
        CHECK_THAT(
            matcher.matches(static_cast<base const&>(object)),
            variant_holds_alternative<expectation::MatchSuccess>());
    }

    SECTION("Does not match, when other instance is provided.")
    {
        constexpr derived other{};
        CHECK_THAT(
            matcher.matches(static_cast<base const&>(other)),
            variant_holds_alternative<expectation::MatchFailure>());
    }
}

#if MIMICPP_CONFIG_USE_FMT

    #include <fmt/ranges.h> // required: this is where fmt's range detection, and so the ADL begin/end lookup, lives

namespace
{
    struct PtrRange
    {
        int* data(); // deliberately non-const
    };

    // Found through ADL by fmt's range detection; the body is instantiated to deduce the return type.
    template <typename T>
        requires std::same_as<std::remove_const_t<T>, PtrRange>
    auto begin(T* ptr)
    {
        return ptr->data(); // OK for PtrRange*, hard error for PtrRange const*
    }
}

TEST_CASE(
    "matches::instance does not fall for ADL-tricks with custom types.",
    "[matcher]")
{
    PtrRange obj{};
    auto const matcher = matches::instance(obj);

    CHECK_THAT(
        matcher.matches(std::as_const(obj)),
        variant_holds_alternative<expectation::MatchSuccess>());
}

#endif
