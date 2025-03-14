//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include <source_location>

#ifndef MIMICPP_CONFIG_MINIMAL_PRETTY_TYPE_PRINTING

using namespace mimicpp;

namespace
{
    template <typename T>
    [[nodiscard]]
    StringT name_of()
    {
        return printing::type::detail::apply_basic_transformations(
            printing::type::type_name<T>());
    }
}

struct TypeScopeVisitor_cpp_my_type
{
};

namespace
{
    struct my_type
    {
    };

    struct
    {

    } constexpr anonStruct [[maybe_unused]]{};

    class
    {

    } constexpr anonClass [[maybe_unused]]{};

    enum
    {

    } constexpr anonEnum [[maybe_unused]]{};
}

namespace test_ns
{
    namespace
    {
        struct my_type
        {
        };
    }
}

TEST_CASE(
    "printing::type::detail::ScopeIterator advances the scopes of a given type-identifier.",
    "[print][detail]")
{
    SECTION("When type in global namespace is given.")
    {
        StringT const rawName = name_of<TypeScopeVisitor_cpp_my_type>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        SECTION("First scope refers to the actual type name.")
        {
            auto const topLevelScope = visitor();
            CHECK(topLevelScope);
            CHECK_FALSE(topLevelScope->functionInfo);
            CHECK_FALSE(topLevelScope->templateInfo);
            CHECK_THAT(
                StringT{topLevelScope->identifier},
                Catch::Matchers::Equals("TypeScopeVisitor_cpp_my_type"));

            SECTION("No subsequent scopes exist.")
            {
                CHECK(std::nullopt == visitor());
            }
        }
    }

    SECTION("When type in anonymous-namespace is given.")
    {
        StringT const rawName = name_of<my_type>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        SECTION("First scope refers to the anon-ns.")
        {
            auto const anonNsScope = visitor();
            CHECK(anonNsScope);
            CHECK_FALSE(anonNsScope->functionInfo);
            CHECK_FALSE(anonNsScope->templateInfo);
            CHECK_THAT(
                StringT{anonNsScope->identifier},
                Catch::Matchers::ContainsSubstring("anonymous")
                    && Catch::Matchers::ContainsSubstring("namespace"));

            SECTION("Second scope refers to the actual type name.")
            {
                auto const topLevelScope = visitor();
                CHECK(topLevelScope);
                CHECK_FALSE(topLevelScope->functionInfo);
                CHECK_FALSE(topLevelScope->templateInfo);
                CHECK_THAT(
                    StringT{topLevelScope->identifier},
                    Catch::Matchers::Equals("my_type"));

                SECTION("No subsequent scopes exist.")
                {
                    CHECK(std::nullopt == visitor());
                }
            }
        }
    }

    SECTION("When local-type is given.")
    {
        struct my_type
        {
        };

        StringT const rawName = name_of<my_type>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        SECTION("First scope refers the test-case.")
        {
            auto const anonNsScope = visitor();
            CHECK(anonNsScope);
            CHECK_THAT(
                StringT{anonNsScope->identifier},
                Catch::Matchers::Matches(R"(CATCH2_INTERNAL_TEST_\d+)"));
            CHECK(anonNsScope->functionInfo);
            // may or may not have a return type
            CHECK_THAT(
                anonNsScope->functionInfo->argList,
                Catch::Matchers::IsEmpty());
            CHECK_THAT(
                anonNsScope->functionInfo->specs,
                Catch::Matchers::IsEmpty());
            CHECK_FALSE(anonNsScope->templateInfo);

            SECTION("Second scope refers to the actual type name.")
            {
                auto const topLevelScope = visitor();
                CHECK(topLevelScope);
                CHECK_FALSE(topLevelScope->functionInfo);
                CHECK_FALSE(topLevelScope->templateInfo);
                CHECK_THAT(
                    StringT{topLevelScope->identifier},
                    Catch::Matchers::Equals("my_type"));

                SECTION("No subsequent scopes exist.")
                {
                    CHECK(std::nullopt == visitor());
                }
            }
        }
    }

    SECTION("When type in arbitrarily nested namespace is given.")
    {
        StringT const rawName = name_of<test_ns::my_type>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        SECTION("First scope refers to the test-ns.")
        {
            auto const testNsScope = visitor();
            CHECK(testNsScope);
            CHECK_FALSE(testNsScope->functionInfo);
            CHECK_FALSE(testNsScope->templateInfo);
            CHECK_THAT(
                StringT{testNsScope->identifier},
                Catch::Matchers::Equals("test_ns"));

            SECTION("Second scope refers to the anon-ns.")
            {
                auto const anonNsScope = visitor();
                CHECK(anonNsScope);
                CHECK_FALSE(anonNsScope->functionInfo);
                CHECK_FALSE(anonNsScope->templateInfo);
                CHECK_THAT(
                    StringT{anonNsScope->identifier},
                    Catch::Matchers::ContainsSubstring("anonymous")
                        && Catch::Matchers::ContainsSubstring("namespace"));

                SECTION("Third scope refers to the actual type name.")
                {
                    auto const topLevelScope = visitor();
                    CHECK(topLevelScope);
                    CHECK_FALSE(topLevelScope->functionInfo);
                    CHECK_FALSE(topLevelScope->templateInfo);
                    CHECK_THAT(
                        StringT{topLevelScope->identifier},
                        Catch::Matchers::Equals("my_type"));

                    SECTION("No subsequent scopes exist.")
                    {
                        CHECK(std::nullopt == visitor());
                    }
                }
            }
        }
    }

    SECTION("When anon-type is given.")
    {
        StringT const rawName = GENERATE(
            name_of<decltype(anonClass)>(),
            name_of<decltype(anonStruct)>(),
            name_of<decltype(anonEnum)>());
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());

        auto const topLevelScope = visitor();
        CHECK(topLevelScope);
        CHECK_FALSE(topLevelScope->functionInfo);
        CHECK_FALSE(topLevelScope->templateInfo);
        // The actual result is too different between all platforms.
        // It's important that something is there; everything else would be too much of a hassle.
        CHECK_THAT(
            StringT{topLevelScope->identifier},
            !Catch::Matchers::IsEmpty());

        REQUIRE_FALSE(visitor());
    }
}

namespace
{
    [[maybe_unused]] constexpr auto anon_structLambda = [] {
        struct
        {
        } constexpr obj [[maybe_unused]]{};

        return obj;
    };

    [[maybe_unused]] constexpr auto anon_classLambda = [] {
        struct
        {
        } constexpr obj [[maybe_unused]]{};

        return obj;
    };

    [[maybe_unused]] constexpr auto anon_enumLambda = [] {
        enum
        {
        } constexpr obj [[maybe_unused]]{};

        return obj;
    };

    [[maybe_unused]] constexpr auto my_typeLambda = [] {
        struct my_type
        {
        };

        return my_type{};
    };

    [[maybe_unused]] constexpr auto my_typeUnaryLambda = [](int) {
        struct my_type
        {
        };

        return my_type{};
    };

    [[maybe_unused]] constexpr auto my_typeBinaryLambda = [](int, float) {
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

    [[maybe_unused]] constexpr auto my_typeNoexceptLambda = []() noexcept {
        struct my_type
        {
        };

        return my_type{};
    };

    [[maybe_unused]] constexpr auto my_typeNestedLambda = [] {
        constexpr auto inner = [] {
            struct my_type
            {
            };

            return my_type{};
        };

        return inner();
    };

    [[maybe_unused]] constexpr auto my_typeComplexNestedLambda = [] {
        [[maybe_unused]] constexpr auto dummy = [] {};
        [[maybe_unused]] constexpr auto dummy2 = [] {};

        constexpr auto inner = [](int, float) {
            struct my_type
            {
            };

            return my_type{};
        };

        [[maybe_unused]] constexpr auto dummy3 = [] {};
        [[maybe_unused]] constexpr auto dummy4 = [] {};

        return inner(42, 4.2f);
    };

    void checkNamedScope(
        printing::type::detail::ScopeIterator& iter,
        auto const& nameMatcher)
    {
        auto const scope = iter();
        REQUIRE(scope);
        REQUIRE_FALSE(scope->templateInfo);
        REQUIRE_FALSE(scope->functionInfo);

        CHECK_THAT(
            StringT{scope->identifier},
            nameMatcher);
    }

    template <
        typename ReturnTypeMatcher = Catch::Matchers::IsEmptyMatcher,
        typename ArgListMatcher = Catch::Matchers::IsEmptyMatcher,
        typename SpecsMatcher = Catch::Matchers::IsEmptyMatcher>
    struct function_info_matchers
    {
        ReturnTypeMatcher returnTypeMatcher{};
        ArgListMatcher argListMatcher{};
        SpecsMatcher specsMatcher{};
    };

    template <
        typename ArgListMatcher = Catch::Matchers::IsEmptyMatcher,
        typename SpecsMatcher = Catch::Matchers::IsEmptyMatcher>
    struct template_info_matchers
    {
        ArgListMatcher argListMatcher{};
        SpecsMatcher specsMatcher{};
    };

    void checkFunctionScope(
        printing::type::detail::ScopeIterator& iter,
        auto const& nameMatcher,
        auto const& infoMatchers = template_info_matchers{})
    {
        auto const fnScope = iter();
        REQUIRE(fnScope);

        CHECK_FALSE(fnScope->templateInfo);
        CHECK_THAT(
            StringT{fnScope->identifier},
            nameMatcher);

        REQUIRE(fnScope->functionInfo);
        CHECK_THAT(
            StringT{fnScope->functionInfo->returnType},
            infoMatchers.returnTypeMatcher);
        CHECK_THAT(
            StringT{fnScope->functionInfo->argList},
            infoMatchers.argListMatcher);
        CHECK_THAT(
            StringT{fnScope->functionInfo->specs},
            infoMatchers.specsMatcher);
    }

    void checkTemplateFunctionScope(
        printing::type::detail::ScopeIterator& iter,
        auto const& nameMatcher,
        auto const& templateInfoMatchers,
        auto const& functionInfoMatchers)
    {
        auto const fnScope = iter();
        REQUIRE(fnScope);

        CHECK_THAT(
            StringT{fnScope->identifier},
            nameMatcher);

        REQUIRE(fnScope->functionInfo);
        CHECK_THAT(
            StringT{fnScope->functionInfo->returnType},
            functionInfoMatchers.returnTypeMatcher);
        CHECK_THAT(
            StringT{fnScope->functionInfo->argList},
            functionInfoMatchers.argListMatcher);
        CHECK_THAT(
            StringT{fnScope->functionInfo->specs},
            functionInfoMatchers.specsMatcher);

        REQUIRE(fnScope->templateInfo);
        CHECK_THAT(
            StringT{fnScope->templateInfo->argList},
            templateInfoMatchers.argListMatcher);
        CHECK_THAT(
            StringT{fnScope->templateInfo->specs},
            templateInfoMatchers.specsMatcher);
    }

    void checkLambdaScope(printing::type::detail::ScopeIterator& iter)
    {
        auto const info = iter();

        REQUIRE(info);
        CHECK_FALSE(info->templateInfo);
        CHECK_FALSE(info->functionInfo);

    #if MIMICPP_DETAIL_IS_CLANG

        // clang often uses the `anon-type` form for lambdas
        CHECKED_IF(info->identifier.starts_with('$'))
        {
            CHECK_THAT(
                StringT{info->identifier},
                Catch::Matchers::Matches(R"(\$_\d+)"));
        }

        // but sometimes uses actual `lambda` form
        CHECKED_IF(info->identifier.starts_with("lambda"))
        {
        #if MIMICPP_DETAIL_USES_LIBCXX
            CHECK_THAT(
                StringT{info->identifier},
                Catch::Matchers::Equals("lambda"));
        #else
            CHECK_THAT(
                StringT{info->identifier},
                Catch::Matchers::Matches(R"(lambda#\d+)"));
        #endif
        }
    #elif MIMICPP_DETAIL_IS_GCC
        CHECK_THAT(
            StringT{info->identifier},
            Catch::Matchers::Matches(R"(lambda#\d+)"));
    #endif
    }

    void checkLambdaName(
        [[maybe_unused]] StringT const& expectedName,
        [[maybe_unused]] auto& iter)
    {
    #if MIMICPP_DETAIL_IS_GCC
        auto const nameScope = visitor();
        CHECK(nameScope);
        CHECK_FALSE(nameScope->functionInfo);
        CHECK_FALSE(nameScope->templateInfo);
        CHECK_THAT(
            StringT{nameScope->identifier},
            Catch::Matchers::Equals(expectedName));
    #endif
    }

    void checkLambdaCallScope(
        printing::type::detail::ScopeIterator& iter,
        auto const& infoMatchers)
    {
        checkLambdaScope(iter);
        checkFunctionScope(iter, Catch::Matchers::Equals("operator()"), infoMatchers);
    }
}

TEST_CASE(
    "printing::type::detail::ScopeIterator supports all kinds of lambdas.",
    "[print][detail]")
{
    SECTION("When immutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeLambda", iter);
        checkLambdaScope(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When unary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeUnaryLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeUnaryLambda", iter);
        checkLambdaScope(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When binary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeBinaryLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeBinaryLambda", iter);
        checkLambdaScope(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When mutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeMutableLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeMutableLambda", iter);
        checkLambdaScope(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When noexcept lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeNoexceptLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeNoexceptLambda", iter);
        checkLambdaScope(iter);
        REQUIRE_FALSE(iter());
    }
}

TEST_CASE(
    "printing::type::detail::ScopeIterator supports lambda-local types.",
    "[print][detail]")
{
    constexpr auto checkTopLevel = [](auto& visitor) {
        auto const topLevelScope = visitor();
        CHECK(topLevelScope);
        CHECK_FALSE(topLevelScope->functionInfo);
        CHECK_FALSE(topLevelScope->templateInfo);
        CHECK_THAT(
            StringT{topLevelScope->identifier},
            Catch::Matchers::Equals("my_type"));
    };

    SECTION("When anon lambda-local type is given.")
    {
        auto const [expectedLambdaName, rawName] = GENERATE(
            (table<StringT, StringT>)({
                { "anon_classLambda",  name_of<decltype(anon_classLambda())>()},
                {"anon_structLambda", name_of<decltype(anon_structLambda())>()},
                {  "anon_enumLambda",   name_of<decltype(anon_enumLambda())>()}
        }));
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName(expectedLambdaName, iter);
        checkLambdaCallScope(
            iter,
            function_info_matchers{.specsMatcher = Catch::Matchers::Equals("const")});

        auto const topLevelScope = iter();
        CHECK(topLevelScope);
        CHECK_FALSE(topLevelScope->functionInfo);
        CHECK_FALSE(topLevelScope->templateInfo);
        CHECK_THAT(
            topLevelScope->identifier,
            !Catch::Matchers::IsEmpty());

        REQUIRE_FALSE(iter());
    }

    SECTION("When immutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeLambda", iter);
        checkLambdaCallScope(
            iter,
            function_info_matchers{.specsMatcher = Catch::Matchers::Equals("const")});
        checkTopLevel(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When unary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeUnaryLambda(42))>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeUnaryLambda", iter);
        checkLambdaCallScope(
            iter,
            function_info_matchers{
                .argListMatcher = Catch::Matchers::Equals("int"),
                .specsMatcher = Catch::Matchers::Equals("const")});
        checkTopLevel(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When binary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeBinaryLambda(42, 4.2f))>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeBinaryLambda", iter);
        checkLambdaCallScope(
            iter,
            function_info_matchers{
                .argListMatcher = Catch::Matchers::Matches(R"(int,\s?float)"),
                .specsMatcher = Catch::Matchers::Equals("const")});
        checkTopLevel(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When mutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeMutableLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeMutableLambda", iter);
        checkLambdaCallScope(iter, function_info_matchers{});
        checkTopLevel(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When noexcept lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeNoexceptLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeNoexceptLambda", iter);
        checkLambdaCallScope(
            iter,
            function_info_matchers{.specsMatcher = Catch::Matchers::Equals("const")});
        checkTopLevel(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When nested lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeNestedLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeNestedLambda", iter);
        checkLambdaCallScope(
            iter,
            function_info_matchers{.specsMatcher = Catch::Matchers::Equals("const")});
        checkLambdaCallScope(
            iter,
            function_info_matchers{.specsMatcher = Catch::Matchers::Equals("const")});
        checkTopLevel(iter);
        REQUIRE_FALSE(iter());
    }

    SECTION("When complex nested lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeComplexNestedLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkLambdaName("my_typeComplexNestedLambda", iter);
        checkLambdaCallScope(
            iter,
            function_info_matchers{.specsMatcher = Catch::Matchers::Equals("const")});
        checkLambdaCallScope(
            iter,
            function_info_matchers{
                .argListMatcher = Catch::Matchers::Matches(R"(int,\s?float)"),
                .specsMatcher = Catch::Matchers::Equals("const")});
        checkTopLevel(iter);
        REQUIRE_FALSE(iter());
    }
}

namespace
{
    [[maybe_unused]] auto anon_classFreeFunction()
    {
        class
        {
        } constexpr obj [[maybe_unused]]{};

        return obj;
    }

    [[maybe_unused]] auto anon_structFreeFunction()
    {
        struct
        {
        } constexpr obj [[maybe_unused]]{};

        return obj;
    }

    [[maybe_unused]] auto anon_enumFreeFunction()
    {
        enum
        {
        } constexpr obj [[maybe_unused]]{};

        return obj;
    }

    [[maybe_unused]] auto my_typeFreeFunction()
    {
        struct my_type
        {
        };

        return my_type{};
    }

    [[maybe_unused]] auto my_typeNoexceptFreeFunction() noexcept
    {
        struct my_type
        {
        };

        return my_type{};
    }

    template <typename T>
    [[maybe_unused]] auto* my_typeTemplateFreeFunction()
    {
        struct my_type
        {
        } static constexpr obj [[maybe_unused]]{};

        return &obj;
    }
}

TEST_CASE(
    "printing::type::detail::ScopeIterator supports all kinds of free-functions.",
    "[print][detail]")
{
    SECTION("When anon function-local type is given.")
    {
        auto const [expectedFunctionName, rawName] = GENERATE(
            (table<StringT, StringT>)({
                { "anon_classFreeFunction",  name_of<decltype(anon_classFreeFunction())>()},
                {"anon_structFreeFunction", name_of<decltype(anon_structFreeFunction())>()},
                {  "anon_enumFreeFunction",   name_of<decltype(anon_enumFreeFunction())>()}
        }));
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkFunctionScope(
            iter,
            Catch::Matchers::Equals(expectedFunctionName),
            function_info_matchers{});
        checkNamedScope(
            iter,
            !Catch::Matchers::IsEmpty());
        REQUIRE_FALSE(iter());
    }

    SECTION("When named function-local type is given.")
    {
        auto const [expectedFunctionName, rawName] = GENERATE(
            (table<StringT, StringT>)({
                {        "my_typeFreeFunction",         name_of<decltype(my_typeFreeFunction())>()},
                {"my_typeNoexceptFreeFunction", name_of<decltype(my_typeNoexceptFreeFunction())>()}
        }));
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkFunctionScope(
            iter,
            Catch::Matchers::Equals(expectedFunctionName),
            function_info_matchers{});
        checkNamedScope(
            iter,
            Catch::Matchers::Equals("my_type"));
        REQUIRE_FALSE(iter());
    }

    SECTION("When named template-function-local type is given.")
    {
        StringT const rawName = name_of<decltype(my_typeTemplateFreeFunction<int>())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator iter{rawName};

        REQUIRE(iter());
        checkTemplateFunctionScope(
            iter,
            Catch::Matchers::Equals("my_typeTemplateFreeFunction"),
            template_info_matchers{.argListMatcher = Catch::Matchers::Equals("int")},
            function_info_matchers{});
        checkNamedScope(
            iter,
            Catch::Matchers::Matches(R"(my_type const\s*\*)"));
        REQUIRE_FALSE(iter());
    }
}

TEST_CASE(
    "printing::type::detail::ScopeIterator supports all kinds of function-pointer.",
    "[print][detail]")
{
    SECTION("When a general function-pointer is given.")
    {
        StringT const rawName = name_of<decltype(&my_typeFreeFunction)>();
        CAPTURE(rawName);

        auto const scope = printing::type::detail::gather_scope_info(rawName);
        REQUIRE_FALSE(scope.templateInfo);
        REQUIRE(scope.functionInfo);
        CHECK_THAT(
            scope.functionInfo->argList,
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            scope.functionInfo->specs,
            Catch::Matchers::IsEmpty());

        SECTION("When visiting identifier.")
        {
            printing::type::detail::ScopeIterator iter{scope.identifier};

            checkNamedScope(
                iter,
                Catch::Matchers::Equals("(*)"));
            REQUIRE_FALSE(iter());
        }

        SECTION("When visiting return-type.")
        {
            printing::type::detail::ScopeIterator iter{scope.functionInfo->returnType};

            REQUIRE(iter());
            checkFunctionScope(
                iter,
                Catch::Matchers::Equals("my_typeFreeFunction"),
                function_info_matchers{});
            checkNamedScope(
                iter,
                Catch::Matchers::Equals("my_type"));
            REQUIRE_FALSE(iter());
        }
    }

    SECTION("When a template function-pointer is given.")
    {
        StringT const rawName = name_of<decltype(&my_typeTemplateFreeFunction<int>)>();
        CAPTURE(rawName);

        auto const scope = printing::type::detail::gather_scope_info(rawName);
        REQUIRE_FALSE(scope.templateInfo);

        REQUIRE(scope.functionInfo);
        CHECK_THAT(
            scope.functionInfo->argList,
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            scope.functionInfo->specs,
            Catch::Matchers::IsEmpty());

        SECTION("When visiting identifier.")
        {
            printing::type::detail::ScopeIterator iter{scope.identifier};

            checkNamedScope(
                iter,
                Catch::Matchers::Equals("(*)"));
            REQUIRE_FALSE(iter());
        }

        SECTION("When visiting return-type.")
        {
            printing::type::detail::ScopeIterator iter{scope.functionInfo->returnType};

            REQUIRE(iter());
            checkTemplateFunctionScope(
                iter,
                Catch::Matchers::Equals("my_typeTemplateFreeFunction"),
                template_info_matchers{.argListMatcher = Catch::Matchers::Equals("int")},
                function_info_matchers{});
            checkNamedScope(
                iter,
                Catch::Matchers::Matches(R"(my_type const\s*\*)"));
            REQUIRE_FALSE(iter());
        }
    }

    SECTION("When a template-function-pointer in template-function-pointer is given.")
    {
        using type_t = decltype(&my_typeTemplateFreeFunction<int>);
        StringT const rawName = name_of<decltype(&my_typeTemplateFreeFunction<type_t>)>();
        CAPTURE(rawName);

        auto const scope = printing::type::detail::gather_scope_info(rawName);
        REQUIRE_FALSE(scope.templateInfo);

        REQUIRE(scope.functionInfo);
        CHECK_THAT(
            scope.functionInfo->argList,
            Catch::Matchers::IsEmpty());
        CHECK_THAT(
            scope.functionInfo->specs,
            Catch::Matchers::IsEmpty());

        SECTION("When visiting identifier.")
        {
            printing::type::detail::ScopeIterator iter{scope.identifier};

            checkNamedScope(
                iter,
                Catch::Matchers::Equals("(*)"));
            REQUIRE_FALSE(iter());
        }

        SECTION("When visiting return-type.")
        {
            printing::type::detail::ScopeIterator iter{scope.functionInfo->returnType};

            REQUIRE(iter());

            auto const fnScope = iter();
            REQUIRE(fnScope);
            CHECK_THAT(
                StringT{fnScope->identifier},
                Catch::Matchers::Equals("my_typeTemplateFreeFunction"));

            REQUIRE(fnScope->functionInfo);
            CHECK_THAT(
                fnScope->functionInfo->returnType,
                Catch::Matchers::IsEmpty());
            CHECK_THAT(
                fnScope->functionInfo->argList,
                Catch::Matchers::IsEmpty());
            CHECK_THAT(
                fnScope->functionInfo->specs,
                Catch::Matchers::IsEmpty());

            REQUIRE(fnScope->templateInfo);

            SECTION("When visiting the template argument.")
            {
                CAPTURE(fnScope->templateInfo->argList);
                auto const argScope = printing::type::detail::gather_scope_info(fnScope->templateInfo->argList);
                REQUIRE_FALSE(argScope.templateInfo);
                REQUIRE(argScope.functionInfo);
                CHECK_THAT(
                    argScope.functionInfo->argList,
                    Catch::Matchers::IsEmpty());
                CHECK_THAT(
                    argScope.functionInfo->specs,
                    Catch::Matchers::IsEmpty());
                CHECK_THAT(
                    StringT{argScope.functionInfo->returnType},
                    Catch::Matchers::EndsWith("::my_type const *")
                        || Catch::Matchers::EndsWith("::my_type const*"));
                CHECK_THAT(
                    StringT{argScope.identifier},
                    Catch::Matchers::Equals("(*)"));
            }

            checkNamedScope(
                iter,
                Catch::Matchers::Matches(R"(my_type const\s*\*)"));
            REQUIRE_FALSE(iter());
        }
    }
}

#endif
