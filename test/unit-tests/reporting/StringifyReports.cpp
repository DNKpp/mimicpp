//          Copyright Dominic (DNKpp) Koepke 2024 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/reporting/StringifyReports.hpp"

using namespace mimicpp;

namespace
{
    inline reporting::control_state_t const commonApplicableState = reporting::state_applicable{13, 1337, 42};
    inline reporting::control_state_t const commonInapplicableState = reporting::state_inapplicable{13, 1337, 42, {sequence::rating{1}}, {sequence::Tag{1337}}};
    inline reporting::control_state_t const commonSaturatedState = reporting::state_saturated{42, 42, 42};

    [[nodiscard, maybe_unused]]
    Stacktrace make_shallow_stacktrace()
    {
        Stacktrace stacktrace = stacktrace::current();
        CHECK(!stacktrace.empty());

        // When evaluating the stacktrace via regex, this will fail on msvc, because the regex-state becomes too big.
        // So, we limit the stacktrace here, to have a manageable size.
        constexpr std::size_t maxLength{6u};
        const auto size = std::min(maxLength, stacktrace.size());
        const auto skip = stacktrace.size() - size;
        stacktrace = stacktrace::current(skip);
        CHECK(size == stacktrace.size());

        return stacktrace;
    }
}

TEST_CASE(
    "reporting::stringify_full_match converts the information to a pretty formatted text.",
    "[reporting]")
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

    auto const text = reporting::stringify_full_match(callReport, expectationReport);

    // note the Adherence reordering
    std::string const regex =
        R"(Matched Call originated from `.+`#L\d+, `.+`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
	Chose Expectation defined at `.+`#L\d+, `.+`
	With Adherence\(s\):
	  \+ expect: arg\[0\] > 0
	  \+ expect: arg\[1\] not empty
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_full_match omits \"Where\"-Section, when no arguments exist.",
    "[reporting]")
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

    auto const text = reporting::stringify_full_match(callReport, expectationReport);

    std::string const regex =
        R"(Matched Call originated from `.+`#L\d+, `.+`
	Chose Expectation defined at `.+`#L\d+, `.+`
	With Adherence\(s\):
	  \+ expect: some requirement
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_full_match omits \"With Adherence(s)\"-Section, when no requirements exist.",
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

    auto const text = reporting::stringify_full_match(callReport, expectationReport);

    std::string const regex =
        R"(Matched Call originated from `.+`#L\d+, `.+`
	Chose Expectation defined at `.+`#L\d+, `.+`
	Without any Requirements.
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "reporting::stringify_full_match adds the Stacktrace, if existing.",
    "[reporting]")
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

    auto const text = reporting::stringify_full_match(callReport, expectationReport);
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

TEST_CASE(
    "reporting::stringify_inapplicable_matches converts the information to a pretty formatted text.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {
            {{reporting::TypeReport::make<int>(), "1337"},
             {reporting::TypeReport::make<std::string>(), "\"Hello, World!\""}}},
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport1{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = commonInapplicableState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {
                 {"expect: arg[1] not empty",
             std::nullopt,
             "expect: arg[0] > 0"}}
    };

    reporting::ExpectationReport const expectationReport2{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name2"},
        .controlReport = commonSaturatedState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {{"expect: test"}}
    };

    std::vector expectationReports{expectationReport1, expectationReport2};
    auto const text = reporting::stringify_inapplicable_matches(callReport, expectationReports);

    // note the Adherence reordering
    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
2 inapplicable but otherwise matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Because it's not head of 1 Sequence\(s\) \(2 total\).
	With Adherence\(s\):
	  \+ expect: arg\[0\] > 0
	  \+ expect: arg\[1\] not empty

	#2 Expectation defined at `.+`#L\d+, `.+`
	Because it's already saturated \(matched 42 out of 42 times\).
	With Adherence\(s\):
	  \+ expect: test
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_inapplicable_matches omits \"Where\"-Section, when no arguments exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = commonSaturatedState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {{"expect: some requirement"}}
    };

    std::vector expectationReports{expectationReport};
    auto const text = reporting::stringify_inapplicable_matches(callReport, expectationReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
1 inapplicable but otherwise matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Because it's already saturated \(matched 42 out of 42 times\).
	With Adherence\(s\):
	  \+ expect: some requirement
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_inapplicable_matches omits \"With Adherence(s)\"-Section, when no requirements exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = commonSaturatedState,
        .finalizerDescription = std::nullopt
    };

    std::vector expectationReports{expectationReport};
    auto const text = reporting::stringify_inapplicable_matches(callReport, expectationReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
1 inapplicable but otherwise matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Because it's already saturated \(matched 42 out of 42 times\).
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "reporting::stringify_inapplicable_matches adds the Stacktrace, if existing.",
    "[reporting]")
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
        .controlReport = commonInapplicableState,
        .finalizerDescription = std::nullopt
    };

    std::vector expectationReports{expectationReport};
    auto const text = reporting::stringify_inapplicable_matches(callReport, expectationReports);
    CAPTURE(text);
    auto const stacktraceBegin = std::ranges::search(text, std::string_view{"Stacktrace:\n"}).begin();
    REQUIRE(stacktraceBegin != text.cend());
    REQUIRE_THAT(
        (std::string{text.cbegin(), stacktraceBegin}),
        Catch::Matchers::EndsWith("Because it's not head of 1 Sequence(s) (2 total).\n\n"));

    std::string const stacktraceRegex =
        R"(Stacktrace:
#0 `.+`#L\d+, `.+`
(?:#\d+ `.*`#L\d+, `.*`\n)*)";
    REQUIRE_THAT(
        (std::string{stacktraceBegin, text.cend()}),
        Catch::Matchers::Matches(stacktraceRegex));
}

#endif

TEST_CASE(
    "reporting::stringify_no_matches converts the information to a pretty formatted text.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {
            {{reporting::TypeReport::make<int>(), "1337"},
             {reporting::TypeReport::make<std::string>(), "\"Hello, World!\""}}},
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport1{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {
                 {std::nullopt,
             "expect: arg[1] not empty",
             std::nullopt,
             "expect: arg[0] > 0",
             std::nullopt}}
    };
    reporting::RequirementOutcomes const outcomes1{
        .outcomes = {{false, true, false, false, true}}};

    reporting::ExpectationReport const expectationReport2{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name2"},
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {
                 {"expect: violated",
             "expect: adhered"}}
    };
    reporting::RequirementOutcomes const outcomes2{
        .outcomes = {false, true}
    };

    std::vector noMatchReports{
        reporting::NoMatchReport{expectationReport1, outcomes1},
        reporting::NoMatchReport{expectationReport2, outcomes2}
    };
    auto const text = reporting::stringify_no_matches(callReport, noMatchReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
2 non-matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Due to Violation\(s\):
	  \- expect: arg\[0\] > 0
	  \- 2 Requirement\(s\) failed without further description\.
	With Adherence\(s\):
	  \+ expect: arg\[1\] not empty

	#2 Expectation defined at `.+`#L\d+, `.+`
	Due to Violation\(s\):
	  \- expect: violated
	With Adherence\(s\):
	  \+ expect: adhered
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_no_matches omits \"Where\"-Section, when no arguments exist.",
    "[reporting]")
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
        .requirementDescriptions = {
                 {
                "expect: adherence",
                "expect: violation",
            }}
    };
    reporting::RequirementOutcomes const outcomes{
        .outcomes = {true, false}
    };

    std::vector noMatchReports{
        reporting::NoMatchReport{expectationReport, outcomes},
    };
    auto const text = reporting::stringify_no_matches(callReport, noMatchReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
1 non-matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Due to Violation\(s\):
	  \- expect: violation
	With Adherence\(s\):
	  \+ expect: adherence
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_no_matches omits \"With Adherence(s)\"-Section, when no requirements exist.",
    "[reporting]")
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
        .requirementDescriptions = {{"expect: violation"}}
    };

    reporting::RequirementOutcomes const outcomes{
        .outcomes = {false}};

    std::vector noMatchReports{
        reporting::NoMatchReport{expectationReport, outcomes}
    };
    auto const text = reporting::stringify_no_matches(callReport, noMatchReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
1 non-matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Due to Violation\(s\):
	  \- expect: violation
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_no_matches has special treatment, when no expectations exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromLoc = std::source_location::current(),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    std::vector<reporting::NoMatchReport> noMatchReports{};
    auto const text = reporting::stringify_no_matches(callReport, noMatchReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
No Expectations available!
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "reporting::stringify_no_matches adds the Stacktrace, if existing.",
    "[reporting]")
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
        .requirementDescriptions = {{"expect: violation"}}
    };

    reporting::RequirementOutcomes const outcomes{
        .outcomes = {false}};

    std::vector noMatchReports{
        reporting::NoMatchReport{expectationReport, outcomes}
    };
    auto const text = reporting::stringify_no_matches(callReport, noMatchReports);
    CAPTURE(text);
    auto const stacktraceBegin = std::ranges::search(text, std::string_view{"Stacktrace:\n"}).begin();
    REQUIRE(stacktraceBegin != text.cend());
    REQUIRE_THAT(
        (std::string{text.cbegin(), stacktraceBegin}),
        Catch::Matchers::EndsWith("- expect: violation\n\n"));

    std::string const stacktraceRegex =
        R"(Stacktrace:
#0 `.+`#L\d+, `.+`
(?:#\d+ `.*`#L\d+, `.*`\n)*)";
    REQUIRE_THAT(
        (std::string{stacktraceBegin, text.cend()}),
        Catch::Matchers::Matches(stacktraceRegex));
}

#endif

TEST_CASE(
    "reporting::stringify_unfulfilled_expectation converts the information to a pretty formatted text.",
    "[reporting]")
{
    static constexpr auto maxSize = std::numeric_limits<int>::max();
    auto [expectedTimesText, expectedDiff, state] = GENERATE(
        (table<std::string, int, reporting::control_state_t>)({
            {           "exactly once", 1,         reporting::state_applicable{.min = 1, .max = 1, .count = 0}},
            {          "exactly twice", 2,         reporting::state_applicable{.min = 2, .max = 2, .count = 0}},
            {       "exactly 42 times", 5,      reporting::state_applicable{.min = 42, .max = 42, .count = 37}},

            {  "between 1 and 2 times", 1,         reporting::state_applicable{.min = 1, .max = 2, .count = 0}},
            {"between 42 and 47 times", 5,      reporting::state_applicable{.min = 42, .max = 47, .count = 37}},

            {          "at least once", 1,   reporting::state_applicable{.min = 1, .max = maxSize, .count = 0}},
            {         "at least twice", 2,   reporting::state_applicable{.min = 2, .max = maxSize, .count = 0}},
            {      "at least 42 times", 5, reporting::state_applicable{.min = 42, .max = maxSize, .count = 37}}
    }));

    reporting::ExpectationReport const expectationReport{
        .info = {.sourceLocation = std::source_location::current(), .mockName = "Mock-Name"},
        .controlReport = state
    };

    auto const text = reporting::stringify_unfulfilled_expectation(expectationReport);
    std::string const regex = format::format(
        R"(Unfulfilled Expectation defined at `.+`#L\d+, `.+`
	Because matching {} was expected => requires {} further match\(es\)\.
)",
        expectedTimesText,
        expectedDiff);
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_unhandled_exception converts the information to a pretty formatted text.",
    "[reporting]")
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
        .finalizerDescription = std::nullopt
    };

    SECTION("When std::exception is given.")
    {
        auto const exceptionPtr = std::make_exception_ptr(std::runtime_error{"Something went wrong."});
        auto const text = reporting::stringify_unhandled_exception(
            callReport,
            expectationReport,
            exceptionPtr);

        std::string const regex =
            R"(Unhandled Exception with message `Something went wrong\.`
	While checking Expectation defined at `.+`#L\d+, `.+`
For Call originated from `.+`#L\d+, `.+`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
)";
        REQUIRE_THAT(
            text,
            Catch::Matchers::Matches(regex));
    }

    SECTION("When unknown exception is given.")
    {
        auto const exceptionPtr = std::make_exception_ptr(42);
        auto const text = reporting::stringify_unhandled_exception(
            callReport,
            expectationReport,
            exceptionPtr);

        std::string const regex =
            R"(Unhandled Exception of unknown type\.
	While checking Expectation defined at `.+`#L\d+, `.+`
For Call originated from `.+`#L\d+, `.+`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
)";
        REQUIRE_THAT(
            text,
            Catch::Matchers::Matches(regex));
    }
}

TEST_CASE(
    "reporting::stringify_unhandled_exception omits \"Where\"-Section, when no arguments exist.",
    "[reporting]")
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

    auto const exceptionPtr = std::make_exception_ptr(std::runtime_error{"Something went wrong."});
    auto const text = reporting::stringify_unhandled_exception(
        callReport,
        expectationReport,
        exceptionPtr);

    std::string const regex =
        R"(Unhandled Exception with message `Something went wrong\.`
	While checking Expectation defined at `.+`#L\d+, `.+`
For Call originated from `.+`#L\d+, `.+`
)";
    REQUIRE_THAT(
        text,
        Catch::Matchers::Matches(regex));
}
