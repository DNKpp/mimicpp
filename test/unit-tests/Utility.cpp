//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Utility.hpp"

#include <cstdint>
#include <tuple>

using namespace mimicpp;

TEST_CASE(
    "detail::expand_tuple appends elements, until the desired size is reached.",
    "[detail][utility]")
{
    // on clang-19, libc++ and c++23, the direct tuple comparisons won't compile.
    // so use std::equal_to as workaround.
    // see: https://github.com/llvm/llvm-project/issues/113087

    SECTION("When tuple is empty")
    {
        std::tuple input{};

        SECTION("And n == 0")
        {
            const std::tuple result = detail::expand_tuple<int, 0>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<>, decltype(result)>);
        }

        SECTION("And n == 1")
        {
            const std::tuple result = detail::expand_tuple<int, 1>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<int>, decltype(result)>);
            REQUIRE(std::equal_to{}(std::tuple{0}, result));
        }

        SECTION("And n == 2")
        {
            const std::tuple result = detail::expand_tuple<int, 2>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<int, int>, decltype(result)>);
            REQUIRE(std::equal_to{}(std::tuple{0, 0}, result));
        }
    }

    SECTION("When tuple has size 1")
    {
        std::tuple input{42.};

        SECTION("And n == 1")
        {
            const std::tuple result = detail::expand_tuple<int, 1>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<double>, decltype(result)>);
        }

        SECTION("And n == 2")
        {
            const std::tuple result = detail::expand_tuple<int, 2>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<double, int>, decltype(result)>);
            REQUIRE(std::equal_to{}(std::tuple{42., 0}, result));
        }

        SECTION("And n == 3")
        {
            const std::tuple result = detail::expand_tuple<int, 3>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<double, int, int>, decltype(result)>);
            REQUIRE(std::equal_to{}(std::tuple{42., 0, 0}, result));
        }
    }

    SECTION("When tuple has size 2")
    {
        std::tuple input{1337., 42.f};

        SECTION("And n == 2")
        {
            const std::tuple result = detail::expand_tuple<int, 2>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<double, float>, decltype(result)>);
            REQUIRE(std::equal_to{}(std::tuple{1337., 42.f}, result));
        }

        SECTION("And n == 3")
        {
            const std::tuple result = detail::expand_tuple<int, 3>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<double, float, int>, decltype(result)>);
            REQUIRE(std::equal_to{}(std::tuple{1337., 42.f, 0}, result));
        }

        SECTION("And n == 4")
        {
            const std::tuple result = detail::expand_tuple<int, 4>(std::move(input));
            STATIC_REQUIRE(std::same_as<const std::tuple<double, float, int, int>, decltype(result)>);
            REQUIRE(std::equal_to{}(std::tuple{1337., 42.f, 0, 0}, result));
        }
    }

    SECTION("It keeps the existing elements as they are.")
    {
        std::string arg{"Hello, World"};

        SECTION("When argument is const.")
        {
            const std::tuple result = detail::expand_tuple<int, 1>(std::tuple<const std::string>(arg));
            STATIC_REQUIRE(std::same_as<const std::tuple<const std::string>, decltype(result)>);
            REQUIRE(arg == std::get<0>(result));
        }

        SECTION("When argument is lvalue-ref.")
        {
            const std::tuple result = detail::expand_tuple<int, 1>(std::tuple<std::string&>(arg));
            STATIC_REQUIRE(std::same_as<const std::tuple<std::string&>, decltype(result)>);
            REQUIRE(&arg == &std::get<0>(result));
        }

        SECTION("When argument is const lvalue-ref.")
        {
            const std::tuple result = detail::expand_tuple<int, 1>(std::tuple<const std::string&>(arg));
            STATIC_REQUIRE(std::same_as<const std::tuple<const std::string&>, decltype(result)>);
            REQUIRE(&arg == &std::get<0>(result));
        }

        SECTION("When argument is rvalue-ref.")
        {
            const std::tuple result = detail::expand_tuple<int, 1>(std::tuple<std::string&&>(std::move(arg)));
            STATIC_REQUIRE(std::same_as<const std::tuple<std::string&&>, decltype(result)>);
            REQUIRE(&arg == &std::get<0>(result));
        }

        SECTION("When argument is const rvalue-ref.")
        {
            const std::tuple result = detail::expand_tuple<int, 1>(std::tuple<const std::string&&>(std::move(arg)));
            STATIC_REQUIRE(std::same_as<const std::tuple<const std::string&&>, decltype(result)>);
            REQUIRE(&arg == &std::get<0>(result));
        }
    }
}
