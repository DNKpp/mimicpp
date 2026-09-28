//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include "Common.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE(
    "printing::type::prettify_type supports array types.",
    "[print][print::type]",
    testing::modifier_maker<testing::mods::decayed>,
    testing::modifier_maker<testing::mods::add_lvalue_ref>,
    testing::modifier_maker<testing::mods::add_rvalue_ref>,
    testing::modifier_maker<testing::mods::add_pointer>)
{
    std::string const arrayMod = TestType::suffix.empty()
                                   ? ""
                                   : "\\(" + std::string{TestType::suffix} + "\\)";
    CAPTURE(arrayMod);

    std::ostringstream ss{};

    SECTION("When an unbounded array type is given.")
    {
        using T = testing::mod_type_t<TestType, int[]>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches("int" + arrayMod + R"(\[\])"));
    }

    SECTION("When an bounded array type is given.")
    {
        using T = testing::mod_type_t<TestType, int[42]>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches("int" + arrayMod + R"(\[42\])"));
    }

    SECTION("When multi-dim array is given.")
    {
        using T = testing::mod_type_t<TestType, int[][42][1337]>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches("int" + arrayMod + R"(\[\]\[42\]\[1337\])"));
    }
}
