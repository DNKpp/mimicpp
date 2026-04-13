//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/SourceLocation.hpp"

using namespace mimicpp;

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES

namespace
{
    StringT const topLevelLambdaPattern =
        R"((\$_\d+|lambda(#\d+|\d+)?))";

    StringT const lambdaScopePattern = topLevelLambdaPattern + "::";

    StringT const testCaseScopePattern = R"(CATCH2_INTERNAL_TEST_\d+::)";
    StringT const callOpScopePattern = R"(operator\s?\(\)::)";
}

TEST_CASE(
    "printing::type::prettify_type enhances local type-names appearance.",
    "[!mayfail][print]")
{
    StringStreamT ss{};

    SECTION("When local type is queried inside the current scope.")
    {
        struct my_type
        {
        };

        StringT const rawName{printing::type::type_name<my_type>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches("(" + testCaseScopePattern + ")?my_type"));
    }

    SECTION("When local type is queried inside a lambda.")
    {
        std::invoke(
            [&] {
                struct my_type
                {
                };

                StringT const rawName{printing::type::type_name<my_type>()};
                CAPTURE(rawName);

                printing::type::prettify_type(
                    std::ostreambuf_iterator{ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(ss).str(),
                    Catch::Matchers::Matches(
                        "("
                        + testCaseScopePattern
                        + lambdaScopePattern
                        + callOpScopePattern
                        + ")?my_type"));
            });
    }

    SECTION("When local type is queried inside a member-function.")
    {
        struct outer
        {
            void operator()(StringStreamT& _ss) const
            {
                struct my_type
                {
                };

                StringT const rawName{printing::type::type_name<my_type>()};
                CAPTURE(rawName);

                printing::type::prettify_type(
                    std::ostreambuf_iterator{_ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(_ss).str(),
                    Catch::Matchers::Matches(
                        "("
                        + testCaseScopePattern
                        + "outer::"
                        + callOpScopePattern
                        + ")?my_type"));
            }
        };

        outer{}(ss);
    }

    SECTION("When local type is queried inside a lambda with higher arity.")
    {
        // Todo: This case will currently fail, because parser does not handle arrays.

        int d1{};
        int d2[1]{};
        int* ptr = &d1;
        std::invoke(
            [](
                StringStreamT* _ss,
                [[maybe_unused]] int&& ref,
                [[maybe_unused]] int(&arrRef)[1],
                [[maybe_unused]] int*& ptrRef) {
                struct my_type
                {
                };

                StringT const rawName{printing::type::type_name<my_type>()};
                CAPTURE(rawName);

                printing::type::prettify_type(
                    std::ostreambuf_iterator{*_ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(*_ss).str(),
                    Catch::Matchers::Matches(
                        "("
                        + testCaseScopePattern
                        + lambdaScopePattern
                        + callOpScopePattern
                        + ")?my_type"));
            },
            &ss,
            std::move(d1),
            d2,
            ptr);
    }

    SECTION("When local type is queried inside a nested-lambda with higher arity.")
    {
        std::invoke(
            [](StringStreamT* _ss) {
                struct other_type
                {
                };

                std::invoke(
                    [&]([[maybe_unused]] other_type const& dummy) {
                        struct my_type
                        {
                        };

                        StringT const rawName{printing::type::type_name<my_type>()};
                        CAPTURE(rawName);

                        printing::type::prettify_type(
                            std::ostreambuf_iterator{*_ss},
                            rawName);
                        REQUIRE_THAT(
                            std::move(*_ss).str(),
                            Catch::Matchers::Matches(
                                "("
                                + testCaseScopePattern
                                + lambdaScopePattern
                                + callOpScopePattern
                                + lambdaScopePattern
                                + callOpScopePattern
                                + ")?my_type"));
                    },
                    other_type{});
            },
            &ss);
    }
}

#endif
