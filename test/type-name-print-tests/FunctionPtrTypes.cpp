//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include "Common.hpp"

using namespace mimicpp;

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances function-ptr return types.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    std::string const prefixPattern{R"(void \(\*)"};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When a function-type returns a function-ptr.")
    {
        using Return = void (*)();
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<T()>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(prefixPattern + suffixPattern + R"(\)\(\) \(\))"));
    }

    SECTION("When a function-ptr-type returns a function-ptr.")
    {
        using Return = void (*)();
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<T (*)()>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(prefixPattern + suffixPattern + R"(\)\(\) \(\*\)\(\))"));
    }

    SECTION("When a function-type returns a function-ptr, which returns a function-ptr.")
    {
        using Return1 = void (*)();
        using Return2 = Return1 (*)();
        using T = testing::mod_type_t<TestType, Return2>;
        std::string const rawName{printing::type::type_name<T()>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(prefixPattern + R"(\)\(\) \(\*)" + suffixPattern + R"(\)\(\) \(\))"));
    }

    SECTION("When a function-ptr-type returns a function-ptr, which returns a function-ptr.")
    {
        using Return1 = void (*)();
        using Return2 = Return1 (*)();
        using T = testing::mod_type_t<TestType, Return2>;
        std::string const rawName{printing::type::type_name<T (*)()>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(prefixPattern + R"(\)\(\) \(\*)" + suffixPattern + R"(\)\(\) \(\*\)\(\))"));
    }
}
