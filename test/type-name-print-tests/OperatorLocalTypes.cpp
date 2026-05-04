//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include "Common.hpp"

using namespace mimicpp;

namespace
{
    class special_operators
    {
    public:
        [[nodiscard]]
        auto operator<(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator<=(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator>(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator>=(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator<(std::string) const noexcept
        {
            struct my_type
            {
                [[nodiscard]]
                auto operator>=(int) const noexcept
                {
                    struct my_type
                    {
                    };

                    return my_type{};
                }
            };

            return my_type{}.operator>=(42);
        }

        [[nodiscard]]
        auto operator<=(std::string) const noexcept
        {
            struct my_type
            {
                [[nodiscard]]
                auto operator>(int) const noexcept
                {
                    struct my_type
                    {
                    };

                    return my_type{};
                }
            };

            return my_type{}.operator>(42);
        }

        [[nodiscard]]
        auto operator>(std::string) const noexcept
        {
            struct my_type
            {
                [[nodiscard]]
                auto operator<=(int) const noexcept
                {
                    struct my_type
                    {
                    };

                    return my_type{};
                }
            };

            return my_type{}.operator<=(42);
        }

        [[nodiscard]]
        auto operator>=(std::string) const noexcept
        {
            struct my_type
            {
                [[nodiscard]]
                auto operator<(int) const noexcept
                {
                    struct my_type
                    {
                    };

                    return my_type{};
                }
            };

            return my_type{}.operator<(42);
        }

        [[nodiscard]]
        auto operator<=>(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator()(int) const
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator[](int) const
        {
            struct my_type
            {
            };

            return my_type{};
        }
    };
}

TEST_CASE(
    "printing::type::prettify_type supports operator<, <=, >, >= and <=>.",
    "[print][print::type]")
{
    std::ostringstream ss{};

    SECTION("When ordering operator is used.")
    {
        auto const [expectedFunctionName, rawName] = GENERATE((table<std::string, std::string_view>)({
            {R"(operator\s?<)",  printing::type::type_name<decltype(special_operators{}.operator<(42))>() },
            {R"(operator\s?<=)", printing::type::type_name<decltype(special_operators{}.operator<=(42))>()},
            {R"(operator\s?>)",  printing::type::type_name<decltype(special_operators{}.operator>(42))>() },
            {R"(operator\s?>=)", printing::type::type_name<decltype(special_operators{}.operator>=(42))>()}
        }));
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                testing::anonNsScopePattern
                + R"(special_operators::)"
                + expectedFunctionName
                + "::my_type"));
    }

    SECTION("When nested ordering operator is used.")
    {
        auto const [expectedFunctionName, expectedNestedFunctionName, rawName] = GENERATE(
            (table<std::string, std::string, std::string_view>)({
                {R"(operator\s?<)",  R"(operator\s?>=)", printing::type::type_name<decltype(special_operators{}.operator<(""))>() },
                {R"(operator\s?<=)", R"(operator\s?>)",  printing::type::type_name<decltype(special_operators{}.operator<=(""))>()},
                {R"(operator\s?>)",  R"(operator\s?<=)", printing::type::type_name<decltype(special_operators{}.operator>(""))>() },
                {R"(operator\s?>=)", R"(operator\s?<)",  printing::type::type_name<decltype(special_operators{}.operator>=(""))>()}
        }));
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                testing::anonNsScopePattern
                + R"(special_operators::)"
                + expectedFunctionName
                + "::my_type::"
                + expectedNestedFunctionName
                + "::my_type"));
    }

    SECTION("When spaceship-operator is used.")
    {
        std::string const rawName{printing::type::type_name<decltype(special_operators{}.operator<=>(42))>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                testing::anonNsScopePattern
                + "special_operators::"
                + R"(operator\s?<=>::)"
                + "my_type"));
    }
}

TEST_CASE(
    "printing::type::prettify_type supports operator().",
    "[print][print::type]")
{
    std::ostringstream ss{};

    SECTION("When identifier contains operator() scope.")
    {
        std::string const rawName{printing::type::type_name<decltype(special_operators{}.operator()(42))>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const scopePattern = testing::anonNsScopePattern + "special_operators::" + R"(operator\s?\(\)::)";
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + "my_type"));
    }

    SECTION("When member-function-pointer to operator() is given.")
    {
        std::string const rawName{printing::type::type_name<decltype(&special_operators::operator())>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const returnPattern = testing::anonNsScopePattern + R"(special_operators::operator\s?\(\)::my_type)";
        std::string const scopePattern = testing::anonNsScopePattern + "special_operators::";
        std::string const argListPattern = R"(\(int\))";
        std::string const funSuffixPattern = R"(\s?const)";

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                returnPattern
                + R"(\()" + scopePattern + R"(\*\))"
                + argListPattern
                + funSuffixPattern));
    }
}

TEST_CASE(
    "printing::type::prettify_type supports operator[].",
    "[print][print::type]")
{
    std::ostringstream ss{};

    SECTION("When identifier contains operator[] scope.")
    {
        std::string const rawName{printing::type::type_name<decltype(special_operators{}.operator[](42))>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const scopePattern = testing::anonNsScopePattern + "special_operators::" + R"(operator\s?\[\]::)";
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + "my_type"));
    }

    SECTION("When member-function-pointer to operator[] is given.")
    {
        std::string const rawName{printing::type::type_name<decltype(&special_operators::operator[])>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const returnPattern = testing::anonNsScopePattern + R"(special_operators::operator\s?\[\]::my_type)";
        std::string const scopePattern = testing::anonNsScopePattern + "special_operators::";
        std::string const argListPattern = R"(\(int\))";
        std::string const funSuffixPattern = R"(\s?const)";

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                returnPattern
                + R"(\()" + scopePattern + R"(\*\))"
                + argListPattern
                + funSuffixPattern));
    }
}
