//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/String.hpp"

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
    "is_character(_v) determines, whether the given type is a character type.",
    "[string][string::traits]",
    ((bool expected, typename T), expected, T),
    (false, int),
    (false, bool),
    (false, std::string),
    (false, const char),
    (false, char&),
    (false, char&&),
    (false, char*),

    (true, char),
    (true, unsigned char),
    (true, signed char),
    (true, wchar_t),
    (true, char8_t),
    (true, char16_t),
    (true, char32_t))
{
    STATIC_REQUIRE(expected == mimicpp::is_character<T>::value);
    STATIC_REQUIRE(expected == mimicpp::is_character_v<T>);
}

namespace
{
    class CustomCharWithLiteral
    {
    };

    class CustomCharWithoutLiteral
    {
    };
}

template <>
struct mimicpp::is_character<CustomCharWithLiteral>
    : std::true_type
{
};

template <>
inline constexpr StringViewT mimicpp::string_literal_prefix<CustomCharWithLiteral>{"Custom"};

template <>
struct mimicpp::is_character<CustomCharWithoutLiteral>
    : std::true_type
{
};

TEST_CASE(
    "string_literal_prefix yields the printable string-literal prefix for the given char-type.",
    "[string][string::traits]")
{
    SECTION("For char.")
    {
        STATIC_REQUIRE(string_literal_prefix<char>.empty());
    }

    SECTION("For wchar_t.")
    {
        STATIC_REQUIRE(StringViewT{"L"} == string_literal_prefix<wchar_t>);
    }

    SECTION("For char8_t.")
    {
        STATIC_REQUIRE(StringViewT{"u8"} == string_literal_prefix<char8_t>);
    }

    SECTION("For char16_t.")
    {
        STATIC_REQUIRE(StringViewT{"u"} == string_literal_prefix<char16_t>);
    }

    SECTION("For char32_t.")
    {
        STATIC_REQUIRE(StringViewT{"U"} == string_literal_prefix<char32_t>);
    }

    SECTION("For custom types without literal specialization.")
    {
        STATIC_REQUIRE(string_literal_prefix<CustomCharWithoutLiteral>.empty());
    }

    SECTION("For custom types with literal specialization.")
    {
        STATIC_REQUIRE(StringViewT{"Custom"} == string_literal_prefix<CustomCharWithLiteral>);
    }
}

namespace
{
    class CustomString
    {
    public:
        std::string inner{};
    };
}

template <>
struct mimicpp::string_traits<CustomString>
{
    using char_t = char;

    [[nodiscard]]
    static constexpr std::string_view view(const CustomString& str) noexcept
    {
        return std::string_view{str.inner};
    }
};

TEMPLATE_TEST_CASE_SIG(
    "string_traits contains properties for the provided string type.",
    "[string][string::traits]",
    ((bool dummy, typename T, typename Char, typename ViewType), dummy, T, Char, ViewType),
    (true, char*, char, std::string_view),
    (true, const char*, char, std::string_view),
    (true, wchar_t*, wchar_t, std::wstring_view),
    (true, const wchar_t*, wchar_t, std::wstring_view),
    (true, char8_t*, char8_t, std::u8string_view),
    (true, const char8_t*, char8_t, std::u8string_view),
    (true, char16_t*, char16_t, std::u16string_view),
    (true, const char16_t*, char16_t, std::u16string_view),
    (true, char32_t*, char32_t, std::u32string_view),
    (true, const char32_t*, char32_t, std::u32string_view),

    (true, char[], char, std::string_view),
    (true, const char[], char, std::string_view),
    (true, char[42], char, std::string_view),
    (true, const char[42], char, std::string_view),

    (true, std::string, char, std::string),
    (true, std::wstring, wchar_t, std::wstring),
    (true, std::u8string, char8_t, std::u8string),
    (true, std::u16string, char16_t, std::u16string),
    (true, std::u32string, char32_t, std::u32string),

    (true, std::string_view, char, std::string_view),
    (true, std::wstring_view, wchar_t, std::wstring_view),
    (true, std::u8string_view, char8_t, std::u8string_view),
    (true, std::u16string_view, char16_t, std::u16string_view),
    (true, std::u32string_view, char32_t, std::u32string_view),

    (true, CustomString, char, std::string_view))
{
    STATIC_REQUIRE(std::same_as<Char, typename string_traits<T>::char_t>);
    STATIC_REQUIRE(std::same_as<Char, string_char_t<T>>);
    STATIC_REQUIRE(std::same_as<Char, string_char_t<const T>>);
    STATIC_REQUIRE(std::same_as<Char, string_char_t<T&>>);
    STATIC_REQUIRE(std::same_as<Char, string_char_t<const T&>>);
    STATIC_REQUIRE(std::same_as<Char, string_char_t<T&&>>);
    STATIC_REQUIRE(std::same_as<Char, string_char_t<const T&&>>);

    STATIC_REQUIRE(std::ranges::forward_range<string_view_t<T>>);
    STATIC_REQUIRE(std::ranges::forward_range<string_view_t<const T>>);
    STATIC_REQUIRE(std::ranges::forward_range<string_view_t<T&>>);
    STATIC_REQUIRE(std::ranges::forward_range<string_view_t<const T&>>);
    STATIC_REQUIRE(std::ranges::forward_range<string_view_t<T&&>>);
    STATIC_REQUIRE(std::ranges::forward_range<string_view_t<const T&&>>);

    STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<T>>, Char>);
    STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<const T>>, Char>);
    STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<T&>>, Char>);
    STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<const T&>>, Char>);
    STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<T&&>>, Char>);
    STATIC_REQUIRE(std::convertible_to<std::ranges::range_reference_t<string_view_t<const T&&>>, Char>);
}

TEMPLATE_TEST_CASE(
    "string_traits does not work for actual non-string-types.",
    "[string][string::traits]",
    char,
    const char,
    int,
    std::optional<std::string>,
    std::string&)
{
    using traits_t = string_traits<TestType>;

    STATIC_REQUIRE(!requires { traits_t{}; });
}

TEMPLATE_TEST_CASE(
    "Common char-strings support string_traits::view.",
    "[string][string::traits]",
    char*,
    const char*,
    char (&)[14],
    const char (&)[14],
    std::string,
    const std::string,
    std::string_view,
    const std::string_view)
{
    namespace Matches = Catch::Matchers;
    using traits_t = string_traits<std::remove_cvref_t<TestType>>;

    char str[] = "Hello, World!";

    REQUIRE_THAT(
        traits_t::view(static_cast<TestType>(str)),
        Matches::RangeEquals(std::string_view{str}));
}

TEMPLATE_TEST_CASE(
    "Common wchar_t-strings support string_traits::view.",
    "[string][string::traits]",
    wchar_t*,
    const wchar_t*,
    wchar_t (&)[14],
    const wchar_t (&)[14],
    std::wstring,
    const std::wstring,
    std::wstring_view,
    const std::wstring_view)
{
    namespace Matches = Catch::Matchers;
    using traits_t = string_traits<std::remove_cvref_t<TestType>>;

    wchar_t str[] = L"Hello, World!";

    REQUIRE_THAT(
        traits_t::view(static_cast<TestType>(str)),
        Matches::RangeEquals(std::wstring_view{str}));
}

TEMPLATE_TEST_CASE(
    "Common char8_t-strings support string_traits::view.",
    "[string][string::traits]",
    char8_t*,
    const char8_t*,
    char8_t (&)[14],
    const char8_t (&)[14],
    std::u8string,
    const std::u8string,
    std::u8string_view,
    const std::u8string_view)
{
    namespace Matches = Catch::Matchers;
    using traits_t = string_traits<std::remove_cvref_t<TestType>>;

    char8_t str[] = u8"Hello, World!";

    REQUIRE_THAT(
        traits_t::view(static_cast<TestType>(str)),
        Matches::RangeEquals(std::u8string_view{str}));
}

TEMPLATE_TEST_CASE(
    "Common char16_t-strings support string_traits::view.",
    "[string][string::traits]",
    char16_t*,
    const char16_t*,
    char16_t (&)[14],
    const char16_t (&)[14],
    std::u16string,
    const std::u16string,
    std::u16string_view,
    const std::u16string_view)
{
    namespace Matches = Catch::Matchers;
    using traits_t = string_traits<std::remove_cvref_t<TestType>>;

    char16_t str[] = u"Hello, World!";

    REQUIRE_THAT(
        traits_t::view(static_cast<TestType>(str)),
        Matches::RangeEquals(std::u16string_view{str}));
}

TEMPLATE_TEST_CASE(
    "Common char32_t-strings support string_traits::view.",
    "[string][string::traits]",
    char32_t*,
    const char32_t*,
    char32_t (&)[14],
    const char32_t (&)[14],
    std::u32string,
    const std::u32string,
    std::u32string_view,
    const std::u32string_view)
{
    namespace Matches = Catch::Matchers;
    using traits_t = string_traits<std::remove_cvref_t<TestType>>;

    char32_t str[] = U"Hello, World!";

    REQUIRE_THAT(
        traits_t::view(static_cast<TestType>(str)),
        Matches::RangeEquals(std::u32string_view{str}));
}

TEMPLATE_TEST_CASE_SIG(
    "string concept determines, whether the given type can be used as a string-type.",
    "[string]",
    ((bool expected, typename T), expected, T),
    (false, char),
    (false, void*),
    (false, const void*),

    (true, CustomString),

    (true, char*),
    (true, const char*),
    (true, wchar_t*),
    (true, const wchar_t*),
    (true, char8_t*),
    (true, const char8_t*),
    (true, char16_t*),
    (true, const char16_t*),
    (true, char32_t*),
    (true, const char32_t*),

    (true, char[]),
    (true, const char[]),
    (true, char[42]),
    (true, const char[42]),
    (true, wchar_t[]),
    (true, const wchar_t[]),
    (true, wchar_t[42]),
    (true, const wchar_t[42]),
    (true, char8_t[]),
    (true, const char8_t[]),
    (true, char8_t[42]),
    (true, const char8_t[42]),
    (true, char16_t[]),
    (true, const char16_t[]),
    (true, char16_t[42]),
    (true, const char16_t[42]),
    (true, char32_t[]),
    (true, const char32_t[]),
    (true, char32_t[42]),
    (true, const char32_t[42]),

    (true, std::string),
    (true, std::wstring),
    (true, std::u8string),
    (true, std::u16string),
    (true, std::u32string),

    (true, std::string_view),
    (true, std::wstring_view),
    (true, std::u8string_view),
    (true, std::u16string_view),
    (true, std::u32string_view))
{
    STATIC_REQUIRE(expected == mimicpp::string<T>);
    STATIC_REQUIRE(expected == mimicpp::string<const T>);
    STATIC_REQUIRE(expected == mimicpp::string<T&>);
    STATIC_REQUIRE(expected == mimicpp::string<const T&>);
    STATIC_REQUIRE(expected == mimicpp::string<T&&>);
    STATIC_REQUIRE(expected == mimicpp::string<const T&&>);
}

TEMPLATE_TEST_CASE(
    "string_case_fold_converter<char> can be used with all char-strings.",
    "[string]",
    const char*,
    std::string,
    std::string_view)
{
    namespace Matches = Catch::Matchers;

    const auto [lhs, rhs] = GENERATE(
        (table<std::string, const char*>)({
            {             "",              ""},
            {     " !1337\t",      " !1337\t"},
            {"hElLo, wOrLd!", "HeLlO, WoRlD!"},
    }));

    auto convertedRhs = static_cast<TestType>(rhs);
    constexpr string_case_fold_converter<char> caseFolder{};

    auto caseFoldedRhs = std::invoke(
        caseFolder,
        string_traits<TestType>::view(convertedRhs));
    auto caseFoldedLhs = std::invoke(
        caseFolder,
        std::string_view{lhs});

    REQUIRE_THAT(
        caseFoldedLhs,
        Matches::RangeEquals(caseFoldedRhs));
}

#ifndef MIMICPP_CONFIG_EXPERIMENTAL_UNICODE_STR_MATCHER

TEMPLATE_TEST_CASE_SIG(
    "case_foldable_string determines, whether the given string supports normalize conversions.",
    "[string]",
    ((bool expected, typename T), expected, T),
    (false, char),

    (true, char*),
    (true, const char*),
    (true, char[]),
    (true, const char[]),
    (true, char[42]),
    (true, const char[42]),
    (true, std::string),
    (true, std::string_view),

    (false, wchar_t*),
    (false, const wchar_t*),
    (false, wchar_t[]),
    (false, const wchar_t[]),
    (false, wchar_t[42]),
    (false, const wchar_t[42]),
    (false, std::wstring),
    (false, std::wstring_view),

    (false, char8_t*),
    (false, const char8_t*),
    (false, char8_t[]),
    (false, const char8_t[]),
    (false, char8_t[42]),
    (false, const char8_t[42]),
    (false, std::u8string),
    (false, std::u8string_view),

    (false, char16_t*),
    (false, const char16_t*),
    (false, char16_t[]),
    (false, const char16_t[]),
    (false, char16_t[42]),
    (false, const char16_t[42]),
    (false, std::u16string),
    (false, std::u16string_view),

    (false, char32_t*),
    (false, const char32_t*),
    (false, char32_t[]),
    (false, const char32_t[]),
    (false, char32_t[42]),
    (false, const char32_t[42]),
    (false, std::u32string),
    (false, std::u32string_view))
{
    STATIC_REQUIRE(expected == case_foldable_string<T>);
    STATIC_REQUIRE(expected == case_foldable_string<const T>);
    STATIC_REQUIRE(expected == case_foldable_string<T&>);
    STATIC_REQUIRE(expected == case_foldable_string<const T&>);
    STATIC_REQUIRE(expected == case_foldable_string<T&&>);
    STATIC_REQUIRE(expected == case_foldable_string<const T&&>);
}

#else

TEMPLATE_TEST_CASE(
    "string_case_fold_converter<wchar_t> can be used with all utf16-strings.",
    "[string]",
    const wchar_t*,
    std::wstring,
    std::wstring_view)
{
    namespace Matches = Catch::Matchers;

    const auto [lhs, rhs] = GENERATE(
        (table<std::wstring, const wchar_t*>)({
            {             L"",              L""},
            {     L" !1337\t",      L" !1337\t"},
            {L"hElLo, wOrLd!", L"HeLlO, WoRlD!"},
            {           L"ss",        L"\u00DF"}, // german "sharp s"
            {           L"SS",        L"\u00DF"}, // german "sharp s"
            {       L"\u01CB",        L"\u01CA"},
            {       L"\u01CC",        L"\u01CC"}
    }));

    auto convertedRhs = static_cast<TestType>(rhs);
    constexpr string_case_fold_converter<wchar_t> caseFolder{};

    auto caseFoldedRhs = std::invoke(
        caseFolder,
        string_traits<TestType>::view(convertedRhs));
    auto caseFoldedLhs = std::invoke(
        caseFolder,
        std::wstring_view{lhs});

    REQUIRE_THAT(
        caseFoldedLhs,
        Matches::RangeEquals(caseFoldedRhs));
}

TEMPLATE_TEST_CASE(
    "string_case_fold_converter<char8_t> can be used with all utf8-strings.",
    "[string]",
    const char8_t*,
    std::u8string,
    std::u8string_view)
{
    namespace Matches = Catch::Matchers;

    const auto [lhs, rhs] = GENERATE(
        (table<std::u8string, const char8_t*>)({
            {             u8"",              u8""},
            {     u8" !1337\t",      u8" !1337\t"},
            {u8"hElLo, wOrLd!", u8"HeLlO, WoRlD!"},
            {           u8"ss",        u8"\u00DF"}, // german "sharp s"
            {           u8"SS",        u8"\u00DF"}, // german "sharp s"
            {       u8"\u01CB",        u8"\u01CA"},
            {       u8"\u01CC",        u8"\u01CC"}
    }));

    auto convertedRhs = static_cast<TestType>(rhs);
    constexpr string_case_fold_converter<char8_t> caseFolder{};

    auto caseFoldedRhs = std::invoke(
        caseFolder,
        string_traits<TestType>::view(convertedRhs));
    auto caseFoldedLhs = std::invoke(
        caseFolder,
        std::u8string_view{lhs});

    REQUIRE_THAT(
        caseFoldedLhs,
        Matches::RangeEquals(caseFoldedRhs));
}

TEMPLATE_TEST_CASE(
    "string_case_fold_converter<char16_t> can be used with all utf16-strings.",
    "[string]",
    const char16_t*,
    std::u16string,
    std::u16string_view)
{
    namespace Matches = Catch::Matchers;

    const auto [lhs, rhs] = GENERATE(
        (table<std::u16string, const char16_t*>)({
            {             u"",              u""},
            {     u" !1337\t",      u" !1337\t"},
            {u"hElLo, wOrLd!", u"HeLlO, WoRlD!"},
            {           u"ss",        u"\u00DF"}, // german "sharp s"
            {           u"SS",        u"\u00DF"}, // german "sharp s"
            {       u"\u01CB",        u"\u01CA"},
            {       u"\u01CC",        u"\u01CC"}
    }));

    auto convertedRhs = static_cast<TestType>(rhs);
    constexpr string_case_fold_converter<char16_t> caseFolder{};

    auto caseFoldedRhs = std::invoke(
        caseFolder,
        string_traits<TestType>::view(convertedRhs));
    auto caseFoldedLhs = std::invoke(
        caseFolder,
        std::u16string_view{lhs});

    REQUIRE_THAT(
        caseFoldedLhs,
        Matches::RangeEquals(caseFoldedRhs));
}

TEMPLATE_TEST_CASE(
    "string_case_fold_converter<char32_t> can be used with all utf32-strings.",
    "[string]",
    const char32_t*,
    std::u32string,
    std::u32string_view)
{
    namespace Matches = Catch::Matchers;

    const auto [lhs, rhs] = GENERATE(
        (table<std::u32string, const char32_t*>)({
            {             U"",              U""},
            {     U" !1337\t",      U" !1337\t"},
            {U"hElLo, wOrLd!", U"HeLlO, WoRlD!"},
            {           U"ss",        U"\u00DF"}, // german "sharp s"
            {           U"SS",        U"\u00DF"}, // german "sharp s"
            {       U"\u01CB",        U"\u01CA"},
            {       U"\u01CC",        U"\u01CC"}
    }));

    auto convertedRhs = static_cast<TestType>(rhs);
    constexpr string_case_fold_converter<char32_t> caseFolder{};

    auto caseFoldedRhs = std::invoke(
        caseFolder,
        string_traits<TestType>::view(convertedRhs));
    auto caseFoldedLhs = std::invoke(
        caseFolder,
        std::u32string_view{lhs});

    REQUIRE_THAT(
        caseFoldedLhs,
        Matches::RangeEquals(caseFoldedRhs));
}

TEMPLATE_TEST_CASE_SIG(
    "case_foldable_string determines, whether the given string supports normalize conversions.",
    "[string]",
    ((bool expected, typename T), expected, T),
    (false, char),

    (true, char*),
    (true, const char*),
    (true, char[]),
    (true, const char[]),
    (true, char[42]),
    (true, const char[42]),
    (true, std::string),
    (true, std::string_view),

    (true, wchar_t*),
    (true, const wchar_t*),
    (true, wchar_t[]),
    (true, const wchar_t[]),
    (true, wchar_t[42]),
    (true, const wchar_t[42]),
    (true, std::wstring),
    (true, std::wstring_view),

    (true, char8_t*),
    (true, const char8_t*),
    (true, char8_t[]),
    (true, const char8_t[]),
    (true, char8_t[42]),
    (true, const char8_t[42]),
    (true, std::u8string),
    (true, std::u8string_view),

    (true, char16_t*),
    (true, const char16_t*),
    (true, char16_t[]),
    (true, const char16_t[]),
    (true, char16_t[42]),
    (true, const char16_t[42]),
    (true, std::u16string),
    (true, std::u16string_view),

    (true, char32_t*),
    (true, const char32_t*),
    (true, char32_t[]),
    (true, const char32_t[]),
    (true, char32_t[42]),
    (true, const char32_t[42]),
    (true, std::u32string),
    (true, std::u32string_view))
{
    STATIC_REQUIRE(expected == case_foldable_string<T>);
    STATIC_REQUIRE(expected == case_foldable_string<const T>);
    STATIC_REQUIRE(expected == case_foldable_string<T&>);
    STATIC_REQUIRE(expected == case_foldable_string<const T&>);
    STATIC_REQUIRE(expected == case_foldable_string<T&&>);
    STATIC_REQUIRE(expected == case_foldable_string<const T&&>);
}

#endif
