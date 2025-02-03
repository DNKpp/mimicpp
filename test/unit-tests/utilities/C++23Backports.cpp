//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/utilities/C++23Backports.hpp"

#include <concepts>
#include <cstdint>
#include <limits>

using namespace mimicpp;

TEMPLATE_TEST_CASE(
    "to_underlying converts the enum value to its underlying representation.",
    "[utility]",
    std::int8_t,
    std::uint8_t,
    std::int16_t,
    std::uint16_t,
    std::int32_t,
    std::uint32_t,
    std::int64_t,
    std::uint64_t)
{
    using UnderlyingT = TestType;

    SECTION("When an enum value is given.")
    {
        enum Test : UnderlyingT
        {
        };

        const UnderlyingT value = GENERATE(
            std::numeric_limits<UnderlyingT>::min(),
            0,
            1,
            std::numeric_limits<UnderlyingT>::max());

        STATIC_REQUIRE(std::same_as<UnderlyingT, decltype(util::to_underlying(Test{value}))>);
        REQUIRE(value == util::to_underlying(Test{value}));
    }

    SECTION("When an class enum value is given.")
    {
        enum class Test : UnderlyingT
        {
        };

        const UnderlyingT value = GENERATE(
            std::numeric_limits<UnderlyingT>::min(),
            0,
            1,
            std::numeric_limits<UnderlyingT>::max());

        STATIC_REQUIRE(std::same_as<UnderlyingT, decltype(util::to_underlying(Test{value}))>);
        REQUIRE(value == util::to_underlying(Test{value}));
    }
}
