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

    [[maybe_unused]] constexpr std::string_view stacktraceToken{"Stacktrace:\n"};

    template <typename Signature>
    [[nodiscard]]
    reporting::TargetReport make_common_target_report(StringT name = "Mock-Name")
    {
        return reporting::TargetReport{
            .name = std::move(name),
            .overloadReport = reporting::TypeReport::make<Signature>()};
    }
}

TEST_CASE(
    "reporting::stringify_full_match converts the information to a pretty formatted text.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void(int, std::string)>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {
            {{reporting::TypeReport::make<int>(), "1337"},
             {reporting::TypeReport::make<std::string>(), "\"Hello, World!\""}}},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void(int, std::string)>(),
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
	On Target `Mock-Name` used Overload `void\(int, std::string\)`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
	Chose Expectation defined at `.+`#L\d+, `.+`
	With Adherence\(s\):
	  \+ expect: arg\[0\] > 0
	  \+ expect: arg\[1\] not empty
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_full_match omits \"Where\"-Section, when no arguments exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {{"expect: some requirement"}}
    };

    auto const text = reporting::stringify_full_match(callReport, expectationReport);

    std::string const regex =
        R"(Matched Call originated from `.+`#L\d+, `.+`
	On Target `Mock-Name` used Overload `void\(\)`
	Chose Expectation defined at `.+`#L\d+, `.+`
	With Adherence\(s\):
	  \+ expect: some requirement
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_full_match omits \"With Adherence(s)\"-Section, when no requirements exist.",
    "[reporting][detail]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt
    };

    auto const text = reporting::stringify_full_match(callReport, expectationReport);

    std::string const regex =
        R"(Matched Call originated from `.+`#L\d+, `.+`
	On Target `Mock-Name` used Overload `void\(\)`
	Chose Expectation defined at `.+`#L\d+, `.+`
	Without any Requirements.
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "reporting::stringify_full_match adds the Stacktrace, if existing.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .stacktrace = stacktrace::current(0u, 5u),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt
    };

    auto const text = reporting::stringify_full_match(callReport, expectationReport);
    CAPTURE(text);
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
#0 `.*StringifyReports.cpp`#L\d+, `.+`
(?:#\d+ `.*`#L\d+, `.*`\n){4})";
    CHECK_THAT(
        (std::string{stacktraceBegin, text.cend()}),
        Catch::Matchers::Matches(stacktraceRegex));
}

#endif

TEST_CASE(
    "reporting::stringify_inapplicable_matches converts the information to a pretty formatted text.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void(int, std::string)>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {
            {{reporting::TypeReport::make<int>(), "1337"},
             {reporting::TypeReport::make<std::string>(), "\"Hello, World!\""}}},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport1{
        .target = make_common_target_report<void(int, std::string)>(),
        .controlReport = commonInapplicableState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {
                 {"expect: arg[1] not empty",
             std::nullopt,
             "expect: arg[0] > 0"}}
    };

    reporting::ExpectationReport const expectationReport2{
        .target = make_common_target_report<void(int, std::string)>("Mock-Name2"),
        .controlReport = commonSaturatedState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {{"expect: test"}}
    };

    std::vector expectationReports{expectationReport1, expectationReport2};
    auto const text = reporting::stringify_inapplicable_matches(callReport, expectationReports);

    // note the Adherence reordering
    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
	On Target `Mock-Name` used Overload `void\(int, std::string\)`
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
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_inapplicable_matches omits \"Where\"-Section, when no arguments exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonSaturatedState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {{"expect: some requirement"}}
    };

    std::vector expectationReports{expectationReport};
    auto const text = reporting::stringify_inapplicable_matches(callReport, expectationReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
	On Target `Mock-Name` used Overload `void\(\)`
1 inapplicable but otherwise matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Because it's already saturated \(matched 42 out of 42 times\).
	With Adherence\(s\):
	  \+ expect: some requirement
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_inapplicable_matches omits \"With Adherence(s)\"-Section, when no requirements exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonSaturatedState,
        .finalizerDescription = std::nullopt
    };

    std::vector expectationReports{expectationReport};
    auto const text = reporting::stringify_inapplicable_matches(callReport, expectationReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
	On Target `Mock-Name` used Overload `void\(\)`
1 inapplicable but otherwise matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Because it's already saturated \(matched 42 out of 42 times\).
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "reporting::stringify_inapplicable_matches adds the Stacktrace, if existing.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .stacktrace = stacktrace::current(0u, 5u),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonInapplicableState,
        .finalizerDescription = std::nullopt
    };

    std::vector expectationReports{expectationReport};
    auto const text = reporting::stringify_inapplicable_matches(callReport, expectationReports);
    CAPTURE(text);
    auto const stacktraceBegin = std::ranges::search(text, stacktraceToken).begin();
    REQUIRE(stacktraceBegin != text.cend());
    REQUIRE_THAT(
        (std::string{text.cbegin(), stacktraceBegin}),
        Catch::Matchers::EndsWith("Because it's not head of 1 Sequence(s) (2 total).\n\n"));

    std::string const stacktraceRegex =
        R"(Stacktrace:
#0 `.*StringifyReports.cpp`#L\d+, `.+`
(?:#\d+ `.*`#L\d+, `.*`\n){4})";
    CHECK_THAT(
        (std::string{stacktraceBegin, text.cend()}),
        Catch::Matchers::Matches(stacktraceRegex));
}

#endif

TEST_CASE(
    "reporting::stringify_no_matches converts the information to a pretty formatted text.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void(int, std::string)>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {
            {{reporting::TypeReport::make<int>(), "1337"},
             {reporting::TypeReport::make<std::string>(), "\"Hello, World!\""}}},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport1{
        .target = make_common_target_report<void(int, std::string)>(),
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
        .target = make_common_target_report<void()>("Mock-Name2"),
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
	On Target `Mock-Name` used Overload `void\(int, std::string\)`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
2 applicable non-matching Expectation\(s\):
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
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_no_matches filters reports from inapplicable expectations.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport1{
        .target = make_common_target_report<void()>(),
        .controlReport = GENERATE(commonInapplicableState, commonSaturatedState),
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {{"expect: violation1"}}};
    reporting::RequirementOutcomes const outcomes1{
        .outcomes = {false}};

    reporting::ExpectationReport const expectationReport2{
        .target = make_common_target_report<void()>(),
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt,
        .requirementDescriptions = {{"expect: violation2"}}};
    reporting::RequirementOutcomes const outcomes2{
        .outcomes = {false}};

    std::vector noMatchReports{
        reporting::NoMatchReport{expectationReport1, outcomes1},
        reporting::NoMatchReport{expectationReport2, outcomes2}
    };
    auto const text = reporting::stringify_no_matches(callReport, noMatchReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
	On Target `Mock-Name` used Overload `void\(\)`
1 applicable non-matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Due to Violation\(s\):
	  \- expect: violation2
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_no_matches omits \"Where\"-Section, when no arguments exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
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
	On Target `Mock-Name` used Overload `void\(\)`
1 applicable non-matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Due to Violation\(s\):
	  \- expect: violation
	With Adherence\(s\):
	  \+ expect: adherence
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_no_matches omits \"With Adherence(s)\"-Section, when no requirements exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
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
	On Target `Mock-Name` used Overload `void\(\)`
1 applicable non-matching Expectation\(s\):
	#1 Expectation defined at `.+`#L\d+, `.+`
	Due to Violation\(s\):
	  \- expect: violation
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_no_matches has special treatment, when no applicable expectations exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    std::vector<reporting::NoMatchReport> noMatchReports{};

    SECTION("When no reports are contained.")
    {
    }

    SECTION("When inapplicable match reports are contained.")
    {
        reporting::ExpectationReport const expectationReport{
            .target = make_common_target_report<void()>(),
            .controlReport = commonInapplicableState,
            .requirementDescriptions = {{"expect: violation"}}};

        reporting::RequirementOutcomes const outcomes{
            .outcomes = {false}};

        noMatchReports.emplace_back(expectationReport, outcomes);
    }

    SECTION("When saturated match reports are contained.")
    {
        reporting::ExpectationReport const expectationReport{
            .target = make_common_target_report<void()>(),
            .controlReport = commonSaturatedState,
            .requirementDescriptions = {{"expect: violation"}}};

        reporting::RequirementOutcomes const outcomes{
            .outcomes = {false}};

        noMatchReports.emplace_back(expectationReport, outcomes);
    }

    auto const text = reporting::stringify_no_matches(callReport, noMatchReports);

    std::string const regex =
        R"(Unmatched Call originated from `.+`#L\d+, `.+`
	On Target `Mock-Name` used Overload `void\(\)`
No applicable Expectations available!
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "reporting::stringify_no_matches adds the Stacktrace, if existing.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .stacktrace = stacktrace::current(0u, 5u),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
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
    auto const stacktraceBegin = std::ranges::search(text, stacktraceToken).begin();
    REQUIRE(stacktraceBegin != text.cend());
    REQUIRE_THAT(
        (std::string{text.cbegin(), stacktraceBegin}),
        Catch::Matchers::EndsWith("- expect: violation\n\n"));

    std::string const stacktraceRegex =
        R"(Stacktrace:
#0 `.*StringifyReports.cpp`#L\d+, `.+`
(?:#\d+ `.*`#L\d+, `.*`\n){4})";
    CHECK_THAT(
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
            {           "exactly once", 1,                                                           reporting::state_applicable{.min = 1, .max = 1, .count = 0}},
            {          "exactly twice", 2,                                                           reporting::state_applicable{.min = 2, .max = 2, .count = 0}},
            {       "exactly 42 times", 5,                                                        reporting::state_applicable{.min = 42, .max = 42, .count = 37}},

            {  "between 1 and 2 times", 1,                                                           reporting::state_applicable{.min = 1, .max = 2, .count = 0}},
            {"between 42 and 47 times", 5,                                                        reporting::state_applicable{.min = 42, .max = 47, .count = 37}},

            {          "at least once", 1,                                                     reporting::state_applicable{.min = 1, .max = maxSize, .count = 0}},
            {         "at least twice", 2,                                                     reporting::state_applicable{.min = 2, .max = maxSize, .count = 0}},
            {      "at least 42 times", 5,                                                   reporting::state_applicable{.min = 42, .max = maxSize, .count = 37}},

            {           "exactly once", 1,         reporting::state_inapplicable{.min = 1, .max = 1, .count = 0, .inapplicableSequences = {sequence::Tag{1337}}}},
            {          "exactly twice", 2,         reporting::state_inapplicable{.min = 2, .max = 2, .count = 0, .inapplicableSequences = {sequence::Tag{1337}}}},
            {       "exactly 42 times", 5,      reporting::state_inapplicable{.min = 42, .max = 42, .count = 37, .inapplicableSequences = {sequence::Tag{1337}}}},

            {  "between 1 and 2 times", 1,         reporting::state_inapplicable{.min = 1, .max = 2, .count = 0, .inapplicableSequences = {sequence::Tag{1337}}}},
            {"between 42 and 47 times", 5,      reporting::state_inapplicable{.min = 42, .max = 47, .count = 37, .inapplicableSequences = {sequence::Tag{1337}}}},

            {          "at least once", 1,   reporting::state_inapplicable{.min = 1, .max = maxSize, .count = 0, .inapplicableSequences = {sequence::Tag{1337}}}},
            {         "at least twice", 2,   reporting::state_inapplicable{.min = 2, .max = maxSize, .count = 0, .inapplicableSequences = {sequence::Tag{1337}}}},
            {      "at least 42 times", 5, reporting::state_inapplicable{.min = 42, .max = maxSize, .count = 37, .inapplicableSequences = {sequence::Tag{1337}}}}
    }));

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
        .controlReport = state
    };

    auto const text = reporting::stringify_unfulfilled_expectation(expectationReport);
    std::string const regex = format::format(
        R"(Unfulfilled Expectation defined at `.+`#L\d+, `.+`
	Of Target `Mock-Name` related to Overload `void\(\)`
	Because matching {} was expected => requires {} further match\(es\)\.
)",
        expectedTimesText,
        expectedDiff);
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

TEST_CASE(
    "reporting::stringify_unhandled_exception converts the information to a pretty formatted text.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void(int, std::string)>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {
            {{reporting::TypeReport::make<int>(), "1337"},
             {reporting::TypeReport::make<std::string>(), "\"Hello, World!\""}}},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void(int, std::string)>(),
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
	On Target `Mock-Name` used Overload `void\(int, std::string\)`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
)";
        CHECK_THAT(
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
	On Target `Mock-Name` used Overload `void\(int, std::string\)`
	Where:
		arg\[0\] => int: 1337
		arg\[1\] => std::string: "Hello, World!"
)";
        CHECK_THAT(
            text,
            Catch::Matchers::Matches(regex));
    }
}

TEST_CASE(
    "reporting::stringify_unhandled_exception omits \"Where\"-Section, when no arguments exist.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
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
	On Target `Mock-Name` used Overload `void\(\)`
)";
    CHECK_THAT(
        text,
        Catch::Matchers::Matches(regex));
}

#if MIMICPP_DETAIL_HAS_WORKING_STACKTRACE_BACKEND

TEST_CASE(
    "reporting::stringify_unhandled_exception adds the Stacktrace, if existing.",
    "[reporting]")
{
    reporting::CallReport const callReport{
        .target = make_common_target_report<void()>(),
        .returnTypeInfo = reporting::TypeReport::make<void>(),
        .argDetails = {},
        .stacktrace = stacktrace::current(0u, 5u),
        .fromCategory = ValueCategory::any,
        .fromConstness = Constness::any};

    reporting::ExpectationReport const expectationReport{
        .target = make_common_target_report<void()>(),
        .controlReport = commonApplicableState,
        .finalizerDescription = std::nullopt};
    auto const exceptionPtr = std::make_exception_ptr(std::runtime_error{"Something went wrong."});
    auto const text = reporting::stringify_unhandled_exception(
        callReport,
        expectationReport,
        exceptionPtr);
    CAPTURE(text);
    auto const stacktraceBegin = std::ranges::search(text, stacktraceToken).begin();
    REQUIRE(stacktraceBegin != text.cend());
    REQUIRE_THAT(
        (std::string{text.cbegin(), stacktraceBegin}),
        Catch::Matchers::EndsWith("On Target `Mock-Name` used Overload `void()`\n\n"));

    std::string const stacktraceRegex =
        R"(Stacktrace:
#0 `.*StringifyReports.cpp`#L\d+, `.+`
(?:#\d+ `.*`#L\d+, `.*`\n){4})";
    CHECK_THAT(
        (std::string{stacktraceBegin, text.cend()}),
        Catch::Matchers::Matches(stacktraceRegex));
}

#endif
