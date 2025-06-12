//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/StatePrinter.hpp"
#include "mimic++/printing/state/C++23Backports.hpp"
#include "mimic++/printing/state/CommonTypes.hpp"

#include <memory>
#include <span>

using namespace mimicpp;

namespace
{
    using StringPrintIteratorT = std::string::iterator;
    using WStringPrintIteratorT = std::wstring::iterator;

    class NonPrinter
    {
    };

    class CustomIntPrinter
    {
    public:
        template <typename OutIter>
        static OutIter print(OutIter out, [[maybe_unused]] int& value)
        {
            return out;
        }
    };

    class CustomConstIntPrinter
    {
    public:
        template <typename OutIter>
        static OutIter print(OutIter out, [[maybe_unused]] const int& value)
        {
            return out;
        }
    };
}

TEMPLATE_TEST_CASE_SIG(
    "printer_for determines whether the given type satisfies the requirements.",
    "[print]",
    ((bool expected, typename Printer, typename OutIter, typename T), expected, Printer, OutIter, T),
    (false, NonPrinter, StringPrintIteratorT, int),
    (false, NonPrinter, WStringPrintIteratorT, int),

    (false, CustomIntPrinter, StringPrintIteratorT, const float),
    (false, CustomIntPrinter, WStringPrintIteratorT, const float),
    (true, CustomIntPrinter, StringPrintIteratorT, int),
    (true, CustomIntPrinter, WStringPrintIteratorT, int),
    (false, CustomIntPrinter, StringPrintIteratorT, const int),
    (false, CustomIntPrinter, WStringPrintIteratorT, const int),

    (false, CustomConstIntPrinter, StringPrintIteratorT, const std::string),
    (false, CustomConstIntPrinter, WStringPrintIteratorT, const std::wstring),
    (true, CustomConstIntPrinter, StringPrintIteratorT, int),
    (true, CustomConstIntPrinter, WStringPrintIteratorT, int),
    (true, CustomConstIntPrinter, StringPrintIteratorT, const int),
    (true, CustomConstIntPrinter, WStringPrintIteratorT, const int))
{
    STATIC_REQUIRE(expected == printer_for<Printer, OutIter, T>);
}

namespace
{
    class NonPrintable
    {
    };

    class CustomPrintable
    {
    };

    class InternalPrintable
    {
    };

    class CustomAndInternalPrintable
    {
    };

    class FormatPrintable
    {
    };

    class FormatAndCustomPrintable
    {
    };
}

template <>
class custom::Printer<CustomPrintable>
{
public:
    inline static int printCallCounter{0};

    static auto print(print_iterator auto out, const CustomPrintable&)
    {
        ++printCallCounter;
        return format::format_to(out, "CustomPrintable");
    }
};

template <>
struct printing::detail::state::common_type_printer<InternalPrintable>
{
    inline static int printCallCounter{0};

    static auto print(print_iterator auto out, const InternalPrintable&)
    {
        ++printCallCounter;
        return format::format_to(out, "InternalPrintable");
    }
};

template <>
class custom::Printer<CustomAndInternalPrintable>
{
public:
    inline static int printCallCounter{0};

    static auto print(print_iterator auto out, const CustomAndInternalPrintable&)
    {
        ++printCallCounter;
        return format::format_to(out, "CustomAndInternalPrintable - custom::Printer");
    }
};

template <>
struct printing::detail::state::common_type_printer<CustomAndInternalPrintable>
{
    inline static int printCallCounter{0};

    static auto print(print_iterator auto out, const CustomAndInternalPrintable&)
    {
        ++printCallCounter;
        return format::format_to(out, "CustomAndInternalPrintable - detail::Printer");
    }
};

template <typename Char>
struct format::formatter<FormatPrintable, Char>
{
    inline static int printCallCounter{0};

    static constexpr auto parse(auto& ctx)
    {
        return ctx.begin();
    }

    static auto format(const FormatPrintable&, auto& ctx)
    {
        ++printCallCounter;
        return format::format_to(ctx.out(), "FormatPrintable");
    }
};

template <>
class custom::Printer<FormatAndCustomPrintable>
{
public:
    inline static int printCallCounter{0};

    static auto print(print_iterator auto out, const FormatAndCustomPrintable&)
    {
        ++printCallCounter;
        return format::format_to(out, "FormatAndCustomPrintable");
    }
};

template <typename Char>
struct format::formatter<FormatAndCustomPrintable, Char>
{
    inline static int printCallCounter{0};

    static constexpr auto parse(auto& ctx)
    {
        return ctx.begin();
    }

    static auto format(const FormatAndCustomPrintable&, auto& ctx)
    {
        ++printCallCounter;
        return format::format_to(ctx.out(), "FormatAndCustomPrintable");
    }
};

TEST_CASE(
    "print selects the best option to print a given value.",
    "[print]")
{
    std::basic_stringstream<CharT> stream{};

    SECTION("custom::Printer specialization is supported.")
    {
        using PrinterT = custom::Printer<CustomPrintable>;
        PrinterT::printCallCounter = 0;

        constexpr CustomPrintable value{};

        print(std::ostreambuf_iterator{stream}, value);
        REQUIRE(PrinterT::printCallCounter == 1);

        REQUIRE_THAT(
            std::move(stream).str(),
            Catch::Matchers::Equals("CustomPrintable"));
    }

    SECTION("internal Printer specializations are supported.")
    {
        using PrinterT = printing::detail::state::common_type_printer<InternalPrintable>;
        PrinterT::printCallCounter = 0;

        constexpr InternalPrintable value{};

        print(std::ostreambuf_iterator{stream}, value);
        REQUIRE(PrinterT::printCallCounter == 1);

        REQUIRE_THAT(
            std::move(stream).str(),
            Catch::Matchers::Equals("InternalPrintable"));
    }

    SECTION("custom::Printer is preferred, if both printers have valid specializations.")
    {
        using CustomPrinterT = custom::Printer<CustomAndInternalPrintable>;
        using DetailPrinterT = printing::detail::state::common_type_printer<CustomAndInternalPrintable>;
        CustomPrinterT::printCallCounter = 0;
        DetailPrinterT::printCallCounter = 0;

        constexpr CustomAndInternalPrintable value{};

        print(std::ostreambuf_iterator{stream}, value);
        REQUIRE(CustomPrinterT::printCallCounter == 1);
        REQUIRE(DetailPrinterT::printCallCounter == 0);

        REQUIRE_THAT(
            stream.str(),
            Catch::Matchers::Equals("CustomAndInternalPrintable - custom::Printer"));
    }

    SECTION("format::formatter specialization is supported.")
    {
        using PrinterT = format::formatter<FormatPrintable, CharT>;
        PrinterT::printCallCounter = 0;

        constexpr FormatPrintable value{};

        print(std::ostreambuf_iterator{stream}, value);
        REQUIRE(PrinterT::printCallCounter == 1);
        REQUIRE_THAT(
            std::move(stream).str(),
            Catch::Matchers::Equals("FormatPrintable"));
    }

    SECTION("custom::Printer specialization is preferred.")
    {
        using PrinterT = custom::Printer<FormatAndCustomPrintable>;
        PrinterT::printCallCounter = 0;

        constexpr FormatAndCustomPrintable value{};

        print(std::ostreambuf_iterator{stream}, value);
        REQUIRE(PrinterT::printCallCounter == 1);
        REQUIRE_THAT(
            std::move(stream).str(),
            Catch::Matchers::Equals("FormatAndCustomPrintable"));
    }

    SECTION("Strings have special treatment.")
    {
        SECTION("Empty strings are supported.")
        {
            const std::string str{};

            print(std::ostreambuf_iterator{stream}, str);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals("\"\""));
        }

        SECTION("Strings with arbitrary size are supported.")
        {
            const std::string str{"Hello, World!"};

            print(std::ostreambuf_iterator{stream}, str);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals("\"Hello, World!\""));
        }

        SECTION("Raw string literals are supported.")
        {
            print(std::ostreambuf_iterator{stream}, "Hello, World!");
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals("\"Hello, World!\""));
        }

        SECTION("std::string_views are supported.")
        {
            print(std::ostreambuf_iterator{stream}, StringViewT{"Hello, World!"});
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals("\"Hello, World!\""));
        }
    }

#ifdef __cpp_lib_source_location
    SECTION("std::source_location has specialized printer.")
    {
        std::source_location constexpr loc = std::source_location::current();

        REQUIRE_THAT(
            mimicpp::print(loc),
            Catch::Matchers::Matches(
                // should be close enough to what is allowed on all systems
                R"(`((\d|\w|_|-|\+|\*)+(\\|/)){2})"
                R"(StatePrinter\.cpp`)"
                R"(#L\d+, `.+`)"));
    }
#endif

    SECTION("std::optional and std::nullopt_t have special treatment")
    {
        REQUIRE_THAT(
            mimicpp::print(std::nullopt),
            Catch::Matchers::Equals("nullopt"));
        REQUIRE_THAT(
            mimicpp::print(std::optional<int>{}),
            Catch::Matchers::Equals("nullopt"));
        REQUIRE_THAT(
            mimicpp::print(std::optional<NonPrintable>{}),
            Catch::Matchers::Equals("nullopt"));
        REQUIRE_THAT(
            mimicpp::print(std::optional{1337}),
            Catch::Matchers::Equals("1337"));
        REQUIRE_THAT(
            mimicpp::print(std::optional{NonPrintable{}}),
            Catch::Matchers::Equals("{?}"));
    }

    SECTION("When nothing matches, a default token is inserted.")
    {
        constexpr NonPrintable value{};
        print(std::ostreambuf_iterator{stream}, value);
        REQUIRE_THAT(
            std::move(stream).str(),
            Catch::Matchers::Equals("{?}"));
    }
}

TEST_CASE(
    "All forward-ranges are printable.",
    "[print]")
{
    StringStreamT stream{};

    SECTION("Empty ranges are supported.")
    {
        const std::vector<int> vec{};

        StringT const expected{"[]"};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, vec);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                print(vec),
                Catch::Matchers::Equals(expected));
        }
    }

    SECTION("Ranges with single element are supported.")
    {
        const std::vector vec{42};

        StringT const expected{"[42]"};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, vec);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                print(vec),
                Catch::Matchers::Equals(expected));
        }
    }

    SECTION("Ranges with multiple elements are supported.")
    {
        const std::vector vec{42, 1337};

        StringT const expected{"[42, 1337]"};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, vec);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                print(vec),
                Catch::Matchers::Equals(expected));
        }
    }

    SECTION("Range elements are forwarded to the print function..")
    {
        const std::vector vec{NonPrintable{}, NonPrintable{}, NonPrintable{}};

        StringT const expected{"[{?}, {?}, {?}]"};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, vec);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                print(vec),
                Catch::Matchers::Equals(expected));
        }
    }

    SECTION("All std views are supported.")
    {
        std::vector const vec{42, 1337};

        // mutable func removes const begin/end overloads from transform_view
        auto func = [](const auto v) mutable { return 2 * v; };

        StringT const expected{"[84, 2674]"};

        SECTION("Printing to specific out-iter.")
        {
            print(
                std::ostreambuf_iterator{stream},
                vec | std::views::transform(func));
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                print(vec | std::views::transform(func)),
                Catch::Matchers::Equals(expected));
        }
    }
}

TEST_CASE(
    "All tuple-like types are printable.",
    "[print]")
{
    StringStreamT stream{};

    SECTION("Empty tuples.")
    {
        StringT const expected{"()"};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, std::tuple{});
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                mimicpp::print(std::tuple{}),
                Catch::Matchers::Equals(expected));
        }
    }

    SECTION("Tuples with printable elements.")
    {
        StringT const expected{"(1337)"};

        constexpr std::tuple tuple{1337};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, tuple);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                mimicpp::print(tuple),
                Catch::Matchers::Equals(expected));
        }
    }

    SECTION("Tuples with mixed elements.")
    {
        StringT const expected{"({?}, 1337)"};

        constexpr std::tuple tuple{NonPrintable{}, 1337};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, tuple);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                mimicpp::print(tuple),
                Catch::Matchers::Equals(expected));
        }
    }

    SECTION("Tuples with view element.")
    {
        StringT const expected{"([42, 1337])"};

        // mutable func removes const begin/end overloads from transform_view
        auto func = [](const auto v) mutable { return v; };
        std::vector const vec{42, 1337};
        std::tuple tuple{vec | std::views::transform(func)};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, tuple);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                mimicpp::print(tuple),
                Catch::Matchers::Equals(expected));
        }
    }

    SECTION("Pairs with printable elements.")
    {
        StringT const expected{"(1337, 42)"};

        constexpr std::pair pair{1337, 42};

        SECTION("Printing to specific out-iter.")
        {
            print(std::ostreambuf_iterator{stream}, pair);
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals(expected));
        }

        SECTION("Printing to string")
        {
            REQUIRE_THAT(
                mimicpp::print(pair),
                Catch::Matchers::Equals(expected));
        }
    }
}

TEST_CASE(
    "Pointer-likes are printed in hex-format."
    "[print]")
{
    SECTION("Explicit nullptr.")
    {
        REQUIRE_THAT(
            mimicpp::print(nullptr),
            Catch::Matchers::Matches("nullptr"));
    }

    SECTION("Raw pointers.")
    {
        REQUIRE_THAT(
            mimicpp::print(reinterpret_cast<const void*>(std::uintptr_t{})),
            Catch::Matchers::Matches("nullptr"));

        REQUIRE_THAT(
            mimicpp::print(reinterpret_cast<const char*>(std::uintptr_t{0x1234u})),
            Catch::Matchers::Matches("0x0{1,12}1234"));
        REQUIRE_THAT(
            mimicpp::print(reinterpret_cast<const std::string*>(std::uintptr_t{0x1234u})),
            Catch::Matchers::Matches("0x0{1,12}1234"));

#if not MIMICPP_DETAIL_IS_32BIT
        REQUIRE_THAT(
            mimicpp::print(reinterpret_cast<const void*>(std::uintptr_t{0x1234'5678'90AB'CDEFu})),
            Catch::Matchers::Matches("0x1234567890[Aa][Bb][Cc][Dd][Ee][Ff]"));
#else
        REQUIRE_THAT(
            mimicpp::print(reinterpret_cast<const void*>(std::uintptr_t{0x90AB'CDEFu})),
            Catch::Matchers::Matches("0x90[Aa][Bb][Cc][Dd][Ee][Ff]"));
#endif
    }

    SECTION("Std smart-pointers are unwrapped.")
    {
        REQUIRE_THAT(
            mimicpp::print(std::unique_ptr<int>{}),
            Catch::Matchers::Matches("nullptr"));

        auto const uniqueInt = std::make_unique<int>(42);
        REQUIRE_THAT(
            mimicpp::print(uniqueInt),
            Catch::Matchers::Matches(print(uniqueInt.get())));

        REQUIRE_THAT(
            mimicpp::print(std::shared_ptr<int>{}),
            Catch::Matchers::Matches("nullptr"));
        auto sharedInt = std::make_shared<int>(42);
        REQUIRE_THAT(
            mimicpp::print(sharedInt),
            Catch::Matchers::Matches(print(sharedInt.get())));

        REQUIRE_THAT(
            mimicpp::print(std::weak_ptr<int>{}),
            Catch::Matchers::Matches("nullptr"));
        REQUIRE_THAT(
            mimicpp::print(std::weak_ptr{sharedInt}),
            Catch::Matchers::Matches(print(sharedInt.get())));
        sharedInt.reset();
        REQUIRE_THAT(
            mimicpp::print(std::weak_ptr{sharedInt}),
            Catch::Matchers::Matches("nullptr"));
    }
}

namespace
{
    struct my_char
    {
        char c{};

        bool operator==(const my_char&) const = default;
    };

    class MyString
    {
    public:
        std::vector<my_char> inner{};
    };

    class MyNonPrintableString
    {
    public:
        std::string inner{};
    };
}

template <>
struct mimicpp::is_character<my_char>
    : public std::true_type
{
};

template <>
class custom::Printer<my_char>
{
public:
    static auto print(auto outIter, const my_char myChar)
    {
        return mimicpp::print(std::move(outIter), myChar.c);
    }
};

template <>
struct mimicpp::string_traits<MyString>
{
    using char_t = my_char;
    // explicitly use view-type which isn't printable as string
    using view_t = std::span<const char_t>;

    [[nodiscard]]
    static constexpr view_t view(const MyString& str) noexcept
    {
        return std::span{str.inner};
    }
};

template <>
struct mimicpp::string_traits<MyNonPrintableString>
{
    using char_t = char;
    // explicitly use view-type which isn't printable as string
    using view_t = std::span<const char>;

    [[nodiscard]]
    static constexpr view_t view(const MyNonPrintableString& str) noexcept
    {
        return std::span{str.inner};
    }
};

TEST_CASE(
    "print supports printing of custom char-types and even strings of custom char-type.",
    "[print]")
{
    SECTION("my_char can be printed.")
    {
        StringStreamT stream{};
        print(std::ostreambuf_iterator{stream}, my_char{'A'});
        REQUIRE_THAT(
            std::move(stream).str(),
            Catch::Matchers::Equals("A"));

        SECTION("And MyString can be printed.")
        {
            stream = StringStreamT{}; // clang-16 with libc++ doesn't clear by str()&&
            STATIC_REQUIRE(string<MyString>);

            print(
                std::ostreambuf_iterator{
                    stream
            },
                MyString{{{'A'}, {'b'}, {'C'}}});
            REQUIRE_THAT(
                std::move(stream).str(),
                Catch::Matchers::Equals("\"AbC\""));
        }
    }
}

TEST_CASE(
    "print supports printing of non-printable strings of formattable char-type.",
    "[print]")
{
    STATIC_REQUIRE(string<MyNonPrintableString>);

    StringStreamT stream{};
    print(
        std::ostreambuf_iterator{stream},
        MyNonPrintableString{"AbC"});
    REQUIRE_THAT(
        std::move(stream).str(),
        Catch::Matchers::Equals("\"AbC\""));
}

TEST_CASE(
    "print supports printing of temporaries.",
    "[print]")
{
    SECTION("Something printable.")
    {
        StringStreamT stream{};
        print(std::ostreambuf_iterator{stream}, 42);
        REQUIRE_THAT(
            std::move(stream).str(),
            Catch::Matchers::Equals("42"));
    }

    SECTION("Something non-printable.")
    {
        StringStreamT stream{};
        print(std::ostreambuf_iterator{stream}, NonPrintable{});
        REQUIRE_THAT(
            std::move(stream).str(),
            Catch::Matchers::Equals("{?}"));
    }
}

TEST_CASE(
    "ValueCategory is printable.",
    "[print]")
{
    namespace Matches = Catch::Matchers;

    SECTION("When valid ValueCategory is given.")
    {
        const auto [expected, category] = GENERATE(
            (table<StringT, ValueCategory>)({
                {   "any",    ValueCategory::any},
                {"rvalue", ValueCategory::rvalue},
                {"lvalue", ValueCategory::lvalue},
        }));

        REQUIRE_THAT(
            mimicpp::print(category),
            Matches::Equals(expected));
    }
}

TEST_CASE(
    "Constness is printable.",
    "[print]")
{
    namespace Matches = Catch::Matchers;

    SECTION("When valid Constness is given.")
    {
        const auto [expected, category] = GENERATE(
            (table<StringT, Constness>)({
                {    "any",       Constness::any},
                {  "const",  Constness::as_const},
                {"mutable", Constness::non_const},
        }));

        REQUIRE_THAT(
            mimicpp::print(category),
            Matches::Equals(expected));
    }
}

TEMPLATE_TEST_CASE(
    "wchar_t strings will be formatted as value range.",
    "[print]",
    std::wstring,
    const std::wstring,
    std::wstring_view,
    const std::wstring_view)
{
    const auto [expected, source] = GENERATE(
        (table<std::string, const wchar_t*>)({
            {                                                    "L\"\"",          L""},
            {            "L\"0x48, 0x65, 0x2c, 0x20, 0x6c, 0x6c, 0x6f\"",   L"He, llo"},
            {"L\"0x20, 0x48, 0x65, 0x2c, 0x20, 0x6c, 0x6c, 0x6f, 0x20\"", L" He, llo "}
    }));

    StringStreamT out{};
    print(std::ostreambuf_iterator{out}, static_cast<TestType>(source));
    REQUIRE_THAT(
        std::move(out).str(),
        Catch::Matchers::Equals(expected));
}

TEMPLATE_TEST_CASE(
    "char8_t strings will be formatted as value range.",
    "[print]",
    std::u8string,
    const std::u8string,
    std::u8string_view,
    const std::u8string_view)
{
    const auto [expected, source] = GENERATE(
        (table<std::string, const char8_t*>)({
            {                                                    "u8\"\"",          u8""},
            {            "u8\"0x48, 0x65, 0x2c, 0x20, 0x6c, 0x6c, 0x6f\"",   u8"He, llo"},
            {"u8\"0x20, 0x48, 0x65, 0x2c, 0x20, 0x6c, 0x6c, 0x6f, 0x20\"", u8" He, llo "}
    }));

    StringStreamT out{};
    print(std::ostreambuf_iterator{out}, static_cast<TestType>(source));
    REQUIRE_THAT(
        std::move(out).str(),
        Catch::Matchers::Equals(expected));
}

TEMPLATE_TEST_CASE(
    "char16_t strings will be formatted as value range.",
    "[print]",
    std::u16string,
    const std::u16string,
    std::u16string_view,
    const std::u16string_view)
{
    const auto [expected, source] = GENERATE(
        (table<std::string, const char16_t*>)({
            {                                                    "u\"\"",          u""},
            {            "u\"0x48, 0x65, 0x2c, 0x20, 0x6c, 0x6c, 0x6f\"",   u"He, llo"},
            {"u\"0x20, 0x48, 0x65, 0x2c, 0x20, 0x6c, 0x6c, 0x6f, 0x20\"", u" He, llo "}
    }));

    StringStreamT out{};
    print(std::ostreambuf_iterator{out}, static_cast<TestType>(source));
    REQUIRE_THAT(
        std::move(out).str(),
        Catch::Matchers::Equals(expected));
}

TEMPLATE_TEST_CASE(
    "char32_t strings will be formatted as value range.",
    "[print]",
    std::u32string,
    const std::u32string,
    std::u32string_view,
    const std::u32string_view)
{
    const auto [expected, source] = GENERATE(
        (table<std::string, const char32_t*>)({
            {                                                    "U\"\"",          U""},
            {            "U\"0x48, 0x65, 0x2c, 0x20, 0x6c, 0x6c, 0x6f\"",   U"He, llo"},
            {"U\"0x20, 0x48, 0x65, 0x2c, 0x20, 0x6c, 0x6c, 0x6f, 0x20\"", U" He, llo "}
    }));

    StringStreamT out{};
    print(std::ostreambuf_iterator{out}, static_cast<TestType>(source));
    REQUIRE_THAT(
        std::move(out).str(),
        Catch::Matchers::Equals(expected));
}
