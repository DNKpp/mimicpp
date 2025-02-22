//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

using namespace mimicpp;

#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

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

    [[maybe_unused]] auto my_typeFreeFunction()
    {
        struct my_type
        {
        };

        return my_type{};
    }
}

TEST_CASE(
    "printing::detail::prettify_identifier enhances names appearance.",
    "[print][detail]")
{
    StringStreamT ss{};

    SECTION("When type-name in anonymous-namespace is given.")
    {
        StringT const rawName = printing::detail::type_name<my_type>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Equals("(anon ns)::my_type"));
    }

    SECTION("When nested type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<outer_type::my_type>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Equals("(anon ns)::outer_type::my_type"));
    }

    SECTION("When lambda-local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(my_typeLambda())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                R"((?:my_typeLambda::)?)" // gcc and clang produce this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                "my_type"));
    }

    SECTION("When mutable lambda-local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(my_typeMutableLambda())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                R"((?:my_typeMutableLambda::)?)" // gcc produces this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                "my_type"));
    }

    SECTION("When noexcept lambda-local type-name is given.")
    {
        // noexcept doesn't seem to be part of the spec list
        StringT const rawName = printing::detail::type_name<decltype(my_typeNoexceptLambda())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                R"((?:my_typeNoexceptLambda::)?)" // gcc produces this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                "my_type"));
    }

    SECTION("When nested lambda-local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(my_typeNestedLambda())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                R"((?:my_typeNestedLambda::)?)" // gcc produces this extra scope
                R"((lambda#\d+::)"
                R"(\(operator\(\)\)::){2})"
                "my_type"));
    }

    SECTION("When free-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(my_typeFreeFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                R"(\(my_typeFreeFunction\)::)"
                "my_type"));
    }

    SECTION("When public function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typeFunction\)::)"
                "my_type"));
    }

    SECTION("When public noexcept function local type-name is given.")
    {
        // noexcept has no effect
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeNoexceptFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typeNoexceptFunction\)::)"
                "my_type"));
    }

    SECTION("When public const-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeConstFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typeConstFunction\)::)"
                "my_type"));
    }

    SECTION("When public static-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type::my_typeStaticFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typeStaticFunction\)::)"
                "my_type"));
    }

    SECTION("When public lvalue-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(std::declval<outer_type&>().my_typeLvalueFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typeLvalueFunction\)::)"
                "my_type"));
    }

    SECTION("When public const lvalue-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(std::declval<outer_type const&>().my_typeConstLvalueFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typeConstLvalueFunction\)::)"
                "my_type"));
    }

    SECTION("When public rvalue-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeRvalueFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typeRvalueFunction\)::)"
                "my_type"));
    }

    SECTION("When public const rvalue-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(std::declval<outer_type const&&>().my_typeConstRvalueFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typeConstRvalueFunction\)::)"
                "my_type"));
    }

    SECTION("When private function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeIndirectlyPrivateFunction())>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(my_typePrivateFunction\)::)"
                "my_type"));
    }

    SECTION("When public operator local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.operator+(42))>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                "outer_type::"
                R"(\(operator\+\)::)"
                "my_type"));
    }
}

TEST_CASE(
    "printing::detail::prettify_identifier enhances local type-names appearance.",
    "[print][detail]")
{
    StringStreamT ss{};

    SECTION("When local type is queried inside the current scope.")
    {
        struct my_type
        {
        };

        StringT const rawName = printing::detail::type_name<my_type>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(R"(\(CATCH2_INTERNAL_TEST_\d+\)::my_type)"));
    }

    SECTION("When local type is queried inside a lambda.")
    {
        std::invoke(
            [&] {
                struct my_type
                {
                };

                StringT const rawName = printing::detail::type_name<my_type>();
                CAPTURE(rawName);

                printing::type::detail::prettify_identifier(
                    std::ostreambuf_iterator{ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(ss).str(),
                    Catch::Matchers::Matches(
                        R"(\(CATCH2_INTERNAL_TEST_\d+\)::)"
                        R"(lambda#\d+::)"
                        R"(\(operator\(\)\)::)"
                        "my_type"));
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

                StringT const rawName = printing::detail::type_name<my_type>();
                CAPTURE(rawName);

                printing::type::detail::prettify_identifier(
                    std::ostreambuf_iterator{_ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(_ss).str(),
                    Catch::Matchers::Matches(
                        R"(\(CATCH2_INTERNAL_TEST_\d+\)::)"
                        "outer::"
                        R"(\(operator\(\)\)::)"
                        "my_type"));
            }
        };

        outer{}(ss);
    }

    SECTION("When local type is queried inside a lambda with higher arity.")
    {
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

                StringT const rawName = printing::detail::type_name<my_type>();
                CAPTURE(rawName);

                printing::type::detail::prettify_identifier(
                    std::ostreambuf_iterator{*_ss},
                    rawName);
                REQUIRE_THAT(
                    std::move(*_ss).str(),
                    Catch::Matchers::Matches(
                        R"(\(CATCH2_INTERNAL_TEST_\d+\)::)"
                        R"(lambda#\d+::)"
                        R"(\(operator\(\)\)::)"
                        "my_type"));
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

                        StringT const rawName = printing::detail::type_name<my_type>();
                        CAPTURE(rawName);

                        printing::type::detail::prettify_identifier(
                            std::ostreambuf_iterator{*_ss},
                            rawName);
                        REQUIRE_THAT(
                            std::move(*_ss).str(),
                            Catch::Matchers::Matches(
                                R"(\(CATCH2_INTERNAL_TEST_\d+\)::)"
                                R"(lambda#\d+::)"
                                R"(\(operator\(\)\)::)"
                                R"(lambda#\d+::)"
                                R"(\(operator\(\)\)::)"
                                "my_type"));
                    },
                    other_type{});
            },
            &ss);
    }
}

TEST_CASE(
    "printing::type::detail::prettify_identifier type-names enhances function type-names appearance.",
    "[print][detail]")
{
    StringStreamT ss{};

    SECTION("When function-local type is returned.")
    {
        using return_t = decltype(my_typeLambda());
        StringT const rawName = printing::detail::type_name<return_t()>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                "my_type"
                R"(\s*\(\))"));
    }

    SECTION("When function-local type is parameter.")
    {
        using param_t = decltype(my_typeLambda());
        StringT const rawName = printing::detail::type_name<void(param_t)>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                R"(void\s*\()"
                R"(\(anon ns\)::)"
                R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                "my_type"
                R"(\))"));
    }
}

TEST_CASE(
    "printing::type::detail::prettify_identifier type-names enhances function-pointer type-names appearance.",
    "[print][detail]")
{
    StringStreamT ss{};

    SECTION("When function-local type is returned.")
    {
        using return_t = decltype(my_typeLambda());
        StringT const rawName = printing::detail::type_name<return_t (*)()>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                "my_type"
                R"(\s*\(\*\)\(\))"));
    }

    SECTION("When function-local type is parameter.")
    {
        using param_t = decltype(my_typeLambda());
        StringT const rawName = printing::detail::type_name<void (*)(param_t)>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                R"(void\s*\(\*\)\()"
                R"(\(anon ns\)::)"
                R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                "my_type"
                R"(\))"));
    }
}

namespace
{
    template <typename... Ts>
    struct my_template
    {
    };
}

TEST_CASE(
    "printing::type::detail::prettify_identifier type-names enhances template-dependant type-names appearance.",
    "[print][detail]")
{
    StringStreamT ss{};

    SECTION("When arbitrary template name is given.")
    {
        using type_t = decltype(my_typeLambda());
        StringT const rawName = printing::detail::type_name<my_template<type_t&, type_t const&&>>();
        CAPTURE(rawName);

        printing::type::detail::prettify_identifier(
            std::ostreambuf_iterator{ss},
            rawName);
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                R"(\(anon ns\)::)"
                R"(my_template<)"
                R"(\(anon ns\)::)"
                R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                R"(my_type\s*&)"
                R"(,\s*)"
                R"(\(anon ns\)::)"
                R"((?:my_typeLambda::)?)" // gcc produces this extra scope
                R"(lambda#\d+::)"
                R"(\(operator\(\)\)::)"
                R"(my_type\s+const\s*&&)"
                ">"));
    }

    /*SECTION("When template name in anonymous-namespace is given.")
    {
        StringT const rawName = printing::detail::type_name<my_template<int>>();
        CAPTURE(rawName);

        StringT const finalName = printing::detail::prettify_template_name(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Equals("(anon ns)::my_template"));
    }

    SECTION("When nested template name is given.")
    {
        StringT const rawName = printing::detail::type_name<outer_type::my_template<int>>();
        CAPTURE(rawName);

        StringT const finalName = printing::detail::prettify_template_name(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Equals("(anon ns)::outer_type::my_template"));
    }

    SECTION("When function-local template name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(testLambda<int>())>();
        CAPTURE(rawName);

        StringT const finalName = printing::detail::prettify_template_name(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_template)"));
    }*/
}

#endif
