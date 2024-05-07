// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"

#include "mimic++/Expectation.hpp"
#include "mimic++/Printer.hpp"

#include "TestTypes.hpp"

#include <functional>
#include <optional>
#include <ranges>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace
{
	class ExpectationMock final
		: public mimicpp::Expectation<void()>
	{
	public:
		using CallInfoT = mimicpp::call::info_for_signature_t<void()>;

		MAKE_CONST_MOCK0(report, mimicpp::ExpectationReport(), override);
		MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept override);
		MAKE_CONST_MOCK1(matches, mimicpp::MatchReport(const CallInfoT&), override);
		MAKE_MOCK1(consume, void(const CallInfoT&), override);
		MAKE_MOCK1(finalize_call, void(const CallInfoT&), override);
	};
}

TEST_CASE(
	"mimicpp::ExpectationCollection collects expectations and reports when they are removed but unfulfilled.",
	"[expectation]"
)
{
	using StorageT = mimicpp::ExpectationCollection<void()>;

	StorageT storage{};
	auto expectation = std::make_shared<ExpectationMock>();

	REQUIRE_NOTHROW(storage.push(expectation));

	ScopedReporter reporter{};
	SECTION("When expectation is satisfied, nothing is reported.")
	{
		REQUIRE_CALL(*expectation, is_satisfied())
			.RETURN(true);
		REQUIRE_NOTHROW(storage.remove(expectation));
		REQUIRE_THAT(
			reporter.unfulfilled_expectations(),
			Catch::Matchers::IsEmpty());
	}

	SECTION("When expectation is unfulfilled, get's reported.")
	{
		const mimicpp::ExpectationReport expReport{
			.timesDescription = "times description"
		};

		REQUIRE_CALL(*expectation, is_satisfied())
			.RETURN(false);
		REQUIRE_CALL(*expectation, report())
			.RETURN(expReport);
		REQUIRE_NOTHROW(storage.remove(expectation));
		REQUIRE_THAT(
			reporter.unfulfilled_expectations(),
			Catch::Matchers::SizeIs(1));
		REQUIRE(expReport == reporter.unfulfilled_expectations().at(0));
	}
}

namespace
{
	inline const mimicpp::MatchReport commonNoMatchReport{
		.timesReport = {
			.isApplicable = true
		},
		.expectationReports = {
			{
				.isMatching = false
			}
		}
	};

	inline const mimicpp::MatchReport commonFullMatchReport{
		.timesReport = {
			.isApplicable = true
		},
		.expectationReports = {}
	};

	inline const mimicpp::MatchReport commonInapplicableMatchReport{
		.timesReport = {
			.isApplicable = false
		},
		.expectationReports = {}
	};
}

TEST_CASE(
	"mimicpp::ExpectationCollection queries its expectations, whether they match the call, in reverse order of construction.",
	"[expectation]"
)
{
	using namespace mimicpp::call;
	using StorageT = mimicpp::ExpectationCollection<void()>;
	using CallInfoT = Info<void>;
	using trompeloeil::_;

	ScopedReporter reporter{};
	StorageT storage{};
	std::vector<std::shared_ptr<ExpectationMock>> expectations(4);
	for (auto& exp : expectations)
	{
		exp = std::make_shared<ExpectationMock>();
		storage.push(exp);
	}

	constexpr CallInfoT call{
		.args = {},
		.fromCategory = mimicpp::ValueCategory::any,
		.fromConstness = mimicpp::Constness::any
	};

	SECTION("If a full match is found.")
	{
		trompeloeil::sequence sequence{};
		REQUIRE_CALL(*expectations[3], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(commonNoMatchReport);
		REQUIRE_CALL(*expectations[2], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(commonNoMatchReport);
		REQUIRE_CALL(*expectations[1], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(commonFullMatchReport);
		// expectations[3] is never queried
		REQUIRE_CALL(*expectations[1], consume(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence);
		REQUIRE_CALL(*expectations[1], finalize_call(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence);

		REQUIRE_NOTHROW(storage.handle_call(call));
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.inapplicable_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.full_match_reports(),
			Catch::Matchers::SizeIs(1));
	}

	SECTION("If at least one matches but is inapplicable.")
	{
		using match_report_t = mimicpp::MatchReport;
		const auto [count, result0, result1, result2, result3] = GENERATE(
			(table<std::size_t, match_report_t, match_report_t, match_report_t, match_report_t>)(
				{
				{1u, commonInapplicableMatchReport, commonNoMatchReport, commonNoMatchReport, commonNoMatchReport},
				{1u, commonNoMatchReport, commonInapplicableMatchReport, commonNoMatchReport, commonNoMatchReport},
				{1u, commonNoMatchReport, commonNoMatchReport, commonInapplicableMatchReport, commonNoMatchReport},
				{1u, commonNoMatchReport, commonNoMatchReport, commonNoMatchReport, commonInapplicableMatchReport},

				{2u, commonInapplicableMatchReport, commonNoMatchReport, commonInapplicableMatchReport, commonNoMatchReport},
				{2u, commonNoMatchReport, commonInapplicableMatchReport, commonNoMatchReport, commonInapplicableMatchReport},
				{3u, commonInapplicableMatchReport, commonNoMatchReport, commonInapplicableMatchReport, commonInapplicableMatchReport},
				{4u, commonInapplicableMatchReport, commonInapplicableMatchReport, commonInapplicableMatchReport,
				commonInapplicableMatchReport}
				}));

		trompeloeil::sequence sequence{};
		REQUIRE_CALL(*expectations[3], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(result0);
		REQUIRE_CALL(*expectations[2], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(result1);
		REQUIRE_CALL(*expectations[1], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(result2);
		REQUIRE_CALL(*expectations[0], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(result3);

		REQUIRE_THROWS_AS(
			storage.handle_call(call),
			NonApplicableMatchError);
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.inapplicable_match_reports(),
			Catch::Matchers::SizeIs(count));
		REQUIRE_THAT(
			reporter.full_match_reports(),
			Catch::Matchers::IsEmpty());
	}

	SECTION("If none matches.")
	{
		trompeloeil::sequence sequence{};
		REQUIRE_CALL(*expectations[3], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(commonNoMatchReport);
		REQUIRE_CALL(*expectations[2], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(commonNoMatchReport);
		REQUIRE_CALL(*expectations[1], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(commonNoMatchReport);
		REQUIRE_CALL(*expectations[0], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(commonNoMatchReport);

		REQUIRE_THROWS_AS(
			storage.handle_call(call),
			NoMatchError);
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Catch::Matchers::SizeIs(4));
		REQUIRE_THAT(
			reporter.inapplicable_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.full_match_reports(),
			Catch::Matchers::IsEmpty());
	}
}

TEST_CASE(
	"Unhandled exceptions during mimicpp::ExpectationCollection::handle_call are reported.",
	"[expectation]"
)
{
	namespace Matches = Catch::Matchers;

	using namespace mimicpp::call;
	using StorageT = mimicpp::ExpectationCollection<void()>;
	using CallInfoT = Info<void>;
	using trompeloeil::_;

	ScopedReporter reporter{};
	StorageT storage{};
	auto throwingExpectation = std::make_shared<ExpectationMock>();
	auto otherExpectation = std::make_shared<ExpectationMock>();
	storage.push(otherExpectation);
	storage.push(throwingExpectation);

	constexpr CallInfoT call{
		.args = {},
		.fromCategory = mimicpp::ValueCategory::any,
		.fromConstness = mimicpp::Constness::any
	};

	struct Exception
	{
	};

	const mimicpp::ExpectationReport throwingReport{
		.timesDescription = "times description"
	};

	const auto matches = [&](const auto& info)
	{
		try
		{
			std::rethrow_exception(info.exception);
		}
		catch (const Exception&)
		{
			return info.call == mimicpp::make_call_report(call)
				&& info.expectation ==throwingReport;
		}
		catch (...)
		{
			return false;
		}
	};

	SECTION("When an exception is thrown during matches.")
	{
		REQUIRE_CALL(*throwingExpectation, matches(_))
			.THROW(Exception{});
		REQUIRE_CALL(*throwingExpectation, report())
			.RETURN(throwingReport);
		REQUIRE_CALL(*otherExpectation, matches(_))
			.RETURN(commonFullMatchReport);
		REQUIRE_CALL(*otherExpectation, consume(_));
		REQUIRE_CALL(*otherExpectation, finalize_call(_));
		REQUIRE_NOTHROW(storage.handle_call(call));

		CHECK_THAT(
			reporter.full_match_reports(),
			Matches::SizeIs(1));
		CHECK_THAT(
			reporter.no_match_reports(),
			Matches::IsEmpty());
		CHECK_THAT(
			reporter.inapplicable_match_reports(),
			Matches::IsEmpty());

		REQUIRE_THAT(
			reporter.unhandled_exceptions(),
			Matches::SizeIs(1));
		REQUIRE(matches(reporter.unhandled_exceptions().front()));
	}
}

TEMPLATE_TEST_CASE_SIG(
	"mimicpp::expectation_policy_for determines, whether the type can be used in combination with the given signature.",
	"[expectation]",
	((bool expected, typename Policy, typename Signature), expected, Policy, Signature),
	(false, PolicyFake<void()>, int()),	// incompatible return type
	(false, PolicyFake<void()>, void(int)),	// incompatible param
	(true, PolicyFake<void()>, void()),
	(true, PolicyFacade<void(), std::reference_wrapper<PolicyFake<void()>>, UnwrapReferenceWrapper>, void())
)
{
	STATIC_REQUIRE(expected == mimicpp::expectation_policy_for<Policy, Signature>);
}

TEMPLATE_TEST_CASE_SIG(
	"mimicpp::finalize_policy_for determines, whether the type can be used in combination with the given signature.",
	"[expectation]",
	((bool expected, typename Policy, typename Signature), expected, Policy, Signature),
	(false, FinalizerFake<void()>, int()),	// incompatible return type
	(false, FinalizerFake<void()>, void(int)),	// incompatible param
	(true, FinalizerFake<void()>, void()),
	(true, FinalizerFacade<void(), std::reference_wrapper<FinalizerFake<void()>>, UnwrapReferenceWrapper>, void())
)
{
	STATIC_REQUIRE(expected == mimicpp::finalize_policy_for<Policy, Signature>);
}

TEMPLATE_TEST_CASE_SIG(
	"mimicpp::times_policy determines, whether the type satisfies the requirements.",
	"[expectation]",
	((bool expected, typename Policy), expected, Policy),
	(true, TimesFake),
	(true, TimesFacade<std::reference_wrapper<TimesFake>, UnwrapReferenceWrapper>)
)
{
	STATIC_REQUIRE(expected == mimicpp::times_policy<Policy>);
}

TEST_CASE(
	"mimicpp::BasicExpectation stores std::source_location.",
	"[expectation]"
)
{
	using TimesT = TimesFake;
	using FinalizerT = FinalizerFake<void()>;

	constexpr auto loc = std::source_location::current();

	mimicpp::BasicExpectation<void(), TimesT, FinalizerT> expectation{
		loc,
		TimesT{},
		FinalizerT{}
	};

	REQUIRE(mimicpp::is_same_source_location(loc, expectation.from()));
}

TEST_CASE(
	"Times policy of mimicpp::BasicExpectation controls, how often its expectations must be matched.",
	"[expectation]"
)
{
	using trompeloeil::_;
	using SignatureT = void();
	using FinalizerT = FinalizerFake<SignatureT>;
	using PolicyMockT = PolicyMock<SignatureT>;
	using PolicyRefT = PolicyFacade<SignatureT, std::reference_wrapper<PolicyMock<SignatureT>>, UnwrapReferenceWrapper>;
	using TimesPolicyT = TimesFacade<std::reference_wrapper<TimesMock>, UnwrapReferenceWrapper>;
	using CallInfoT = mimicpp::call::info_for_signature_t<SignatureT>;
	using TimesReportT = mimicpp::MatchReport::Times;

	const CallInfoT call{
		.args = {},
		.fromCategory = mimicpp::ValueCategory::any,
		.fromConstness = mimicpp::Constness::any
	};

	TimesMock times{};

	SECTION("With no other expectation policies.")
	{
		mimicpp::BasicExpectation<SignatureT, TimesPolicyT, FinalizerT> expectation{
			std::source_location::current(),
			std::ref(times),
			FinalizerT{}
		};

		SECTION("When calling is_satisfied.")
		{
			const bool isSatisfied = GENERATE(false, true);
			REQUIRE_CALL(times, is_satisfied())
				.RETURN(isSatisfied);
			REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());
		}

		SECTION("When times is applicable, call is matched.")
		{
			REQUIRE_CALL(times, is_applicable())
				.RETURN(true);
			REQUIRE_CALL(times, describe_state())
				.RETURN("times state applicable");
			const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
			REQUIRE(matchReport.timesReport == TimesReportT{true, "times state applicable"});
			REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(matchReport));
		}

		SECTION("When times is not applicable => inapplicable.")
		{
			REQUIRE_CALL(times, is_applicable())
				.RETURN(false);
			REQUIRE_CALL(times, describe_state())
				.RETURN("times state inapplicable");
			const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
			REQUIRE(matchReport.timesReport == TimesReportT{false, "times state inapplicable"});
			REQUIRE(mimicpp::MatchResult::inapplicable == evaluate_match_report(matchReport));
		}

		SECTION("Consume calls times.consume().")
		{
			REQUIRE_CALL(times, consume());
			REQUIRE_NOTHROW(expectation.consume(call));
		}
	}

	SECTION("With other expectation policies.")
	{
		PolicyMockT policy{};
		mimicpp::BasicExpectation<SignatureT, TimesPolicyT, FinalizerT, PolicyRefT> expectation{
			std::source_location::current(),
			std::ref(times),
			FinalizerT{},
			std::ref(policy)
		};

		SECTION("When times is not satisfied.")
		{
			REQUIRE_CALL(times, is_satisfied())
				.RETURN(false);
			REQUIRE(!std::as_const(expectation).is_satisfied());
		}

		SECTION("When times is satisfied, result depends on expectation policy.")
		{
			REQUIRE_CALL(times, is_satisfied())
				.RETURN(true);
			const bool isSatisfied = GENERATE(false, true);
			REQUIRE_CALL(policy, is_satisfied())
				.RETURN(isSatisfied);
			REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());
		}

		SECTION("When policy is not matched, then the result is always no match.")
		{
			REQUIRE_CALL(policy, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(false);
			REQUIRE_CALL(policy, describe())
				.RETURN(mimicpp::StringT{});
			const bool isApplicable = GENERATE(false, true);
			REQUIRE_CALL(times, is_applicable())
				.RETURN(isApplicable);
			REQUIRE_CALL(times, describe_state())
				.RETURN(std::nullopt);
			REQUIRE(mimicpp::MatchResult::none == evaluate_match_report(std::as_const(expectation).matches(call)));
		}

		SECTION("When policy is matched.")
		{
			SECTION("And when times is applicable => ok")
			{
				REQUIRE_CALL(times, is_applicable())
					.RETURN(true);
				REQUIRE_CALL(policy, matches(_))
					.LR_WITH(&_1 == &call)
					.RETURN(true);
				REQUIRE_CALL(policy, describe())
					.RETURN(mimicpp::StringT{});
				REQUIRE_CALL(times, describe_state())
					.RETURN(std::nullopt);
				REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(std::as_const(expectation).matches(call)));
			}

			SECTION("And when times not applicable => inapplicable")
			{
				REQUIRE_CALL(times, is_applicable())
					.RETURN(false);
				REQUIRE_CALL(policy, matches(_))
					.LR_WITH(&_1 == &call)
					.RETURN(true);
				REQUIRE_CALL(policy, describe())
					.RETURN(mimicpp::StringT{});
				REQUIRE_CALL(times, describe_state())
					.RETURN(std::nullopt);
				REQUIRE(mimicpp::MatchResult::inapplicable == evaluate_match_report(std::as_const(expectation).matches(call)));
			}
		}

		SECTION("Consume calls times.consume().")
		{
			REQUIRE_CALL(times, consume());
			REQUIRE_CALL(policy, consume(_))
				.LR_WITH(&_1 == &call);
			REQUIRE_NOTHROW(expectation.consume(call));
		}
	}
}

TEMPLATE_TEST_CASE(
	"mimicpp::BasicExpectation can be extended with an arbitrary policy amount.",
	"[expectation]",
	void(),
	int()
)
{
	using trompeloeil::_;
	using FinalizerT = FinalizerFake<TestType>;
	using PolicyMockT = PolicyMock<TestType>;
	using PolicyRefT = PolicyFacade<TestType, std::reference_wrapper<PolicyMock<TestType>>, UnwrapReferenceWrapper>;
	using CallInfoT = mimicpp::call::info_for_signature_t<TestType>;
	using ExpectationReportT = mimicpp::MatchReport::Expectation;
	using TimesReportT = mimicpp::MatchReport::Times;

	const CallInfoT call{
		.args = {},
		.fromCategory = mimicpp::ValueCategory::any,
		.fromConstness = mimicpp::Constness::any
	};

	SECTION("With no policies at all.")
	{
		mimicpp::BasicExpectation<TestType, TimesFake, FinalizerT> expectation{
			std::source_location::current(),
			TimesFake{.isSatisfied = true},
			FinalizerT{}
		};

		REQUIRE(std::as_const(expectation).is_satisfied());
		const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
		REQUIRE(matchReport.timesReport == TimesReportT{true});
		REQUIRE_THAT(
			matchReport.expectationReports,
			Catch::Matchers::IsEmpty());
		REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(matchReport));
		REQUIRE_NOTHROW(expectation.consume(call));
	}

	SECTION("With one policy.")
	{
		PolicyMockT policy{};
		mimicpp::BasicExpectation<TestType, TimesFake, FinalizerT, PolicyRefT> expectation{
			std::source_location::current(),
			TimesFake{.isSatisfied = true},
			FinalizerT{},
			PolicyRefT{std::ref(policy)}
		};

		const bool isSatisfied = GENERATE(false, true);
		REQUIRE_CALL(policy, is_satisfied())
			.RETURN(isSatisfied);
		REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());

		SECTION("When matched => ok match")
		{
			REQUIRE_CALL(policy, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(true);
			REQUIRE_CALL(policy, describe())
				.RETURN("policy description");
			const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
			REQUIRE(matchReport.timesReport == TimesReportT{true});
			REQUIRE_THAT(
				matchReport.expectationReports,
				Catch::Matchers::RangeEquals(std::vector<ExpectationReportT>{{true, "policy description"}}));
			REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(matchReport));
		}

		SECTION("When not matched => no match")
		{
			REQUIRE_CALL(policy, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(false);
			REQUIRE_CALL(policy, describe())
				.RETURN("policy description");
			const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
			REQUIRE(matchReport.timesReport == TimesReportT{true});
			REQUIRE_THAT(
				matchReport.expectationReports,
				Catch::Matchers::RangeEquals(std::vector<ExpectationReportT>{{false, "policy description"}}));
			REQUIRE(mimicpp::MatchResult::none == evaluate_match_report(matchReport));
		}

		REQUIRE_CALL(policy, consume(_))
			.LR_WITH(&_1 == &call);
		expectation.consume(call);
	}

	SECTION("With two policies.")
	{
		PolicyMockT policy1{};
		PolicyMockT policy2{};
		mimicpp::BasicExpectation<TestType, TimesFake, FinalizerT, PolicyRefT, PolicyRefT> expectation{
			std::source_location::current(),
			TimesFake{.isSatisfied = true},
			FinalizerT{},
			PolicyRefT{std::ref(policy1)},
			PolicyRefT{std::ref(policy2)}
		};

		SECTION("When calling is_satisfied()")
		{
			const bool isSatisfied1 = GENERATE(false, true);
			const bool isSatisfied2 = GENERATE(false, true);
			const bool expectedIsSatisfied = isSatisfied1 && isSatisfied2;
			REQUIRE_CALL(policy1, is_satisfied())
				.RETURN(isSatisfied1);
			auto policy2Expectation = std::invoke(
				[&]() -> std::unique_ptr<trompeloeil::expectation>
				{
					if (isSatisfied1)
					{
						return NAMED_REQUIRE_CALL(policy2, is_satisfied())
							.RETURN(isSatisfied2);
					}
					return nullptr;
				});

			REQUIRE(expectedIsSatisfied == std::as_const(expectation).is_satisfied());
		}

		SECTION("When both matches => ok match")
		{
			REQUIRE_CALL(policy1, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(true);
			REQUIRE_CALL(policy1, describe())
				.RETURN("policy1 description");
			REQUIRE_CALL(policy2, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(true);
			REQUIRE_CALL(policy2, describe())
				.RETURN("policy2 description");

			const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
			REQUIRE(matchReport.timesReport == TimesReportT{true});
			REQUIRE_THAT(
				matchReport.expectationReports,
				Catch::Matchers::UnorderedRangeEquals(
					std::vector<ExpectationReportT>{
					{true, "policy1 description"},
					{true, "policy2 description"}
					}));
			REQUIRE(mimicpp::MatchResult::full == evaluate_match_report(matchReport));
		}

		SECTION("When at least one not matches => no match")
		{
			const auto [isMatching1, isMatching2] = GENERATE(
				(table<bool, bool>)({
					{false, true},
					{true, false},
					{false, false}
				}));

			REQUIRE_CALL(policy1, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(isMatching1);
			REQUIRE_CALL(policy1, describe())
				.RETURN("policy1 description");
			REQUIRE_CALL(policy2, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(isMatching2);
			REQUIRE_CALL(policy2, describe())
				.RETURN("policy2 description");

			const mimicpp::MatchReport matchReport = std::as_const(expectation).matches(call);
			REQUIRE(matchReport.timesReport == TimesReportT{true});
			REQUIRE_THAT(
				matchReport.expectationReports,
				Catch::Matchers::UnorderedRangeEquals(
					std::vector<ExpectationReportT>{
					{isMatching1, "policy1 description"},
					{isMatching2, "policy2 description"}
					}));
			REQUIRE(mimicpp::MatchResult::none == evaluate_match_report(matchReport));
		}

		SECTION("When calling consume()")
		{
			REQUIRE_CALL(policy1, consume(_))
			   .LR_WITH(&_1 == &call);
			REQUIRE_CALL(policy2, consume(_))
				.LR_WITH(&_1 == &call);
			expectation.consume(call);
		}
	}
}

TEST_CASE(
	"mimicpp::BasicExpectation::report gathers information about the expectation.",
	"[expectation]"
)
{
	namespace Matches = Catch::Matchers;

	using FinalizerPolicyT = FinalizerFake<void()>;
	using TimesPolicyT = TimesFake;

	SECTION("Finalizer policy has no description.")
	{
		// Todo:
	}

	SECTION("Times policy is queried.")
	{
		using TimesT = TimesFacade<std::reference_wrapper<TimesMock>, UnwrapReferenceWrapper>;

		TimesMock times{};
		mimicpp::BasicExpectation<
				void(),
				TimesT,
				FinalizerPolicyT>
			expectation{
				std::source_location::current(),
				TimesT{std::ref(times)},
				FinalizerPolicyT{}
			};

		REQUIRE_CALL(times, describe_state())
			.RETURN("times description");

		const mimicpp::ExpectationReport report = expectation.report();
		REQUIRE(report.timesDescription);
		REQUIRE_THAT(
			*report.timesDescription,
			Matches::Equals("times description"));
	}

	SECTION("Expectation policies are queried.")
	{
		using PolicyT = PolicyFacade<
			void(),
			std::reference_wrapper<PolicyMock<void()>>,
			UnwrapReferenceWrapper>;

		PolicyMock<void()> policy{};
		mimicpp::BasicExpectation<
				void(),
				TimesPolicyT,
				FinalizerPolicyT,
				PolicyT>
			expectation{
				std::source_location::current(),
				TimesPolicyT{},
				FinalizerPolicyT{},
				PolicyT{std::ref(policy)}
			};

		REQUIRE_CALL(policy, describe())
			.RETURN("expectation description");

		const mimicpp::ExpectationReport report = expectation.report();
		REQUIRE_THAT(
			report.expectationDescriptions,
			Matches::SizeIs(1));
		REQUIRE(report.expectationDescriptions[0]);
		REQUIRE_THAT(
			*report.expectationDescriptions[0],
			Matches::Equals("expectation description"));
	}
}

TEMPLATE_TEST_CASE(
	"mimicpp::BasicExpectation finalizer can be exchanged.",
	"[expectation]",
	void(),
	int()
)
{
	using trompeloeil::_;
	using SignatureT = TestType;
	using FinalizerT = FinalizerMock<SignatureT>;
	using FinalizerRefT = FinalizerFacade<SignatureT, std::reference_wrapper<FinalizerT>, UnwrapReferenceWrapper>;
	using CallInfoT = mimicpp::call::info_for_signature_t<SignatureT>;

	const CallInfoT call{
		.args = {},
		.fromCategory = mimicpp::ValueCategory::any,
		.fromConstness = mimicpp::Constness::any
	};

	FinalizerT finalizer{};
	mimicpp::BasicExpectation<SignatureT, TimesFake, FinalizerRefT> expectation{
		std::source_location::current(),
		TimesFake{},
		std::ref(finalizer)
	};

	class Exception
	{
	};
	REQUIRE_CALL(finalizer, finalize_call(_))
		.LR_WITH(&_1 == &call)
		.THROW(Exception{});
	REQUIRE_THROWS_AS(expectation.finalize_call(call), Exception);
}

TEST_CASE("ScopedExpectation is a non-copyable, but movable type.")
{
	using ScopedExpectationT = mimicpp::ScopedExpectation<void()>;

	STATIC_REQUIRE(!std::is_copy_constructible_v<ScopedExpectationT>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<ScopedExpectationT>);
	STATIC_REQUIRE(std::is_move_constructible_v<ScopedExpectationT>);
	STATIC_REQUIRE(std::is_move_assignable_v<ScopedExpectationT>);
}

TEST_CASE(
	"ScopedExpectation handles expectation lifetime and removes from the ExpectationCollection.",
	"[expectation]")
{
	using trompeloeil::_;
	using SignatureT = void();
	using ExpectationT = ExpectationMock;
	using ScopedExpectationT = mimicpp::ScopedExpectation<SignatureT>;
	using CollectionT = mimicpp::ExpectationCollection<SignatureT>;

	auto collection = std::make_shared<CollectionT>();
	auto innerExpectation = std::make_shared<ExpectationT>();
	std::optional<ScopedExpectationT> expectation{
		std::in_place,
		collection,
		innerExpectation
	};

	SECTION("When calling is_satisfied()")
	{
		const bool isSatisfied = GENERATE(false, true);
		REQUIRE_CALL(*innerExpectation, is_satisfied())
			.RETURN(isSatisfied);
		REQUIRE(isSatisfied == std::as_const(expectation)->is_satisfied());
	}

	SECTION("When ScopedExpectation is moved.")
	{
		ScopedExpectationT otherExpectation = *std::move(expectation);

		SECTION("When calling is_satisfied()")
		{
			const bool isSatisfied = GENERATE(false, true);
			REQUIRE_CALL(*innerExpectation, is_satisfied())
				.RETURN(isSatisfied);
			REQUIRE(isSatisfied == std::as_const(otherExpectation).is_satisfied());
			REQUIRE_THROWS_AS(std::as_const(expectation)->is_satisfied(), std::runtime_error);
		}

		SECTION("And then move assigned.")
		{
			expectation = std::move(otherExpectation);

			SECTION("When calling is_satisfied()")
			{
				const bool isSatisfied = GENERATE(false, true);
				REQUIRE_CALL(*innerExpectation, is_satisfied())
					.RETURN(isSatisfied);
				REQUIRE(isSatisfied == std::as_const(expectation)->is_satisfied());
				REQUIRE_THROWS_AS(std::as_const(otherExpectation).is_satisfied(), std::runtime_error);
			}

			// just move back, so we can unify the cleanup process
			otherExpectation = *std::move(expectation);
		}

		SECTION("And then self move assigned.")
		{
			otherExpectation = std::move(otherExpectation);

			SECTION("When calling is_satisfied()")
			{
				const bool isSatisfied = GENERATE(false, true);
				REQUIRE_CALL(*innerExpectation, is_satisfied())
					.RETURN(isSatisfied);
				REQUIRE(isSatisfied == std::as_const(otherExpectation).is_satisfied());
			}
		}

		// just move back, so we can unify the cleanup process
		expectation = std::move(otherExpectation);
	}

	// indirectly via remove
	REQUIRE_CALL(*innerExpectation, is_satisfied())
		.RETURN(true);
	expectation.reset();
}
