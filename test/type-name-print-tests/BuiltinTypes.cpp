//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include "Common.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE(
    "printing::type::prettify_type handles built-in integral types correctly.",
    "[print][print::type]",
    char,
    short,
    int,
    long,
    long long)
{
    std::ostringstream ss{};

    SECTION("When is given as-is.")
    {
        std::string const name{printing::type::type_name<TestType>()};
        CAPTURE(name);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            name);
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(name));
    }

    SECTION("When explicit signed name is given.")
    {
        using T = std::make_signed_t<TestType>;
        std::string const name{printing::type::type_name<T>()};
        CAPTURE(name);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            name);
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(name));
    }

    SECTION("When unsigned name is given.")
    {
        using T = std::make_unsigned_t<TestType>;
        std::string const name{printing::type::type_name<T>()};
        CAPTURE(name);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            name);
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(name));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type handles bool correctly.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);
    using T = testing::mod_type_t<TestType, bool>;

    std::string const name{printing::type::type_name<T>()};
    CAPTURE(name);

    std::ostringstream ss{};
    printing::type::prettify_type(
        std::ostreambuf_iterator{ss},
        name);
    CHECK_THAT(
        std::move(ss).str(),
        Catch::Matchers::Matches("bool" + suffixPattern));
}

namespace
{
    template <typename T>
    [[nodiscard]]
    std::pair<std::string, std::string> make_case(std::string expected)
    {
        return {
            std::move(expected),
            std::string{printing::type::type_name<T>()}};
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type handles built-in float types correctly.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    auto const [expectedBase, rawName] = GENERATE(
        (make_case<testing::mod_type_t<TestType, float>>("float")),
        (make_case<testing::mod_type_t<TestType, double>>("double")),
        (make_case<testing::mod_type_t<TestType, long double>>("long double")));
    CAPTURE(rawName);

    std::ostringstream ss{};
    printing::type::prettify_type(
        std::ostreambuf_iterator{ss},
        rawName);
    CHECK_THAT(
        std::move(ss).str(),
        Catch::Matchers::Matches(expectedBase + suffixPattern));
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type handles built-in character types correctly.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    auto const [expectedBase, rawName] = GENERATE(
        (make_case<testing::mod_type_t<TestType, char>>("char")),
        (make_case<testing::mod_type_t<TestType, wchar_t>>("wchar_t")),
        (make_case<testing::mod_type_t<TestType, char8_t>>("char8_t")),
        (make_case<testing::mod_type_t<TestType, char16_t>>("char16_t")),
        (make_case<testing::mod_type_t<TestType, char32_t>>("char32_t")));
    CAPTURE(rawName);

    std::ostringstream ss{};
    printing::type::prettify_type(
        std::ostreambuf_iterator{ss},
        rawName);
    CHECK_THAT(
        std::move(ss).str(),
        Catch::Matchers::Matches(expectedBase + suffixPattern));
}
