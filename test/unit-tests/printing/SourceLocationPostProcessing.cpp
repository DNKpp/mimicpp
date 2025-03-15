//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include <source_location>

using namespace mimicpp;

#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

constexpr auto type_post_processing_lambda_loc = [] {
    return std::source_location::current();
};

constexpr auto type_post_processing_nested_lambda_loc = [] {
    return [] {
        return std::source_location::current();
    }();
};

namespace
{
    [[nodiscard]]
    constexpr std::source_location loc_fun()
    {
        [[maybe_unused]] constexpr auto dummy = [] {};
        [[maybe_unused]] constexpr auto dummy2 = [] {};
        constexpr auto inner = [] {
            return std::source_location::current();
        };
        [[maybe_unused]] constexpr auto dummy3 = [] {};

        return inner();
    }

    [[nodiscard]]
    constexpr std::source_location loc_anon_lambda_fun()
    {
        return [] {
            return std::source_location::current();
        }();
    }

    constexpr auto my_typeLambda = [] {
        struct my_type
        {
        };

        return my_type{};
    };

    template <typename... Ts>
    struct my_template
    {
        struct my_type
        {
        };

        std::source_location foo(my_type)
        {
            return std::source_location::current();
        }

        auto bar(my_type const&, std::source_location* outLoc)
        {
            if (outLoc)
            {
                *outLoc = std::source_location::current();
            }

            struct bar_type
            {
            };

            return bar_type{};
        }
    };

    StringT const topLevelLambdaPattern =
        R"((\(anonymous class\)))";

    StringT const lambdaCallOpPattern = topLevelLambdaPattern + R"(::operator\(\))";

    StringT const anonNsScopePattern = R"(\{anon-ns\}::)";
    StringT const anonTypePattern = R"((<unnamed (class|struct|enum)>|\(anonymous (class|struct|enum)\)))";
    StringT const testCasePattern = R"(CATCH2_INTERNAL_TEST_\d+)";
    StringT const locReturnPattern = "(auto|std::source_location) ";
}

TEST_CASE(
    "printing::type::prettify_identifier enhances std::source_location::function_name appearance.",
    "[print]")
{
    StringStreamT ss{};

    SECTION("When general function is given.")
    {
        constexpr auto loc = std::source_location::current();
        CAPTURE(loc.function_name());

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches("void " + testCasePattern + R"(\(\))"));
    }

    SECTION("When lambda is given.")
    {
        constexpr auto loc = type_post_processing_lambda_loc();
        CAPTURE(loc.function_name());

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            loc.function_name());

    #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Equals("lambda"));
    #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + lambdaCallOpPattern
                + R"(\(\)(const)?)"));
    #endif
    }

    SECTION("When nested lambda is given.")
    {
        constexpr auto loc = type_post_processing_nested_lambda_loc();
        CAPTURE(loc.function_name());

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            loc.function_name());

    #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Equals("lambda::lambda"));
    #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + lambdaCallOpPattern
                + "::"
                + lambdaCallOpPattern
                + R"(\(\)(const)?)"));
    #endif
    }

    SECTION("When function-local lambda is given.")
    {
        constexpr auto loc = loc_fun();
        CAPTURE(loc.function_name());

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            loc.function_name());

    #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(anonNsScopePattern + "loc_fun::lambda"));
    #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + anonNsScopePattern
                + "loc_fun::"
                + lambdaCallOpPattern
                + R"(\(\)(const)?)"));
    #endif
    }

    SECTION("When function-local anon-struct is given.")
    {
        struct
        {
            constexpr std::source_location operator()() const
            {
                return std::source_location::current();
            }
        } constexpr obj{};

        constexpr auto loc = obj();
        CAPTURE(loc.function_name());

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + testCasePattern
                + "::"
                + anonTypePattern
                + "::"
                  R"(operator\(\))"
                  R"(\(\)const)"));
    }

    SECTION("When function-local anon-class is given.")
    {
        class
        {
        public:
            constexpr std::source_location operator()() const
            {
                return std::source_location::current();
            }
        } constexpr obj{};

        constexpr auto loc = obj();
        CAPTURE(loc.function_name());

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + testCasePattern
                + "::"
                + anonTypePattern
                + "::"
                  R"(operator\(\))"
                  R"(\(\)const)"));
    }

    SECTION("When function-local anon-lambda is given.")
    {
        constexpr auto loc = loc_anon_lambda_fun();
        CAPTURE(loc.function_name());

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            loc.function_name());

    #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(anonNsScopePattern + "loc_anon_lambda_fun::lambda"));
    #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + anonNsScopePattern
                + "loc_anon_lambda_fun::"
                + lambdaCallOpPattern
                + R"(\(\)(const)?)"));
    #endif
    }

    SECTION("When template-dependant function is given.")
    {
        using type_t = decltype(my_typeLambda());
        auto const loc = my_template<type_t, int>{}.foo({});
        CAPTURE(loc.function_name());

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + anonNsScopePattern
                + "my_template::foo"
                  R"(\((\{anon-ns\}::my_template::)?my_type\))"));
    }
}

#endif
