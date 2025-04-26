//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include <source_location>

using namespace mimicpp;

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES

namespace
{
    struct outer_type
    {
        template <typename T>
        struct my_template
        {
        };

        struct my_type
        {
        };

        auto my_typeFunction()
        {
            struct my_type
            {
            };

            return my_type{};
        }

        auto my_typeNoexceptFunction() noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        auto my_typeConstFunction() const
        {
            struct my_type
            {
            };

            return my_type{};
        }

        auto my_typeLvalueFunction() &
        {
            struct my_type
            {
            };

            return my_type{};
        }

        auto my_typeConstLvalueFunction() const&
        {
            struct my_type
            {
            };

            return my_type{};
        }

        auto my_typeRvalueFunction() &&
        {
            struct my_type
            {
            };

            return my_type{};
        }

        auto my_typeConstRvalueFunction() const&&
        {
            struct my_type
            {
            };

            return my_type{};
        }

        static auto my_typeStaticFunction()
        {
            struct my_type
            {
            };

            return my_type{};
        }

        auto operator+(int)
        {
            struct my_type
            {
            };

            return my_type{};
        }

    private:
        auto my_typePrivateFunction()
        {
            struct my_type
            {
            };

            return my_type{};
        }

    public:
        auto my_typeIndirectlyPrivateFunction()
        {
            return my_typePrivateFunction();
        }
    };

    struct my_type
    {
    };

    constexpr auto my_typeLambda = [] {
        struct my_type
        {
        };

        return my_type{};
    };

    [[maybe_unused]] auto my_typeMutableLambda = []() mutable {
        struct my_type
        {
        };

        return my_type{};
    };

    constexpr auto my_typeNoexceptLambda = []() noexcept {
        struct my_type
        {
        };

        return my_type{};
    };

    constexpr auto my_typeNestedLambda = [] {
        constexpr auto inner = [] {
            struct my_type
            {
            };

            return my_type{};
        };

        return inner();
    };

    constexpr auto my_typeNestedLambda2 = [] {
        [[maybe_unused]] constexpr auto dummy = [] {};
        [[maybe_unused]] constexpr auto dummy2 = [] {};

        constexpr auto inner = [] {
            struct my_type
            {
            };

            return my_type{};
        };
        return inner();
    };

    [[maybe_unused]] auto my_typeFreeFunction()
    {
        struct my_type
        {
        };

        return my_type{};
    }

    StringT const topLevelLambdaPattern =
        R"((\$_\d+|lambda(#\d+|\d+)?))";

    StringT const lambdaScopePattern = topLevelLambdaPattern + "::";

    StringT const anonNsScopePattern = R"(\{anon-ns\}::)";
    StringT const anonTypePattern = R"((\$_\d+|\{unnamed type#\d+\}|<unnamed-type-anon_(class|struct|enum)>))";
    StringT const testCaseScopePattern = R"(CATCH2_INTERNAL_TEST_\d+::)";
    StringT const callOpScopePattern = R"(operator\(\)::)";
}

TEST_CASE(
    "printing::type::prettify_identifier enhances names appearance.",
    "[print]")
{
    StringStreamT ss{};

    SECTION("When type-name in anonymous-namespace is given.")
    {
        StringT const rawName = printing::type::type_name<my_type>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(anonNsScopePattern + "my_type"));
    }

    SECTION("When anon-class is given.")
    {
        class
        {
        } constexpr anon_class [[maybe_unused]]{};

        StringT const rawName = printing::type::type_name<decltype(anon_class)>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(testCaseScopePattern + anonTypePattern));
    }

    SECTION("When anon-struct is given.")
    {
        class
        {
        } constexpr anon_struct [[maybe_unused]]{};

        StringT const rawName = printing::type::type_name<decltype(anon_struct)>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(testCaseScopePattern + anonTypePattern));
    }

    SECTION("When anon-enum is given.")
    {
        enum
        {
        } constexpr anon_enum [[maybe_unused]]{};

        StringT const rawName = printing::type::type_name<decltype(anon_enum)>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(testCaseScopePattern + anonTypePattern));
    }

    SECTION("When nested type-name is given.")
    {
        StringT const rawName = printing::type::type_name<outer_type::my_type>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(anonNsScopePattern + "outer_type::my_type"));
    }

    SECTION("When lambda is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(my_typeLambda)>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"((auto )?)"
                + anonNsScopePattern
                + R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                + topLevelLambdaPattern));
    }

    SECTION("When lambda with params is given.")
    {
        [[maybe_unused]] constexpr auto lambda = [](std::string const&) {};
        StringT const rawName = printing::type::type_name<decltype(lambda)>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(testCaseScopePattern + topLevelLambdaPattern));
    }

    SECTION("When lambda-local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(my_typeLambda())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"((?:my_typeLambda::)?)" // gcc and clang produce this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type"));
    }

    SECTION("When mutable lambda-local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(my_typeMutableLambda())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"((?:my_typeMutableLambda::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type"));
    }

    SECTION("When noexcept lambda-local type-name is given.")
    {
        // noexcept doesn't seem to be part of the spec list
        StringT const rawName = printing::type::type_name<decltype(my_typeNoexceptLambda())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"((?:my_typeNoexceptLambda::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type"));
    }

    SECTION("When nested lambda-local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(my_typeNestedLambda())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"((?:my_typeNestedLambda::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type"));
    }

    SECTION("When nested lambda-local type-name is given (more inner lambdas).")
    {
        StringT const rawName = printing::type::type_name<decltype(my_typeNestedLambda2())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"((?:my_typeNestedLambda2::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type"));
    }

    SECTION("When free-function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(my_typeFreeFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "my_typeFreeFunction::"
                  "my_type"));
    }

    SECTION("When public function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(outer_type{}.my_typeFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typeFunction::"
                  "my_type"));
    }

    SECTION("When public noexcept function local type-name is given.")
    {
        // noexcept has no effect
        StringT const rawName = printing::type::type_name<decltype(outer_type{}.my_typeNoexceptFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typeNoexceptFunction::"
                  "my_type"));
    }

    SECTION("When public const-function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(outer_type{}.my_typeConstFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typeConstFunction::"
                  "my_type"));
    }

    SECTION("When public static-function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(outer_type::my_typeStaticFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typeStaticFunction::"
                  "my_type"));
    }

    SECTION("When public lvalue-function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(std::declval<outer_type&>().my_typeLvalueFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typeLvalueFunction::"
                  "my_type"));
    }

    SECTION("When public const lvalue-function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(std::declval<outer_type const&>().my_typeConstLvalueFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typeConstLvalueFunction::"
                  "my_type"));
    }

    SECTION("When public rvalue-function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(outer_type{}.my_typeRvalueFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typeRvalueFunction::"
                  "my_type"));
    }

    SECTION("When public const rvalue-function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(std::declval<outer_type const&&>().my_typeConstRvalueFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typeConstRvalueFunction::"
                  "my_type"));
    }

    SECTION("When private function local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(outer_type{}.my_typeIndirectlyPrivateFunction())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  "my_typePrivateFunction::"
                  "my_type"));
    }

    SECTION("When public operator local type-name is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(outer_type{}.operator+(42))>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "outer_type::"
                  R"(operator\+::)"
                  "my_type"));
    }
}

TEST_CASE(
    "printing::type::prettify_identifier enhances local type-names appearance.",
    "[!mayfail][print]")
{
    StringStreamT ss{};

    SECTION("When local type is queried inside the current scope.")
    {
        struct my_type
        {
        };

        StringT const rawName = printing::type::type_name<my_type>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(testCaseScopePattern + R"(my_type)"));
    }

    SECTION("When local type is queried inside a lambda.")
    {
        std::invoke(
            [&] {
                struct my_type
                {
                };

                StringT const rawName = printing::type::type_name<my_type>();
                CAPTURE(rawName);

                printing::type::prettify_identifier(
                    std::ostreambuf_iterator{ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(ss).str(),
                    Catch::Matchers::Matches(
                        testCaseScopePattern
                        + lambdaScopePattern
                        + callOpScopePattern
                        + "my_type"));
            });
    }

    SECTION("When local type is queried inside a member-function.")
    {
        struct outer
        {
            void operator()(StringStreamT& _ss) const
            {
                struct my_type
                {
                };

                StringT const rawName = printing::type::type_name<my_type>();
                CAPTURE(rawName);

                printing::type::prettify_identifier(
                    std::ostreambuf_iterator{_ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(_ss).str(),
                    Catch::Matchers::Matches(
                        testCaseScopePattern
                        + "outer::"
                        + callOpScopePattern
                        + "my_type"));
            }
        };

        outer{}(ss);
    }

    SECTION("When local type is queried inside a lambda with higher arity.")
    {
        // Todo: This case will currently fail, because parser does not handle arrays.

        int d1{};
        int d2[1]{};
        int* ptr = &d1;
        std::invoke(
            [](
                StringStreamT* _ss,
                [[maybe_unused]] int&& ref,
                [[maybe_unused]] int(&arrRef)[1],
                [[maybe_unused]] int*& ptrRef) {
                struct my_type
                {
                };

                StringT const rawName = printing::type::type_name<my_type>();
                CAPTURE(rawName);

                printing::type::prettify_identifier(
                    std::ostreambuf_iterator{*_ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(*_ss).str(),
                    Catch::Matchers::Matches(
                        testCaseScopePattern
                        + lambdaScopePattern
                        + callOpScopePattern
                        + "my_type"));
            },
            &ss,
            std::move(d1),
            d2,
            ptr);
    }

    SECTION("When local type is queried inside a nested-lambda with higher arity.")
    {
        std::invoke(
            [](StringStreamT* _ss) {
                struct other_type
                {
                };

                std::invoke(
                    [&]([[maybe_unused]] other_type const& dummy) {
                        struct my_type
                        {
                        };

                        StringT const rawName = printing::type::type_name<my_type>();
                        CAPTURE(rawName);

                        printing::type::prettify_identifier(
                            std::ostreambuf_iterator{*_ss},
                            rawName);
                        REQUIRE_THAT(
                            std::move(*_ss).str(),
                            Catch::Matchers::Matches(
                                testCaseScopePattern
                                + lambdaScopePattern
                                + callOpScopePattern
                                + lambdaScopePattern
                                + callOpScopePattern
                                + "my_type"));
                    },
                    other_type{});
            },
            &ss);
    }
}

TEST_CASE(
    "printing::type::prettify_identifier type-names enhances function type-names appearance.",
    "[print]")
{
    StringStreamT ss{};

    SECTION("When function-local type is returned.")
    {
        using return_t = decltype(my_typeLambda());
        StringT const rawName = printing::type::type_name<return_t()>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type "
                  R"(\(\))"));
    }

    SECTION("When function-local type is parameter.")
    {
        using param_t = decltype(my_typeLambda());
        StringT const rawName = printing::type::type_name<void(param_t)>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                R"(void \()"
                + anonNsScopePattern
                + R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type"
                  R"(\))"));
    }
}

TEST_CASE(
    "printing::type::prettify_identifier type-names enhances function-pointer type-names appearance.",
    "[print]")
{
    StringStreamT ss{};

    SECTION("When function-local type is returned.")
    {
        using return_t = decltype(my_typeLambda());
        StringT const rawName = printing::type::type_name<return_t (*)()>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type "
                  R"(\(\*\)\(\))"));
    }

    SECTION("When function-local type is parameter.")
    {
        using param_t = decltype(my_typeLambda());
        StringT const rawName = printing::type::type_name<void (*)(param_t)>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                R"(void \(\*\)\()"
                + anonNsScopePattern
                + R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + "my_type"
                  R"(\))"));
    }
}

namespace
{
    template <typename... Ts>
    struct my_template
    {
        struct my_type
        {
        };

        std::source_location foo(my_type)
        {
            return std::source_location::current();
        }

        auto bar(my_type const&, std::source_location* outLoc)
        {
            if (outLoc)
            {
                *outLoc = std::source_location::current();
            }

            struct bar_type
            {
            };

            return bar_type{};
        }
    };
}

TEST_CASE(
    "printing::type::prettify_identifier enhances template type-names appearance.",
    "[print]")
{
    StringStreamT ss{};

    SECTION("When template name in anonymous-namespace is given.")
    {
        StringT const rawName = printing::type::type_name<my_template<int>>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(anonNsScopePattern + "my_template<int>"));
    }

    SECTION("When template-dependant name is given.")
    {
        StringT const rawName = printing::type::type_name<my_template<int, std::string const&&>::my_type>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(anonNsScopePattern + "my_template::my_type"));
    }

    SECTION("When template-dependant member-function-pointer is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(&my_template<my_template<>>::foo)>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        StringT const argListPattern =
    #if MIMICPP_DETAIL_IS_MSVC // it seems msvc applies an address instead of anonymous-namespace
            R"(A0x\w+::my_template::my_type)";
    #else
            anonNsScopePattern + R"(my_template::my_type)";
    #endif
        StringT const pattern =
            "std::source_location " // return type
            R"(\()"
            + anonNsScopePattern
            + R"(my_template::\*\))"
              R"(\()"
            + argListPattern
            + R"(\))";

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(pattern));
    }

    SECTION("When template-dependant member-function-pointer, returning local type, is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(&my_template<my_template<>>::bar)>();
        CAPTURE(rawName);

        StringT const returnPattern =
    #if MIMICPP_DETAIL_IS_MSVC // it seems msvc applies an address instead of anonymous-namespace
            R"(A0x\w+::)"
    #else
            anonNsScopePattern +
    #endif
            R"(my_template::bar::bar_type)";

        StringT const argListPattern =
    #if MIMICPP_DETAIL_IS_MSVC // it seems msvc applies an address instead of anonymous-namespace
            R"(A0x\w+::)"
    #else
            anonNsScopePattern +
    #endif
            R"(my_template::my_type const\s?&, std::source_location\s?\*)";

        StringT const pattern =
            returnPattern
            + R"( \()"
            + anonNsScopePattern
            + R"(my_template::\*\))"
              R"(\()"
            + argListPattern
            + R"(\))";

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(pattern));
    }

    SECTION("When arbitrary template name is given.")
    {
        using type_t = decltype(my_typeLambda());
        StringT const rawName = printing::type::type_name<my_template<type_t&, std::string const&&>>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"(my_template<)"
                + anonNsScopePattern
                + R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                + lambdaScopePattern
                + callOpScopePattern
                + R"(my_type\s?&)"
                  R"(,\s?)"
                  R"(std::basic_string const\s?&&)"
                  ">"));
    }
}

namespace
{
    class special_operators
    {
    public:
        [[nodiscard]]
        auto operator<(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator<=(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator>(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator>=(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator<(std::string) const noexcept
        {
            struct my_type
            {
                [[nodiscard]]
                auto operator>=(int) const noexcept
                {
                    struct my_type
                    {
                    };

                    return my_type{};
                }
            };

            return my_type{}.operator>=(42);
        }

        [[nodiscard]]
        auto operator<=(std::string) const noexcept
        {
            struct my_type
            {
                [[nodiscard]]
                auto operator>(int) const noexcept
                {
                    struct my_type
                    {
                    };

                    return my_type{};
                }
            };

            return my_type{}.operator>(42);
        }

        [[nodiscard]]
        auto operator>(std::string) const noexcept
        {
            struct my_type
            {
                [[nodiscard]]
                auto operator<=(int) const noexcept
                {
                    struct my_type
                    {
                    };

                    return my_type{};
                }
            };

            return my_type{}.operator<=(42);
        }

        [[nodiscard]]
        auto operator>=(std::string) const noexcept
        {
            struct my_type
            {
                [[nodiscard]]
                auto operator<(int) const noexcept
                {
                    struct my_type
                    {
                    };

                    return my_type{};
                }
            };

            return my_type{}.operator<(42);
        }

        [[nodiscard]]
        auto operator<=>(int) const noexcept
        {
            struct my_type
            {
            };

            return my_type{};
        }

        [[nodiscard]]
        auto operator()(int) const
        {
            struct my_type
            {
            };

            return my_type{};
        }
    };
}

TEST_CASE(
    "printing::type::prettify_identifier supports operator<, <=, >, >= and <=>.",
    "[print]")
{
    StringStreamT ss{};

    SECTION("When ordering operator is used.")
    {
        auto const [expectedFunctionName, rawName] = GENERATE(
            (table<StringT, StringT>)({
                { R"(operator<)",  printing::type::type_name<decltype(special_operators{}.operator<(42))>()},
                {R"(operator<=)", printing::type::type_name<decltype(special_operators{}.operator<=(42))>()},
                { R"(operator>)",  printing::type::type_name<decltype(special_operators{}.operator>(42))>()},
                {R"(operator>=)", printing::type::type_name<decltype(special_operators{}.operator>=(42))>()}
        }));
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"(special_operators::)"
                + expectedFunctionName
                + "::my_type"));
    }

    SECTION("When nested ordering operator is used.")
    {
        auto const [expectedFunctionName, expectedNestedFunctionName, rawName] = GENERATE(
            (table<StringT, StringT, StringT>)({
                { R"(operator<)", R"(operator>=)",  printing::type::type_name<decltype(special_operators{}.operator<(""))>()},
                {R"(operator<=)",  R"(operator>)", printing::type::type_name<decltype(special_operators{}.operator<=(""))>()},
                { R"(operator>)", R"(operator<=)",  printing::type::type_name<decltype(special_operators{}.operator>(""))>()},
                {R"(operator>=)",  R"(operator<)", printing::type::type_name<decltype(special_operators{}.operator>=(""))>()}
        }));
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"(special_operators::)"
                + expectedFunctionName
                + "::my_type::"
                + expectedNestedFunctionName
                + "::my_type"));
    }

    SECTION("When spaceship-operator is used.")
    {
        StringT const rawName = printing::type::type_name<decltype(special_operators{}.operator<=>(42))>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "special_operators::"
                + "operator<=>::"
                + "my_type"));
    }
}

TEST_CASE(
    "printing::type::prettify_identifier supports operator().",
    "[print]")
{
    StringStreamT ss{};

    SECTION("When identifier contains operator() scope.")
    {
        StringT const rawName = printing::type::type_name<decltype(special_operators{}.operator()(42))>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "special_operators::"
                + R"(operator\(\)::)"
                + "my_type"));
    }

    SECTION("When member-function-pointer to operator() is given.")
    {
        StringT const rawName = printing::type::type_name<decltype(&special_operators::operator())>();
        CAPTURE(rawName);

        printing::type::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        StringT const returnPattern =
    #if MIMICPP_DETAIL_IS_MSVC // it seems msvc applies an address instead of anonymous-namespace
            R"(A0x\w+::)"
    #else
            anonNsScopePattern +
    #endif
            R"(special_operators::operator\(\)::my_type )";

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                returnPattern
                + R"(\()"
                + anonNsScopePattern
                + R"(special_operators::\*\))"
                + R"(\(int\)\s?const)"));
    }
}

TEST_CASE(
    "printing::type::prettify_identifier omits function args with just `void` content.",
    "[print]")
{
    StringT const name = "return my_function<void>(void)";

    StringStreamT ss{};
    printing::type::prettify_identifier(
        std::ostreambuf_iterator{ss},
        name);

    REQUIRE_THAT(
        ss.str(),
        Catch::Matchers::Equals(+"return my_function<void>()"));
}

#endif
