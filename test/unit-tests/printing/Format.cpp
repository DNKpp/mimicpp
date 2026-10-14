//          Copyright Dominic (DNKpp) Koepke 2024-2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/Format.hpp"
#include "mimic++/printing/state/Print.hpp"

using namespace mimicpp;

namespace
{
    class NonPrintable
    {
    };
}

TEMPLATE_TEST_CASE_SIG(
    "format::formattable determines whether the given type has a format::formatter specialization.",
    "[print]",
    ((bool expected, typename T, typename Char), expected, T, Char),
    (true, int, char),
    (true, const int, char),
    (true, int&, char),
    (true, const int&, char),
    (true, int, wchar_t),
    (false, NonPrintable, char))
{
    STATIC_REQUIRE(expected == format::formattable<T, Char>);
}

TEST_CASE(
    "format::fallback_formattable_t supports the formatting options of the underlying type.",
    "[print]")
{
    CHECK_THAT(
        format::format("{}", format::fallback_formattable("Hello, World!")),
        Catch::Matchers::Equals("Hello, World!"));

    constexpr int value{42};
    CHECK_THAT(
        format::format("1234 {:#x} {} 5678", format::fallback_formattable(value), format::fallback_formattable("Hello, World")),
        Catch::Matchers::Equals(R"(1234 0x2a Hello, World 5678)"));
}

namespace
{
    struct Printable
    {
        int value{};
    };
}

template <>
class mimicpp::custom::Printer<::Printable>
{
public:
    static auto print(auto outIter, Printable const& object)
    {
        return format::format_to(
            outIter,
            "Object of Printable has value: {}",
            object.value);
    }
};

TEST_CASE(
    "format::fallback_formattable falls back to mimicpp::print if no formatter is available.",
    "[print]")
{
    CHECK_THAT(
        format::format("{}", format::fallback_formattable(Printable{.value = 56})),
        Catch::Matchers::Equals("Object of Printable has value: 56"));
}
