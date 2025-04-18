//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/Algorithm.hpp"

using namespace mimicpp;

TEST_CASE(
    "util::partition_by on empty ranges has no effect.",
    "[util][util::algorithm]")
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
    "[util][util::algorithm]")
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
    "[util][util::algorithm]")
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
    "[util][util::algorithm]")
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
    "[util][util::algorithm]")
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
    "util::find_next_unwrapped_token returns a subrange to the next token.",
    "[util][util::algorithm]")
{
    constexpr std::array openingBrackets{'<', '(', '[', '{'};
    constexpr std::array closingBrackets{'>', ')', ']', '}'};
    auto [expectedIndex, str] = GENERATE(
        (table<std::size_t, std::string_view>)({
            { 0,                  ","},
            { 1,                 " ,"},
            { 5,          "(abc), ()"},
            {13, "(, {}<,>) [,], abc"}
    }));
    CAPTURE(str, expectedIndex);

    auto const token = util::find_next_unwrapped_token(str, ",", openingBrackets, closingBrackets);

    CHECK(token.begin() == str.cbegin() + expectedIndex);
    CHECK(token.end() == str.cbegin() + expectedIndex + 1);
    CHECK(',' == *token.begin());
}

TEST_CASE(
    "util::find_next_unwrapped_token can handle tokens, which are part of either collection.",
    "[util][util::algorithm]")
{
    constexpr std::array openingBrackets{'<', '(', '[', '{'};
    constexpr std::array closingBrackets{'>', ')', ']', '}'};
    auto [expectedIndex, token, str] = GENERATE(
        (table<std::size_t, std::string_view, std::string_view>)({
            { 0, "(",                    "()"},
            { 1, ")",                    "()"},
            { 1, "(",        " (, [(abc)] ) "},
            {12, ")",        " (, [(abc)] ) "},
            {15, "<", "(, {}<,>) [,], <> abc"},
            {16, ">", "(, {}<,>) [,], <> abc"}
    }));
    CAPTURE(str, token, expectedIndex);

    auto const match = util::find_next_unwrapped_token(str, token, openingBrackets, closingBrackets);

    CHECK(match.begin() == str.cbegin() + expectedIndex);
    CHECK(match.end() == str.cbegin() + expectedIndex + 1);
    CHECK_THAT(
        (std::string{match.begin(), match.end()}),
        Catch::Matchers::Equals(std::string{token}));
}

TEST_CASE(
    "util::find_next_unwrapped_token returns {end, end}, if no next unwrapped token exists.",
    "[util][util::algorithm]")
{
    constexpr std::array openingBrackets{'<', '(', '[', '{'};
    constexpr std::array closingBrackets{'>', ')', ']', '}'};
    auto const str = GENERATE(
        as<std::string_view>{},
        "",
        "abc",
        "{,}",
        "(,)",
        "[,]",
        "<,>");
    CAPTURE(str);

    auto const token = util::find_next_unwrapped_token(str, ",", openingBrackets, closingBrackets);

    CHECK(token.begin() == str.cend());
    CHECK(token.end() == str.cend());
}

TEST_CASE(
    "util::prefix_range returns the subrange to the elements, which have the given prefix.",
    "[util][util::algorithm]")
{
    SECTION("Empty collection is supported.")
    {
        std::vector<std::string> const collection{};

        auto const result = util::prefix_range(collection, StringViewT{"foo"});

        CHECK_THAT(
            result,
            Catch::Matchers::IsEmpty());
    }

    SECTION("Empty prefix is supported.")
    {
        std::vector<std::string> const collection{"bar", "bfoo"};

        auto const result = util::prefix_range(collection, StringViewT{});

        CHECK(collection.cbegin() == result.begin());
        CHECK_THAT(
            result,
            Catch::Matchers::RangeEquals(std::vector{"bar", "bfoo"}));
    }

    SECTION("When no element starts with prefix.")
    {
        std::vector<std::string> const collection{"bar", "bfoo"};

        auto const result = util::prefix_range(collection, StringViewT{"foo"});

        CHECK_THAT(
            result,
            Catch::Matchers::IsEmpty());
    }

    SECTION("When some elements starts with prefix.")
    {
        std::vector<std::string> const collection{"a foo", "foo", "foo-bar", "foofoo", "no-foo"};

        auto const result = util::prefix_range(collection, StringViewT{"foo"});

        CHECK(collection.cbegin() + 1 == result.begin());
        CHECK_THAT(
            result,
            Catch::Matchers::RangeEquals(std::vector{"foo", "foo-bar", "foofoo"}));
    }
}

TEST_CASE(
    "util::concat_arrays concatenates an arbitrary amount of arrays.",
    "[util][util::algorithm]")
{
    SECTION("Single array is supported.")
    {
        constexpr std::array source{42, 1337};

        constexpr auto result = util::concat_arrays(source);
        STATIC_CHECK(std::same_as<int, std::ranges::range_value_t<decltype(result)>>);

        CHECK_THAT(
            result,
            Catch::Matchers::RangeEquals(source));
    }

    SECTION("Two arrays are supported.")
    {
        constexpr std::array first{42, 1337};
        constexpr std::array second{-42, -1337, 42};

        auto const result = util::concat_arrays(first, second);
        STATIC_CHECK(std::same_as<int, std::ranges::range_value_t<decltype(result)>>);

        CHECK_THAT(
            result,
            Catch::Matchers::RangeEquals(std::array{42, 1337, -42, -1337, 42}));
    }

    SECTION("Arbitrary amount of arrays are supported.")
    {
        constexpr std::array first{42, 1337};
        constexpr std::array second{-42, -1337, 42};

        auto const result = util::concat_arrays(first, second, first);
        STATIC_CHECK(std::same_as<int, std::ranges::range_value_t<decltype(result)>>);

        CHECK_THAT(
            result,
            Catch::Matchers::RangeEquals(std::array{42, 1337, -42, -1337, 42, 42, 1337}));
    }

    SECTION("Empty arrays are supported.")
    {
        constexpr std::array<int, 0> source{};

        auto const result = util::concat_arrays(source, source);
        STATIC_CHECK(std::same_as<int, std::ranges::range_value_t<decltype(result)>>);

        CHECK_THAT(
            result,
            Catch::Matchers::IsEmpty());
    }
}

TEST_CASE(
    "util::binary_find finds the required element in the container.",
    "[util][util::algorithm]")
{
    SECTION("When container contains just a single element.")
    {
        std::vector const collection = {42};

        auto const result = util::binary_find(collection, 42);

        CHECK(result == collection.cbegin());
    }

    SECTION("When value is first element.")
    {
        std::vector const collection = {42, 1337, 1338};

        auto const result = util::binary_find(collection, 42);

        CHECK(result == collection.cbegin());
    }

    SECTION("When value is last element.")
    {
        std::vector const collection = {42, 1337, 1338};

        auto const result = util::binary_find(collection, 1338);

        CHECK(result == collection.cbegin() + 2);
    }

    SECTION("When value is somewhere in the middle.")
    {
        std::vector const collection = {42, 1337, 1338};

        auto const result = util::binary_find(collection, 1337);

        CHECK(result == collection.cbegin() + 1);
    }
}

TEST_CASE(
    "util::binary_find returns end-iterator, when element is not contained.",
    "[util][util::algorithm]")
{
    SECTION("When container is empty.")
    {
        std::vector<int> const collection{};

        auto const result = util::binary_find(collection, 42);

        CHECK(result == collection.cend());
    }

    SECTION("When container is not empty, but value is not contained.")
    {
        std::vector const collection = {42, 1337, 1338};
        auto const value = GENERATE(-1, 0, 43, 1336, 1339);

        auto const result = util::binary_find(collection, value);

        CHECK(result == collection.cend());
    }
}

TEST_CASE(
    "util::contains determines, whether the specified value is contained.",
    "[util][util::algorithm]")
{
    SECTION("When container is empty.")
    {
        std::vector<int> const collection{};

        CHECK(!util::contains(collection, 42));
    }

    SECTION("When container contains just a single element.")
    {
        std::vector const collection = {42};

        CHECK(util::contains(collection, 42));
        CHECK(!util::contains(collection, 41));
    }

    SECTION("When container contains multiple elements.")
    {
        std::vector const collection = {42, 1337, 1338};

        CHECK(util::contains(collection, 42));
        CHECK(util::contains(collection, 1337));
        CHECK(util::contains(collection, 1338));

        CHECK(!util::contains(collection, 41));
        CHECK(!util::contains(collection, 1339));
    }
}
