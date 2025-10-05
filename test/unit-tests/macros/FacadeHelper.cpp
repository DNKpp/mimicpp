//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/macros/FacadeHelper.hpp"
#include "mimic++/Facade.hpp"

TEST_CASE(
    "MIMICPP_DETAIL_MAKE_SIGNATURE_LIST creates a list of signatures from the given arguments.",
    "[facade][detail]")
{
    STATIC_REQUIRE(
        std::same_as<
            std::tuple<>,
            std::tuple<MIMICPP_DETAIL_MAKE_SIGNATURE_LIST()>>);

    STATIC_REQUIRE(
        std::same_as<
            std::tuple<void()>,
            std::tuple<MIMICPP_DETAIL_MAKE_SIGNATURE_LIST((void, (), , ))>>);

    STATIC_REQUIRE(
        std::same_as<
            std::tuple<const int&(float&&) const noexcept, void()>,
            std::tuple<MIMICPP_DETAIL_MAKE_SIGNATURE_LIST(
                (const int&, (float&&), const noexcept, ),
                (void, (), , ))>>);
}

TEST_CASE(
    "MIMICPP_DETAIL_MAKE_PARAM_LIST creates the param list for the given types.",
    "[mock][mock::interface]")
{
    namespace Matches = Catch::Matchers;

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST()),
        Matches::Equals(""));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST(int)),
        Matches::Equals("int arg_i"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST(const int&, int&&)),
        Matches::Matches("const int& arg_i\\s*, int&& arg_ii"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST(int, int)),
        Matches::Matches("int arg_i\\s*, int arg_ii"));

    REQUIRE_THAT(
        MIMICPP_DETAIL_STRINGIFY(MIMICPP_DETAIL_MAKE_PARAM_LIST((std::tuple<int, float>))),
        Matches::Equals("std::tuple<int, float> arg_i"));
}
