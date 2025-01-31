//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/StringifyReports.hpp"

using namespace mimicpp;

namespace
{
    inline reporting::control_state_t const commonApplicableState = reporting::state_applicable{13, 1337, 42};
    [[nodiscard]]
    Stacktrace make_shallow_stacktrace()
    {
        Stacktrace stacktrace = stacktrace::current();
        CHECK(!stacktrace.empty());

        // the std::regex on windows is too complex, so we limit it
        constexpr std::size_t maxLength{6u};
        const auto size = std::min(maxLength, stacktrace.size());
        const auto skip = stacktrace.size() - size;
        stacktrace = stacktrace::current(skip);
        CHECK(size == stacktrace.size());

        return stacktrace;
    }
}

TEST_CASE(
    "detail::stringify_full_match converts the information to a pretty formatted text.",
    "[reporting][detail]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {
            {{reporting::TypeReport::make<int>(), "1337"},
             {reporting::TypeReport::make<std::string>(), "\"Hello, World!\""}}},
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {
                 {"expect: arg[1] not empty",
             std::nullopt,
             "expect: arg[0] > 0"}}
    };

    auto const text = reporting::detail::stringify_full_match(callReport, expectationReport);

    // note the Adherence reordering
    std::string const regex =
        R"(Matched Call from `.+`#L\d+, `.+`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
	Chose Expectation from `.+`#L\d+, `.+`
	With Adherence\(s\):
	  \+ expect: arg\[0\] > 0
	  \+ expect: arg\[1\] not empty
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "detail::stringify_full_match omits \"Where\"-Section, when no arguments exist.",
    "[reporting][detail]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {{"expect: some requirement"}}
    };

    auto const text = reporting::detail::stringify_full_match(callReport, expectationReport);

    std::string const regex =
        R"(Matched Call from `.+`#L\d+, `.+`
	Chose Expectation from `.+`#L\d+, `.+`
	With Adherence\(s\):
	  \+ expect: some requirement
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "detail::stringify_full_match omits \"With Adherence(s)\"-Section, when no requirements exist.",
    "[reporting][detail]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt
    };

    auto const text = reporting::detail::stringify_full_match(callReport, expectationReport);

    std::string const regex =
        R"(Matched Call from `.+`#L\d+, `.+`
	Chose Expectation from `.+`#L\d+, `.+`
	Without any Requirements.
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "detail::stringify_full_match adds the Stacktrace, if existing.",
    "[reporting][detail]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromLoc = std::source_location::current(),
        .stacktrace = make_shallow_stacktrace(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt
    };

    auto const text = reporting::detail::stringify_full_match(callReport, expectationReport);
    CAPTURE(text);
    constexpr std::string_view stacktraceToken{"Stacktrace:\n"};
    auto const stacktraceBegin = std::ranges::search(text, stacktraceToken).begin();
    REQUIRE(stacktraceBegin != text.cend());
    std::string_view const upper{
        text.cbegin(),
        stacktraceBegin};
    REQUIRE_THAT(
        std::string{upper},
        Catch::Matchers::EndsWith("Without any Requirements.\n\n"));

    std::string const stacktraceRegex =
        R"(Stacktrace:
#0 `.+`#L\d+, `.+`
(?:#\d+ `.*`#L\d+, `.*`\n)*)";
    REQUIRE_THAT(
        (std::string{stacktraceBegin, text.cend()}),
        Catch::Matchers::Matches(stacktraceRegex));
}

#endif
