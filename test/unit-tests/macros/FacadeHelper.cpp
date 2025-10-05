//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/macros/FacadeHelper.hpp"
#include "mimic++/Facade.hpp"

using namespace mimicpp;

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

namespace
{
    struct single_pack_helper
    {
        template <typename... Args>
        [[nodiscard]]
        decltype(auto) operator()(Args&&... arg_i)
        {
            return MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE(i, , (Args...));
        }
    };
}

TEST_CASE(
    "MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE constructs a tuple from all kind of args (may be a parameter or a parameter pack).",
    "[mock][mock::interface]")
{
    SECTION("When a single parameter is given.")
    {
        int arg_i{};
        std::tuple t = MIMICPP_DETAIL_FORWARD_ARG_AS_TUPLE(i, , int);
        STATIC_REQUIRE(std::same_as<std::tuple<int&&>, decltype(t)>);

        CHECK(&arg_i == &std::get<0>(t));
    }

    SECTION("When an empty parameter-pack is given.")
    {
        std::tuple t = single_pack_helper{}();
        STATIC_REQUIRE(std::same_as<std::tuple<>, decltype(t)>);
    }

    SECTION("When a non-empty parameter-pack is given.")
    {
        std::string str{};
        int value{42};
        std::tuple t = single_pack_helper{}(str, std::move(value));
        STATIC_REQUIRE(std::same_as<std::tuple<std::string&, int&&>, decltype(t)>);

        CHECK(&str == &std::get<0>(t));
        CHECK(&value == &std::get<1>(t));
    }
}

TEST_CASE(
    "MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE returns a comma separated list of tuples.",
    "[mock][mock::interface]")
{
    SECTION("When no arguments are given.")
    {
        std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE());
        STATIC_REQUIRE(std::same_as<std::tuple<>, decltype(result)>);
    }

    SECTION("When single argument is given.")
    {
        // This has to be named exactly like this, because the macro internally expects that.
        int arg_i{};

        SECTION("By value")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<int&&>>, decltype(result)>);
            CHECK(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By const value")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(const int));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<const int&&>>, decltype(result)>);
            CHECK(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By lvalue-ref.")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int&));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<int&>>, decltype(result)>);
            CHECK(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By const lvalue-ref.")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(const int&));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<const int&>>, decltype(result)>);
            CHECK(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By rvalue-ref.")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int&&));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<int&&>>, decltype(result)>);
            CHECK(&arg_i == &std::get<0>(std::get<0>(result)));
        }

        SECTION("By const rvalue-ref")
        {
            std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(const int&&));
            STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<const int&&>>, decltype(result)>);
            CHECK(&arg_i == &std::get<0>(std::get<0>(result)));
        }
    }

    SECTION("When multiple arguments are given.")
    {
        // This has to be named exactly like this, because the macro internally expects that.
        int arg_i{};
        std::string arg_ii{};

        std::tuple result = std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int, std::string&));
        STATIC_REQUIRE(std::same_as<std::tuple<std::tuple<int&&>, std::tuple<std::string&>>, decltype(result)>);
        auto& [param0, param1] = result;
        CHECK(&arg_i == &std::get<0>(param0));
        CHECK(&arg_ii == &std::get<0>(param1));
    }

    SECTION("When parameter pack is given.")
    {
        SECTION("As only argument.")
        {
            const auto packHelper = [&]<typename... Args>(Args&&... arg_i) {
                return MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(Args...);
            };

            std::string str{};
            int value{42};

            std::tuple result = packHelper(str, std::move(value));
            STATIC_REQUIRE(std::same_as<std::tuple<std::string&, int&&>, decltype(result)>);
            CHECK(&str == &std::get<0>(result));
            CHECK(&value == &std::get<1>(result));
        }

        SECTION("As front argument.")
        {
            int arg_ii{};
            const auto packHelper = [&]<typename... Args>(Args&&... arg_i) {
                return std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(Args..., int&));
            };

            std::string str{};
            int value{42};

            std::tuple result = packHelper(str, std::move(value));
            STATIC_REQUIRE(
                std::same_as<
                    std::tuple<std::tuple<std::string&, int&&>, std::tuple<int&>>,
                    decltype(result)>);
            auto& [param0, param1] = result;
            CHECK(&str == &std::get<0>(param0));
            CHECK(&value == &std::get<1>(param0));
            CHECK(&arg_ii == &std::get<0>(param1));
        }

        SECTION("As back argument.")
        {
            int arg_i{};
            const auto packHelper = [&]<typename... Args>(Args&&... arg_ii) {
                return std::make_tuple(MIMICPP_DETAIL_FORWARD_ARGS_AS_TUPLE(int&, Args...));
            };

            std::string str{};
            int value{42};

            std::tuple result = packHelper(str, std::move(value));
            STATIC_REQUIRE(
                std::same_as<
                    std::tuple<std::tuple<int&>, std::tuple<std::string&, int&&>>,
                    decltype(result)>);
            auto& [param0, param1] = result;
            CHECK(&str == &std::get<0>(param1));
            CHECK(&value == &std::get<1>(param1));
            CHECK(&arg_i == &std::get<0>(param0));
        }
    }
}
