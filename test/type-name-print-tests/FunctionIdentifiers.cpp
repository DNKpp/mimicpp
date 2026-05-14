//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/printing/TypePrinter.hpp"
#include "mimic++/utilities/Stacktrace.hpp"

#include "Common.hpp"

using namespace mimicpp;

namespace
{
    std::string const testCasePattern = R"(CATCH2_INTERNAL_TEST_\d+)";
}

TEST_CASE(
    "printing::type::prettify_function omits function args with just `void` content.",
    "[print][print::type]")
{
    std::string const name{"ret my_function<void>(void)"};

    std::ostringstream ss{};
    printing::type::prettify_function(
        std::ostreambuf_iterator{ss},
        name);

    CHECK_THAT(
        ss.str(),
        Catch::Matchers::Equals(+"ret my_function<void>()"));
}

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
}

// The `__builtin_FUNCTION` function generates more unreliable names.
#ifdef MIMICPP_DETAIL_HAS_SOURCE_LOCATION
    #define PRETTIFY_FUNCTION_SOURCE_LOCATION_FUNCTION_SHOULDFAIL
#else
    #define PRETTIFY_FUNCTION_SOURCE_LOCATION_FUNCTION_SHOULDFAIL "[!shouldfail]"
#endif

TEST_CASE(
    "printing::type::prettify_function enhances std::source_location::function_name appearance.",
    PRETTIFY_FUNCTION_SOURCE_LOCATION_FUNCTION_SHOULDFAIL "[print][print::type]")
{
    std::ostringstream ss{};

    SECTION("When a general function is given.")
    {
        util::SourceLocation constexpr loc{};
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches("void " + testCasePattern + R"(\(\))"));
    }

    SECTION("When a lambda is given.")
    {
        constexpr auto loc = type_post_processing_lambda_loc();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        auto const alt1 = testing::lambda_pattern();
        auto const alt2 = "auto " + testing::anonTypePattern + R"(::operator\(\)\(\) const)";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(testing::alternative_pattern(alt1, alt2)));
    }

    SECTION("When a nested lambda is given.")
    {
        constexpr auto loc = type_post_processing_nested_lambda_loc();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        auto const alt1 = testing::lambda_pattern() + "::" + testing::lambda_pattern();
        auto const alt2 = "auto " + testing::anonTypePattern + R"(::operator\(\)::)" + testing::anonTypePattern + R"(::operator\(\)\(\) const)";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(testing::alternative_pattern(alt1, alt2)));
    }

    SECTION("When a function-local lambda is given.")
    {
        constexpr auto loc = loc_fun();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        auto const scopePattern = testing::anonNsScopePattern + "loc_fun::";
        auto const alt1 = scopePattern + testing::lambda_pattern();
        auto const alt2 = "auto " + scopePattern + testing::anonTypePattern + R"(::operator\(\)\(\) const)";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(testing::alternative_pattern(alt1, alt2)));
    }

    SECTION("When a function-local anon-struct is given.")
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
            loc.function_name());

        auto const returnPattern = testing::maybe_pattern("mimicpp::") + "util::SourceLocation ";
        auto const scopePattern = testCasePattern + "::<unnamed struct>::";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(returnPattern + scopePattern + R"(operator\(\)\(\) const)"));
    }

    SECTION("When a function-local anon-class is given.")
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
            loc.function_name());

        auto const returnPattern = testing::maybe_pattern("mimicpp::") + "util::SourceLocation ";
        auto const scopePattern = testCasePattern + "::<unnamed class>::";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(returnPattern + scopePattern + R"(operator\(\)\(\) const)"));
    }

    SECTION("When a function-local anon-lambda is given.")
    {
        constexpr auto loc = loc_anon_lambda_fun();
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        auto const scopePattern = testing::anonNsScopePattern + "loc_anon_lambda_fun::";
        auto const alt1 = scopePattern + testing::lambda_pattern();
        auto const alt2 = "auto " + scopePattern + testing::anonTypePattern + R"(::operator\(\)\(\) const)";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(testing::alternative_pattern(alt1, alt2)));
    }

    SECTION("When a template-dependant function is given.")
    {
        using type_t = decltype(my_typeLambda());
        auto const loc = my_template<type_t, int>{}.foo({});
        CAPTURE(loc.function_name());

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            loc.function_name());

        auto const returnPattern = testing::maybe_pattern("mimicpp::") + "util::SourceLocation ";
        auto const scopePattern = testing::anonNsScopePattern + "my_template::";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(returnPattern + scopePattern + R"(foo\(my_type\))"));
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

        operator util::Stacktrace()
        {
            return util::stacktrace::current();
        }

        operator util::Stacktrace() const
        {
            return util::stacktrace::current();
        }
    };
}

// Todo: The parser currently cannot properly distinguish between conversion-functions and types containing a conversion-op scope.
TEST_CASE(
    "printing::type::prettify_function supports conversion-operators.",
    "[!shouldfail][print][print::type]")
{
    auto const scopePattern = testing::anonNsScopePattern + "conversion::";
    std::ostringstream ss{};

    SECTION("When getting function name via source_location")
    {
        auto const targetId = "SourceLocation";

        SECTION("and when converted to simple type via non-const function.")
        {
            conversion conv{};
            auto const loc = static_cast<util::SourceLocation>(conv);
            std::string const fnName{loc.function_name()};
            CAPTURE(fnName);

            printing::type::prettify_function(
                std::ostreambuf_iterator{ss},
                fnName);

            CHECK_THAT(
                ss.str(),
                Catch::Matchers::Matches(scopePattern + "operator " + targetId + R"(\(\))"));
        }

        SECTION("and when converted to simple type via const function.")
        {
            conversion const conv{};
            auto const loc = static_cast<util::SourceLocation>(conv);
            std::string const fnName{loc.function_name()};
            CAPTURE(fnName);

            printing::type::prettify_function(
                std::ostreambuf_iterator{ss},
                fnName);

            CHECK_THAT(
                ss.str(),
                Catch::Matchers::Matches(scopePattern + "operator " + targetId + R"(\(\) const)"));
        }
    }

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND
    SECTION("When getting function name via stacktrace")
    {
        auto const targetId = "Stacktrace";

        SECTION("and when converted to simple type via non-const function.")
        {
            conversion conv{};
            auto const trace = static_cast<util::Stacktrace>(conv);
            std::string const fnName = trace.description(0u);
            CAPTURE(fnName);

            printing::type::prettify_function(
                std::ostreambuf_iterator{ss},
                fnName);

            CHECK_THAT(
                ss.str(),
                Catch::Matchers::Matches(scopePattern + "operator " + targetId + R"(\(\))"));
        }

        SECTION("When converted to simple type via const function.")
        {
            conversion const conv{};
            auto const trace = static_cast<util::Stacktrace>(conv);
            std::string const fnName = trace.description(0u);
            CAPTURE(fnName);

            printing::type::prettify_function(
                std::ostreambuf_iterator{ss},
                fnName);

            CHECK_THAT(
                ss.str(),
                Catch::Matchers::Matches(scopePattern + "operator " + targetId + R"(\(\) const)"));
        }
    }
#endif
}

// Todo: Let's ignore these cases for now.
/*#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

constexpr auto function_type_post_processing_lambda_stacktrace = [] {
    return util::stacktrace::current();
};

constexpr auto function_type_post_processing_nested_lambda_stacktrace = [] {
    return [] {
        return util::stacktrace::current();
    }();
};

namespace
{
    [[nodiscard]]
    util::Stacktrace stacktrace_fun()
    {
        [[maybe_unused]] constexpr auto dummy = [] {};
        [[maybe_unused]] constexpr auto dummy2 = [] {};
        constexpr auto inner = [] {
            return util::stacktrace::current();
        };
        [[maybe_unused]] constexpr auto dummy3 = [] {};

        return inner();
    }

    [[nodiscard]]
    util::Stacktrace stacktrace_anon_lambda_fun()
    {
        return [] {
            return util::stacktrace::current();
        }();
    }

    template <typename... Ts>
    struct my_stacktrace_template
    {
        struct my_type
        {
        };

        [[nodiscard]]
        util::Stacktrace foo(my_type)
        {
            return util::stacktrace::current();
        }
    };
}

TEST_CASE(
    "printing::type::prettify_function enhances Stacktrace::description appearance.",
    "[print]")
{
    std::ostringstream ss{};

    SECTION("When a general function is given.")
    {
        auto const trace = util::stacktrace::current();
        REQUIRE_FALSE(trace.empty());
        std::string const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(testCasePattern + R"((\(\))?)"));
    }

    SECTION("When a lambda is given.")
    {
        auto const trace = function_type_post_processing_lambda_stacktrace();
        REQUIRE_FALSE(trace.empty());
        std::string const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(testing::lambda_pattern() + R"((\(\)(\s?const)?)?)"));
    }

    SECTION("When a nested lambda is given.")
    {
        auto const trace = function_type_post_processing_nested_lambda_stacktrace();
        REQUIRE_FALSE(trace.empty());
        std::string const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        auto const scopePattern = testing::lambda_pattern() + " const::";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + testing::lambda_pattern() + " const"));
    }

    SECTION("When a function-local lambda is given.")
    {
        auto const trace = stacktrace_fun();
        REQUIRE_FALSE(trace.empty());
        std::string const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        auto const scopePattern = testing::anonNsScopePattern + "stacktrace_fun::";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + testing::lambda_pattern() + " const"));
    }

    SECTION("When a function-local anon-struct is given.")
    {
        struct
        {
            [[nodiscard]]
            util::Stacktrace operator()() const
            {
                return util::stacktrace::current();
            }
        } constexpr obj{};

        auto const trace = obj();
        REQUIRE_FALSE(trace.empty());
        std::string const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        auto const scopePattern = testCasePattern + "stacktrace_fun::" + testing::anonTypePattern + "::";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + R"(operator\(\)\(\) const)"));
    }

    SECTION("When a function-local anon-class is given.")
    {
        class
        {
        public:
            [[nodiscard]]
            util::Stacktrace operator()() const
            {
                return util::stacktrace::current();
            }
        } constexpr obj{};

        auto const trace = obj();
        REQUIRE_FALSE(trace.empty());
        std::string const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        auto const scopePattern = "::" + testCasePattern + "::" + testing::anonTypePattern + "::";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + R"(::operator\(\)\(\) const)"));
    }

    SECTION("When a function-local anon-lambda is given.")
    {
        auto const trace = stacktrace_anon_lambda_fun();
        REQUIRE_FALSE(trace.empty());
        std::string const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        auto const scopePattern = testing::anonNsScopePattern + "::stacktrace_anon_lambda_fun::";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + testing::lambda_pattern()));
    }

    SECTION("When a template-dependant function is given.")
    {
        using type_t = decltype(my_typeLambda());
        auto const trace = my_stacktrace_template<type_t, int>{}.foo({});
        REQUIRE_FALSE(trace.empty());
        std::string const name = trace.description(0u);
        CAPTURE(name);

        printing::type::prettify_function(
            std::ostreambuf_iterator{ss},
            name);

        auto const scopePattern = testing::anonNsScopePattern + "::my_stacktrace_template::";
        auto const argPattern = testing::anonNsScopePattern + "::my_stacktrace_template::my_type";
        CHECK_THAT(
            ss.str(),
            Catch::Matchers::Matches(scopePattern + R"(foo\()" + argPattern + R"(\))"));
    }
}

#endif*/
