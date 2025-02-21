//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/Regex.hpp"

using namespace mimicpp;

TEST_CASE(
    "util::regex_replace_all replaces all regex occurrences with the format-string.",
    "[util]")
{
    SECTION("When an empty string is given, nothing has to be done.")
    {
        auto const result = util::regex_replace_all(std::string{}, std::regex{"\\d"}, std::string_view{});
        REQUIRE_THAT(
            result,
            Catch::Matchers::Equals(""));
    }

    SECTION("When an empty fmt is given, the tokens are just removed.")
    {
        std::string const input{"00Test12t1T123"};
        auto const result = util::regex_replace_all(
            input,
            std::regex{"\\d"},
            std::string_view{});
        REQUIRE_THAT(
            result,
            Catch::Matchers::Equals("TesttT"));
    }

    SECTION("The fmt replaces all (even on the fly introduced) tokens.")
    {
        std::string const input{" >> > > > "};
        auto const result = util::regex_replace_all(
            input,
            std::regex{R"(\>\s+\>)"},
            std::string_view{">>"});
        REQUIRE_THAT(
            result,
            Catch::Matchers::Equals(" >>>>> "));
    }
}

TEST_CASE(
    "util::regex_find_corresponding_suffix returns empty string-view, when no corresponding suffix can be found.",
    "[util]")
{
    SECTION("When string is empty.")
    {
        auto const suffix = util::regex_find_corresponding_suffix(
            std::string_view{},
            std::regex{R"(\()"},
            std::regex{R"(\))"});

        CHECK_THAT(
            suffix,
            Catch::Matchers::IsEmpty());
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

        auto const suffix = util::regex_find_corresponding_suffix(
            str,
            std::regex{R"(\()"},
            std::regex{R"(\))"});

        CHECK_THAT(
            suffix,
            Catch::Matchers::IsEmpty());
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

        auto const suffix = util::regex_find_corresponding_suffix(
            str,
            std::regex{R"(\()"},
            std::regex{R"(\))"});

        CHECK_THAT(
            suffix,
            Catch::Matchers::IsEmpty());
    }
}

TEST_CASE(
    "util::regex_find_corresponding_suffix returns the suffix.",
    "[util]")
{
    auto [expectedIndex, str] = GENERATE(
        (table<std::size_t, std::string_view>)({
            {0,          ")"},
            {0,        ")()"},
            {7, "(abc)de)()"},
            {4,      "(()))"}
    }));
    CAPTURE(str, expectedIndex);

    auto const suffix = util::regex_find_corresponding_suffix(
        str,
        std::regex{R"(\()"},
        std::regex{R"(\))"});

    CHECK_THAT(
        std::string{suffix},
        Catch::Matchers::Equals(")"));
    CHECK(std::cmp_equal(expectedIndex, suffix.data() - str.data()));
}
