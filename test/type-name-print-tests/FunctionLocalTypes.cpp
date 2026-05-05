//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"

#include "Common.hpp"

using namespace mimicpp;

namespace
{
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

    constexpr auto my_type2Lambda = [](int, std::string const&) {
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
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances lambda-local type-names.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When a lambda without arguments is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(my_typeLambda())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + testing::lambda_pattern() + "::");
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    // This case is broken, because the `mutable` keyword cannot be handled with the current implementation.
    // We'll likely have to create a custom parsing rule for cases like `<lambda()> mutable`.
    /*SECTION("When a mutable lambda without arguments is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(my_typeMutableLambda())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + testing::lambda_pattern() + " mutable::");
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }*/

    SECTION("When a noexcept lambda without arguments is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(my_typeNoexceptLambda())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + testing::lambda_pattern() + "::");
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a lambda with arguments is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(my_type2Lambda(42, ""))>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + testing::lambda_pattern(R"(\.{3})") + "::");
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a nested lambda is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(my_typeNestedLambda())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(
            testing::anonNsScopePattern
            + testing::lambda_pattern() + "::"
            + testing::lambda_pattern() + "::");
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a more complex nested lambda is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(my_typeNestedLambda2())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(
            testing::anonNsScopePattern
            + testing::lambda_pattern() + "::"
            + testing::lambda_pattern() + "::");
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }
}

namespace
{
    struct fun_outer
    {
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

    [[maybe_unused]] auto my_typeFreeFunction()
    {
        struct my_type
        {
        };

        return my_type{};
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances function-local type-names.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When a free-function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(my_typeFreeFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "my_typeFreeFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a public function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(fun_outer{}.my_typeFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a noexcept function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(fun_outer{}.my_typeNoexceptFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeNoexceptFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a const function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(fun_outer{}.my_typeConstFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeConstFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a lvalue function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(std::declval<fun_outer&>().my_typeLvalueFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeLvalueFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a const-lvalue function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(std::declval<fun_outer const&>().my_typeConstLvalueFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeConstLvalueFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a rvalue function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(std::declval<fun_outer&&>().my_typeRvalueFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeRvalueFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a const-rvalue function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(std::declval<fun_outer const&&>().my_typeConstRvalueFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeConstRvalueFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a private function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(fun_outer{}.my_typeIndirectlyPrivateFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typePrivateFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a operator is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(fun_outer{}.operator+(42))>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + R"(fun_outer::operator\s?\+::)");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }

    SECTION("When a static function is given.")
    {
        using T = testing::mod_type_t<TestType, decltype(fun_outer::my_typeStaticFunction())>;
        std::string const rawName{printing::type::type_name<T>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeStaticFunction::");

        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(scopePattern + "my_type" + suffixPattern));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances function-local return types of function-types.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    std::string const functionArgsPattern{R"(\(\))"};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When a lambda-local type is given.")
    {
        using Return = decltype(my_typeLambda());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<T()>()}; // This is a function-type which returns T!
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + testing::lambda_pattern() + "::");
        auto const returnTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(returnTypePattern + functionArgsPattern));
    }

    SECTION("When a free-function local type is given.")
    {
        using Return = decltype(my_typeFreeFunction());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<T()>()}; // This is a function-type which returns T!
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "my_typeFreeFunction::");
        auto const returnTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(returnTypePattern + functionArgsPattern));
    }

    SECTION("When a member-function local type is given.")
    {
        using Return = decltype(fun_outer{}.my_typeFunction());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<T()>()}; // This is a function-type which returns T!
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeFunction::");
        auto const returnTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(returnTypePattern + functionArgsPattern));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances function-local return types of function-ptr-types.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    std::string const funArgsPattern{R"(\(\))"};
    std::string const funPtrPattern{R"(\(\*\))"};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When a lambda-local type is given.")
    {
        using Return = decltype(my_typeLambda());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<T (*)()>()}; // This is a function-ptr-type which returns T!
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + testing::lambda_pattern() + "::");
        auto const returnTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(returnTypePattern + "\\s?" + funPtrPattern + funArgsPattern));
    }

    SECTION("When a free-function local type is given.")
    {
        using Return = decltype(my_typeFreeFunction());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<T (*)()>()}; // This is a function-ptr-type which returns T!
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "my_typeFreeFunction::");
        auto const returnTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(returnTypePattern + "\\s?" + funPtrPattern + funArgsPattern));
    }

    SECTION("When a member-function local type is given.")
    {
        using Return = decltype(fun_outer{}.my_typeFunction());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<T (*)()>()}; // This is a function-ptr-type which returns T!
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeFunction::");
        auto const returnTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(returnTypePattern + "\\s?" + funPtrPattern + funArgsPattern));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances function-local parameter types of function-types.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When a lambda-local type is given.")
    {
        using Return = decltype(my_typeLambda());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<void(T)>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + testing::lambda_pattern() + "::");
        auto const paramTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches("void\\(" + paramTypePattern + "\\)"));
    }

    SECTION("When a free-function local type is given.")
    {
        using Return = decltype(my_typeFreeFunction());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<void(T)>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "my_typeFreeFunction::");
        auto const paramTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches("void\\(" + paramTypePattern + "\\)"));
    }

    SECTION("When a member-function local type is given.")
    {
        using Return = decltype(fun_outer{}.my_typeFunction());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<void(T)>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeFunction::");
        auto const paramTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches("void\\(" + paramTypePattern + "\\)"));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "printing::type::prettify_type enhances function-local parameter types of function-ptr-types.",
    "[print][print::type]",
    testing::common_mod_list)
{
    std::string const suffixPattern{TestType::suffix};
    std::string const prefixPattern{R"(void\s?\(\*\)\()"};
    CAPTURE(suffixPattern);

    std::ostringstream ss{};

    SECTION("When a lambda-local type is given.")
    {
        using Return = decltype(my_typeLambda());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<void (*)(T)>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + testing::lambda_pattern() + "::");
        auto const paramTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(prefixPattern + paramTypePattern + "\\)"));
    }

    SECTION("When a free-function local type is given.")
    {
        using Return = decltype(my_typeFreeFunction());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<void (*)(T)>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "my_typeFreeFunction::");
        auto const paramTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(prefixPattern + paramTypePattern + "\\)"));
    }

    SECTION("When a member-function local type is given.")
    {
        using Return = decltype(fun_outer{}.my_typeFunction());
        using T = testing::mod_type_t<TestType, Return>;
        std::string const rawName{printing::type::type_name<void (*)(T)>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);

        auto const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "fun_outer::my_typeFunction::");
        auto const paramTypePattern = scopePattern + "my_type" + suffixPattern;
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(prefixPattern + paramTypePattern + "\\)"));
    }
}

namespace
{
    struct dtor_outer
    {
        std::string& out;

        ~dtor_outer()
        {
            struct my_type
            {
            };

            out = printing::type::type_name<my_type>();
        }
    };
}

TEST_CASE(
    "printing::type::prettify_type supports destructors.",
    "[print][print::type]")
{
    std::string rawName{};
    std::ignore = dtor_outer{rawName};
    CAPTURE(rawName);

    std::ostringstream ss{};
    printing::type::prettify_type(
        std::ostreambuf_iterator{ss},
        rawName);

    std::string const scopePattern = testing::maybe_pattern(testing::anonNsScopePattern + "dtor_outer::~dtor_outer::");
    CHECK_THAT(
        ss.str(),
        Catch::Matchers::Matches(scopePattern + "my_type"));
}

TEST_CASE(
    "printing::type::prettify_type enhances type-names from the local scope.",
    "[print][print::type]")
{
    static std::string const nullaryLambdaScopePattern = R"(<lambda\(\)>::)";
    static std::string const lambdaScopePattern = R"(<lambda\(\.{3}\)>::)";
    static std::string const testCaseScopePattern = R"(CATCH2_INTERNAL_TEST_\d+::)";
    static std::string const callOpScopePattern = R"(operator\(\)::)";

    std::ostringstream ss{};

    SECTION("When local type is queried inside the current scope.")
    {
        struct my_type
        {
        };

        std::string const rawName{printing::type::type_name<my_type>()};
        CAPTURE(rawName);

        printing::type::prettify_type(
            std::ostreambuf_iterator{ss},
            rawName);
        CHECK_THAT(
            std::move(ss).str(),
            Catch::Matchers::Matches(testing::maybe_pattern(testCaseScopePattern) + "my_type"));
    }

    SECTION("When local type is queried inside a lambda.")
    {
        std::invoke(
            [&] {
                struct my_type
                {
                };

                std::string const rawName{printing::type::type_name<my_type>()};
                CAPTURE(rawName);

                printing::type::prettify_type(
                    std::ostreambuf_iterator{ss},
                    rawName);

                auto const scopePattern = testing::maybe_pattern(testCaseScopePattern + nullaryLambdaScopePattern);
                CHECK_THAT(
                    std::move(ss).str(),
                    Catch::Matchers::Matches(scopePattern + "my_type"));
            });
    }

    SECTION("When local type is queried inside a member-function.")
    {
        struct outer
        {
            void operator()(std::ostringstream& _ss) const
            {
                struct my_type
                {
                };

                std::string const rawName{printing::type::type_name<my_type>()};
                CAPTURE(rawName);

                printing::type::prettify_type(
                    std::ostreambuf_iterator{_ss},
                    rawName);

                auto const scopePattern = testing::maybe_pattern(testCaseScopePattern + "outer::" + callOpScopePattern);
                CHECK_THAT(
                    std::move(_ss).str(),
                    Catch::Matchers::Matches(scopePattern + "my_type"));
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
                std::ostringstream* _ss,
                [[maybe_unused]] int&& ref,
                [[maybe_unused]] int (&arrRef)[1],
                [[maybe_unused]] int*& ptrRef) {
                struct my_type
                {
                };

                std::string const rawName{printing::type::type_name<my_type>()};
                CAPTURE(rawName);

                printing::type::prettify_type(
                    std::ostreambuf_iterator{*_ss},
                    rawName);

                auto const scopePattern = testing::maybe_pattern(testCaseScopePattern + lambdaScopePattern);
                CHECK_THAT(
                    std::move(*_ss).str(),
                    Catch::Matchers::Matches(scopePattern + "my_type"));
            },
            &ss,
            std::move(d1),
            d2,
            ptr);
    }

    SECTION("When local type is queried inside a nested-lambda with higher arity.")
    {
        std::invoke(
            [](std::ostringstream* _ss) {
                struct other_type
                {
                };

                std::invoke(
                    [&]([[maybe_unused]] other_type const& dummy) {
                        struct my_type
                        {
                        };

                        std::string const rawName{printing::type::type_name<my_type>()};
                        CAPTURE(rawName);

                        printing::type::prettify_type(
                            std::ostreambuf_iterator{*_ss},
                            rawName);
                        auto const scopePattern = testing::maybe_pattern(
                            testCaseScopePattern
                            + lambdaScopePattern
                            + lambdaScopePattern);
                        CHECK_THAT(
                            std::move(*_ss).str(),
                            Catch::Matchers::Matches(scopePattern + "my_type"));
                    },
                    other_type{});
            },
            &ss);
    }
}
