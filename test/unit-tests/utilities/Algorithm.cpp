//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/Algorithm.hpp"

using namespace mimicpp;

TEST_CASE(
    "util::partition_by on empty ranges has no effect.",
    "[util][util::partition_by]")
{
    std::vector<std::string> target{};
    std::vector<int> control{};

    std::ranges::subrange const result = util::partition_by(target, control, std::bind_front(std::equal_to{}, 42));
    REQUIRE_THAT(
        result,
        Catch::Matchers::IsEmpty());
    REQUIRE_THAT(
        target,
        Catch::Matchers::IsEmpty());
    REQUIRE_THAT(
        control,
        Catch::Matchers::IsEmpty());
}

TEST_CASE(
    "util::partition_by supports ranges with just one element.",
    "[util][util::partition_by]")
{
    std::vector<std::string> target{"Hello, World!"};
    std::vector control{42};

    SECTION("When element satisfies the predicate, it's not part of the result range.")
    {
        std::ranges::subrange const result = util::partition_by(target, control, std::bind_front(std::equal_to{}, 42));
        REQUIRE_THAT(
            result,
            Catch::Matchers::IsEmpty());
    }

    SECTION("When element does not satisfy the predicate, it's part of the result range.")
    {
        std::ranges::subrange const result = util::partition_by(target, control, std::bind_front(std::not_equal_to{}, 42));
        REQUIRE_THAT(
            result,
            Catch::Matchers::RangeEquals(std::vector{"Hello, World!"}));
    }

    REQUIRE_THAT(
        target,
        Catch::Matchers::RangeEquals(std::vector{"Hello, World!"}));
    REQUIRE_THAT(
        control,
        Catch::Matchers::RangeEquals(std::vector{42}));
}

TEST_CASE(
    "util::partition_by supports ranges with multiple elements.",
    "[util][util::partition_by]")
{
    std::vector<std::string> target{"Test1", "Test2", "Test3"};
    std::vector control{1, 2, 3};

    SECTION("When none of the elements satisfy the predicate, they are all listed in the result range.")
    {
        std::ranges::subrange const result = util::partition_by(target, control, std::bind_front(std::less{}, 10));
        REQUIRE_THAT(
            result,
            Catch::Matchers::UnorderedRangeEquals(std::vector{"Test1", "Test2", "Test3"}));
    }

    SECTION("When all elements satisfy the predicate, none of them is listed in the result range.")
    {
        std::ranges::subrange const result = util::partition_by(target, control, std::bind_front(std::greater{}, 10));
        REQUIRE_THAT(
            result,
            Catch::Matchers::IsEmpty());
        REQUIRE_THAT(
            target,
            Catch::Matchers::RangeEquals(std::vector{"Test1", "Test2", "Test3"}));
        REQUIRE_THAT(
            control,
            Catch::Matchers::RangeEquals(std::vector{1, 2, 3}));
    }

    SECTION("When some of the elements do not satisfy the predicate, these are listed in the result range.")
    {
        std::ranges::subrange const result = util::partition_by(target, control, [](int const val) { return 0 == val % 2; });
        REQUIRE_THAT(
            result,
            Catch::Matchers::UnorderedRangeEquals(std::vector{"Test1", "Test3"}));
        REQUIRE_THAT(
            target | std::views::drop(1),
            Catch::Matchers::RangeEquals(result));
        REQUIRE_THAT(
            target | std::views::take(1),
            Catch::Matchers::RangeEquals(std::vector{"Test2"}));
        REQUIRE_THAT(
            control | std::views::drop(1),
            Catch::Matchers::UnorderedRangeEquals(std::vector{1, 3}));
        REQUIRE_THAT(
            control | std::views::take(1),
            Catch::Matchers::RangeEquals(std::vector{2}));
    }

    REQUIRE_THAT(
        target,
        Catch::Matchers::UnorderedRangeEquals(std::vector{"Test1", "Test2", "Test3"}));
    REQUIRE_THAT(
        control,
        Catch::Matchers::UnorderedRangeEquals(std::vector{1, 2, 3}));
}

TEST_CASE(
    "util::find_closing_token returns end iterator, when no corresponding closing-token can be found.",
    "[util][util::find_closing_token]")
{
    SECTION("When string is empty.")
    {
        constexpr std::string_view str{};

        auto const iter = util::find_closing_token(str, '(', ')');

        CHECK(iter == str.cend());
    }

    SECTION("When string does not contain suffix-pattern.")
    {
        auto const str = GENERATE(
            as<std::string_view>{},
            "a",
            "(",
            "a(",
            "a(b",
            "abc");
        CAPTURE(str);

        auto const iter = util::find_closing_token(str, '(', ')');

        CHECK(iter == str.cend());
    }

    SECTION("When string contains at least as many prefix-patterns as suffix-patterns.")
    {
        auto const str = GENERATE(
            as<std::string_view>{},
            "()",
            "()(",
            "((()))",
            "(())() ()",
            "abc()d(ef)");
        CAPTURE(str);

        auto const iter = util::find_closing_token(str, '(', ')');

        CHECK(iter == str.cend());
    }
}

TEST_CASE(
    "util::find_closing_token returns the iterator to the closing-token.",
    "[util][util::find_closing_token]")
{
    auto [expectedIndex, str] = GENERATE(
        (table<std::size_t, std::string_view>)({
            {0,          ")"},
            {0,        ")()"},
            {7, "(abc)de)()"},
            {4,      "(()))"}
    }));
    CAPTURE(str, expectedIndex);

    auto const iter = util::find_closing_token(str, '(', ')');

    CHECK(iter == str.cbegin() + expectedIndex);
    CHECK(')' == *iter);
}

TEST_CASE(
    "util::find_next_comma returns the iterator to the next comma.",
    "[util][util::find_next_comma]")
{
    auto [expectedIndex, str] = GENERATE(
        (table<std::size_t, std::string_view>)({
            { 0,                  ","},
            { 1,                 " ,"},
            { 5,          "(abc), ()"},
            {13, "(, {}<,>) [,], abc"}
    }));
    CAPTURE(str, expectedIndex);

    auto const iter = util::find_next_comma(str);

    CHECK(iter == str.cbegin() + expectedIndex);
    CHECK(',' == *iter);
}

TEST_CASE(
    "util::find_next_comma returns end-iterator, when no next comma exists.",
    "[util][util::find_next_comma]")
{
    auto const str = GENERATE(
        as<std::string_view>{},
        "",
        "abc",
        "{,}",
        "(,)",
        "[,]",
        "<,>");
    CAPTURE(str);

    auto const iter = util::find_next_comma(str);

    CHECK(iter == str.cend());
}
