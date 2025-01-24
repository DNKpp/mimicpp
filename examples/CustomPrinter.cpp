//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Printer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace
{
    //! [my_type]
    struct my_type
    {
        int data{};
    };

    //! [my_type]
}

//! [my_type printer]
template <>
class mimicpp::custom::Printer<my_type>
{
public:
    static auto print(auto outIter, const my_type& object)
    {
        return format::format_to(
            outIter,
            "Object of my_type has value: {}",
            object.data);
    }
};

//! [my_type printer]

TEST_CASE(
    "mimicpp::custom::Printer can be specialized for arbitrary user types.",
    "[example][example::printer]")
{
    //! [my_type print]
    constexpr my_type myObject{42};
    const std::string text = mimicpp::print(myObject);

    REQUIRE_THAT(
        text,
        Catch::Matchers::Equals("Object of my_type has value: 42"));
    //! [my_type print]
}

//! [my_type type-printer]
template <>
class mimicpp::custom::TypePrinter<my_type>
{
public:
    static consteval std::string_view name() noexcept
    {
        return {"This-Is-My-Mighty-Type"};
    }
};

//! [my_type type-printer]

TEST_CASE(
    "mimicpp::custom::TypePrinter can be specialized for arbitrary types."
    "[example][example::printer]")
{
    //! [my_type type-print]
    const std::string name = mimicpp::print_type<my_type>();

    REQUIRE_THAT(
        name,
        Catch::Matchers::Equals("This-Is-My-Mighty-Type"));

    const std::string qualifiedName = mimicpp::print_type<my_type* const&&>();
    REQUIRE_THAT(
        qualifiedName,
        Catch::Matchers::Equals("This-Is-My-Mighty-Type* const&&"));
    //! [my_type type-print]
}
