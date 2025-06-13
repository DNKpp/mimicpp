//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/SourceLocation.hpp"

using namespace mimicpp;

TEST_CASE(
    "util::SourceLocation is default constructible.",
    "[util]")
{
    util::SourceLocation constexpr before{};
    util::SourceLocation constexpr loc{};
    util::SourceLocation constexpr after{};

    CHECK_THAT(
        std::string{loc.file_name()},
        Catch::Matchers::Equals(std::string{before.file_name()}));
    CHECK_THAT(
        std::string{loc.function_name()},
        Catch::Matchers::Equals(std::string{before.function_name()}));
    CHECK(before.line() < loc.line());
    CHECK(loc.line() < after.line());
}

TEST_CASE(
    "util::SourceLocation can be constructed with a particular source-location.",
    "[util]")
{
    using traits = util::source_location::backend_traits<util::source_location::InstalledBackend>;
    auto constexpr source = traits::current();
    util::SourceLocation constexpr loc{source};

    CHECK_THAT(
        std::string{loc.file_name()},
        Catch::Matchers::Equals(source.file_name()));
    CHECK_THAT(
        std::string{loc.function_name()},
        Catch::Matchers::Equals(source.function_name()));
    CHECK(source.line() == loc.line());
}

TEST_CASE(
    "util::SourceLocation is equality-comparable.")
{
    SECTION("Compares equal, when both sides denote the same source-location.")
    {
        util::SourceLocation constexpr loc{};
        util::SourceLocation constexpr other{loc};

        CHECK(loc == other);
        CHECK(other == loc);
        CHECK_FALSE(loc != other);
        CHECK_FALSE(other != loc);
    }

    SECTION("Compares unequal, when line differs.")
    {
        util::SourceLocation constexpr loc{};
        util::SourceLocation constexpr other{};
        REQUIRE(other.line() != loc.line());

        CHECK_FALSE(loc == other);
        CHECK_FALSE(other == loc);
        CHECK(loc != other);
        CHECK(other != loc);
    }

    SECTION("Compares unequal, when function differs.")
    {
        util::SourceLocation constexpr loc{};
        auto constexpr other = std::invoke(
            [] {
                return util::SourceLocation{};
            });

        CHECK_FALSE(loc == other);
        CHECK_FALSE(other == loc);
        CHECK(loc != other);
        CHECK(other != loc);
    }

    SECTION("Compares unequal, when source-file differs.")
    {
        // trust me, bro
    }
}
