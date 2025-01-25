//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/PathPrinter.hpp"

using namespace mimicpp;

namespace
{
    using PathT = std::filesystem::path;

    [[nodiscard]]
    PathT make_path(PathT path)
    {
        path.make_preferred();
        return path;
    }
}

TEST_CASE(
    "print_path prints just the last 3 path elements.",
    "[print]")
{
    REQUIRE_THAT(
        print_path("foo"),
        Catch::Matchers::Equals(make_path("foo").string()));
    REQUIRE_THAT(
        print_path("foo.cpp"),
        Catch::Matchers::Equals(make_path("foo.cpp").string()));

    REQUIRE_THAT(
        print_path("abc/foo.cpp"),
        Catch::Matchers::Equals(make_path("abc/foo.cpp").string()));
    REQUIRE_THAT(
        print_path("/abc/foo.cpp"),
        Catch::Matchers::Equals(make_path("/abc/foo.cpp").string()));

    REQUIRE_THAT(
        print_path("abc/def/foo.cpp"),
        Catch::Matchers::Equals(make_path("abc/def/foo.cpp").string()));
    REQUIRE_THAT(
        print_path("/abc/def/foo.cpp"),
        Catch::Matchers::Equals(make_path("abc/def/foo.cpp").string()));

    REQUIRE_THAT(
        print_path("abc/def/g/h/i/foo.cpp"),
        Catch::Matchers::Equals(make_path("h/i/foo.cpp").string()));

    REQUIRE_THAT(
        print_path("abc//def\\foo.cpp"),
        Catch::Matchers::Equals(make_path("abc/def/foo.cpp").string()));
    REQUIRE_THAT(
        print_path("C://abc/foo.cpp"),
        Catch::Matchers::Equals(make_path("C:/abc/foo.cpp").string()));
}
