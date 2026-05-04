//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include "Common.hpp"

using namespace mimicpp;

namespace
{
    namespace test
    {
        struct my_type
        {
        };
    }

    template <typename T>
    struct my_template
    {
        struct my_type
        {
        };

        auto foo(my_type)
        {
            return util::SourceLocation{};
        }

        auto bar(my_type const&, util::SourceLocation* outLoc)
        {
            if (outLoc)
            {
                *outLoc = util::SourceLocation{};
            }

            struct bar_type
            {
            };

            return bar_type{};
        }
    };

    template <typename... Ts>
    struct my_variadic_template
    {
        struct my_type
        {
        };

        auto foo(my_type)
        {
            return util::SourceLocation{};
        }

        auto bar(my_type const&, util::SourceLocation* outLoc)
        {
            if (outLoc)
            {
                *outLoc = util::SourceLocation{};
            }

            struct bar_type
            {
            };

            return bar_type{};
        }
    };
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances template-type-names within an anonymous-namespace scope.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    auto const scopePattern = testing::anonNsScopePattern;

    std::ostringstream ss{};

    SECTION("When a template-type with a single param is given.")
    {
        using T = testing::mod_type_t<TestType, my_template<int>>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + "my_template<int>" + suffixPattern));
    }

    SECTION("When a variadic template-type without params is given.")
    {
        using T = testing::mod_type_t<TestType, my_variadic_template<>>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + "my_variadic_template<>" + suffixPattern));
    }

    SECTION("When a variadic template-type with multiple params is given.")
    {
        using T = testing::mod_type_t<TestType, my_variadic_template<int&, float const&, test::my_type*>>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const argListPattern = "int&, float const&, " + scopePattern + "test::my_type\\*";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + "my_variadic_template<" + argListPattern + ">" + suffixPattern));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances template dependant type-names.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    auto const scopePattern = testing::anonNsScopePattern;

    std::ostringstream ss{};

    SECTION("When a template-type with a single param is given.")
    {
        using T = testing::mod_type_t<TestType, my_template<int>::my_type>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + "my_template::my_type" + suffixPattern));
    }

    SECTION("When a variadic template-type without params is given.")
    {
        using T = testing::mod_type_t<TestType, my_variadic_template<>::my_type>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + "my_variadic_template::my_type" + suffixPattern));
    }

    SECTION("When a variadic template-type with multiple params is given.")
    {
        using T = testing::mod_type_t<TestType, my_variadic_template<int&, float const&, test::my_type*>::my_type>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + "my_variadic_template::my_type" + suffixPattern));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances template dependant member-fun-ptr type-names.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    std::string const returnTypePattern = "mimicpp::util::SourceLocation";
    std::string const scopePattern = testing::anonNsScopePattern;

    std::ostringstream ss{};

    SECTION("When a template-type with a single param is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(&my_template<int>::foo)>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const argTypePattern = "\\(" + testing::anonNsScopePattern + "my_template::my_type\\)";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                returnTypePattern
                + "\\(" + scopePattern + "my_template::\\*" + suffixPattern + "\\)"
                + argTypePattern));
    }

    SECTION("When a variadic template-type without params is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(&my_variadic_template<>::foo)>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const argTypePattern = "\\(" + testing::anonNsScopePattern + "my_variadic_template::my_type\\)";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                returnTypePattern
                + "\\(" + scopePattern + "my_variadic_template::\\*" + suffixPattern + "\\)"
                + argTypePattern));
    }

    SECTION("When a variadic template-type with multiple params is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(&my_variadic_template<int&, float const&, test::my_type*>::foo)>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const argTypePattern = "\\(" + testing::anonNsScopePattern + "my_variadic_template::my_type\\)";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                returnTypePattern
                + "\\(" + scopePattern + "my_variadic_template::\\*" + suffixPattern + "\\)"
                + argTypePattern));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances template dependant member-fun-ptr, which returns a local-type, type-names.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When a template-type with a single param is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(&my_template<int>::bar)>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const argListPattern = "\\(" + testing::anonNsScopePattern + R"(my_template::my_type const\s?&, mimicpp::util::SourceLocation\s?\*\))";
        std::string const ptrPattern = "\\(" + testing::anonNsScopePattern + "my_template::\\*" + suffixPattern + "\\)";
        std::string const returnTypePattern = testing::anonNsScopePattern + "my_template::bar::bar_type";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(returnTypePattern + ptrPattern + argListPattern));
    }

    SECTION("When a variadic template-type without params is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(&my_variadic_template<>::bar)>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const argListPattern = "\\(" + testing::anonNsScopePattern + R"(my_variadic_template::my_type const\s?&, mimicpp::util::SourceLocation\s?\*\))";
        std::string const ptrPattern = "\\(" + testing::anonNsScopePattern + "my_variadic_template::\\*" + suffixPattern + "\\)";
        std::string const returnTypePattern = testing::anonNsScopePattern + "my_variadic_template::bar::bar_type";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(returnTypePattern + ptrPattern + argListPattern));
    }

    SECTION("When a variadic template-type with multiple params is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(&my_variadic_template<int&, float const&, test::my_type*>::bar)>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        std::string const argListPattern = "\\(" + testing::anonNsScopePattern + R"(my_variadic_template::my_type const\s?&, mimicpp::util::SourceLocation\s?\*\))";
        std::string const ptrPattern = "\\(" + testing::anonNsScopePattern + "my_variadic_template::\\*" + suffixPattern + "\\)";
        std::string const returnTypePattern = testing::anonNsScopePattern + "my_variadic_template::bar::bar_type";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(returnTypePattern + ptrPattern + argListPattern));
    }
}
