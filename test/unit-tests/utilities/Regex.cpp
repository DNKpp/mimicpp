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
