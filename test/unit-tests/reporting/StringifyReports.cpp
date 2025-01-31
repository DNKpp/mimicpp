//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/StringifyReports.hpp"

using namespace mimicpp;

namespace
{
    inline reporting::control_state_t const commonApplicableState = reporting::state_applicable{13, 1337, 42};
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

    std::string const regex =
        R"(Matched Call from .+\[\d+(?::\d+)?\], .+
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
	Chose Expectation from .+\[\d+(?::\d+)?\], .+
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
        R"(Matched Call from .+\[\d+(?::\d+)?\], .+
	Chose Expectation from .+\[\d+(?::\d+)?\], .+
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
        R"(Matched Call from .+\[\d+(?::\d+)?\], .+
	Chose Expectation from .+\[\d+(?::\d+)?\], .+
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
        .stacktrace = stacktrace::current(),
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
    auto const stacktraceBegin = std::ranges::search(text, stacktraceToken).cbegin();
    REQUIRE(stacktraceBegin != text.cend());
    std::string_view const upper{
        text.cbegin(),
        stacktraceBegin};
    REQUIRE_THAT(
        std::string{upper},
        Catch::Matchers::EndsWith("Without any Requirements.\n\n"));

    // thanks to msvc's poor regex implementation, I must parse the entries line by line...
    std::string_view stacktracePart{stacktraceBegin, text.cend()};
    REQUIRE_THAT(
        std::string{stacktracePart},
        Catch::Matchers::StartsWith(std::string{stacktraceToken}));
    stacktracePart.remove_prefix(stacktraceToken.size());

    constexpr std::string_view newlineToken{"\n"};
    for (auto const i : std::views::iota(0u, callReport.stacktrace.size()))
    {
        std::ostringstream ss{};
        // I'm just interested in the general pattern: #i {file.name}[{line}], {text}
        // {file.name} and {text} may be empty for deeper entries
        ss << "#" << i << " "
           << (i < 3 ? ".+\\[\\d+\\], .+" : ".*\\[\\d+\\], .*")
           << "\n";
        auto const lineEnd = std::ranges::search(stacktracePart, newlineToken).cend();
        std::string line{stacktracePart.cbegin(), lineEnd};
        REQUIRE_THAT(
            line,
            Catch::Matchers::Matches(ss.str()));
        stacktracePart.remove_prefix(line.size());
    }

    REQUIRE_THAT(
        stacktracePart,
        Catch::Matchers::IsEmpty());
}

#endif
