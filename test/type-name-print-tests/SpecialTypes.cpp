//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/StaticString.hpp"

#include "Common.hpp"

using namespace mimicpp;

namespace
{
    std::string const anonTypePattern = R"(<unnamed (class|struct|enum)>)";

    struct my_type
    {
    };
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances type-names within an anonymous-namespace scope.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);
    using T = testing::mod_type_t<TestType, my_type>;

    std::string const rawName{printing::type::type_name<T>()};
    CAPTURE(rawName);

    std::ostringstream ss{};
    printing::type::prettify_type(
        std::ostreambuf_iterator{ss},
        rawName);

    auto const scopePattern = testing::anonNsScopePattern;
    CHECK_THAT(
        std::move(ss).str(),
        Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
}

namespace
{
    class
    {
    } anon_class [[maybe_unused]]{};

    class
    {
    } anon_struct [[maybe_unused]]{};

    enum
    {
        dummy
    } anon_enum [[maybe_unused]]{};
}

TEMPLATE_PRODUCT_TEST_CASE(
    "printing::type::prettify_type enhances anonymous type-names.",
    "[print][print::type]",
    TESTING_COMMON_MODS,
    (decltype(anon_class),
     decltype(anon_struct),
     decltype(anon_enum)))
{
    using T = TestType::type;
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    std::string const rawName{printing::type::type_name<T>()};
    CAPTURE(rawName);

    std::ostringstream ss{};
    printing::type::prettify_type(
        std::ostreambuf_iterator{ss},
        rawName);

    auto const scopePattern = testing::anonNsScopePattern;
    CHECK_THAT(
        std::move(ss).str(),
        Catch::Matchers::Matches(scopePattern + anonTypePattern + suffixPattern));
}

namespace
{
    struct outer_type
    {
        template <typename T>
        struct my_template
        {
        };

        struct my_type
        {
        };
    };
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances nested type-names.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};
    auto const scopePattern = testing::anonNsScopePattern + "outer_type::";

    SECTION("When nested type-name is given.")
    {
        using T = testing::mod_type_t<TestType, outer_type::my_type>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When nested template type-name is given.")
    {
        using T = testing::mod_type_t<TestType, outer_type::my_template<int>>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_template<int>" + suffixPattern));
    }
}

namespace
{
    [[maybe_unused]] auto nullaryLambda = [] {};
    [[maybe_unused]] auto unaryLambda = [](int volatile* const&) {};
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances lambda appearance.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When lambda is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(nullaryLambda)>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(testing::anonNsScopePattern + testing::lambda_pattern() + suffixPattern));
    }

    SECTION("When lambda with params is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(unaryLambda)>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(testing::anonNsScopePattern + testing::lambda_pattern(R"(\.{3})") + suffixPattern));
    }
}
