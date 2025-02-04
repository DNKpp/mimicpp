//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/C++23Backports.hpp"
#include "mimic++/utilities/Regex.hpp"

#include <ranges>
#include <vector>

using namespace mimicpp;

namespace
{
    template <typename T>
    struct my_template
    {
    };

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

        auto my_typeRvalueFunction() &&
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

    auto my_typeMutableLambda = []() mutable {
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

    auto my_typeFreeFunction()
    {
        struct my_type
        {
        };

        return my_type{};
    }
}

[[nodiscard]]
inline StringT handle_lambda(SMatchT const& matches)
{
    assert(matches.size() == 6 && "Regex out-of-sync.");

    StringStreamT ss{};

    auto const& scope = matches[1];
    auto const& lambdaId = matches[2];
    auto const& paramList = matches[3];
    auto const& lambdaSpecs = matches[4];
    auto const& typeIdentifier = matches[5];

    ss << "(" << scope << "lambda#" << lambdaId << "::operator()(";

    if (paramList != "void")
    {
        ss << paramList;
    }
    ss << ")";

    if (0 != lambdaSpecs.length())
    {
        ss << " " << lambdaSpecs;
    }
    ss << ")::" << typeIdentifier;

    return std::move(ss).str();
}

[[nodiscard]]
constexpr StringT prettify_lambda_scope(auto const& matches)
{
    assert(matches.size() == 4 && "Regex out-of-sync.");

    StringStreamT ss{};

    auto const& lambdaId = matches[1];
    auto const& paramList = matches[2];
    auto const& lambdaSpecs = matches[3];

    ss << "(lambda#" << lambdaId << "::operator()(";

    if (paramList != "void")
    {
        ss << paramList;
    }
    ss << ")";

    if (0 != lambdaSpecs.length())
    {
        ss << " " << lambdaSpecs;
    }
    ss << ")::";

    return std::move(ss).str();
}

[[nodiscard]]
constexpr StringT prettify_function_scope(auto const& matches)
{
    assert(matches.size() == 5 && "Regex out-of-sync.");

    StringStreamT ss{};

    auto const& functionName = matches[1];
    auto const& paramList = matches[2];
    auto const& specs = matches[3];
    auto const& refSpecs = matches[4];

    ss << functionName << "(";

    if (paramList != "void")
    {
        ss << paramList;
    }
    ss << ")";

    if (0 != specs.length())
    {
        ss << " " << specs;
    }
    ss << refSpecs << "::";

    return std::move(ss).str();
}

constexpr StringViewT scopeToken{"::"};
constexpr StringViewT anonymousNamespaceToken{"`anonymous namespace'::"};

inline StringT prettify_scopes(StringT name)
{
    static RegexT const virtualScope{"`\\d+'::"};
    static RegexT const regularScope{R"(\w+::)"};
    static RegexT const lambdaScope{
        "`"
        R"((?:public: ))"   // lambda::operator() is always publicly available
        R"((?:__\w+ ))"     // call-convention
        R"(<lambda_(\d+)>)" // lambda-identifier
        R"(::operator\(\))" // operator()
        R"(\((.*?)\))"      // arg-list
        R"(((?:const)?))"   // const (optional)
        R"((?: __\w+)?)"    // __ptr64 (optional)
        "'::"               //
    };

    static RegexT const functionScope{
        "`"
        R"((?:(?:public|private|protected): )?)" // access specifier (optional)
        "(?:static )?"                           // static (optional)
        R"((?:__\w+ ))"                          // call-convention
        R"((operator.+?|\w+))"                   // function-name
        R"(\((.*?)\))"                           // arg-list
        R"(((?:const)?))"                        // const (optional)
        R"((?: __\w+)?)"                         // __ptr64 (optional)
        R"((&{0,2}))"                            // ref specifier
        R"(\s*'::)"                              //
    };

    StringT result{};
    SVMatchT scopeMatches{};
    while (auto const firstScopeDelimiter = std::ranges::search(name, scopeToken))
    {
        StringViewT const leadingScope{name.cbegin(), firstScopeDelimiter.end()};
        if (leadingScope.ends_with(anonymousNamespaceToken))
        {
            result += "(anon ns)::";
            auto const index = leadingScope.size() - anonymousNamespaceToken.size();
            name.erase(index, anonymousNamespaceToken.size());
        }
        else if (std::regex_search(leadingScope.cbegin(), leadingScope.cend(), scopeMatches, virtualScope))
        {
            auto const index = std::ranges::distance(leadingScope.cbegin(), scopeMatches[0].first);
            name.erase(index, scopeMatches[0].length());
        }
        else if (std::regex_search(leadingScope.cbegin(), leadingScope.cend(), scopeMatches, regularScope))
        {
            result += scopeMatches[0];
            auto const index = std::ranges::distance(leadingScope.cbegin(), scopeMatches[0].first);
            name.erase(index, scopeMatches[0].length());
        }
        else if (StringViewT const fullName{name};
                 std::regex_search(fullName.cbegin(), fullName.cend(), scopeMatches, lambdaScope))
        {
            result += prettify_lambda_scope(scopeMatches);
            auto const index = std::ranges::distance(fullName.cbegin(), scopeMatches[0].first);
            name.erase(index, scopeMatches[0].length());
        }
        else if (std::regex_search(fullName.cbegin(), fullName.cend(), scopeMatches, functionScope))
        {
            result += prettify_function_scope(scopeMatches);
            auto const index = std::ranges::distance(fullName.cbegin(), scopeMatches[0].first);
            name.erase(index, scopeMatches[0].length());
        }
        else
        {
            util::unreachable();
        }
    }

    return result;
}

[[nodiscard]]
inline StringT prettify_identifier(StringT name)
{
    static RegexT const omitClassStructEnum{R"(\b(class|struct|enum)\s+)"};
    name = std::regex_replace(name, omitClassStructEnum, "");

    /*static RegexT const shortenAnonymousNamespace{"`anonymous namespace'::"};
    name = std::regex_replace(name, shortenAnonymousNamespace, "(anon ns)::");*/

    /*static RegexT const lambdaRegex{
        "`"
        R"((?:(?:public|private|protected): )?)" // access spec (optional)
        R"((?:__\w+ ))"                          // call-convention
        R"(((?:.+?::)*?))"                       // prefix-scope
        R"(<lambda_(\d+)>)"                      // lambda-identifier
        R"(::operator\(\))"                      // operator()
        R"(\((.*?)\))"                           // arg-list
        R"((.*?))"                               // specs
        R"((?: __\w+)?)"                         // __ptr64 (optional)
        "'"                                      //
        R"(::`\d+')"                             // some arbitrary number
        "::"                                     //
        R"((\w+))"                               // actual type identifier
    };

    StringT finalName{};
    SMatchT scopeMatches{};
    while (std::regex_search(name, scopeMatches, lambdaScopeRegex))
    {
        finalName += prettify_lambda_scope(scopeMatches);
        name.erase(scopeMatches[0].first, scopeMatches[0].second);
    }

    finalName += name;*/

    if (auto const lastMatch = std::ranges::search(name | std::views::reverse, scopeToken))
    {
        StringViewT const scope{name.cbegin(), lastMatch.begin().base()};
        StringViewT const topLevelIdentifier{name.data() + scope.size(), name.size()};

        // include last ::
        name = prettify_scopes(StringT{scope}) + topLevelIdentifier.data();
    }

    return name;
}

TEST_CASE(
    "printing::detail::prettify_identifier type-names enhances appearance.",
    "[print][detail]")
{
    /*SECTION("When type-name in anonymous-namespace is given.")
    {
        StringT const rawName = printing::detail::type_name<my_type>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Equals("(anon ns)::my_type"));
    }

    SECTION("When nested type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<outer_type::my_type>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Equals("(anon ns)::outer_type::my_type"));
    }

    SECTION("When lambda-local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(my_typeLambda())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(\(anon ns\)::lambda#\d+::operator\(\)\(\) const\)::my_type)"));
    }

    SECTION("When mutable lambda-local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(my_typeMutableLambda())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(\(anon ns\)::lambda#\d+::operator\(\)\(\)\)::my_type)"));
    }

    SECTION("When noexcept lambda-local type-name is given.")
    {
        // noexcept doesn't seem to be part of the spec list
        StringT const rawName = printing::detail::type_name<decltype(my_typeNoexceptLambda())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(\(anon ns\)::lambda#\d+::operator\(\)\(\) const\)::my_type)"));
    }*/

    SECTION("When nested lambda-local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(my_typeNestedLambda())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(\(anon ns\)::lambda#\d+::operator\(\)\(\) const\)::my_type)"));
    }

    SECTION("When free-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(my_typeFreeFunction())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_type)"));
    }

    SECTION("When public function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeFunction())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_type)"));
    }

    SECTION("When public const-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeConstFunction())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_type)"));
    }

    SECTION("When public static-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type::my_typeStaticFunction())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_type)"));
    }

    SECTION("When public lvalue-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(std::declval<outer_type&>().my_typeLvalueFunction())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_type)"));
    }

    SECTION("When public rvalue-function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeRvalueFunction())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_type)"));
    }

    SECTION("When private function local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.my_typeIndirectlyPrivateFunction())>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_type)"));
    }

    SECTION("When public operator local type-name is given.")
    {
        StringT const rawName = printing::detail::type_name<decltype(outer_type{}.operator+(42))>();
        CAPTURE(rawName);

        StringT const finalName = prettify_identifier(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Matches(R"(\(anon ns\)::\w+::my_type)"));
    }
}

/*TEST_CASE(
    "printing::detail::prettify_template_name type-names, enhances templated appearance.",
    "[print][detail]")
{
    SECTION("When arbitrary template name is given.")
    {
        StringT const rawName = printing::detail::type_name<std::vector<int>>();
        CAPTURE(rawName);

        StringT const finalName = printing::detail::prettify_template_name(rawName);
        REQUIRE_THAT(
            finalName,
            Catch::Matchers::Equals("std::vector"));
    }

    SECTION("When template name in anonymous-namespace is given.")
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
    }
}*/
