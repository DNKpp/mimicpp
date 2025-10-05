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
