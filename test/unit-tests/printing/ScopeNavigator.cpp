//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "../../../include/mimic++/printing/type/ScopeNavigator.hpp"

#include "../../../include/mimic++/Mock.hpp"
#include "../../../include/mimic++/matchers/RangeMatchers.hpp"
#include "../../../include/mimic++/printing/TypePrinter.hpp"
#include "../../../include/mimic++/printing/type/PostProcessing.hpp"

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES

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

template <>
class custom::Printer<printing::type::scope_info>
{
public:
    template <print_iterator OutIter>
    static constexpr OutIter print(OutIter out, printing::type::scope_info const& info)
    {
        out = format::format_to(std::move(out), "\'");
        out = std::ranges::copy(info.identifier, std::move(out)).out;

        if (info.templateArgs)
        {
            out = format::format_to(std::move(out), "<{}>", *info.templateArgs);
        }

        if (info.functionArgs)
        {
            out = format::format_to(std::move(out), "({})", *info.functionArgs);
        }

        out = std::ranges::copy(info.specs, std::move(out)).out;
        out = format::format_to(std::move(out), "\'");

        return out;
    }
};

struct TypeScopeNavigator_cpp_my_type
{
};

namespace
{
    template <typename Matcher>
    class NamedScopeMatcher
    {
    public:
        [[nodiscard]]
        explicit constexpr NamedScopeMatcher(Matcher nameMatcher) noexcept
            : m_NameMatcher{std::move(nameMatcher)}
        {
        }

        [[nodiscard]]
        constexpr StringT describe() const
        {
            return "is named scope: \'" + m_NameMatcher.describe() + "\'";
        }

        [[nodiscard]]
        constexpr bool matches(printing::type::scope_info const& info) const
        {
            return !info.functionArgs.has_value()
                && !info.templateArgs.has_value()
                && info.specs.empty()
                && m_NameMatcher.match(StringT{info.identifier});
        }

    private:
        Matcher m_NameMatcher;
    };

    template <typename IdentifierMatcher, typename ArgListMatcher, typename SpecsMatcher>
    class FunctionScopeMatcher
    {
    public:
        [[nodiscard]]
        explicit constexpr FunctionScopeMatcher(
            IdentifierMatcher identifierMatcher,
            ArgListMatcher argListMatcher,
            SpecsMatcher specsMatcher)
            : m_IdentifierMatcher{std::move(identifierMatcher)},
              m_ArgListMatcher{std::move(argListMatcher)},
              m_SpecsMatcher{std::move(specsMatcher)}
        {
        }

        [[nodiscard]]
        constexpr StringT describe() const
        {
            return "is function scope: \'"
                 + m_IdentifierMatcher.describe()
                 + "\' with arg-list: \'"
                 + m_ArgListMatcher.describe()
                 + "\' with specs: \'"
                 + m_SpecsMatcher.describe();
        }

        [[nodiscard]]
        constexpr bool matches(printing::type::scope_info const& info) const
        {
            return !info.templateArgs.has_value()
                && info.functionArgs.has_value()
                && m_IdentifierMatcher.match(StringT{info.identifier})
                && m_ArgListMatcher.match(StringT{*info.functionArgs})
                && m_SpecsMatcher.match(StringT{info.specs});
        }

    private:
        IdentifierMatcher m_IdentifierMatcher;
        ArgListMatcher m_ArgListMatcher;
        SpecsMatcher m_SpecsMatcher;
    };

    static_assert(
        matcher_for<
            FunctionScopeMatcher<Catch::Matchers::StringEqualsMatcher, Catch::Matchers::IsEmptyMatcher, Catch::Matchers::IsEmptyMatcher>,
            printing::type::scope_info>);

    template <typename IdentifierMatcher, typename ArgListMatcher>
    [[nodiscard]]
    constexpr auto matches_function_scope(
        IdentifierMatcher&& identifierMatcher,
        ArgListMatcher&& argListMatcher)
    {
        return FunctionScopeMatcher{
            std::forward<IdentifierMatcher>(identifierMatcher),
            std::forward<ArgListMatcher>(argListMatcher),
            Catch::Matchers::IsEmptyMatcher{}};
    }

    template <typename IdentifierMatcher, typename ArgListMatcher, typename SpecsMatcher>
    [[nodiscard]]
    constexpr auto matches_function_scope(
        IdentifierMatcher&& identifierMatcher,
        ArgListMatcher&& argListMatcher,
        SpecsMatcher&& specsMatcher)
    {
        return FunctionScopeMatcher{
            std::forward<IdentifierMatcher>(identifierMatcher),
            std::forward<ArgListMatcher>(argListMatcher),
            std::forward<SpecsMatcher>(specsMatcher)};
    }

    [[nodiscard]]
    constexpr auto matches_anon_ns_scope()
    {
        return matches::predicate(
            [](printing::type::scope_info const& info) {
                auto const& aliases = printing::type::detail::alias_map();
                auto const iter = aliases.find(info.identifier);

                return !info.functionArgs.has_value()
                    && !info.templateArgs.has_value()
                    && info.specs.empty()
                    && iter != aliases.cend()
                    && iter->second == printing::type::anonymousNamespaceTargetScopeText;
            },
            "is anon-ns scope.",
            "is not anon-ns scope.");
    }

    static_assert(matcher_for<decltype(matches_anon_ns_scope()), printing::type::scope_info>);

    StringT const callOperatorPattern = R"(operator-invoke)";
    StringT const testCasePattern = R"(CATCH2_INTERNAL_TEST_\d+)";
    StringT const lambdaPattern = R"((lambda|(\$_|lambda#)\d+))";
    // The actual result is too different between all platforms.
    // It's important that something is there; everything else would be too much of a hassle.
    StringT const anonTypePattern = R"(\s*.+?\w+.*?\s*)";

    template <typename ArgListMatcher, typename SpecsMatcher>
    [[nodiscard]]
    constexpr auto matches_call_operator_scope(
        ArgListMatcher&& argListMatcher,
        SpecsMatcher&& specsMatcher)
    {
        return FunctionScopeMatcher{
            Catch::Matchers::Equals(callOperatorPattern),
            std::forward<ArgListMatcher>(argListMatcher),
            std::forward<SpecsMatcher>(specsMatcher)};
    }

    template <typename ArgListMatcher>
    [[nodiscard]]
    constexpr auto matches_call_operator_scope(ArgListMatcher&& argListMatcher)
    {
        return FunctionScopeMatcher{
            Catch::Matchers::Equals(callOperatorPattern),
            std::forward<ArgListMatcher>(argListMatcher),
            Catch::Matchers::IsEmpty()};
    }

    [[nodiscard]]
    auto matches_test_case_scope()
    {
        return FunctionScopeMatcher{
            Catch::Matchers::Matches(testCasePattern),
            Catch::Matchers::IsEmpty(),
            Catch::Matchers::IsEmpty()};
    }

    static_assert(matcher_for<decltype(matches_test_case_scope()), printing::type::scope_info>);

    [[nodiscard]]
    auto matches_lambda_scope()
    {
        return NamedScopeMatcher{
            Catch::Matchers::Matches(lambdaPattern)};
    }

    static_assert(matcher_for<decltype(matches_lambda_scope()), printing::type::scope_info>);

    [[nodiscard]]
    auto matches_anon_type_scope()
    {
        return NamedScopeMatcher{
            Catch::Matchers::Matches(anonTypePattern)};
    }

    static_assert(matcher_for<decltype(matches_anon_type_scope()), printing::type::scope_info>);
}

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
    "printing::type::ScopeNavigator advances the scopes of a given type-identifier.",
    "[print][print::type]")
{
    using printing::type::scope_info;

    Mock<void(scope_info const&) const> visitor{};
    printing::type::ScopeNavigator const navigator{std::ref(visitor)};

    SECTION("When type in global namespace is given.")
    {
        StringT const rawName = name_of<TypeScopeNavigator_cpp_my_type>();
        CAPTURE(rawName);

        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("TypeScopeNavigator_cpp_my_type")});

        navigator(rawName);
    }

    SECTION("When type in anonymous-namespace is given.")
    {
        StringT const rawName = name_of<my_type>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When local-type is given.")
    {
        struct my_type
        {
        };

        StringT const rawName = name_of<my_type>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_test_case_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When type in arbitrarily nested namespace is given.")
    {
        StringT const rawName = name_of<test_ns::my_type>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("test_ns")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When anon-type is given.")
    {
        StringT const rawName = GENERATE(
            name_of<decltype(anonClass)>(),
            name_of<decltype(anonStruct)>(),
            name_of<decltype(anonEnum)>());
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(matches_anon_type_scope())
            and expect::in_sequence(sequence);

        navigator(rawName);
    }
}

TEST_CASE(
    "printing::type::ScopeNavigator does not treat placeholders as templates or functions.",
    "[print][print::type]")
{
    using printing::type::scope_info;

    Mock<void(scope_info const&) const> visitor{};
    printing::type::ScopeNavigator const navigator{std::ref(visitor)};

    StringT const placeholderName = GENERATE(
        "<placeholder>",
        "(placeholder)");

    SECTION("When a standalone placeholder is given.")
    {
        CAPTURE(placeholderName);

        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals(placeholderName)});

        navigator(placeholderName);
    }

    SECTION("When top-level placeholder is given.")
    {
        StringT const name = "my_ns::" + placeholderName;
        CAPTURE(name);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_ns")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals(placeholderName)})
            and expect::in_sequence(sequence);

        navigator(name);
    }

    SECTION("When an identifier contains a placeholder.")
    {
        StringT const name = "my_ns::" + placeholderName + "::my_type";
        CAPTURE(name);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_ns")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals(placeholderName)})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(name);
    }
}

TEST_CASE(
    "printing::type::detail::ScopeNavigator supports operator().",
    "[print][print::type]")
{
    using printing::type::scope_info;

    Mock<void(scope_info const&) const> visitor{};
    printing::type::ScopeNavigator const navigator{std::ref(visitor)};

    SECTION("When given standalone.")
    {
        StringT const name = printing::type::detail::apply_basic_transformations(
            "operator()() const&");
        CAPTURE(name);

        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const&")));

        navigator(name);
    }

    SECTION("When given as top-level.")
    {
        StringT const name = printing::type::detail::apply_basic_transformations(
            "my_ns::operator()() const&");
        CAPTURE(name);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_ns")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const&")))
            and expect::in_sequence(sequence);

        navigator(name);
    }

    SECTION("When an identifier contains a operator().")
    {
        StringT const name = printing::type::detail::apply_basic_transformations(
            "my_ns::operator()() const&::my_between::operator()() const&::my_type");
        CAPTURE(name);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_ns")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const&")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_between")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const&")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(name);
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
    };
}

TEST_CASE(
    "printing::type::ScopeNavigator unwraps scopes in ` and '.",
    "[print][print::type]")
{
    using printing::type::scope_info;

    Mock<void(scope_info const&) const> visitor{};
    printing::type::ScopeNavigator const navigator{std::ref(visitor)};

    SECTION("When a simple identifier is wrapped.")
    {
        StringT const name = "`foo'";
        CAPTURE(name);

        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("foo")});

        navigator(name);
    }

    SECTION("When a function is wrapped.")
    {
        auto const[argListPatter, specsPattern, name] = GENERATE(
            (table<StringT, StringT, StringT>)({
                {"", "", "`foo()'"},
                {"int", "", "`foo(int)'"},
                {"int, float", "", "`foo(int, float)'"},
                {"", "const", "`foo() const'"},
                {"", "const&&", "`foo() const&&'"}
            }));
        CAPTURE(argListPatter, specsPattern, name);

        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
                Catch::Matchers::Equals("foo"),
                Catch::Matchers::Matches(argListPatter),
                Catch::Matchers::Matches(specsPattern)));

        navigator(name);
    }

    /*SECTION("When a function with return type is wrapped.")
    {
        StringT const name = "`ret foo(int) const&'";
        CAPTURE(name);

        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
                Catch::Matchers::Equals("foo"),
                Catch::Matchers::Equals("int"),
                Catch::Matchers::Equals("const&")));

        navigator(name);
    }*/

    SECTION("When a nested function is wrapped.")
    {
        StringT const name = "``bar(float) const&'::foo(int) const&&'";
        CAPTURE(name);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
            Catch::Matchers::Equals("bar"),
            Catch::Matchers::Equals("float"),
            Catch::Matchers::Equals("const&")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
            Catch::Matchers::Equals("foo"),
            Catch::Matchers::Equals("int"),
            Catch::Matchers::Equals("const&&")))
            and expect::in_sequence(sequence);

        navigator(name);
    }
}

TEST_CASE(
    "printing::type::ScopeNavigator supports operator<, <=, >, >= and <=>.",
    "[print][print::type]")
{
    using printing::type::scope_info;

    Mock<void(scope_info const&) const> visitor{};
    printing::type::ScopeNavigator const navigator{std::ref(visitor)};

    SECTION("When ordering operator is used.")
    {
        auto const [expectedFunctionName, rawName] = GENERATE(
            (table<StringT, StringT>)({
                {R"(operator-lt)",  name_of<decltype(special_operators{}.operator<(42))>()},
                {R"(operator-le)", name_of<decltype(special_operators{}.operator<=(42))>()},
                {R"(operator-gt)",  name_of<decltype(special_operators{}.operator>(42))>()},
                {R"(operator-ge)", name_of<decltype(special_operators{}.operator>=(42))>()}
        }));
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("special_operators")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
                Catch::Matchers::Equals(expectedFunctionName),
                Catch::Matchers::Equals("int"),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When nested ordering operator is used.")
    {
        auto const [expectedFunctionName, expectedNestedFunctionName, rawName] = GENERATE(
            (table<StringT, StringT, StringT>)({
                {R"(operator-lt)", R"(operator-ge)",  name_of<decltype(special_operators{}.operator<(""))>()},
                {R"(operator-le)", R"(operator-gt)", name_of<decltype(special_operators{}.operator<=(""))>()},
                {R"(operator-gt)", R"(operator-le)",  name_of<decltype(special_operators{}.operator>(""))>()},
                {R"(operator-ge)", R"(operator-lt)", name_of<decltype(special_operators{}.operator>=(""))>()}
        }));
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("special_operators")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
                Catch::Matchers::Equals(expectedFunctionName),
                Catch::Matchers::ContainsSubstring("::basic_string<"),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
                Catch::Matchers::Equals(expectedNestedFunctionName),
                Catch::Matchers::Equals("int"),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When spaceship-operator is used.")
    {
        StringT const rawName = name_of<decltype(special_operators{}.operator<=>(42))>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("special_operators")})
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
                Catch::Matchers::Equals("operator-spaceship"),
                Catch::Matchers::Equals("int"),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
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
}

TEST_CASE(
    "printing::type::ScopeNavigator supports all kinds of lambdas.",
    "[print][print::type]")
{
    using printing::type::scope_info;

    Mock<void(scope_info const&) const> visitor{};
    printing::type::ScopeNavigator const navigator{std::ref(visitor)};

    [[maybe_unused]] auto [maybeExpectedName, rawName] = GENERATE(
        (table<StringT, StringT>)({
            {        "my_typeLambda",         name_of<decltype(my_typeLambda)>()},
            {   "my_typeUnaryLambda",    name_of<decltype(my_typeUnaryLambda)>()},
            {  "my_typeBinaryLambda",   name_of<decltype(my_typeBinaryLambda)>()},
            { "my_typeMutableLambda",  name_of<decltype(my_typeMutableLambda)>()},
            {"my_typeNoexceptLambda", name_of<decltype(my_typeNoexceptLambda)>()}
    }));
    CAPTURE(maybeExpectedName, rawName);

    SequenceT sequence{};
    SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
        and expect::in_sequence(sequence);
    #if MIMICPP_DETAIL_IS_GCC
    SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals(maybeExpectedName)})
        and expect::in_sequence(sequence);
    #endif
    SCOPED_EXP visitor.expect_call(matches_lambda_scope())
        and expect::in_sequence(sequence);

    navigator(rawName);
}

TEST_CASE(
    "printing::type::ScopeNavigator supports lambda-local types.",
    "[print][print::type]")
{
    using printing::type::scope_info;

    Mock<void(scope_info const&) const> visitor{};
    printing::type::ScopeNavigator const navigator{std::ref(visitor)};

    SECTION("When anon lambda-local type is given.")
    {
        [[maybe_unused]] auto const [maybeExpectedName, rawName] = GENERATE(
            (table<StringT, StringT>)({
                { "anon_classLambda",  name_of<decltype(anon_classLambda())>()},
                {"anon_structLambda", name_of<decltype(anon_structLambda())>()},
                {  "anon_enumLambda",   name_of<decltype(anon_enumLambda())>()}
        }));
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
    #if MIMICPP_DETAIL_IS_GCC
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals(maybeExpectedName)})
            and expect::in_sequence(sequence);
    #endif
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(matches_anon_type_scope())
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When immutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeLambda())>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
    #if MIMICPP_DETAIL_IS_GCC
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_typeLambda")})
            and expect::in_sequence(sequence);
    #endif
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_function_scope(
                Catch::Matchers::Matches(callOperatorPattern),
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When unary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeUnaryLambda(42))>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
    #if MIMICPP_DETAIL_IS_GCC
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_typeUnaryLambda")})
            and expect::in_sequence(sequence);
    #endif
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::Equals("int"),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When binary lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeBinaryLambda(42, 4.2f))>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
    #if MIMICPP_DETAIL_IS_GCC
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_typeBinaryLambda")})
            and expect::in_sequence(sequence);
    #endif
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::Matches(R"(int,\s?float)"),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When mutable lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeMutableLambda())>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
    #if MIMICPP_DETAIL_IS_GCC
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_typeMutableLambda")})
            and expect::in_sequence(sequence);
    #endif
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(matches_call_operator_scope(Catch::Matchers::IsEmpty()))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When noexcept lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeNoexceptLambda())>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
    #if MIMICPP_DETAIL_IS_GCC
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_typeNoexceptLambda")})
            and expect::in_sequence(sequence);
    #endif
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        // noexcept doesn't seem to have an influence here.
        SCOPED_EXP visitor.expect_call(matches_call_operator_scope(
            Catch::Matchers::IsEmpty(),
            Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When nested lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeNestedLambda())>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
    #if MIMICPP_DETAIL_IS_GCC
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_typeNestedLambda")})
            and expect::in_sequence(sequence);
    #endif
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }

    SECTION("When complex nested lambda is given.")
    {
        StringT const rawName = name_of<decltype(my_typeComplexNestedLambda())>();
        CAPTURE(rawName);

        SequenceT sequence{};
        SCOPED_EXP visitor.expect_call(matches_anon_ns_scope())
            and expect::in_sequence(sequence);
        #if MIMICPP_DETAIL_IS_GCC
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_typeComplexNestedLambda")})
            and expect::in_sequence(sequence);
        #endif
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::IsEmpty(),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(matches_lambda_scope())
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(
            matches_call_operator_scope(
                Catch::Matchers::Matches(R"(int,\s?float)"),
                Catch::Matchers::Equals("const")))
            and expect::in_sequence(sequence);
        SCOPED_EXP visitor.expect_call(NamedScopeMatcher{Catch::Matchers::Equals("my_type")})
            and expect::in_sequence(sequence);

        navigator(rawName);
    }
}

#endif
