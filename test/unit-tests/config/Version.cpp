//          Copyright Dominic (DNKpp) Koepke 2025 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Config/Version.hpp"

TEST_CASE(
    "MIMICPP_VERSION_MAJOR is an integral number.",
    "[config]")
{
    constexpr int version{MIMICPP_VERSION_MAJOR};
    STATIC_CHECK(0 <= version);
}

TEST_CASE(
    "MIMICPP_VERSION_MINOR is an integral number.",
    "[config]")
{
    constexpr int version{MIMICPP_VERSION_MINOR};
    STATIC_CHECK(0 <= version);
}

TEST_CASE(
    "MIMICPP_VERSION_PATCH is an integral number.",
    "[config]")
{
    constexpr int version{MIMICPP_VERSION_PATCH};
    STATIC_CHECK(0 <= version);
}

TEST_CASE(
    "MIMICPP_VERSION_PATCH is a string-literal, containing a dotted triple.",
    "[config]")
{
    std::string const expected = mimicpp::format::format(
        "{}.{}.{}",
        MIMICPP_VERSION_MAJOR,
        MIMICPP_VERSION_MINOR,
        MIMICPP_VERSION_PATCH);

    CHECK_THAT(
        std::string{MIMICPP_VERSION},
        Catch::Matchers::Equals(expected));
}

TEST_CASE(
    "Accumulated version is greater 0.",
    "[config]")
{
    constexpr int accumulatedVersion{MIMICPP_VERSION_MAJOR + MIMICPP_VERSION_MINOR + MIMICPP_VERSION_PATCH};
    STATIC_CHECK(0 < accumulatedVersion);
}
