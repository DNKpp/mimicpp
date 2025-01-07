//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Mock.hpp"
#include "mimic++/String.hpp"

#include <catch2/catch_test_macros.hpp>

namespace
{
    //! [MyString]
    class MyString
    {
    public:
        explicit MyString(std::string str) noexcept
            : m_Inner{std::move(str)}
        {
        }

        std::string_view view() const noexcept
        {
            return std::string_view{m_Inner};
        }

    private:
        std::string m_Inner{};
    };

    //! [MyString]
}

//! [MyString trait]
template <>
struct mimicpp::string_traits<MyString>
{
    // must be the underlying char type
    using char_t = char;

    // must be a std::ranges::view like type and must be contiguous
    using view_t = std::string_view;

    // must construct a view object
    [[nodiscard]]
    static view_t view(const MyString& str) noexcept
    {
        return str.view();
    }
};

//! [MyString trait]

TEST_CASE(
    "mimic++ supports custom strings.",
    "[example][example::string]")
{
    //! [MyString example]
    STATIC_REQUIRE(mimicpp::string<MyString>);

    namespace expect = mimicpp::expect;
    namespace matches = mimicpp::matches;
    using matches::_;

    mimicpp::Mock<void(MyString)> mock{};
    SCOPED_EXP mock.expect_call("Hello, World!");
    SCOPED_EXP mock.expect_call(matches::str::starts_with("Hi", mimicpp::case_insensitive));

    mock(MyString{"Hello, World!"}); // matches the first expectation
    mock(MyString{"hI, mimic++"});   // matches the second expectation (case-insensitive).
                                     //! [MyString example]
}

//! [custom_char]
struct my_char
{
    char c{};

    bool operator==(const my_char&) const = default;
};

//! [custom_char]

//! [custom_string]
class ComplexString
{
public:
    explicit ComplexString(std::vector<my_char> str) noexcept
        : m_Inner{std::move(str)}
    {
    }

    [[nodiscard]]
    auto begin() const noexcept
    {
        return m_Inner.cbegin();
    }

    [[nodiscard]]
    auto end() const noexcept
    {
        return m_Inner.cend();
    }

private:
    std::vector<my_char> m_Inner{};
};

//! [custom_string]

//! [custom_char trait]
template <>
struct mimicpp::is_character<my_char>
    : public std::true_type
{
};

//! [custom_char trait]

//! [custom_string traits]
template <>
struct mimicpp::string_traits<ComplexString>
{
    // must be the underlying char type
    using char_t = my_char;

    // must be a std::ranges::view like type and must be contiguous
    using view_t = std::span<const char_t>;

    // must construct a view object
    [[nodiscard]]
    static view_t view(const ComplexString& str) noexcept
    {
        return std::span{
            str.begin(),
            str.end()};
    }
};

//! [custom_string traits]

TEST_CASE(
    "mimic++ supports complex custom strings.",
    "[example][example::string]")
{
    //! [custom_string example]
    STATIC_REQUIRE(mimicpp::string<ComplexString>);

    mimicpp::Mock<void(ComplexString)> mock{};
    ComplexString s{
        {{'A'}, {'B'}, {'C'}}
    };
    SCOPED_EXP mock.expect_call(s);

    mock(s);
    //! [custom_string example]
}

//! [custom_char case-folding]
template <>
struct mimicpp::string_case_fold_converter<my_char>
{
    // the string_case_fold_converter must expect the string's view-type and should return
    // a forward-range with the underlying char-type.
    [[nodiscard]]
    auto operator()(string_view_t<ComplexString> view) const
    {
        return view
             | std::views::transform(
                   [](const my_char c) {
                       // see notes of https://en.cppreference.com/w/cpp/string/byte/toupper
                       return my_char{
                           static_cast<char>(
                               static_cast<unsigned char>(std::toupper(c.c)))};
                   });
    }
};

//! [custom_char case-folding]

TEST_CASE(
    "mimic++ supports case-folding for custom char-types.",
    "[example][example::string]")
{
    //! [custom_string case-insensitive example]
    STATIC_REQUIRE(mimicpp::case_foldable_string<ComplexString>);

    namespace matches = mimicpp::matches;

    mimicpp::Mock<void(ComplexString)> mock{};
    SCOPED_EXP mock.expect_call(matches::str::starts_with(ComplexString{
                                                              {{'A'}, {'B'}, {'C'}}
    },
                                                          mimicpp::case_insensitive));

    mock(ComplexString{
        {{'a'}, {'B'}, {'c'}}
    });
    //! [custom_string case-insensitive example]
}
