//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/printing/CommonTypePrinter.hpp"

#include <memory_resource>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>

using namespace mimicpp;

TEST_CASE(
    "print_type supports fundamental types.",
    "[print]")
{
    SECTION("General types")
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
    }

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
            print_type<std::u16string>(),
            Catch::Matchers::Equals("std::u16string"));
        REQUIRE_THAT(
            print_type<std::u32string>(),
            Catch::Matchers::Equals("std::u32string"));

        REQUIRE_THAT(
            print_type<std::pmr::string>(),
            Catch::Matchers::Equals("std::pmr::string"));
        REQUIRE_THAT(
            print_type<std::pmr::wstring>(),
            Catch::Matchers::Equals("std::pmr::wstring"));
        REQUIRE_THAT(
            print_type<std::pmr::u8string>(),
            Catch::Matchers::Equals("std::pmr::u8string"));
        REQUIRE_THAT(
            print_type<std::pmr::u16string>(),
            Catch::Matchers::Equals("std::pmr::u16string"));
        REQUIRE_THAT(
            print_type<std::pmr::u32string>(),
            Catch::Matchers::Equals("std::pmr::u32string"));
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
            print_type<std::u16string_view>(),
            Catch::Matchers::Equals("std::u16string_view"));
        REQUIRE_THAT(
            print_type<std::u32string_view>(),
            Catch::Matchers::Equals("std::u32string_view"));
    }

#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

    SECTION("std optional types")
    {
        REQUIRE_THAT(
            print_type<std::nullopt_t>(),
            Catch::Matchers::Equals("std::nullopt_t"));
        REQUIRE_THAT(
            print_type<std::optional<int>>(),
            Catch::Matchers::Equals("std::optional<int>"));
        REQUIRE_THAT(
            print_type<std::optional<std::string>>(),
            Catch::Matchers::Equals("std::optional<std::string>"));
        REQUIRE_THAT(
            print_type<std::optional<std::optional<int>>>(),
            Catch::Matchers::Equals("std::optional<std::optional<int>>"));
        REQUIRE_THAT(
            print_type<std::optional<std::optional<std::string>>>(),
            Catch::Matchers::Equals("std::optional<std::optional<std::string>>"));
    }

    SECTION("std tuple types")
    {
        REQUIRE_THAT(
            print_type<std::tuple<>>(),
            Catch::Matchers::Equals("std::tuple<>"));
        REQUIRE_THAT(
            print_type<std::tuple<int>>(),
            Catch::Matchers::Equals("std::tuple<int>"));
        REQUIRE_THAT(
            print_type<std::tuple<std::tuple<>>>(),
            Catch::Matchers::Equals("std::tuple<std::tuple<>>"));
        REQUIRE_THAT(
            (print_type<std::tuple<std::tuple<int, float, std::string>>>()),
            Catch::Matchers::Equals("std::tuple<std::tuple<int, float, std::string>>"));
    }

    SECTION("std unique_ptr types")
    {
        REQUIRE_THAT(
            (print_type<std::unique_ptr<int>>()),
            Catch::Matchers::Equals("std::unique_ptr<int>"));
        REQUIRE_THAT(
            (print_type<std::unique_ptr<int[]>>()),
            Catch::Matchers::Equals("std::unique_ptr<int[]>"));
        REQUIRE_THAT(
            (print_type<std::unique_ptr<std::unique_ptr<int>>>()),
            Catch::Matchers::Equals("std::unique_ptr<std::unique_ptr<int>>"));
        REQUIRE_THAT(
            (print_type<std::unique_ptr<std::tuple<float&, std::tuple<>>>>()),
            Catch::Matchers::Equals("std::unique_ptr<std::tuple<float&, std::tuple<>>>"));
        REQUIRE_THAT(
            (print_type<std::unique_ptr<std::tuple<float&, std::tuple<>>[]>>()),
            Catch::Matchers::Equals("std::unique_ptr<std::tuple<float&, std::tuple<>>[]>"));
    }

    SECTION("std shared_ptr and weak_ptr types")
    {
        REQUIRE_THAT(
            print_type<std::shared_ptr<int>>(),
            Catch::Matchers::Equals("std::shared_ptr<int>"));
        REQUIRE_THAT(
            (print_type<std::shared_ptr<std::tuple<float&, std::tuple<>>>>()),
            Catch::Matchers::Equals("std::shared_ptr<std::tuple<float&, std::tuple<>>>"));

        REQUIRE_THAT(
            print_type<std::weak_ptr<int>>(),
            Catch::Matchers::Equals("std::weak_ptr<int>"));
        REQUIRE_THAT(
            (print_type<std::weak_ptr<std::tuple<float&, std::tuple<>>>>()),
            Catch::Matchers::Equals("std::weak_ptr<std::tuple<float&, std::tuple<>>>"));
    }

#endif
}

namespace
{
    struct my_type
    {
    };

    template <unsigned N>
    class FixedString
    {
    public:
        char buffer[N + 1];

        [[nodiscard]]
        explicit consteval FixedString(char const* s) noexcept
        {
            std::ranges::copy_n(s, N, std::ranges::begin(buffer));
            buffer[N] = CharT{0};
        }

        [[nodiscard]]
        explicit(false) constexpr operator StringViewT() const noexcept
        {
            return {std::ranges::data(buffer), N};
        }
    };

    template <unsigned N>
    FixedString(CharT const (&)[N]) -> FixedString<N - 1>;
}

#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING
TEMPLATE_TEST_CASE_SIG(
    "print_type supports all type qualifications.",
    "[print]",
    ((auto baseName, typename T), baseName, T),
    (FixedString{"int"}, int),
    (FixedString{"int*"}, int*),
    (FixedString{"int const*"}, int const*),
    (FixedString{"int volatile*"}, int volatile*),
    (FixedString{"int const volatile*"}, int const volatile*),

    (FixedString{"std::string"}, std::string),
    (FixedString{"std::string*"}, std::string*),
    (FixedString{"std::string const*"}, std::string const*),
    (FixedString{"std::string volatile*"}, std::string volatile*),
    (FixedString{"std::string const volatile*"}, std::string const volatile*),

    (FixedString{"std::vector<int>"}, std::vector<int>),
    (FixedString{"std::vector<int>*"}, std::vector<int>*),
    (FixedString{"std::vector<int> const*"}, std::vector<int> const*),
    (FixedString{"std::vector<int> volatile*"}, std::vector<int> volatile*),
    (FixedString{"std::vector<int> const volatile*"}, std::vector<int> const volatile*),

    (FixedString{"std::vector<std::vector<int>>"}, std::vector<std::vector<int>>),
    (FixedString{"std::vector<std::vector<int>>*"}, std::vector<std::vector<int>>*),
    (FixedString{"std::vector<std::vector<int>> const*"}, std::vector<std::vector<int>> const*),
    (FixedString{"std::vector<std::vector<int>> volatile*"}, std::vector<std::vector<int>> volatile*),
    (FixedString{"std::vector<std::vector<int>> const volatile*"}, std::vector<std::vector<int>> const volatile*),

    (FixedString{"my_type"}, my_type),
    (FixedString{"my_type*"}, my_type*),
    (FixedString{"my_type const*"}, my_type const*),
    (FixedString{"my_type volatile*"}, my_type volatile*),
    (FixedString{"my_type const volatile*"}, my_type const volatile*))
#else
TEMPLATE_TEST_CASE_SIG(
    "print_type supports all type qualifications.",
    "[print]",
    ((auto baseName, typename T), baseName, T),
    (FixedString{"int"}, int),
    (FixedString{"int*"}, int*),
    (FixedString{"int const*"}, int const*),
    (FixedString{"int volatile*"}, int volatile*),
    (FixedString{"int const volatile*"}, int const volatile*),

    (FixedString{"std::string"}, std::string),
    (FixedString{"std::string*"}, std::string*),
    (FixedString{"std::string const*"}, std::string const*),
    (FixedString{"std::string volatile*"}, std::string volatile*),
    (FixedString{"std::string const volatile*"}, std::string const volatile*))
#endif
{
    const StringT name{baseName};

    SECTION("As value type")
    {
        REQUIRE_THAT(
            print_type<T const>(),
            Catch::Matchers::Equals(name + " const"));
        REQUIRE_THAT(
            print_type<T volatile>(),
            Catch::Matchers::Equals(name + " volatile"));
        REQUIRE_THAT(
            print_type<T const volatile>(),
            Catch::Matchers::Equals(name + " const volatile"));
    }

    SECTION("As lvalue-ref type")
    {
        REQUIRE_THAT(
            print_type<T&>(),
            Catch::Matchers::Equals(name + "&"));
        REQUIRE_THAT(
            print_type<T const&>(),
            Catch::Matchers::Equals(name + " const&"));
        REQUIRE_THAT(
            print_type<T volatile&>(),
            Catch::Matchers::Equals(name + " volatile&"));
        REQUIRE_THAT(
            print_type<T const volatile&>(),
            Catch::Matchers::Equals(name + " const volatile&"));
    }

    SECTION("As rvalue-ref type")
    {
        REQUIRE_THAT(
            print_type<T&&>(),
            Catch::Matchers::Equals(name + "&&"));
        REQUIRE_THAT(
            print_type<T const&&>(),
            Catch::Matchers::Equals(name + " const&&"));
        REQUIRE_THAT(
            print_type<T volatile&&>(),
            Catch::Matchers::Equals(name + " volatile&&"));
        REQUIRE_THAT(
            print_type<T const volatile&&>(),
            Catch::Matchers::Equals(name + " const volatile&&"));
    }

    SECTION("As unbounded array type")
    {
        REQUIRE_THAT(
            print_type<T[]>(),
            Catch::Matchers::Equals(name + "[]"));
        REQUIRE_THAT(
            print_type<T const[]>(),
            Catch::Matchers::Equals(name + " const[]"));
        REQUIRE_THAT(
            print_type<T volatile[]>(),
            Catch::Matchers::Equals(name + " volatile[]"));
        REQUIRE_THAT(
            print_type<T const volatile[]>(),
            Catch::Matchers::Equals(name + " const volatile[]"));
    }

    SECTION("As bounded array type")
    {
        REQUIRE_THAT(
            print_type<T[42]>(),
            Catch::Matchers::Equals(name + "[42]"));
        REQUIRE_THAT(
            print_type<T const[42]>(),
            Catch::Matchers::Equals(name + " const[42]"));
        REQUIRE_THAT(
            print_type<T volatile[42]>(),
            Catch::Matchers::Equals(name + " volatile[42]"));
        REQUIRE_THAT(
            print_type<T const volatile[42]>(),
            Catch::Matchers::Equals(name + " const volatile[42]"));

        REQUIRE_THAT(
            print_type<T[42][3]>(),
            Catch::Matchers::Equals(name + "[42][3]"));
        REQUIRE_THAT(
            print_type<T const[42][3]>(),
            Catch::Matchers::Equals(name + " const[42][3]"));
        REQUIRE_THAT(
            print_type<T volatile[42][3]>(),
            Catch::Matchers::Equals(name + " volatile[42][3]"));
        REQUIRE_THAT(
            print_type<T const volatile[42][3]>(),
            Catch::Matchers::Equals(name + " const volatile[42][3]"));
    }

    SECTION("As pointer type")
    {
        REQUIRE_THAT(
            print_type<T*>(),
            Catch::Matchers::Equals(name + "*"));
        REQUIRE_THAT(
            print_type<T const*>(),
            Catch::Matchers::Equals(name + " const*"));
        REQUIRE_THAT(
            print_type<T volatile*>(),
            Catch::Matchers::Equals(name + " volatile*"));
        REQUIRE_THAT(
            print_type<T const volatile*>(),
            Catch::Matchers::Equals(name + " const volatile*"));

        REQUIRE_THAT(
            print_type<T* const>(),
            Catch::Matchers::Equals(name + "* const"));
        REQUIRE_THAT(
            print_type<T const* const>(),
            Catch::Matchers::Equals(name + " const* const"));
        REQUIRE_THAT(
            print_type<T volatile* const>(),
            Catch::Matchers::Equals(name + " volatile* const"));
        REQUIRE_THAT(
            print_type<T const volatile* const>(),
            Catch::Matchers::Equals(name + " const volatile* const"));

        REQUIRE_THAT(
            print_type<T* volatile>(),
            Catch::Matchers::Equals(name + "* volatile"));
        REQUIRE_THAT(
            print_type<T const* volatile>(),
            Catch::Matchers::Equals(name + " const* volatile"));
        REQUIRE_THAT(
            print_type<T volatile* volatile>(),
            Catch::Matchers::Equals(name + " volatile* volatile"));
        REQUIRE_THAT(
            print_type<T const volatile* volatile>(),
            Catch::Matchers::Equals(name + " const volatile* volatile"));

        REQUIRE_THAT(
            print_type<T* const volatile>(),
            Catch::Matchers::Equals(name + "* const volatile"));
        REQUIRE_THAT(
            print_type<T const* const volatile>(),
            Catch::Matchers::Equals(name + " const* const volatile"));
        REQUIRE_THAT(
            print_type<T volatile* const volatile>(),
            Catch::Matchers::Equals(name + " volatile* const volatile"));
        REQUIRE_THAT(
            print_type<T const volatile* const volatile>(),
            Catch::Matchers::Equals(name + " const volatile* const volatile"));
    }

    SECTION("As lvalue-ref pointer type")
    {
        REQUIRE_THAT(
            print_type<T*&>(),
            Catch::Matchers::Equals(name + "*&"));
        REQUIRE_THAT(
            print_type<T const*&>(),
            Catch::Matchers::Equals(name + " const*&"));
        REQUIRE_THAT(
            print_type<T volatile*&>(),
            Catch::Matchers::Equals(name + " volatile*&"));
        REQUIRE_THAT(
            print_type<T const volatile*&>(),
            Catch::Matchers::Equals(name + " const volatile*&"));

        REQUIRE_THAT(
            print_type<T* const&>(),
            Catch::Matchers::Equals(name + "* const&"));
        REQUIRE_THAT(
            print_type<T const* const&>(),
            Catch::Matchers::Equals(name + " const* const&"));
        REQUIRE_THAT(
            print_type<T volatile* const&>(),
            Catch::Matchers::Equals(name + " volatile* const&"));
        REQUIRE_THAT(
            print_type<T const volatile* const&>(),
            Catch::Matchers::Equals(name + " const volatile* const&"));

        REQUIRE_THAT(
            print_type<T* volatile&>(),
            Catch::Matchers::Equals(name + "* volatile&"));
        REQUIRE_THAT(
            print_type<T const* volatile&>(),
            Catch::Matchers::Equals(name + " const* volatile&"));
        REQUIRE_THAT(
            print_type<T volatile* volatile&>(),
            Catch::Matchers::Equals(name + " volatile* volatile&"));
        REQUIRE_THAT(
            print_type<T const volatile* volatile&>(),
            Catch::Matchers::Equals(name + " const volatile* volatile&"));

        REQUIRE_THAT(
            print_type<T* const volatile&>(),
            Catch::Matchers::Equals(name + "* const volatile&"));
        REQUIRE_THAT(
            print_type<T const* const volatile&>(),
            Catch::Matchers::Equals(name + " const* const volatile&"));
        REQUIRE_THAT(
            print_type<T volatile* const volatile&>(),
            Catch::Matchers::Equals(name + " volatile* const volatile&"));
        REQUIRE_THAT(
            print_type<T const volatile* const volatile&>(),
            Catch::Matchers::Equals(name + " const volatile* const volatile&"));
    }

    SECTION("As rvalue-ref pointer type")
    {
        REQUIRE_THAT(
            print_type<T*&&>(),
            Catch::Matchers::Equals(name + "*&&"));
        REQUIRE_THAT(
            print_type<T const*&&>(),
            Catch::Matchers::Equals(name + " const*&&"));
        REQUIRE_THAT(
            print_type<T volatile*&&>(),
            Catch::Matchers::Equals(name + " volatile*&&"));
        REQUIRE_THAT(
            print_type<T const volatile*&&>(),
            Catch::Matchers::Equals(name + " const volatile*&&"));

        REQUIRE_THAT(
            print_type<T* const&&>(),
            Catch::Matchers::Equals(name + "* const&&"));
        REQUIRE_THAT(
            print_type<T const* const&&>(),
            Catch::Matchers::Equals(name + " const* const&&"));
        REQUIRE_THAT(
            print_type<T volatile* const&&>(),
            Catch::Matchers::Equals(name + " volatile* const&&"));
        REQUIRE_THAT(
            print_type<T const volatile* const&&>(),
            Catch::Matchers::Equals(name + " const volatile* const&&"));

        REQUIRE_THAT(
            print_type<T* volatile&&>(),
            Catch::Matchers::Equals(name + "* volatile&&"));
        REQUIRE_THAT(
            print_type<T const* volatile&&>(),
            Catch::Matchers::Equals(name + " const* volatile&&"));
        REQUIRE_THAT(
            print_type<T volatile* volatile&&>(),
            Catch::Matchers::Equals(name + " volatile* volatile&&"));
        REQUIRE_THAT(
            print_type<T const volatile* volatile&&>(),
            Catch::Matchers::Equals(name + " const volatile* volatile&&"));

        REQUIRE_THAT(
            print_type<T* const volatile&&>(),
            Catch::Matchers::Equals(name + "* const volatile&&"));
        REQUIRE_THAT(
            print_type<T const* const volatile&&>(),
            Catch::Matchers::Equals(name + " const* const volatile&&"));
        REQUIRE_THAT(
            print_type<T volatile* const volatile&&>(),
            Catch::Matchers::Equals(name + " volatile* const volatile&&"));
        REQUIRE_THAT(
            print_type<T const volatile* const volatile&&>(),
            Catch::Matchers::Equals(name + " const volatile* const volatile&&"));
    }
}

#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

TEMPLATE_TEST_CASE_SIG(
    "detail::default_arg_for determines, whether the type is default argument for the template.",
    "[print][detail]",
    ((bool expected, typename DefaultArg, template <typename...> typename Template, typename... LeadingArgs), expected, DefaultArg, Template, LeadingArgs...),
    (false, int, std::greater),
    (false, int, std::type_identity),
    (false, int, std::tuple, float, double),

    (true, void, std::greater),
    (true, std::allocator<int>, std::vector, int))
{
    STATIC_REQUIRE(expected == printing::detail::default_arg_for<DefaultArg, Template, type_list<LeadingArgs...>>);
}

TEMPLATE_TEST_CASE_SIG(
    "detail::drop_default_args drops all arguments until the first non default arg is found (beginning at the end).",
    "[print][detail]",
    ((auto dummy, typename Expected, template <typename...> typename Template, typename... Args), dummy, Expected, Template, Args...),
    (std::ignore, type_list<>, std::greater, void),
    (std::ignore, type_list<int>, std::greater, int),
    (std::ignore, type_list<float, double>, std::tuple, float, double),
    (std::ignore, type_list<int>, std::vector, int, std::allocator<int>),
    (std::ignore, type_list<int, std::pmr::polymorphic_allocator<int>>, std::vector, int, std::pmr::polymorphic_allocator<int>))
{
    STATIC_REQUIRE(std::same_as<Expected, typename printing::detail::drop_default_args_for<Template, type_list<Args...>>::type>);
}

namespace
{
    template <typename T>
    struct custom_deleter
        : std::default_delete<T>
    {
    };
}

TEST_CASE(
    "std::unique_ptr with alternative deleter is printed completely.",
    "[print]")
{
    REQUIRE_THAT(
        (print_type<std::unique_ptr<int, custom_deleter<int>>>()),
        Catch::Matchers::Equals("std::unique_ptr<int, custom_deleter<int>>"));
    REQUIRE_THAT(
        (print_type<std::unique_ptr<std::unique_ptr<int, custom_deleter<int>>, custom_deleter<std::unique_ptr<int, custom_deleter<int>>>>>()),
        Catch::Matchers::Equals("std::unique_ptr<std::unique_ptr<int, custom_deleter<int>>, custom_deleter<std::unique_ptr<int, custom_deleter<int>>>>"));
    REQUIRE_THAT(
        (print_type<std::unique_ptr<std::unique_ptr<int>, custom_deleter<std::unique_ptr<int>>>>()),
        Catch::Matchers::Equals("std::unique_ptr<std::unique_ptr<int>, custom_deleter<std::unique_ptr<int>>>"));

    REQUIRE_THAT(
        (print_type<std::unique_ptr<int[], custom_deleter<int[]>>>()),
        Catch::Matchers::Equals("std::unique_ptr<int[], custom_deleter<int[]>>"));
    REQUIRE_THAT(
        (print_type<std::unique_ptr<std::unique_ptr<int>[], custom_deleter<std::unique_ptr<int>[]>>>()),
        Catch::Matchers::Equals("std::unique_ptr<std::unique_ptr<int>[], custom_deleter<std::unique_ptr<int>[]>>"));
}

TEST_CASE(
    "Types in anonymous namespaces are printed nicely.",
    "[print]")
{
    REQUIRE_THAT(
        print_type<my_type>(),
        Catch::Matchers::Equals("my_type"));
}

#endif

namespace
{
    template <typename T>
    struct custom_allocator
        : std::allocator<T>
    {
    };
}

#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING
TEMPLATE_TEST_CASE_SIG(
    "Signatures are printed nicely.",
    "[print]",
    ((auto baseName, typename Return, typename... Params), baseName, Return, Params...),
    (FixedString{"void()"}, void),
    (FixedString{"std::string&(float const&&, char*)"}, std::string&, float const&&, char*),
    (
        FixedString{"std::optional<std::vector<int, custom_allocator<int>>>(std::string&&, std::tuple<std::vector<int>> const*)"},
        (std::optional<std::vector<int, custom_allocator<int>>>, std::string&&, std::tuple<std::vector<int>> const*)))
#else
TEMPLATE_TEST_CASE_SIG(
    "Signatures are printed nicely.",
    "[print]",
    ((auto baseName, typename Return, typename... Params), baseName, Return, Params...),
    (FixedString{"void()"}, void),
    (FixedString{"std::string&(float const&&, char*)"}, std::string&, float const&&, char*))
#endif
{
    SECTION("Plain function.")
    {
        REQUIRE_THAT(
            print_type<Return(Params...)>(),
            Catch::Matchers::Equals(StringT{baseName}));
        REQUIRE_THAT(
            print_type<Return(Params...) noexcept>(),
            Catch::Matchers::Equals(StringT{baseName} + " noexcept"));
    }

    SECTION("const function.")
    {
        REQUIRE_THAT(
            print_type<Return(Params...) const>(),
            Catch::Matchers::Equals(StringT{baseName} + " const"));
        REQUIRE_THAT(
            print_type<Return(Params...) const noexcept>(),
            Catch::Matchers::Equals(StringT{baseName} + " const noexcept"));
    }

    SECTION("& function.")
    {
        REQUIRE_THAT(
            print_type<Return(Params...)&>(),
            Catch::Matchers::Equals(StringT{baseName} + " &"));
        REQUIRE_THAT(
            print_type < Return(Params...) & noexcept > (),
            Catch::Matchers::Equals(StringT{baseName} + " & noexcept"));
    }

    SECTION("const & function.")
    {
        REQUIRE_THAT(
            print_type<Return(Params...) const&>(),
            Catch::Matchers::Equals(StringT{baseName} + " const &"));
        REQUIRE_THAT(
            print_type < Return(Params...) const& noexcept > (),
            Catch::Matchers::Equals(StringT{baseName} + " const & noexcept"));
    }

    SECTION("&& function.")
    {
        REQUIRE_THAT(
            print_type<Return(Params...) &&>(),
            Catch::Matchers::Equals(StringT{baseName} + " &&"));
        REQUIRE_THAT(
            print_type < Return(Params...) && noexcept > (),
            Catch::Matchers::Equals(StringT{baseName} + " && noexcept"));
    }

    SECTION("const && function.")
    {
        REQUIRE_THAT(
            print_type<Return(Params...) const&&>(),
            Catch::Matchers::Equals(StringT{baseName} + " const &&"));
        REQUIRE_THAT(
            print_type < Return(Params...) const&& noexcept > (),
            Catch::Matchers::Equals(StringT{baseName} + " const && noexcept"));
    }
}
