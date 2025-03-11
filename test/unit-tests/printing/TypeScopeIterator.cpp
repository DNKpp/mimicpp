//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include <source_location>

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

    void checkNamedScope(auto const& nameMatcher, auto& visitor)
    {
        auto const scope = visitor();
        REQUIRE(scope);
        REQUIRE_FALSE(scope->templateInfo);
        REQUIRE_FALSE(scope->functionInfo);

        CHECK_THAT(
            StringT{scope->identifier},
            nameMatcher);
    }

    void checkFunctionScope(
        StringT const& expectedName,
        printing::type::detail::function_info const& fnInfo,
        auto& visitor)
    {
        auto const fnScope = visitor();
        REQUIRE(fnScope);

        CHECK_FALSE(fnScope->templateInfo);
        CHECK_THAT(
            StringT{fnScope->identifier},
            Catch::Matchers::Equals(expectedName));

        REQUIRE(fnScope->functionInfo);
        CHECK_THAT(
            StringT{fnScope->functionInfo->returnType},
            Catch::Matchers::Equals(StringT{fnInfo.returnType}));
        CHECK_THAT(
            StringT{fnScope->functionInfo->argList},
            Catch::Matchers::Equals(StringT{fnInfo.argList}));
        CHECK_THAT(
            StringT{fnScope->functionInfo->specs},
            Catch::Matchers::Equals(StringT{fnInfo.specs}));
    }

    void checkTemplateFunctionScope(
        StringT const& expectedName,
        printing::type::detail::template_info const& templateInfo,
        printing::type::detail::function_info const& fnInfo,
        auto& visitor)
    {
        auto const fnScope = visitor();
        REQUIRE(fnScope);

        CHECK_THAT(
            StringT{fnScope->identifier},
            Catch::Matchers::Equals(expectedName));

        REQUIRE(fnScope->functionInfo);
        CHECK_THAT(
            StringT{fnScope->functionInfo->returnType},
            Catch::Matchers::Equals(StringT{fnInfo.returnType}));
        CHECK_THAT(
            StringT{fnScope->functionInfo->argList},
            Catch::Matchers::Equals(StringT{fnInfo.argList}));
        CHECK_THAT(
            StringT{fnScope->functionInfo->specs},
            Catch::Matchers::Equals(StringT{fnInfo.specs}));

        REQUIRE(fnScope->templateInfo);
        CHECK_THAT(
            StringT{fnScope->templateInfo->argList},
            Catch::Matchers::Equals(StringT{templateInfo.argList}));
    }

    void checkLambdaScope(
        [[maybe_unused]] printing::type::detail::function_info const& fnInfo,
        auto& visitor)
    {
        auto const info = visitor();

        REQUIRE(info);
        CHECK_FALSE(info->templateInfo);

#if MIMICPP_DETAIL_IS_CLANG

        // clang often uses the `anon-type` form for lambdas
        CHECKED_IF(info->identifier.starts_with('$'))
        {
            CHECK_FALSE(info->functionInfo);
            CHECK_THAT(
                StringT{info->identifier},
                Catch::Matchers::Matches(R"(\$_\d+)"));
        }

        // but sometimes provides more info
        CHECKED_IF(info->identifier.starts_with("lambda"))
        {
            CHECK(info->functionInfo);
    #if MIMICPP_DETAIL_USES_LIBCXX
            CHECK_THAT(
                StringT{info->identifier},
                Catch::Matchers::Equals("lambda"));
    #else
            CHECK_THAT(
                StringT{info->identifier},
                Catch::Matchers::Matches(R"(lambda#\d+)"));
    #endif
            CHECK(info->functionInfo);
            CHECK_THAT(
                StringT{info->functionInfo->returnType},
                Catch::Matchers::Equals(StringT{fnInfo.returnType}));
            CHECK_THAT(
                StringT{info->functionInfo->argList},
                Catch::Matchers::Equals(StringT{fnInfo.argList}));
            CHECK_THAT(
                StringT{info->functionInfo->specs},
                Catch::Matchers::Equals(StringT{fnInfo.specs}));
        }
#elif MIMICPP_DETAIL_IS_GCC
        CHECK_THAT(
            StringT{info->identifier},
            Catch::Matchers::Matches(R"(lambda#\d+)"));
        CHECK(info->functionInfo);
        CHECK_THAT(
            StringT{info->functionInfo->returnType},
            Catch::Matchers::Equals(StringT{fnInfo.returnType}));
        CHECK_THAT(
            StringT{info->functionInfo->argList},
            Catch::Matchers::Equals(StringT{fnInfo.argList}));
        CHECK_THAT(
            StringT{info->functionInfo->specs},
            Catch::Matchers::Equals(StringT{fnInfo.specs}));
#endif
    }

    void checkLambdaName(
        [[maybe_unused]] StringT const& expectedName,
        [[maybe_unused]] auto& visitor)
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

    void checkLambdaCallScope(printing::type::detail::function_info const& fnInfo, auto& visitor)
    {
        checkLambdaScope(
            {.returnType = {},
             .argList = fnInfo.argList,
             .specs = {}},
            visitor);
        checkFunctionScope("operator()", fnInfo, visitor);
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

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeLambda", visitor);
        checkLambdaScope({}, visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When unary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeUnaryLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeUnaryLambda", visitor);
        checkLambdaScope({.argList = "int"}, visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When binary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeBinaryLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeBinaryLambda", visitor);
        checkLambdaScope({.argList = "int, float"}, visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When mutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeMutableLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeMutableLambda", visitor);
        checkLambdaScope({}, visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When noexcept lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeNoexceptLambda)>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeNoexceptLambda", visitor);
        checkLambdaScope({}, visitor);
        REQUIRE_FALSE(visitor());
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

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName(expectedLambdaName, visitor);
        checkLambdaCallScope({.specs = "const"}, visitor);

        auto const topLevelScope = visitor();
        CHECK(topLevelScope);
        CHECK_FALSE(topLevelScope->functionInfo);
        CHECK_FALSE(topLevelScope->templateInfo);
        CHECK_THAT(
            topLevelScope->identifier,
            !Catch::Matchers::IsEmpty());

        REQUIRE_FALSE(visitor());
    }

    SECTION("When immutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeLambda", visitor);
        checkLambdaCallScope({.specs = "const"}, visitor);
        checkTopLevel(visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When unary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeUnaryLambda(42))>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeUnaryLambda", visitor);
        checkLambdaCallScope({.argList = "int", .specs = "const"}, visitor);
        checkTopLevel(visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When binary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeBinaryLambda(42, 4.2f))>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeBinaryLambda", visitor);
        checkLambdaCallScope({.argList = "int, float", .specs = "const"}, visitor);
        checkTopLevel(visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When mutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeMutableLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeMutableLambda", visitor);
        checkLambdaCallScope({}, visitor);
        checkTopLevel(visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When noexcept lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeNoexceptLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeNoexceptLambda", visitor);
        checkLambdaCallScope({.specs = "const"}, visitor);
        checkTopLevel(visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When nested lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeNestedLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeNestedLambda", visitor);
        checkLambdaCallScope({.specs = "const"}, visitor);
        checkLambdaCallScope({.specs = "const"}, visitor);
        checkTopLevel(visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When complex nested lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeComplexNestedLambda())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkLambdaName("my_typeComplexNestedLambda", visitor);
        checkLambdaCallScope({.specs = "const"}, visitor);
        checkLambdaCallScope({.argList = "int, float", .specs = "const"}, visitor);
        checkTopLevel(visitor);
        REQUIRE_FALSE(visitor());
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

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkFunctionScope(expectedFunctionName, {}, visitor);
        checkNamedScope(
            !Catch::Matchers::IsEmpty(),
            visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When named function-local type is given.")
    {
        auto const [expectedFunctionName, rawName] = GENERATE(
            (table<StringT, StringT>)({
                {        "my_typeFreeFunction",         name_of<decltype(my_typeFreeFunction())>()},
                {"my_typeNoexceptFreeFunction", name_of<decltype(my_typeNoexceptFreeFunction())>()}
        }));
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkFunctionScope(expectedFunctionName, {}, visitor);
        checkNamedScope(
            Catch::Matchers::Equals("my_type"),
            visitor);
        REQUIRE_FALSE(visitor());
    }

    SECTION("When named template-function-local type is given.")
    {
        StringT const rawName = name_of<decltype(my_typeTemplateFreeFunction<int>())>();
        CAPTURE(rawName);

        printing::type::detail::ScopeIterator visitor{rawName};

        REQUIRE(visitor());
        checkTemplateFunctionScope(
            "my_typeTemplateFreeFunction",
            {.argList = "int"},
            {},
            visitor);
        checkNamedScope(
            Catch::Matchers::Equals("my_type const*"),
            visitor);
        REQUIRE_FALSE(visitor());
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
            printing::type::detail::ScopeIterator visitor{scope.identifier};

            checkNamedScope(
                Catch::Matchers::Equals("(*)"),
                visitor);
            REQUIRE_FALSE(visitor());
        }

        SECTION("When visiting return-type.")
        {
            printing::type::detail::ScopeIterator visitor{scope.functionInfo->returnType};

            REQUIRE(visitor());
            checkFunctionScope("my_typeFreeFunction", {}, visitor);
            checkNamedScope(
                Catch::Matchers::Equals("my_type"),
                visitor);
            REQUIRE_FALSE(visitor());
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
            printing::type::detail::ScopeIterator visitor{scope.identifier};

            checkNamedScope(
                Catch::Matchers::Equals("(*)"),
                visitor);
            REQUIRE_FALSE(visitor());
        }

        SECTION("When visiting return-type.")
        {
            printing::type::detail::ScopeIterator visitor{scope.functionInfo->returnType};

            REQUIRE(visitor());

            auto const fnScope = visitor();
            REQUIRE(fnScope);
            checkTemplateFunctionScope(
                "my_typeTemplateFreeFunction",
                {.argList = "int"},
                {},
                visitor);
            checkNamedScope(
                Catch::Matchers::Equals("my_type const*"),
                visitor);
            REQUIRE_FALSE(visitor());
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
            printing::type::detail::ScopeIterator visitor{scope.identifier};

            checkNamedScope(
                Catch::Matchers::Equals("(*)"),
                visitor);
            REQUIRE_FALSE(visitor());
        }

        SECTION("When visiting return-type.")
        {
            printing::type::detail::ScopeIterator visitor{scope.functionInfo->returnType};

            REQUIRE(visitor());

            auto const fnScope = visitor();
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
            CHECK_THAT(
                StringT{fnScope->templateInfo->argList},
                Catch::Matchers::EndsWith("my_typeTemplateFreeFunction<int>()::my_type const* (*)()"));

            checkNamedScope(
                Catch::Matchers::Equals("my_type const*"),
                visitor);

            REQUIRE_FALSE(visitor());
        }
    }
}
