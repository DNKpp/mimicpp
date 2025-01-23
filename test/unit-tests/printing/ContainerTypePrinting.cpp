//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/ContainerTypePrinter.hpp"

using namespace mimicpp;

namespace
{
    template <typename T>
    struct custom_allocator
        : std::allocator<T>
    {
    };

    struct custom_compare
        : std::less<>
    {
    };

    template <typename T>
    struct custom_hash
        : std::hash<T>
    {
    };
}

TEST_CASE(
    "print_type handles std::vector nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            print_type<std::vector<int>>(),
            Catch::Matchers::Equals("std::vector<int>"));
        REQUIRE_THAT(
            print_type<std::vector<std::vector<int>>>(),
            Catch::Matchers::Equals("std::vector<std::vector<int>>"));
        REQUIRE_THAT(
            print_type<std::vector<std::string>>(),
            Catch::Matchers::Equals("std::vector<std::string>"));
        REQUIRE_THAT(
            print_type<std::vector<std::vector<std::string>>>(),
            Catch::Matchers::Equals("std::vector<std::vector<std::string>>"));
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            print_type<std::pmr::vector<int>>(),
            Catch::Matchers::Equals("std::pmr::vector<int>"));
        REQUIRE_THAT(
            print_type<std::pmr::vector<std::pmr::vector<int>>>(),
            Catch::Matchers::Equals("std::pmr::vector<std::pmr::vector<int>>"));
        REQUIRE_THAT(
            print_type<std::pmr::vector<std::string>>(),
            Catch::Matchers::Equals("std::pmr::vector<std::string>"));
        REQUIRE_THAT(
            print_type<std::pmr::vector<std::pmr::vector<std::string>>>(),
            Catch::Matchers::Equals("std::pmr::vector<std::pmr::vector<std::string>>"));
    }

    SECTION("With alternative allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::vector<int, custom_allocator<int>>>()),
            Catch::Matchers::Equals("std::vector<int, custom_allocator<int>>"));
        REQUIRE_THAT(
            (print_type<std::vector<std::vector<int, custom_allocator<int>>, custom_allocator<std::vector<int, custom_allocator<int>>>>>()),
            Catch::Matchers::Equals("std::vector<std::vector<int, custom_allocator<int>>, custom_allocator<std::vector<int, custom_allocator<int>>>>"));
        REQUIRE_THAT(
            (print_type<std::vector<std::vector<int>, custom_allocator<std::vector<int>>>>()),
            Catch::Matchers::Equals("std::vector<std::vector<int>, custom_allocator<std::vector<int>>>"));
    }
}

TEST_CASE(
    "print_type handles std::deque nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            print_type<std::deque<int>>(),
            Catch::Matchers::Equals("std::deque<int>"));
        REQUIRE_THAT(
            print_type<std::deque<std::deque<int>>>(),
            Catch::Matchers::Equals("std::deque<std::deque<int>>"));
        REQUIRE_THAT(
            print_type<std::deque<std::string>>(),
            Catch::Matchers::Equals("std::deque<std::string>"));
        REQUIRE_THAT(
            print_type<std::deque<std::deque<std::string>>>(),
            Catch::Matchers::Equals("std::deque<std::deque<std::string>>"));
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            print_type<std::pmr::deque<int>>(),
            Catch::Matchers::Equals("std::pmr::deque<int>"));
        REQUIRE_THAT(
            print_type<std::pmr::deque<std::pmr::deque<int>>>(),
            Catch::Matchers::Equals("std::pmr::deque<std::pmr::deque<int>>"));
        REQUIRE_THAT(
            print_type<std::pmr::deque<std::string>>(),
            Catch::Matchers::Equals("std::pmr::deque<std::string>"));
        REQUIRE_THAT(
            print_type<std::pmr::deque<std::pmr::deque<std::string>>>(),
            Catch::Matchers::Equals("std::pmr::deque<std::pmr::deque<std::string>>"));
    }

    SECTION("With alternative allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::deque<int, custom_allocator<int>>>()),
            Catch::Matchers::Equals("std::deque<int, custom_allocator<int>>"));
        REQUIRE_THAT(
            (print_type<std::deque<std::deque<int, custom_allocator<int>>, custom_allocator<std::deque<int, custom_allocator<int>>>>>()),
            Catch::Matchers::Equals("std::deque<std::deque<int, custom_allocator<int>>, custom_allocator<std::deque<int, custom_allocator<int>>>>"));
        REQUIRE_THAT(
            (print_type<std::deque<std::deque<int>, custom_allocator<std::deque<int>>>>()),
            Catch::Matchers::Equals("std::deque<std::deque<int>, custom_allocator<std::deque<int>>>"));
    }
}

TEST_CASE(
    "print_type handles std::list nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            print_type<std::list<int>>(),
            Catch::Matchers::Equals("std::list<int>"));
        REQUIRE_THAT(
            print_type<std::list<std::list<int>>>(),
            Catch::Matchers::Equals("std::list<std::list<int>>"));
        REQUIRE_THAT(
            print_type<std::list<std::string>>(),
            Catch::Matchers::Equals("std::list<std::string>"));
        REQUIRE_THAT(
            print_type<std::list<std::list<std::string>>>(),
            Catch::Matchers::Equals("std::list<std::list<std::string>>"));
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            print_type<std::pmr::list<int>>(),
            Catch::Matchers::Equals("std::pmr::list<int>"));
        REQUIRE_THAT(
            print_type<std::pmr::list<std::pmr::list<int>>>(),
            Catch::Matchers::Equals("std::pmr::list<std::pmr::list<int>>"));
        REQUIRE_THAT(
            print_type<std::pmr::list<std::string>>(),
            Catch::Matchers::Equals("std::pmr::list<std::string>"));
        REQUIRE_THAT(
            print_type<std::pmr::list<std::pmr::list<std::string>>>(),
            Catch::Matchers::Equals("std::pmr::list<std::pmr::list<std::string>>"));
    }

    SECTION("With alternative allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::list<int, custom_allocator<int>>>()),
            Catch::Matchers::Equals("std::list<int, custom_allocator<int>>"));
        REQUIRE_THAT(
            (print_type<std::list<std::list<int, custom_allocator<int>>, custom_allocator<std::list<int, custom_allocator<int>>>>>()),
            Catch::Matchers::Equals("std::list<std::list<int, custom_allocator<int>>, custom_allocator<std::list<int, custom_allocator<int>>>>"));
        REQUIRE_THAT(
            (print_type<std::list<std::list<int>, custom_allocator<std::list<int>>>>()),
            Catch::Matchers::Equals("std::list<std::list<int>, custom_allocator<std::list<int>>>"));
    }
}

TEST_CASE(
    "print_type handles std::forward_list nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            print_type<std::forward_list<int>>(),
            Catch::Matchers::Equals("std::forward_list<int>"));
        REQUIRE_THAT(
            print_type<std::forward_list<std::forward_list<int>>>(),
            Catch::Matchers::Equals("std::forward_list<std::forward_list<int>>"));
        REQUIRE_THAT(
            print_type<std::forward_list<std::string>>(),
            Catch::Matchers::Equals("std::forward_list<std::string>"));
        REQUIRE_THAT(
            print_type<std::forward_list<std::forward_list<std::string>>>(),
            Catch::Matchers::Equals("std::forward_list<std::forward_list<std::string>>"));
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            print_type<std::pmr::forward_list<int>>(),
            Catch::Matchers::Equals("std::pmr::forward_list<int>"));
        REQUIRE_THAT(
            print_type<std::pmr::forward_list<std::pmr::forward_list<int>>>(),
            Catch::Matchers::Equals("std::pmr::forward_list<std::pmr::forward_list<int>>"));
        REQUIRE_THAT(
            print_type<std::pmr::forward_list<std::string>>(),
            Catch::Matchers::Equals("std::pmr::forward_list<std::string>"));
        REQUIRE_THAT(
            print_type<std::pmr::forward_list<std::pmr::forward_list<std::string>>>(),
            Catch::Matchers::Equals("std::pmr::forward_list<std::pmr::forward_list<std::string>>"));
    }

    SECTION("With alternative allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::forward_list<int, custom_allocator<int>>>()),
            Catch::Matchers::Equals("std::forward_list<int, custom_allocator<int>>"));
        REQUIRE_THAT(
            (print_type<std::forward_list<std::forward_list<int, custom_allocator<int>>, custom_allocator<std::forward_list<int, custom_allocator<int>>>>>()),
            Catch::Matchers::Equals(
                "std::forward_list<std::forward_list<int, custom_allocator<int>>, custom_allocator<std::forward_list<int, custom_allocator<int>>>>"));
        REQUIRE_THAT(
            (print_type<std::forward_list<std::forward_list<int>, custom_allocator<std::forward_list<int>>>>()),
            Catch::Matchers::Equals("std::forward_list<std::forward_list<int>, custom_allocator<std::forward_list<int>>>"));
    }
}

TEST_CASE(
    "print_type handles std::array nicely.",
    "[print]")
{
    REQUIRE_THAT(
        (print_type<std::array<int, 0>>()),
        Catch::Matchers::Equals("std::array<int, 0>"));
    REQUIRE_THAT(
        (print_type<std::array<int, 42>>()),
        Catch::Matchers::Equals("std::array<int, 42>"));
    REQUIRE_THAT(
        (print_type<std::array<std::vector<std::string>, 1>>()),
        Catch::Matchers::Equals("std::array<std::vector<std::string>, 1>"));
    REQUIRE_THAT(
        (print_type<std::array<std::tuple<float&, std::tuple<>>, 1>>()),
        Catch::Matchers::Equals("std::array<std::tuple<float&, std::tuple<>>, 1>"));
}

TEST_CASE(
    "print_type handles std::span nicely.",
    "[print]")
{
    SECTION("With default dynamic_extend.")
    {
        REQUIRE_THAT(
            (print_type<std::span<int>>()),
            Catch::Matchers::Equals("std::span<int>"));
        REQUIRE_THAT(
            (print_type<std::span<int const>>()),
            Catch::Matchers::Equals("std::span<int const>"));
        REQUIRE_THAT(
            (print_type<std::span<std::vector<std::string>>>()),
            Catch::Matchers::Equals("std::span<std::vector<std::string>>"));
        REQUIRE_THAT(
            (print_type<std::span<std::tuple<float&, std::tuple<>>>>()),
            Catch::Matchers::Equals("std::span<std::tuple<float&, std::tuple<>>>"));
    }

    SECTION("With specific extend.")
    {
        REQUIRE_THAT(
            (print_type<std::span<int, 42>>()),
            Catch::Matchers::Equals("std::span<int, 42>"));
        REQUIRE_THAT(
            (print_type<std::span<int const, 42>>()),
            Catch::Matchers::Equals("std::span<int const, 42>"));
        REQUIRE_THAT(
            (print_type<std::span<std::vector<std::string>, 1>>()),
            Catch::Matchers::Equals("std::span<std::vector<std::string>, 1>"));
        REQUIRE_THAT(
            (print_type<std::span<std::tuple<float&, std::tuple<>>, 1>>()),
            Catch::Matchers::Equals("std::span<std::tuple<float&, std::tuple<>>, 1>"));
    }
}

TEST_CASE(
    "print_type handles std::set nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            print_type<std::set<int>>(),
            Catch::Matchers::Equals("std::set<int>"));
        REQUIRE_THAT(
            print_type<std::set<std::string>>(),
            Catch::Matchers::Equals("std::set<std::string>"));
        REQUIRE_THAT(
            (print_type<std::set<std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::set<std::tuple<int, int>>"));

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::set<int, custom_compare>>()),
                Catch::Matchers::Equals("std::set<int, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::set<std::string, custom_compare>>()),
                Catch::Matchers::Equals("std::set<std::string, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::set<std::tuple<int, int>, custom_compare>>()),
                Catch::Matchers::Equals("std::set<std::tuple<int, int>, custom_compare>"));
        }
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            print_type<std::pmr::set<int>>(),
            Catch::Matchers::Equals("std::pmr::set<int>"));
        REQUIRE_THAT(
            print_type<std::pmr::set<std::string>>(),
            Catch::Matchers::Equals("std::pmr::set<std::string>"));
        REQUIRE_THAT(
            (print_type<std::pmr::set<std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::pmr::set<std::tuple<int, int>>"));

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::set<int, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::set<int, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::pmr::set<std::string, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::set<std::string, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::pmr::set<std::tuple<int, int>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::set<std::tuple<int, int>, custom_compare>"));
        }
    }

    SECTION("With alternative arguments.")
    {
        REQUIRE_THAT(
            (print_type<std::set<int, custom_compare, custom_allocator<int>>>()),
            Catch::Matchers::Equals("std::set<int, custom_compare, custom_allocator<int>>"));
        REQUIRE_THAT(
            (print_type<std::set<std::string, custom_compare, custom_allocator<std::string>>>()),
            Catch::Matchers::Equals("std::set<std::string, custom_compare, custom_allocator<std::string>>"));
        REQUIRE_THAT(
            (print_type<std::set<std::tuple<int, int>, custom_compare, custom_allocator<std::tuple<int, int>>>>()),
            Catch::Matchers::Equals("std::set<std::tuple<int, int>, custom_compare, custom_allocator<std::tuple<int, int>>>"));
    }
}

TEST_CASE(
    "print_type handles std::multiset nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            print_type<std::multiset<int>>(),
            Catch::Matchers::Equals("std::multiset<int>"));
        REQUIRE_THAT(
            print_type<std::multiset<std::string>>(),
            Catch::Matchers::Equals("std::multiset<std::string>"));
        REQUIRE_THAT(
            (print_type<std::multiset<std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::multiset<std::tuple<int, int>>"));

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::multiset<int, custom_compare>>()),
                Catch::Matchers::Equals("std::multiset<int, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::multiset<std::string, custom_compare>>()),
                Catch::Matchers::Equals("std::multiset<std::string, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::multiset<std::tuple<int, int>, custom_compare>>()),
                Catch::Matchers::Equals("std::multiset<std::tuple<int, int>, custom_compare>"));
        }
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            print_type<std::pmr::multiset<int>>(),
            Catch::Matchers::Equals("std::pmr::multiset<int>"));
        REQUIRE_THAT(
            print_type<std::pmr::multiset<std::string>>(),
            Catch::Matchers::Equals("std::pmr::multiset<std::string>"));
        REQUIRE_THAT(
            (print_type<std::pmr::multiset<std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::pmr::multiset<std::tuple<int, int>>"));

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::multiset<int, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::multiset<int, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::pmr::multiset<std::string, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::multiset<std::string, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::pmr::multiset<std::tuple<int, int>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::multiset<std::tuple<int, int>, custom_compare>"));
        }
    }

    SECTION("With alternative arguments.")
    {
        REQUIRE_THAT(
            (print_type<std::multiset<int, custom_compare, custom_allocator<int>>>()),
            Catch::Matchers::Equals("std::multiset<int, custom_compare, custom_allocator<int>>"));
        REQUIRE_THAT(
            (print_type<std::multiset<std::string, custom_compare, custom_allocator<std::string>>>()),
            Catch::Matchers::Equals("std::multiset<std::string, custom_compare, custom_allocator<std::string>>"));
        REQUIRE_THAT(
            (print_type<std::multiset<std::tuple<int, int>, custom_compare, custom_allocator<std::tuple<int, int>>>>()),
            Catch::Matchers::Equals("std::multiset<std::tuple<int, int>, custom_compare, custom_allocator<std::tuple<int, int>>>"));
    }
}

TEST_CASE(
    "print_type handles std::unordered_set nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            print_type<std::unordered_set<int>>(),
            Catch::Matchers::Equals("std::unordered_set<int>"));
        REQUIRE_THAT(
            print_type<std::unordered_set<std::string>>(),
            Catch::Matchers::Equals("std::unordered_set<std::string>"));
        REQUIRE_THAT(
            print_type<std::unordered_set<std::optional<std::string>>>(),
            Catch::Matchers::Equals("std::unordered_set<std::optional<std::string>>"));

        SECTION("With alternative hash.")
        {
            REQUIRE_THAT(
                (print_type<std::unordered_set<std::string, custom_hash<std::string>>>()),
                Catch::Matchers::Equals("std::unordered_set<std::string, custom_hash<std::string>>"));
        }

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::unordered_set<std::string, custom_hash<std::string>, custom_compare>>()),
                Catch::Matchers::Equals("std::unordered_set<std::string, custom_hash<std::string>, custom_compare>"));
        }
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            print_type<std::pmr::unordered_set<int>>(),
            Catch::Matchers::Equals("std::pmr::unordered_set<int>"));
        REQUIRE_THAT(
            print_type<std::pmr::unordered_set<std::string>>(),
            Catch::Matchers::Equals("std::pmr::unordered_set<std::string>"));
        REQUIRE_THAT(
            print_type<std::pmr::unordered_set<std::optional<std::string>>>(),
            Catch::Matchers::Equals("std::pmr::unordered_set<std::optional<std::string>>"));

        SECTION("With alternative hash.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_set<std::string, custom_hash<std::string>>>()),
                Catch::Matchers::Equals("std::pmr::unordered_set<std::string, custom_hash<std::string>>"));
        }

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_set<std::string, custom_hash<std::string>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::unordered_set<std::string, custom_hash<std::string>, custom_compare>"));
        }
    }

    SECTION("With alternative arguments.")
    {
        REQUIRE_THAT(
            (print_type<std::unordered_set<int, custom_hash<int>, custom_compare, custom_allocator<int>>>()),
            Catch::Matchers::Equals("std::unordered_set<int, custom_hash<int>, custom_compare, custom_allocator<int>>"));
        REQUIRE_THAT(
            (print_type<
                std::unordered_set<std::string, custom_hash<std::string>, custom_compare, custom_allocator<std::string>>>()),
            Catch::Matchers::Equals(
                "std::unordered_set<std::string, custom_hash<std::string>, custom_compare, custom_allocator<std::string>>"));
        REQUIRE_THAT(
            (print_type<
                std::unordered_set<
                    std::optional<std::string>,
                    custom_hash<std::optional<std::string>>,
                    custom_compare,
                    custom_allocator<std::optional<std::string>>>>()),
            Catch::Matchers::Equals(
                "std::unordered_set<"
                "std::optional<std::string>, "
                "custom_hash<std::optional<std::string>>, "
                "custom_compare, "
                "custom_allocator<std::optional<std::string>>>"));
    }
}

TEST_CASE(
    "print_type handles std::unordered_multiset nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            print_type<std::unordered_multiset<int>>(),
            Catch::Matchers::Equals("std::unordered_multiset<int>"));
        REQUIRE_THAT(
            print_type<std::unordered_multiset<std::string>>(),
            Catch::Matchers::Equals("std::unordered_multiset<std::string>"));
        REQUIRE_THAT(
            print_type<std::unordered_multiset<std::optional<std::string>>>(),
            Catch::Matchers::Equals("std::unordered_multiset<std::optional<std::string>>"));

        SECTION("With alternative hash.")
        {
            REQUIRE_THAT(
                (print_type<std::unordered_multiset<std::string, custom_hash<std::string>>>()),
                Catch::Matchers::Equals("std::unordered_multiset<std::string, custom_hash<std::string>>"));
        }

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::unordered_multiset<std::string, custom_hash<std::string>, custom_compare>>()),
                Catch::Matchers::Equals("std::unordered_multiset<std::string, custom_hash<std::string>, custom_compare>"));
        }
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            print_type<std::pmr::unordered_multiset<int>>(),
            Catch::Matchers::Equals("std::pmr::unordered_multiset<int>"));
        REQUIRE_THAT(
            print_type<std::pmr::unordered_multiset<std::string>>(),
            Catch::Matchers::Equals("std::pmr::unordered_multiset<std::string>"));
        REQUIRE_THAT(
            print_type<std::pmr::unordered_multiset<std::optional<std::string>>>(),
            Catch::Matchers::Equals("std::pmr::unordered_multiset<std::optional<std::string>>"));

        SECTION("With alternative hash.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_multiset<std::string, custom_hash<std::string>>>()),
                Catch::Matchers::Equals("std::pmr::unordered_multiset<std::string, custom_hash<std::string>>"));
        }

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_multiset<std::string, custom_hash<std::string>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::unordered_multiset<std::string, custom_hash<std::string>, custom_compare>"));
        }
    }

    SECTION("With alternative arguments.")
    {
        REQUIRE_THAT(
            (print_type<std::unordered_multiset<int, custom_hash<int>, custom_compare, custom_allocator<int>>>()),
            Catch::Matchers::Equals("std::unordered_multiset<int, custom_hash<int>, custom_compare, custom_allocator<int>>"));
        REQUIRE_THAT(
            (print_type<
                std::unordered_multiset<std::string, custom_hash<std::string>, custom_compare, custom_allocator<std::string>>>()),
            Catch::Matchers::Equals(
                "std::unordered_multiset<std::string, custom_hash<std::string>, custom_compare, custom_allocator<std::string>>"));
        REQUIRE_THAT(
            (print_type<
                std::unordered_multiset<
                    std::optional<std::string>,
                    custom_hash<std::optional<std::string>>,
                    custom_compare,
                    custom_allocator<std::optional<std::string>>>>()),
            Catch::Matchers::Equals(
                "std::unordered_multiset<"
                "std::optional<std::string>, "
                "custom_hash<std::optional<std::string>>, "
                "custom_compare, "
                "custom_allocator<std::optional<std::string>>>"));
    }
}

TEST_CASE(
    "print_type handles std::map nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::map<std::string, int>>()),
            Catch::Matchers::Equals("std::map<std::string, int>"));
        REQUIRE_THAT(
            (print_type<std::map<int, std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::map<int, std::tuple<int, int>>"));

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::map<std::string, int, custom_compare>>()),
                Catch::Matchers::Equals("std::map<std::string, int, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::map<int, std::tuple<int, int>, custom_compare>>()),
                Catch::Matchers::Equals("std::map<int, std::tuple<int, int>, custom_compare>"));
        }
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::pmr::map<std::string, int>>()),
            Catch::Matchers::Equals("std::pmr::map<std::string, int>"));
        REQUIRE_THAT(
            (print_type<std::pmr::map<int, std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::pmr::map<int, std::tuple<int, int>>"));

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::map<std::string, int, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::map<std::string, int, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::pmr::map<int, std::tuple<int, int>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::map<int, std::tuple<int, int>, custom_compare>"));
        }
    }

    SECTION("With alternative arguments.")
    {
        REQUIRE_THAT(
            (print_type<std::map<std::string, int, custom_compare, custom_allocator<std::pair<const std::string, int>>>>()),
            Catch::Matchers::Equals("std::map<std::string, int, custom_compare, custom_allocator<std::pair<const std::string, int>>>"));
        REQUIRE_THAT(
            (print_type<std::map<int, std::tuple<int, int>, custom_compare, custom_allocator<std::pair<const int, std::tuple<int, int>>>>>()),
            Catch::Matchers::Equals("std::map<int, std::tuple<int, int>, custom_compare, custom_allocator<std::pair<const int, std::tuple<int, int>>>>"));
    }
}

TEST_CASE(
    "print_type handles std::multimap nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::multimap<std::string, int>>()),
            Catch::Matchers::Equals("std::multimap<std::string, int>"));
        REQUIRE_THAT(
            (print_type<std::multimap<int, std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::multimap<int, std::tuple<int, int>>"));

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::multimap<std::string, int, custom_compare>>()),
                Catch::Matchers::Equals("std::multimap<std::string, int, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::multimap<int, std::tuple<int, int>, custom_compare>>()),
                Catch::Matchers::Equals("std::multimap<int, std::tuple<int, int>, custom_compare>"));
        }
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::pmr::multimap<std::string, int>>()),
            Catch::Matchers::Equals("std::pmr::multimap<std::string, int>"));
        REQUIRE_THAT(
            (print_type<std::pmr::multimap<int, std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::pmr::multimap<int, std::tuple<int, int>>"));

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::multimap<std::string, int, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::multimap<std::string, int, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::pmr::multimap<int, std::tuple<int, int>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::multimap<int, std::tuple<int, int>, custom_compare>"));
        }
    }

    SECTION("With alternative arguments.")
    {
        REQUIRE_THAT(
            (print_type<std::multimap<std::string, int, custom_compare, custom_allocator<std::pair<const std::string, int>>>>()),
            Catch::Matchers::Equals("std::multimap<std::string, int, custom_compare, custom_allocator<std::pair<const std::string, int>>>"));
        REQUIRE_THAT(
            (print_type<std::multimap<int, std::tuple<int, int>, custom_compare, custom_allocator<std::pair<const int, std::tuple<int, int>>>>>()),
            Catch::Matchers::Equals("std::multimap<int, std::tuple<int, int>, custom_compare, custom_allocator<std::pair<const int, std::tuple<int, int>>>>"));
    }
}

TEST_CASE(
    "print_type handles std::unordered_map nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::unordered_map<std::string, int>>()),
            Catch::Matchers::Equals("std::unordered_map<std::string, int>"));
        REQUIRE_THAT(
            (print_type<std::unordered_map<int, std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::unordered_map<int, std::tuple<int, int>>"));

        SECTION("With alternative hash.")
        {
            REQUIRE_THAT(
                (print_type<std::unordered_map<std::string, int, custom_hash<std::string>>>()),
                Catch::Matchers::Equals("std::unordered_map<std::string, int, custom_hash<std::string>>"));
            REQUIRE_THAT(
                (print_type<std::unordered_map<int, std::tuple<int, int>, custom_hash<int>>>()),
                Catch::Matchers::Equals("std::unordered_map<int, std::tuple<int, int>, custom_hash<int>>"));
        }

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::unordered_map<std::string, int, custom_hash<std::string>, custom_compare>>()),
                Catch::Matchers::Equals("std::unordered_map<std::string, int, custom_hash<std::string>, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::unordered_map<int, std::tuple<int, int>, custom_hash<int>, custom_compare>>()),
                Catch::Matchers::Equals("std::unordered_map<int, std::tuple<int, int>, custom_hash<int>, custom_compare>"));
        }
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::pmr::unordered_map<std::string, int>>()),
            Catch::Matchers::Equals("std::pmr::unordered_map<std::string, int>"));
        REQUIRE_THAT(
            (print_type<std::pmr::unordered_map<int, std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::pmr::unordered_map<int, std::tuple<int, int>>"));

        SECTION("With alternative hash.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_map<std::string, int, custom_hash<std::string>>>()),
                Catch::Matchers::Equals("std::pmr::unordered_map<std::string, int, custom_hash<std::string>>"));
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_map<int, std::tuple<int, int>, custom_hash<int>>>()),
                Catch::Matchers::Equals("std::pmr::unordered_map<int, std::tuple<int, int>, custom_hash<int>>"));
        }

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_map<std::string, int, custom_hash<std::string>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::unordered_map<std::string, int, custom_hash<std::string>, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_map<int, std::tuple<int, int>, custom_hash<int>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::unordered_map<int, std::tuple<int, int>, custom_hash<int>, custom_compare>"));
        }
    }

    SECTION("With alternative arguments.")
    {
        REQUIRE_THAT(
            (print_type<std::unordered_map<
                 std::string,
                 int,
                 custom_hash<std::string>,
                 custom_compare,
                 custom_allocator<std::pair<const std::string, int>>>>()),
            Catch::Matchers::Equals(
                "std::unordered_map<"
                "std::string, "
                "int, "
                "custom_hash<std::string>, "
                "custom_compare, "
                "custom_allocator<std::pair<const std::string, int>>>"));
        REQUIRE_THAT(
            (print_type<std::unordered_map<
                 int,
                 std::tuple<int, int>,
                 custom_hash<int>,
                 custom_compare,
                 custom_allocator<std::pair<const int, std::tuple<int, int>>>>>()),
            Catch::Matchers::Equals(
                "std::unordered_map<"
                "int, "
                "std::tuple<int, int>, "
                "custom_hash<int>, "
                "custom_compare, "
                "custom_allocator<std::pair<const int, std::tuple<int, int>>>>"));
    }
}

TEST_CASE(
    "print_type handles std::unordered_multimap nicely.",
    "[print]")
{
    SECTION("With default allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::unordered_multimap<std::string, int>>()),
            Catch::Matchers::Equals("std::unordered_multimap<std::string, int>"));
        REQUIRE_THAT(
            (print_type<std::unordered_multimap<int, std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::unordered_multimap<int, std::tuple<int, int>>"));

        SECTION("With alternative hash.")
        {
            REQUIRE_THAT(
                (print_type<std::unordered_multimap<std::string, int, custom_hash<std::string>>>()),
                Catch::Matchers::Equals("std::unordered_multimap<std::string, int, custom_hash<std::string>>"));
            REQUIRE_THAT(
                (print_type<std::unordered_multimap<int, std::tuple<int, int>, custom_hash<int>>>()),
                Catch::Matchers::Equals("std::unordered_multimap<int, std::tuple<int, int>, custom_hash<int>>"));
        }

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::unordered_multimap<std::string, int, custom_hash<std::string>, custom_compare>>()),
                Catch::Matchers::Equals("std::unordered_multimap<std::string, int, custom_hash<std::string>, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::unordered_multimap<int, std::tuple<int, int>, custom_hash<int>, custom_compare>>()),
                Catch::Matchers::Equals("std::unordered_multimap<int, std::tuple<int, int>, custom_hash<int>, custom_compare>"));
        }
    }

    SECTION("With pmr allocator.")
    {
        REQUIRE_THAT(
            (print_type<std::pmr::unordered_multimap<std::string, int>>()),
            Catch::Matchers::Equals("std::pmr::unordered_multimap<std::string, int>"));
        REQUIRE_THAT(
            (print_type<std::pmr::unordered_multimap<int, std::tuple<int, int>>>()),
            Catch::Matchers::Equals("std::pmr::unordered_multimap<int, std::tuple<int, int>>"));

        SECTION("With alternative hash.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_multimap<std::string, int, custom_hash<std::string>>>()),
                Catch::Matchers::Equals("std::pmr::unordered_multimap<std::string, int, custom_hash<std::string>>"));
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_multimap<int, std::tuple<int, int>, custom_hash<int>>>()),
                Catch::Matchers::Equals("std::pmr::unordered_multimap<int, std::tuple<int, int>, custom_hash<int>>"));
        }

        SECTION("With alternative compare.")
        {
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_multimap<std::string, int, custom_hash<std::string>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::unordered_multimap<std::string, int, custom_hash<std::string>, custom_compare>"));
            REQUIRE_THAT(
                (print_type<std::pmr::unordered_multimap<int, std::tuple<int, int>, custom_hash<int>, custom_compare>>()),
                Catch::Matchers::Equals("std::pmr::unordered_multimap<int, std::tuple<int, int>, custom_hash<int>, custom_compare>"));
        }
    }

    SECTION("With alternative arguments.")
    {
        REQUIRE_THAT(
            (print_type<std::unordered_multimap<
                 std::string,
                 int,
                 custom_hash<std::string>,
                 custom_compare,
                 custom_allocator<std::pair<const std::string, int>>>>()),
            Catch::Matchers::Equals(
                "std::unordered_multimap<"
                "std::string, "
                "int, "
                "custom_hash<std::string>, "
                "custom_compare, "
                "custom_allocator<std::pair<const std::string, int>>>"));
        REQUIRE_THAT(
            (print_type<std::unordered_multimap<
                 int,
                 std::tuple<int, int>,
                 custom_hash<int>,
                 custom_compare,
                 custom_allocator<std::pair<const int, std::tuple<int, int>>>>>()),
            Catch::Matchers::Equals(
                "std::unordered_multimap<"
                "int, "
                "std::tuple<int, int>, "
                "custom_hash<int>, "
                "custom_compare, "
                "custom_allocator<std::pair<const int, std::tuple<int, int>>>>"));
    }
}

TEST_CASE(
    "print_type handles std::stack nicely.",
    "[print]")
{
    SECTION("With default container.")
    {
        REQUIRE_THAT(
            print_type<std::stack<int>>(),
            Catch::Matchers::Equals("std::stack<int>"));
        REQUIRE_THAT(
            print_type<std::stack<std::stack<int>>>(),
            Catch::Matchers::Equals("std::stack<std::stack<int>>"));
        REQUIRE_THAT(
            print_type<std::stack<std::string>>(),
            Catch::Matchers::Equals("std::stack<std::string>"));
        REQUIRE_THAT(
            print_type<std::stack<std::stack<std::string>>>(),
            Catch::Matchers::Equals("std::stack<std::stack<std::string>>"));
    }

    SECTION("With alternative container.")
    {
        REQUIRE_THAT(
            (print_type<std::stack<int, std::pmr::vector<int>>>()),
            Catch::Matchers::Equals("std::stack<int, std::pmr::vector<int>>"));
        REQUIRE_THAT(
            (print_type<std::stack<std::stack<int>, std::pmr::vector<std::stack<int>>>>()),
            Catch::Matchers::Equals("std::stack<std::stack<int>, std::pmr::vector<std::stack<int>>>"));
        REQUIRE_THAT(
            (print_type<std::stack<std::string, std::pmr::vector<std::string>>>()),
            Catch::Matchers::Equals("std::stack<std::string, std::pmr::vector<std::string>>"));
        REQUIRE_THAT(
            (print_type<std::stack<std::stack<std::string, std::pmr::vector<std::string>>>>()),
            Catch::Matchers::Equals("std::stack<std::stack<std::string, std::pmr::vector<std::string>>>"));
    }
}

TEST_CASE(
    "print_type handles std::queue nicely.",
    "[print]")
{
    SECTION("With default container.")
    {
        REQUIRE_THAT(
            print_type<std::queue<int>>(),
            Catch::Matchers::Equals("std::queue<int>"));
        REQUIRE_THAT(
            print_type<std::queue<std::queue<int>>>(),
            Catch::Matchers::Equals("std::queue<std::queue<int>>"));
        REQUIRE_THAT(
            print_type<std::queue<std::string>>(),
            Catch::Matchers::Equals("std::queue<std::string>"));
        REQUIRE_THAT(
            print_type<std::queue<std::queue<std::string>>>(),
            Catch::Matchers::Equals("std::queue<std::queue<std::string>>"));
    }

    SECTION("With alternative container.")
    {
        REQUIRE_THAT(
            (print_type<std::queue<int, std::pmr::vector<int>>>()),
            Catch::Matchers::Equals("std::queue<int, std::pmr::vector<int>>"));
        REQUIRE_THAT(
            (print_type<std::queue<std::queue<int>, std::pmr::vector<std::queue<int>>>>()),
            Catch::Matchers::Equals("std::queue<std::queue<int>, std::pmr::vector<std::queue<int>>>"));
        REQUIRE_THAT(
            (print_type<std::queue<std::string, std::pmr::vector<std::string>>>()),
            Catch::Matchers::Equals("std::queue<std::string, std::pmr::vector<std::string>>"));
        REQUIRE_THAT(
            (print_type<std::queue<std::queue<std::string, std::pmr::vector<std::string>>>>()),
            Catch::Matchers::Equals("std::queue<std::queue<std::string, std::pmr::vector<std::string>>>"));
    }
}

TEST_CASE(
    "print_type handles std::priority_queue nicely.",
    "[print]")
{
    SECTION("With default container.")
    {
        REQUIRE_THAT(
            print_type<std::priority_queue<int>>(),
            Catch::Matchers::Equals("std::priority_queue<int>"));
        REQUIRE_THAT(
            print_type<std::priority_queue<std::priority_queue<int>>>(),
            Catch::Matchers::Equals("std::priority_queue<std::priority_queue<int>>"));
        REQUIRE_THAT(
            print_type<std::priority_queue<std::string>>(),
            Catch::Matchers::Equals("std::priority_queue<std::string>"));
        REQUIRE_THAT(
            print_type<std::priority_queue<std::priority_queue<std::string>>>(),
            Catch::Matchers::Equals("std::priority_queue<std::priority_queue<std::string>>"));
    }

    SECTION("With alternative container.")
    {
        REQUIRE_THAT(
            (print_type<std::priority_queue<int, std::pmr::vector<int>>>()),
            Catch::Matchers::Equals("std::priority_queue<int, std::pmr::vector<int>>"));
        REQUIRE_THAT(
            (print_type<std::priority_queue<std::priority_queue<int>, std::pmr::vector<std::priority_queue<int>>>>()),
            Catch::Matchers::Equals("std::priority_queue<std::priority_queue<int>, std::pmr::vector<std::priority_queue<int>>>"));
        REQUIRE_THAT(
            (print_type<std::priority_queue<std::string, std::pmr::vector<std::string>>>()),
            Catch::Matchers::Equals("std::priority_queue<std::string, std::pmr::vector<std::string>>"));
        REQUIRE_THAT(
            (print_type<std::priority_queue<std::priority_queue<std::string, std::pmr::vector<std::string>>>>()),
            Catch::Matchers::Equals("std::priority_queue<std::priority_queue<std::string, std::pmr::vector<std::string>>>"));
    }

    SECTION("With alternative compare.")
    {
        REQUIRE_THAT(
            (print_type<std::priority_queue<int, std::vector<int>, custom_compare>>()),
            Catch::Matchers::Equals("std::priority_queue<int, std::vector<int>, custom_compare>"));
        REQUIRE_THAT(
            (print_type<std::priority_queue<std::priority_queue<int>, std::vector<std::priority_queue<int>>, custom_compare>>()),
            Catch::Matchers::Equals("std::priority_queue<std::priority_queue<int>, std::vector<std::priority_queue<int>>, custom_compare>>"));
        REQUIRE_THAT(
            (print_type<std::priority_queue<std::string, std::vector<std::string>, custom_compare>>()),
            Catch::Matchers::Equals("std::priority_queue<std::string>"));
        REQUIRE_THAT(
            (print_type<std::priority_queue<std::priority_queue<std::string>, std::vector<std::priority_queue<std::string>>, custom_compare>>()),
            Catch::Matchers::Equals("std::priority_queue<std::priority_queue<std::string>, std::vector<std::priority_queue<std::string>>, custom_compare>"));
    }
}
