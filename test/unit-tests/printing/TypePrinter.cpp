//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

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

TEST_CASE(
    "print_type prints common std types nicely.",
    "[print]")
{
    SECTION("std string types")
    {
        REQUIRE_THAT(
            print_type<std::string>(),
            Catch::Matchers::Equals("std::string"));
        REQUIRE_THAT(
            print_type<std::wstring>(),
            Catch::Matchers::Equals("std::wstring"));
        REQUIRE_THAT(
            print_type<std::u8string>(),
            Catch::Matchers::Equals("std::u8string"));
        REQUIRE_THAT(
            print_type<std::u8string>(),
            Catch::Matchers::Equals("std::u16string"));
        REQUIRE_THAT(
            print_type<std::u8string>(),
            Catch::Matchers::Equals("std::u32string"));
    }

    SECTION("std string_view types")
    {
        REQUIRE_THAT(
            print_type<std::string_view>(),
            Catch::Matchers::Equals("std::string_view"));
        REQUIRE_THAT(
            print_type<std::wstring_view>(),
            Catch::Matchers::Equals("std::wstring_view"));
        REQUIRE_THAT(
            print_type<std::u8string_view>(),
            Catch::Matchers::Equals("std::u8string_view"));
        REQUIRE_THAT(
            print_type<std::u8string_view>(),
            Catch::Matchers::Equals("std::u16string_view"));
        REQUIRE_THAT(
            print_type<std::u8string_view>(),
            Catch::Matchers::Equals("std::u32string_view"));
    }

    SECTION("std vector types")
    {
        REQUIRE_THAT(
            print_type<std::vector<int>>(),
            Catch::Matchers::Equals("std::vector<int>"));
        REQUIRE_THAT(
            print_type<std::vector<std::vector<int>>>(),
            Catch::Matchers::Equals("std::vector<std::vector<int>>"));
    }

    SECTION("std optional types")
    {
        REQUIRE_THAT(
            print_type<std::nullopt_t>(),
            Catch::Matchers::Equals("std::nullopt_t"));
        REQUIRE_THAT(
            print_type<std::optional<int>>(),
            Catch::Matchers::Equals("std::optional<int>"));
        REQUIRE_THAT(
            print_type<std::optional<std::optional<int>>>(),
            Catch::Matchers::Equals("std::optional<std::optional<int>>"));
    }
}
