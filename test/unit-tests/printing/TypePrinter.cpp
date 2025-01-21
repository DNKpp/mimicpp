//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

using namespace mimicpp;

TEST_CASE(
    "print_type supports fundamental types.",
    "[print]")
{
    REQUIRE_THAT(
        print_type<void>(),
        Catch::Matchers::Equals("void"));
    REQUIRE_THAT(
        print_type<bool>(),
        Catch::Matchers::Equals("bool"));
    REQUIRE_THAT(
        print_type<std::nullptr_t>(),
        Catch::Matchers::Equals("std::nullptr_t"));
    REQUIRE_THAT(
        print_type<std::byte>(),
        Catch::Matchers::Equals("std::byte"));

    SECTION("Character types.")
    {
        REQUIRE_THAT(
            print_type<char>(),
            Catch::Matchers::Equals("char"));
        REQUIRE_THAT(
            print_type<unsigned char>(),
            Catch::Matchers::Equals("unsigned char"));
        REQUIRE_THAT(
            print_type<signed char>(),
            Catch::Matchers::Equals("signed char"));

        REQUIRE_THAT(
            print_type<char8_t>(),
            Catch::Matchers::Equals("char8_t"));
        REQUIRE_THAT(
            print_type<char16_t>(),
            Catch::Matchers::Equals("char16_t"));
        REQUIRE_THAT(
            print_type<char32_t>(),
            Catch::Matchers::Equals("char32_t"));
        REQUIRE_THAT(
            print_type<wchar_t>(),
            Catch::Matchers::Equals("wchar_t"));
    }

    SECTION("Floating-point types")
    {
        REQUIRE_THAT(
            print_type<float>(),
            Catch::Matchers::Equals("float"));
        REQUIRE_THAT(
            print_type<double>(),
            Catch::Matchers::Equals("double"));
        REQUIRE_THAT(
            print_type<long double>(),
            Catch::Matchers::Equals("long double"));
    }

    SECTION("Integral types")
    {
        REQUIRE_THAT(
            print_type<short>(),
            Catch::Matchers::Equals("short"));
        REQUIRE_THAT(
            print_type<unsigned short>(),
            Catch::Matchers::Equals("unsigned short"));

        REQUIRE_THAT(
            print_type<int>(),
            Catch::Matchers::Equals("int"));
        REQUIRE_THAT(
            print_type<unsigned int>(),
            Catch::Matchers::Equals("unsigned int"));

        REQUIRE_THAT(
            print_type<long>(),
            Catch::Matchers::Equals("long"));
        REQUIRE_THAT(
            print_type<unsigned long>(),
            Catch::Matchers::Equals("unsigned long"));

        REQUIRE_THAT(
            print_type<long long>(),
            Catch::Matchers::Equals("long long"));
        REQUIRE_THAT(
            print_type<unsigned long long>(),
            Catch::Matchers::Equals("unsigned long long"));
    }
}
