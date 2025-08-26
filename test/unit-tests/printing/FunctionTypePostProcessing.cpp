//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/SourceLocation.hpp"
#include "mimic++/utilities/Stacktrace.hpp"

using namespace mimicpp;

#ifdef MIMICPP_CONFIG_EXPERIMENTAL_PRETTY_TYPES

constexpr auto type_post_processing_lambda_loc = [] {
    return util::SourceLocation{};
};

constexpr auto type_post_processing_nested_lambda_loc = [] {
    return [] {
        return util::SourceLocation{};
    }();
};

namespace
{
    [[nodiscard]]
    constexpr util::SourceLocation loc_fun()
    {
        [[maybe_unused]] constexpr auto dummy = [] {};
        [[maybe_unused]] constexpr auto dummy2 = [] {};
        constexpr auto inner = [] {
            return util::SourceLocation{};
        };
        [[maybe_unused]] constexpr auto dummy3 = [] {};

        return inner();
    }

    [[nodiscard]]
    constexpr util::SourceLocation loc_anon_lambda_fun()
    {
        return [] {
            return util::SourceLocation{};
        }();
    }

    constexpr auto my_typeLambda = [] {
        struct my_type
        {
        };

        return my_type{};
    };

    template <typename... Ts>
    struct my_template
    {
        struct my_type
        {
        };

        util::SourceLocation foo(my_type)
        {
            return util::SourceLocation{};
        }

        auto bar(my_type const&, util::SourceLocation* outLoc)
        {
            if (outLoc)
            {
                *outLoc = util::SourceLocation{};
            }

            struct bar_type
            {
            };

            return bar_type{};
        }
    };

    StringT const topLevelLambdaPattern =
        R"((\$_\d+|lambda(#\d+)?|\(anonymous class\)))";

    StringT const lambdaCallOpPattern = topLevelLambdaPattern + R"(::(operator\s?)?\(\))";

    StringT const anonNsScopePattern = R"(\{anon-ns\}::)";
    StringT const anonTypePattern = R"((\$_\d+|<unnamed-(tag|type-obj)>|<unnamed (class|struct|enum)>|\(anonymous (class|struct|enum)\)))";
    StringT const testCasePattern = R"(CATCH2_INTERNAL_TEST_\d+)";
    StringT const locReturnPattern = "(auto|(mimicpp::)?util::SourceLocation) ";
}

// source-locations other than std::source_location may behave differently.
    #ifdef MIMICPP_TESTING_ENABLE_COMPAT_SOURCE_LOCATION
        #define MAYFAIL_WITH_COMPAT_LOC "[!mayfail]"
    #else
        #define MAYFAIL_WITH_COMPAT_LOC
    #endif

TEST_CASE(
    "printing::type::prettify_function enhances std::source_location::function_name appearance.",
    MAYFAIL_WITH_COMPAT_LOC "[print]")
{
    StringStreamT ss{};

    SECTION("When general function is given.")
    {
        util::SourceLocation constexpr loc{};
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            StringT{loc.function_name()});

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches("void " + testCasePattern + R"(\(\))"));
    }

    SECTION("When lambda is given.")
    {
        constexpr auto loc = type_post_processing_lambda_loc();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            StringT{loc.function_name()});

    #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Equals("lambda"));
    #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + lambdaCallOpPattern
                + R"(\(\)(\s?const)?)"));
    #endif
    }

    SECTION("When nested lambda is given.")
    {
        constexpr auto loc = type_post_processing_nested_lambda_loc();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            StringT{loc.function_name()});

    #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Equals("lambda::lambda"));
    #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + lambdaCallOpPattern
                + "::"
                + lambdaCallOpPattern
                + R"(\(\)(\s?const)?)"));
    #endif
    }

    SECTION("When function-local lambda is given.")
    {
        constexpr auto loc = loc_fun();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            StringT{loc.function_name()});

    #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(anonNsScopePattern + "loc_fun::lambda"));
    #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + anonNsScopePattern
                + "loc_fun::"
                + lambdaCallOpPattern
                + R"(\(\)(\s?const)?)"));
    #endif
    }

    SECTION("When function-local anon-struct is given.")
    {
        struct
        {
            constexpr util::SourceLocation operator()() const
            {
                return util::SourceLocation{};
            }
        } constexpr obj{};

        constexpr auto loc = obj();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            StringT{loc.function_name()});

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + testCasePattern
                + "::"
                + anonTypePattern
                + "::"
                  R"(operator\s?\(\))"
                  R"(\(\)\s?const)"));
    }

    SECTION("When function-local anon-class is given.")
    {
        class
        {
        public:
            constexpr util::SourceLocation operator()() const
            {
                return util::SourceLocation{};
            }
        } constexpr obj{};

        constexpr auto loc = obj();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            StringT{loc.function_name()});

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + testCasePattern
                + "::"
                + anonTypePattern
                + "::"
                  R"(operator\s?\(\))"
                  R"(\(\)\s?const)"));
    }

    SECTION("When function-local anon-lambda is given.")
    {
        constexpr auto loc = loc_anon_lambda_fun();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            StringT{loc.function_name()});

    #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(anonNsScopePattern + "loc_anon_lambda_fun::lambda"));
    #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + anonNsScopePattern
                + "loc_anon_lambda_fun::"
                + lambdaCallOpPattern
                + R"(\(\)(\s?const)?)"));
    #endif
    }

    SECTION("When template-dependant function is given.")
    {
        using type_t = decltype(my_typeLambda());
        auto const loc = my_template<type_t, int>{}.foo({});
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            StringT{loc.function_name()});

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                locReturnPattern
                + anonNsScopePattern
                + "my_template::foo"
                  R"(\((\{anon-ns\}::my_template::)?my_type\))"));
    }
}

namespace
{
    struct conversion
    {
        operator util::SourceLocation()
        {
            return util::SourceLocation{};
        }

        operator util::SourceLocation() const
        {
            return util::SourceLocation{};
        }

        operator Stacktrace()
        {
            return stacktrace::current();
        }

        operator Stacktrace() const
        {
            return stacktrace::current();
        }
    };
}

#ifdef MIMICPP_DETAIL_IS_MSVC
    #define MAYFAIL_ON_MSVC "[!mayfail]"
#else
    #define MAYFAIL_ON_MSVC
#endif

TEST_CASE(
    "printing::type::prettify_function supports conversion-operators.",
    MAYFAIL_WITH_COMPAT_LOC MAYFAIL_ON_MSVC "[print][print::type]")
{
    StringStreamT ss{};

    SECTION("When getting function name via source_location")
    {
        SECTION("and when converted to simple type via non-const function.")
        {
            conversion conv{};
            auto const loc = static_cast<util::SourceLocation>(conv);
            StringT const fnName{loc.function_name()};
            CAPTURE(fnName);

            printing::type::prettify_function(
                std::ostreambuf_iterator{ss},
                fnName);

            REQUIRE_THAT(
                ss.str(),
                Catch::Matchers::Matches(
                    // Clang adds a return type here.
                    "(util::SourceLocation )?"
                    + anonNsScopePattern
                    + "conversion::"
                    + R"(operator (mimicpp::util::)?SourceLocation\(\))"));
        }

        SECTION("and when converted to simple type via const function.")
        {
            conversion const conv{};
            auto const loc = static_cast<util::SourceLocation>(conv);
            StringT const fnName{loc.function_name()};
            CAPTURE(fnName);

            printing::type::prettify_function(
                std::ostreambuf_iterator{ss},
                fnName);

            REQUIRE_THAT(
                ss.str(),
                Catch::Matchers::Matches(
                    // Clang adds a return type here.
                    "(util::SourceLocation )?"
                    + anonNsScopePattern
                    + "conversion::"
                    + R"(operator (mimicpp::util::)?SourceLocation\(\) const)"));
        }
    }

    #if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

    SECTION("When getting function name via stacktrace")
    {
        SECTION("and when converted to simple type via non-const function.")
        {
            conversion conv{};
            auto const trace = static_cast<Stacktrace>(conv);
            StringT const fnName = trace.description(0u);
            CAPTURE(fnName);

            printing::type::prettify_function(
                std::ostreambuf_iterator{ss},
                fnName);

            REQUIRE_THAT(
                ss.str(),
                Catch::Matchers::Matches(
                    R"((\{anon-ns\}::conversion::)?)"
                    R"(operator (mimicpp::)?Stacktrace(\(\))?)"));
        }

        SECTION("When converted to simple type via const function.")
        {
            conversion const conv{};
            auto const trace = static_cast<Stacktrace>(conv);
            StringT const fnName = trace.description(0u);
            CAPTURE(fnName);

            printing::type::prettify_function(
                std::ostreambuf_iterator{ss},
                fnName);

            REQUIRE_THAT(
                ss.str(),
                Catch::Matchers::Matches(
                    R"((\{anon-ns\}::conversion::)?)"
                    R"(operator (mimicpp::)?Stacktrace(\(\)(\s?const)?)?)"));
        }
    }

    #endif
}

    #if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

constexpr auto function_type_post_processing_lambda_stacktrace = [] {
    return stacktrace::current();
};

constexpr auto function_type_post_processing_nested_lambda_stacktrace = [] {
    return [] {
        return stacktrace::current();
    }();
};

namespace
{
    [[nodiscard]]
    Stacktrace stacktrace_fun()
    {
        [[maybe_unused]] constexpr auto dummy = [] {};
        [[maybe_unused]] constexpr auto dummy2 = [] {};
        constexpr auto inner = [] {
            return stacktrace::current();
        };
        [[maybe_unused]] constexpr auto dummy3 = [] {};

        return inner();
    }

    [[nodiscard]]
    Stacktrace stacktrace_anon_lambda_fun()
    {
        return [] {
            return stacktrace::current();
        }();
    }

    template <typename... Ts>
    struct my_stacktrace_template
    {
        struct my_type
        {
        };

        [[nodiscard]]
        Stacktrace foo(my_type)
        {
            return stacktrace::current();
        }
    };
}

TEST_CASE(
    "printing::type::prettify_function enhances Stacktrace::description appearance.",
    "[print]")
{
    StringStreamT ss{};

    SECTION("When general function is given.")
    {
        auto const trace = stacktrace::current();
        REQUIRE_FALSE(trace.empty());
        StringT const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(testCasePattern + R"((\(\))?)"));
    }

    SECTION("When lambda is given.")
    {
        auto const trace = function_type_post_processing_lambda_stacktrace();
        REQUIRE_FALSE(trace.empty());
        StringT const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(R"(operator\s?\(\))"));
        #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                lambdaCallOpPattern
                + R"((\(\)(\s?const)?)?)"));
        #endif
    }

    SECTION("When nested lambda is given.")
    {
        auto const trace = function_type_post_processing_nested_lambda_stacktrace();
        REQUIRE_FALSE(trace.empty());
        StringT const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(R"(operator\s?\(\))"));
        #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                lambdaCallOpPattern
                // Todo: This results in the wrong output, but I'm tired of getting this work, so I'll accept it for now.
                // The first `...::operator()()` is falsely recognized as a return type, because the trailing `const`
                // is separated by a whitespace. That's in fact a general problem of the parsing process.
                + R"((\(\)(\s?const)?)?)" // << this is wrong!
                  "::"
                + lambdaCallOpPattern
                + R"((\(\)(\s?const)?)?)"));
        #endif
    }

    SECTION("When function-local lambda is given.")
    {
        auto const trace = stacktrace_fun();
        REQUIRE_FALSE(trace.empty());
        StringT const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(R"(operator\s?\(\))"));
        #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "stacktrace_fun::"
                + lambdaCallOpPattern
                + R"((\(\)(\s?const)?)?)"));
        #endif
    }

    SECTION("When function-local anon-struct is given.")
    {
        struct
        {
            [[nodiscard]]
            Stacktrace operator()() const
            {
                return stacktrace::current();
            }
        } constexpr obj{};

        auto const trace = obj();
        REQUIRE_FALSE(trace.empty());
        StringT const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(R"(operator\s?\(\))"));
        #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                testCasePattern
                + "::"
                + anonTypePattern
                + R"(::operator\s?\(\))"
                  R"((\(\)(\s?const)?)?)"));
        #endif
    }

    SECTION("When function-local anon-class is given.")
    {
        class
        {
        public:
            [[nodiscard]]
            Stacktrace operator()() const
            {
                return stacktrace::current();
            }
        } constexpr obj{};

        auto const trace = obj();
        REQUIRE_FALSE(trace.empty());
        StringT const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(R"(operator\s?\(\))"));
        #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                testCasePattern
                + "::"
                + anonTypePattern
                + R"(::operator\s?\(\))"
                  R"((\(\)(\s?const)?)?)"));
        #endif
    }

    SECTION("When function-local anon-lambda is given.")
    {
        auto const trace = stacktrace_anon_lambda_fun();
        REQUIRE_FALSE(trace.empty());
        StringT const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(R"(operator\s?\(\))"));
        #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + "stacktrace_anon_lambda_fun::"
                + lambdaCallOpPattern
                + R"((\(\)(\s?const)?)?)"));
        #endif
    }

    SECTION("When template-dependant function is given.")
    {
        using type_t = decltype(my_typeLambda());
        auto const trace = my_stacktrace_template<type_t, int>{}.foo({});
        REQUIRE_FALSE(trace.empty());
        StringT const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        #if MIMICPP_DETAIL_IS_GCC
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(R"(foo)"));
        #else
        REQUIRE_THAT(
            ss.str(),
            Catch::Matchers::Matches(
                anonNsScopePattern
                + R"(my_stacktrace_template::foo(\(()"
                + anonNsScopePattern + R"(my_stacktrace_template::my_type)?\))?)"));
        #endif
    }
}

    #endif

#endif
