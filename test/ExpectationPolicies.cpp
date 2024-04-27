// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ExpectationPolicies.hpp"
#include "TestReporter.hpp"
#include "TestTypes.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace mimicpp;

TEMPLATE_TEST_CASE(
	"Given types satisfy mimicpp::times_policy concept.",
	"[expectation][expectation::builder]",
	(expectation_policies::Times<0, 1>),
	(expectation_policies::Times<1, 1>),
	expectation_policies::RuntimeTimes,
	expectation_policies::InitTimes
)
{
	STATIC_REQUIRE(mimicpp::times_policy<TestType>);
}

TEMPLATE_TEST_CASE_SIG(
	"Given types satisfy mimicpp::finalize_policy_for concept.",
	"[expectation][expectation::builder]",
	((bool dummy, typename Policy, typename Sig), dummy, Policy, Sig),
	(true, expectation_policies::InitFinalize, void()),
	(true, expectation_policies::InitFinalize, void(int)),
	(true, expectation_policies::Throws<std::runtime_error>, void()),
	(true, expectation_policies::Throws<std::runtime_error>, void(int)),
	(true, expectation_policies::Throws<std::runtime_error>, double(int))
)
{
	STATIC_REQUIRE(mimicpp::finalize_policy_for<Policy, Sig>);
}

TEMPLATE_TEST_CASE(
	"Given types satisfy mimicpp::expectation_policy_for concept.",
	"[expectation][expectation::builder]",
	expectation_policies::Category<ValueCategory::lvalue>,
	expectation_policies::Category<ValueCategory::rvalue>,
	expectation_policies::Category<ValueCategory::any>,
	expectation_policies::Constness<Constness::as_const>,
	expectation_policies::Constness<Constness::non_const>,
	expectation_policies::Constness<Constness::any>
)
{
	STATIC_REQUIRE(mimicpp::expectation_policy_for<TestType, void()>);
}

TEST_CASE(
	"expectation_policies::InitFinalize does nothing.",
	"[expectation][expectation::builder]"
)
{
	const call::Info<void> call{
		.params = {},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	constexpr expectation_policies::InitFinalize policy{};

	REQUIRE_NOTHROW(policy.finalize_call(call));
}

TEST_CASE(
	"expectation_policies::InitTimes allows exactly one consume.",
	"[expectation][expectation::builder]"
)
{
	expectation_policies::InitTimes policy{};
	REQUIRE(!policy.is_satisfied());
	REQUIRE(!policy.is_saturated());

	REQUIRE_NOTHROW(policy.consume());

	REQUIRE(policy.is_satisfied());
	REQUIRE(policy.is_saturated());
}

TEMPLATE_TEST_CASE_SIG(
	"expectation_policies::Times is configurable at compile-time.",
	"[expectation][expectation::builder]",
	((std::size_t min, std::size_t max), min, max),
	(0, 0),
	(0, 1),
	(0, 2),
	(1, 2),
	(2, 2)
)
{
	expectation_policies::Times<min, max> policy{};

	for ([[maybe_unused]] auto i : std::views::iota(0u, min))
	{
		REQUIRE(!policy.is_satisfied());
		REQUIRE(!policy.is_saturated());

		REQUIRE_NOTHROW(policy.consume());
	}

	REQUIRE(policy.is_satisfied());

	for ([[maybe_unused]] auto i : std::views::iota(min, max))
	{
		REQUIRE(policy.is_satisfied());
		REQUIRE(!policy.is_saturated());

		REQUIRE_NOTHROW(policy.consume());
	}

	REQUIRE(policy.is_satisfied());
	REQUIRE(policy.is_saturated());
}

TEST_CASE(
	"expectation_policies::Times is configurable at runtime.",
	"[expectation][expectation::builder]"
)
{
	const std::size_t min = GENERATE(0u, 1u, 2u, 3u, 4u);
	const std::size_t max = min + GENERATE(0u, 1u, 2u, 3u, 4u);

	expectation_policies::RuntimeTimes policy{min, max};

	for ([[maybe_unused]] auto i : std::views::iota(0u, min))
	{
		REQUIRE(!policy.is_satisfied());
		REQUIRE(!policy.is_saturated());

		REQUIRE_NOTHROW(policy.consume());
	}

	REQUIRE(policy.is_satisfied());

	for ([[maybe_unused]] auto i : std::views::iota(min, max))
	{
		REQUIRE(policy.is_satisfied());
		REQUIRE(!policy.is_saturated());

		REQUIRE_NOTHROW(policy.consume());
	}

	REQUIRE(policy.is_satisfied());
	REQUIRE(policy.is_saturated());
}

TEST_CASE(
	"expectation_policies::RuntimeTimes does throw, when invalid config is given.",
	"[expectation][expectation::builder]"
)
{
	const std::size_t max = GENERATE(0u, 1u, 2u, 3u, 4u);
	const std::size_t min = max + GENERATE(1u, 2u, 3u, 4u);

	REQUIRE_THROWS_AS(
		(expectation_policies::RuntimeTimes{min, max}),
		std::runtime_error);
}

TEMPLATE_TEST_CASE_SIG(
	"expectation_policies::Category checks whether the given call::Info matches.",
	"[expectation][expectation::policy]",
	((ValueCategory category), category),
	(ValueCategory::lvalue),
	(ValueCategory::rvalue),
	(ValueCategory::any)
)
{
	using SignatureT = void();
	using CallInfoT = call::info_for_signature_t<SignatureT>;
	using PolicyT = expectation_policies::Category<category>;
	STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

	constexpr PolicyT policy{};
	SECTION("Policy is always satisfied.")
	{
		REQUIRE(policy.is_satisfied());
	}

	const CallInfoT call{
		.params = {},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	if (is_matching(call.fromCategory, category))
	{
		SECTION("When call and policy category matches, success is returned.")
		{
			const auto result = policy.matches(call);

			REQUIRE(result.matched);
			REQUIRE_THAT(
				result.msg.value(),
				Catch::Matchers::Equals(format::format(" matches Category {}", category)));
		}

		SECTION("Policy doesn't consume, but asserts on wrong category.")
		{
			REQUIRE_NOTHROW(policy.consume(call));
		}
	}
	else
	{
		SECTION("When call and policy category mismatch, failure is returned.")
		{
			const auto result = policy.matches(call);

			REQUIRE(!result.matched);
			REQUIRE_THAT(
				result.msg.value(),
				Catch::Matchers::Equals(format::format(" does not match Category {}", category)));
		}
	}
}

TEMPLATE_TEST_CASE_SIG(
	"expectation_policies::Constness checks whether the given call::Info matches.",
	"[expectation][expectation::policy]",
	((Constness constness), constness),
	(Constness::as_const),
	(Constness::non_const),
	(Constness::any)
)
{
	using SignatureT = void();
	using CallInfoT = call::info_for_signature_t<SignatureT>;
	using PolicyT = expectation_policies::Constness<constness>;
	STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

	constexpr PolicyT policy{};
	SECTION("Policy is always satisfied.")
	{
		REQUIRE(policy.is_satisfied());
	}

	const CallInfoT call{
		.params = {},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	if (is_matching(call.fromConstness, constness))
	{
		SECTION("When call and policy constness matches, success is returned.")
		{
			const auto result = policy.matches(call);

			REQUIRE(result.matched);
			REQUIRE_THAT(
				result.msg.value(),
				Catch::Matchers::Equals(format::format(" matches Constness {}", constness)));
		}

		SECTION("Policy doesn't consume, but asserts on wrong constness.")
		{
			REQUIRE_NOTHROW(policy.consume(call));
		}
	}
	else
	{
		SECTION("When call and policy constness mismatch, failure is returned.")
		{
			const auto result = policy.matches(call);

			REQUIRE(!result.matched);
			REQUIRE_THAT(
				result.msg.value(),
				Catch::Matchers::Equals(format::format(" does not match Constness {}", constness)));
		}
	}
}

TEST_CASE(
	"expectation_policies::ReturnsResultOf forwards the invocation result during finalize_call().",
	"[expectation][expectation::policy]"
)
{
	using trompeloeil::_;

	using SignatureT = int&();
	using CallInfoT = call::info_for_signature_t<SignatureT>;

	const CallInfoT call{
		.params = {},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	int value{42};
	InvocableMock<int&, const CallInfoT&> action{};
	expectation_policies::ReturnsResultOf policy{std::ref(action)};
	STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

	REQUIRE_CALL(action, Invoke(_))
		.LR_WITH(&call == &_1)
		.LR_RETURN(value);
	int& result = policy.finalize_call(call);
	REQUIRE(&value == &result);
}

TEST_CASE(
	"expectation_policies::Throws always throws an exception during finalize_call().",
	"[expectation][expectation::policy]"
)
{
	struct test_exception
	{
	};

	SECTION("When void is returned.")
	{
		using SignatureT = void();
		using CallInfoT = call::info_for_signature_t<SignatureT>;
		using PolicyT = expectation_policies::Throws<test_exception>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{test_exception{}};

		REQUIRE_THROWS_AS(
			policy.finalize_call(call),
			test_exception);
	}

	SECTION("When non-void is returned.")
	{
		using SignatureT = int&&();
		using CallInfoT = call::info_for_signature_t<SignatureT>;
		using PolicyT = expectation_policies::Throws<test_exception>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{test_exception{}};

		REQUIRE_THROWS_AS(
			policy.finalize_call(call),
			test_exception);
	}
}

TEST_CASE(
	"expectation_policies::ArgumentMatcher checks whether the given call::Info matches.",
	"[expectation][expectation::policy]"
)
{
	SECTION("Policy works on unary signature.")
	{
		using SignatureT = void(int);
		using CallInfoT = call::info_for_signature_t<SignatureT>;

		MatcherMock<int> matcher{};
		const auto policy = expectation_policies::make_argument_matcher<SignatureT, 0>(
			MatcherFacade{std::ref(matcher), UnwrapReferenceWrapper{}});
		STATIC_REQUIRE(expectation_policy_for<std::remove_const_t<decltype(policy)>, SignatureT>);

		int param{42};
		const CallInfoT call{
			.params = {param},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		SECTION("Policy is always satisfied.")
		{
			REQUIRE(policy.is_satisfied());
		}

		SECTION("Policy doesn't consume.")
		{
			REQUIRE_NOTHROW(policy.consume(call));
		}

		SECTION("When matches is called, policy forwards param to predicate and describer.")
		{
			const auto [match, msg] = GENERATE(
				(table<bool, std::string>)({
					{false, "param[0] does not match 42"},
					{true, "param[0] matches 42"}
					}));

			trompeloeil::sequence sequence{};
			REQUIRE_CALL(matcher, matches(42))
				.IN_SEQUENCE(sequence)
				.RETURN(match);
			REQUIRE_CALL(matcher, describe(42))
				.IN_SEQUENCE(sequence)
				.RETURN("42");

			const auto result = policy.matches(call);
			REQUIRE(match == result.matched);
			REQUIRE_THAT(
				result.msg.value(),
				Catch::Matchers::Equals(msg));
		}
	}

	SECTION("Policy works on binary signature.")
	{
		using SignatureT = void(int, const std::string&);
		using CallInfoT = call::info_for_signature_t<SignatureT>;

		MatcherMock<int> matcher1{};
		const auto policy1 = expectation_policies::make_argument_matcher<SignatureT, 0>(
			MatcherFacade{std::ref(matcher1), UnwrapReferenceWrapper{}});
		STATIC_REQUIRE(expectation_policy_for<std::remove_const_t<decltype(policy1)>, SignatureT>);

		MatcherMock<std::string> matcher2{};
		const auto policy2 = expectation_policies::make_argument_matcher<SignatureT, 1>(
			MatcherFacade{std::ref(matcher2), UnwrapReferenceWrapper{}});
		STATIC_REQUIRE(expectation_policy_for<std::remove_const_t<decltype(policy2)>, SignatureT>);

		int param0{42};
		const std::string param1{"Hello, World!"};
		const CallInfoT call{
			.params = {param0, param1},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		SECTION("Policy is always satisfied.")
		{
			REQUIRE(policy1.is_satisfied());
			REQUIRE(policy2.is_satisfied());
		}

		SECTION("Policy doesn't consume.")
		{
			REQUIRE_NOTHROW(policy1.consume(call));
			REQUIRE_NOTHROW(policy2.consume(call));
		}

		SECTION("When matches is called, policy forwards param to predicate and describer.")
		{
			SECTION("On param0.")
			{
				const auto [match, msg] = GENERATE(
					(table<bool, std::string>)({
						{false, "param[0] does not match 42"},
						{true, "param[0] matches 42"}
						}));

				trompeloeil::sequence sequence{};
				REQUIRE_CALL(matcher1, matches(42))
					.IN_SEQUENCE(sequence)
					.RETURN(match);
				REQUIRE_CALL(matcher1, describe(42))
					.IN_SEQUENCE(sequence)
					.RETURN("42");

				const auto result = policy1.matches(call);
				REQUIRE(match == result.matched);
				REQUIRE_THAT(
					result.msg.value(),
					Catch::Matchers::Equals(msg));
			}

			SECTION("On param2.")
			{
				const auto [match, msg] = GENERATE(
					(table<bool, std::string>)({
						{false, "param[1] does not match Hello, World!"},
						{true, "param[1] matches Hello, World!"}
						}));

				trompeloeil::sequence sequence{};
				REQUIRE_CALL(matcher2, matches(param1))
					.IN_SEQUENCE(sequence)
					.RETURN(match);
				REQUIRE_CALL(matcher2, describe(param1))
					.IN_SEQUENCE(sequence)
					.RETURN("Hello, World!");

				const auto result = policy2.matches(call);
				REQUIRE(match == result.matched);
				REQUIRE_THAT(
					result.msg.value(),
					Catch::Matchers::Equals(msg));
			}
		}
	}
}

TEST_CASE(
	"Invalid configurations of expectation_policies::ParamsSideEffect do not satisfy expectation_policy_for concept.",
	"[expectation][expectation::policy]"
)
{
	using ActionT = InvocableMock<void, int>;

	SECTION("Signature without params.")
	{
		STATIC_REQUIRE(
			!expectation_policy_for<
			decltype(expectation_policies::make_param_side_effect<0>(std::declval<ActionT>())),
			void()>);
	}

	SECTION("Index out of bounds.")
	{
		STATIC_REQUIRE(
			!expectation_policy_for<
			decltype(expectation_policies::make_param_side_effect<1>(std::declval<ActionT>())),
			void(int)>);
	}

	SECTION("Action not applicable.")
	{
		STATIC_REQUIRE(
			!expectation_policy_for<
			decltype(expectation_policies::make_param_side_effect<0, 0>(std::declval<ActionT>())),
			void(int)>);
	}
}

TEST_CASE(
	"expectation_policies::ParamsSideEffect invokes the specified function on consume.",
	"[expectation][expectation::policy]"
)
{
	using trompeloeil::_;

	SECTION("Supports multiple params.")
	{
		int param0{1337};
		double param1{4.2};
		const call::Info<void, int&, double&&> info{
			.params = {param0, param1},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		SECTION("In order.")
		{
			InvocableMock<void, int&, const double&> action{};
			expectation_policies::ParamsSideEffect policy = expectation_policies::make_param_side_effect<0, 1>(std::ref(action));
			STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(int&, double&&)>);
			REQUIRE(std::as_const(policy).is_satisfied());
			REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));

			REQUIRE_CALL(action, Invoke(_, _))
				.LR_WITH(&param0 == &_1)
				.LR_WITH(&param1 == &_2);
			REQUIRE_NOTHROW(policy.consume(info));
		}

		SECTION("In reverse order.")
		{
			InvocableMock<void, const double&, int&> action{};
			expectation_policies::ParamsSideEffect policy = expectation_policies::make_param_side_effect<1, 0>(std::ref(action));
			STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(int&, double&&)>);
			REQUIRE(std::as_const(policy).is_satisfied());
			REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));

			REQUIRE_CALL(action, Invoke(_, _))
				.LR_WITH(&param0 == &_2)
				.LR_WITH(&param1 == &_1);
			REQUIRE_NOTHROW(policy.consume(info));
		}

		SECTION("Just the first.")
		{
			InvocableMock<void, int&> action{};
			expectation_policies::ParamsSideEffect policy = expectation_policies::make_param_side_effect<0>(std::ref(action));
			STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(int&, double&&)>);
			REQUIRE(std::as_const(policy).is_satisfied());
			REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));

			REQUIRE_CALL(action, Invoke(_))
				.LR_WITH(&param0 == &_1);
			REQUIRE_NOTHROW(policy.consume(info));
		}

		SECTION("Just the second.")
		{
			InvocableMock<void, const double&> action{};
			expectation_policies::ParamsSideEffect policy = expectation_policies::make_param_side_effect<1>(std::ref(action));
			STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(int&, double&&)>);
			REQUIRE(std::as_const(policy).is_satisfied());
			REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));

			REQUIRE_CALL(action, Invoke(_))
				.LR_WITH(&param1 == &_1);
			REQUIRE_NOTHROW(policy.consume(info));
		}

		SECTION("Arbitrarily mixed.")
		{
			InvocableMock<void, const double&, int&, int&, const double&, const double&, int&> action{};
			expectation_policies::ParamsSideEffect policy = expectation_policies::make_param_side_effect<1, 0, 0, 1, 1, 0>(
				std::ref(action));
			STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(int&, double&&)>);
			REQUIRE(std::as_const(policy).is_satisfied());
			REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));

			REQUIRE_CALL(action, Invoke(_, _, _, _, _, _))
				.LR_WITH(&_1 == &param1)
				.LR_WITH(&_2 == &param0)
				.LR_WITH(&_3 == &param0)
				.LR_WITH(&_4 == &param1)
				.LR_WITH(&_5 == &param1)
				.LR_WITH(&_6 == &param0);
			REQUIRE_NOTHROW(policy.consume(info));
		}
	}
}

TEMPLATE_TEST_CASE_SIG(
	"expectation_policies::ParamsSideEffect takes the param category into account.",
	"[expectation][expectation::policy]",
	((bool expectSameAddress, typename ActionParam, typename SigParam), expectSameAddress, ActionParam, SigParam),
	(false, int, int),
	(false, int, int&),
	(false, int, const int&),
	(false, int, int&&),
	(false, int, const int&&),

	(true, int&, int),
	(true, int&, int&),
	(true, int&, int&&),

	(true, const int&, int),
	(true, const int&, int&),
	(true, const int&, const int&),
	(true, const int&, int&&),
	(true, const int&, const int&&)
)
{
	using trompeloeil::_;

	int param0{1337};
	const call::Info<void, SigParam> info{
		.params = {param0},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	InvocableMock<void, ActionParam> action{};
	expectation_policies::ParamsSideEffect policy = expectation_policies::make_param_side_effect<0>(std::ref(action));
	STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(SigParam)>);
	REQUIRE(std::as_const(policy).is_satisfied());
	REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));
	REQUIRE_CALL(action, Invoke(param0))
		.LR_WITH(expectSameAddress == (&_1 == &param0));
	REQUIRE_NOTHROW(policy.consume(info));
}

TEST_CASE(
	"Invalid configurations of expectation_policies::AllParamsSideEffect do not satisfy expectation_policy_for concept.",
	"[expectation][expectation::policy]"
)
{
	using ActionT = InvocableMock<void, int>;
	SECTION("Action has more params than expected.")
	{
		STATIC_REQUIRE(
			!expectation_policy_for<
			expectation_policies::AllParamsSideEffect<ActionT>,
			void()>);
	}

	SECTION("Action has less params than expected.")
	{
		STATIC_REQUIRE(
			!expectation_policy_for<
			expectation_policies::AllParamsSideEffect<ActionT>,
			void(double, int)>);
	}
}

TEST_CASE(
	"expectation_policies::AllParamsSideEffect invokes the specified function on consume.",
	"[expectation][expectation::policy]"
)
{
	using trompeloeil::_;

	SECTION("Signatures without params.")
	{
		const call::Info<void> info{
			.params = {},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		InvocableMock<void> action{};
		expectation_policies::AllParamsSideEffect policy{std::ref(action)};
		STATIC_REQUIRE(expectation_policy_for<decltype(policy), void()>);
		REQUIRE(std::as_const(policy).is_satisfied());
		REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));
		REQUIRE_CALL(action, Invoke());
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("Unary signatures.")
	{
		int param0{1337};
		const call::Info<void, int&> info{
			.params = {param0},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		InvocableMock<void, int&> action{};
		expectation_policies::AllParamsSideEffect policy{std::ref(action)};
		STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(int&)>);
		REQUIRE(std::as_const(policy).is_satisfied());
		REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));
		REQUIRE_CALL(action, Invoke(_))
			.LR_WITH(&_1 == &param0);
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("Binary signatures.")
	{
		int param0{1337};
		double param1{4.2};
		const call::Info<void, int&, double&> info{
			.params = {param0, param1},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		InvocableMock<void, int&, double&> action{};
		expectation_policies::AllParamsSideEffect policy{std::ref(action)};
		STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(int&, double&)>);
		REQUIRE(std::as_const(policy).is_satisfied());
		REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));
		REQUIRE_CALL(action, Invoke(_, _))
			.LR_WITH(&_1 == &param0)
			.LR_WITH(&_2 == &param1);
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("Arbitrary signatures.")
	{
		int param0{1337};
		double param1{4.2};
		std::string param2{"Hello, World!"};
		const call::Info<void, int&, double&, std::string&> info{
			.params = {param0, param1, param2},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		InvocableMock<void, int&, double&, std::string&> action{};
		expectation_policies::AllParamsSideEffect policy{std::ref(action)};
		STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(int&, double&, std::string&)>);
		REQUIRE(std::as_const(policy).is_satisfied());
		REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));
		REQUIRE_CALL(action, Invoke(_, _, _))
			.LR_WITH(&_1 == &param0)
			.LR_WITH(&_2 == &param1)
			.LR_WITH(&_3 == &param2);
		REQUIRE_NOTHROW(policy.consume(info));
	}
}

TEMPLATE_TEST_CASE_SIG(
	"expectation_policies::AllParamsSideEffect takes the param category into account.",
	"[expectation][expectation::policy]",
	((bool expectSameAddress, typename ActionParam, typename SigParam), expectSameAddress, ActionParam, SigParam),
	(false, int, int),
	(false, int, int&),
	(false, int, const int&),
	(false, int, int&&),
	(false, int, const int&&),

	(true, int&, int),
	(true, int&, int&),
	(true, int&, int&&),

	(true, const int&, int),
	(true, const int&, int&),
	(true, const int&, const int&),
	(true, const int&, int&&),
	(true, const int&, const int&&)
)
{
	using trompeloeil::_;

	int param0{1337};
	const call::Info<void, SigParam> info{
		.params = {param0},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	InvocableMock<void, ActionParam> action{};
	expectation_policies::AllParamsSideEffect policy{std::ref(action)};
	STATIC_REQUIRE(expectation_policy_for<decltype(policy), void(SigParam)>);
	REQUIRE(std::as_const(policy).is_satisfied());
	REQUIRE(call::SubMatchResult{true} == std::as_const(policy).matches(info));
	REQUIRE_CALL(action, Invoke(param0))
		.LR_WITH(expectSameAddress == (&_1 == &param0));
	REQUIRE_NOTHROW(policy.consume(info));
}

TEST_CASE(
	"then::apply_param creates expectation_policies::ParamsSideEffect.",
	"[expectation][expectation::factories]"
)
{
	using trompeloeil::_;

	int param0{1337};
	double param1{4.2};
	std::string param2{"Hello, World!"};
	const call::Info<void, int&, double&, std::string&> info{
		.params = {param0, param1, param2}
	};

	SECTION("Index 0.")
	{
		InvocableMock<void, int&> action{};
		expectation_policies::ParamsSideEffect policy = then::apply_param<0>(std::ref(action));
		REQUIRE_CALL(action, Invoke(_))
				.LR_WITH(&_1 == &param0);
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("Index 1.")
	{
		InvocableMock<void, const double&> action{};
		expectation_policies::ParamsSideEffect policy = then::apply_param<1>(std::ref(action));
		REQUIRE_CALL(action, Invoke(_))
				.LR_WITH(&_1 == &param1);
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("Index 2.")
	{
		InvocableMock<void, std::string> action{};
		expectation_policies::ParamsSideEffect policy = then::apply_param<2>(std::ref(action));
		REQUIRE_CALL(action, Invoke("Hello, World!"));
		REQUIRE_NOTHROW(policy.consume(info));
	}
}

TEST_CASE(
	"then::apply_params creates expectation_policies::ParamsSideEffect.",
	"[expectation][expectation::factories]"
)
{
	using trompeloeil::_;

	int param0{1337};
	double param1{4.2};
	std::string param2{"Hello, World!"};
	const call::Info<void, int&, double&, std::string&> info{
		.params = {param0, param1, param2}
	};

	SECTION("Indices 0, 1, 2.")
	{
		InvocableMock<void, int&, double&, std::string&> action{};
		expectation_policies::ParamsSideEffect policy = then::apply_params<0, 1, 2>(std::ref(action));
		REQUIRE_CALL(action, Invoke(_, _, _))
				.LR_WITH(&_1 == &param0)
				.LR_WITH(&_2 == &param1)
				.LR_WITH(&_3 == &param2);
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("Indices 2, 1, 0")
	{
		InvocableMock<void, std::string&, double&, int&> action{};
		expectation_policies::ParamsSideEffect policy = then::apply_params<2, 1, 0>(std::ref(action));
		REQUIRE_CALL(action, Invoke(_, _, _))
				.LR_WITH(&_1 == &param2)
				.LR_WITH(&_2 == &param1)
				.LR_WITH(&_3 == &param0);
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("Zero indices.")
	{
		InvocableMock<void> action{};
		expectation_policies::ParamsSideEffect policy = then::apply_params<>(std::ref(action));
		REQUIRE_CALL(action, Invoke());
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("Arbitrarily mixed.")
	{
		InvocableMock<void, double&, int&, std::string&, std::string&, int&> action{};
		expectation_policies::ParamsSideEffect policy = then::apply_params<1, 0, 2, 2, 0>(std::ref(action));
		REQUIRE_CALL(action, Invoke(_, _, _, _, _))
				.LR_WITH(&_1 == &param1)
				.LR_WITH(&_2 == &param0)
				.LR_WITH(&_3 == &param2)
				.LR_WITH(&_4 == &param2)
				.LR_WITH(&_5 == &param0);
		REQUIRE_NOTHROW(policy.consume(info));
	}
}

TEST_CASE(
	"then::apply_all_params creates expectation_policies::AllParamsSideEffect.",
	"[expectation][expectation::factories]"
)
{
	using trompeloeil::_;

	SECTION("When signature has zero params.")
	{
		const call::Info<void> info{
			.params = {}
		};

		InvocableMock<void> action{};
		expectation_policies::AllParamsSideEffect policy = then::apply_all_params(std::ref(action));
		REQUIRE_CALL(action, Invoke());
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("When signature has one param.")
	{
		int param0{1337};
		const call::Info<void, int&> info{
			.params = {param0}
		};

		InvocableMock<void, int&> action{};
		expectation_policies::AllParamsSideEffect policy = then::apply_all_params(std::ref(action));
		REQUIRE_CALL(action, Invoke(_))
			.LR_WITH(&_1 == &param0);
		REQUIRE_NOTHROW(policy.consume(info));
	}

	SECTION("When signature has multiple params.")
	{
		int param0{1337};
		double param1{4.2};
		const call::Info<void, int&, double&> info{
			.params = {param0, param1}
		};

		InvocableMock<void, int&, double&> action{};
		expectation_policies::AllParamsSideEffect policy = then::apply_all_params(std::ref(action));
		REQUIRE_CALL(action, Invoke(_, _))
			.LR_WITH(&_1 == &param0)
			.LR_WITH(&_2 == &param1);
		REQUIRE_NOTHROW(policy.consume(info));
	}
}

TEST_CASE(
	"then::apply creates expectation_policies::ParamsSideEffect.",
	"[expectation][expectation::factories]"
)
{
	int param0{1337};
	double param1{4.2};
	std::string param2{"Hello, World!"};
	const call::Info<void, int&, double&, std::string&> info{
		.params = {param0, param1, param2}
	};

	InvocableMock<void> action{};
	expectation_policies::ParamsSideEffect policy = then::apply(std::ref(action));
	REQUIRE_CALL(action, Invoke());
	REQUIRE_NOTHROW(policy.consume(info));
}

TEST_CASE(
	"mimicpp::expect::times and similar factories with nttp limits create expectation_policies::Times.",
	"[expectation][expectation::factories]"
)
{
	SECTION("times with binary limits.")
	{
		expectation_policies::Times times = expect::times<2, 5>();
		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();
		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(times.is_saturated());
	}

	SECTION("times with unary limits.")
	{
		expectation_policies::Times times = expect::times<3>();
		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();

		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(times.is_saturated());
	}

	SECTION("at_most")
	{
		expectation_policies::Times times = expect::at_most<3>();
		REQUIRE(times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(times.is_saturated());
	}

	SECTION("at_least")
	{
		expectation_policies::Times times = expect::at_least<3>();
		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();

		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(!times.is_saturated());

		for ([[maybe_unused]] const auto i : std::views::iota(0, 10))
		{
			times.consume();

			REQUIRE(times.is_satisfied());
			REQUIRE(!times.is_saturated());
		}
	}

	SECTION("once")
	{
		expectation_policies::Times times = expect::once();
		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(times.is_saturated());
	}

	SECTION("twice")
	{
		expectation_policies::Times times = expect::twice();
		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(times.is_saturated());
	}
}

TEST_CASE(
	"mimicpp::expect::times and similar factories with runtime limits create expectation_policies::RuntimeTimes.",
	"[expectation][expectation::factories]"
)
{
	SECTION("times with binary limits.")
	{
		expectation_policies::RuntimeTimes times = expect::times(2, 5);
		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();
		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(times.is_saturated());
	}

	SECTION("times with unary limits.")
	{
		expectation_policies::RuntimeTimes times = expect::times(3);
		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();

		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(times.is_saturated());
	}

	SECTION("at_most")
	{
		expectation_policies::RuntimeTimes times = expect::at_most(3);
		REQUIRE(times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(times.is_saturated());
	}

	SECTION("at_least")
	{
		expectation_policies::RuntimeTimes times = expect::at_least(3);
		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();
		times.consume();

		REQUIRE(!times.is_satisfied());
		REQUIRE(!times.is_saturated());

		times.consume();

		REQUIRE(times.is_satisfied());
		REQUIRE(!times.is_saturated());

		for ([[maybe_unused]] const auto i : std::views::iota(0, 10))
		{
			times.consume();

			REQUIRE(times.is_satisfied());
			REQUIRE(!times.is_saturated());
		}
	}
}

TEST_CASE(
	"mimicpp::expect::returns_result_of creates expectation_policies::ReturnsResultOf.",
	"[expectation][expectation::factories]"
)
{
	using trompeloeil::_;

	using SignatureT = int&();
	using CallInfoT = call::info_for_signature_t<SignatureT>;

	const CallInfoT call{
		.params = {},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	InvocableMock<int&> action{};
	expectation_policies::ReturnsResultOf policy = finally::returns_result_of(std::ref(action));
	STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

	int value{42};
	REQUIRE_CALL(action, Invoke())
		.LR_RETURN(value);
	int& result = policy.finalize_call(call);
	REQUIRE(&value == &result);
}

TEST_CASE(
	"mimicpp::expect::returns creates expectation_policies::ReturnsResultOf.",
	"[expectation][expectation::factories]"
)
{
	SECTION("When a value is returned.")
	{
		using CallInfoT = call::Info<int>;
		const CallInfoT call{
			.params = {},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		const int value = GENERATE(range(0, 5));

		SECTION("And when value is stored.")
		{
			expectation_policies::ReturnsResultOf policy = finally::returns(value);
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), int()>);

			REQUIRE(value == policy.finalize_call(call));
			REQUIRE(value == policy.finalize_call(call));
		}

		SECTION("And when std::reference_wrapper is stored.")
		{
			expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), int()>);

			REQUIRE(value == policy.finalize_call(call));
			REQUIRE(value == policy.finalize_call(call));
		}
	}

	SECTION("When a lvalue ref is returned.")
	{
		using CallInfoT = call::Info<int&>;
		const CallInfoT call{
			.params = {},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		SECTION("And when value is stored.")
		{
			expectation_policies::ReturnsResultOf policy = finally::returns(42);
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&()>);

			int& result = policy.finalize_call(call);
			REQUIRE(42 == result);
			REQUIRE(&result == &policy.finalize_call(call));
		}

		SECTION("And when std::reference_wrapper is stored.")
		{
			int value{42};
			expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&()>);

			REQUIRE(&value == &policy.finalize_call(call));
			REQUIRE(&value == &policy.finalize_call(call));
		}
	}

	SECTION("When a const lvalue ref is returned.")
	{
		using CallInfoT = call::Info<const int&>;
		const CallInfoT call{
			.params = {},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		SECTION("And when value is stored.")
		{
			expectation_policies::ReturnsResultOf policy = finally::returns(42);
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), const int&()>);

			const int& result = policy.finalize_call(call);
			REQUIRE(42 == result);
			REQUIRE(&result == &policy.finalize_call(call));
		}

		SECTION("And when std::reference_wrapper is stored.")
		{
			constexpr int value{42};
			expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), const int&()>);

			REQUIRE(&value == &policy.finalize_call(call));
			REQUIRE(&value == &policy.finalize_call(call));
		}
	}

	SECTION("When a rvalue ref is returned.")
	{
		using CallInfoT = call::Info<int&&>;
		const CallInfoT call{
			.params = {},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		SECTION("And when value is stored.")
		{
			expectation_policies::ReturnsResultOf policy = finally::returns(42);
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&&()>);

			int&& result = policy.finalize_call(call);
			REQUIRE(42 == result);
			REQUIRE(42 == policy.finalize_call(call));
		}

		SECTION("And when std::reference_wrapper is stored.")
		{
			int value{42};
			expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), int&&()>);

			int&& result = policy.finalize_call(call);
			REQUIRE(&value == std::addressof(result));
			int&& secondResult = policy.finalize_call(call);
			REQUIRE(&value == std::addressof(secondResult));
		}
	}

	SECTION("When a const rvalue ref is returned.")
	{
		using CallInfoT = call::Info<const int&&>;
		const CallInfoT call{
			.params = {},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		SECTION("And when value is stored.")
		{
			expectation_policies::ReturnsResultOf policy = finally::returns(42);
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), const int&&()>);

			const int&& result = policy.finalize_call(call);
			REQUIRE(42 == result);
			REQUIRE(42 == policy.finalize_call(call));
		}

		SECTION("And when std::reference_wrapper is stored.")
		{
			int value{42};
			expectation_policies::ReturnsResultOf policy = finally::returns(std::ref(value));
			STATIC_REQUIRE(finalize_policy_for<decltype(policy), const int&&()>);

			const int&& result = policy.finalize_call(call);
			REQUIRE(&value == std::addressof(result));
			const int&& secondResult = policy.finalize_call(call);
			REQUIRE(&value == std::addressof(secondResult));
		}
	}
}

TEST_CASE(
	"mimicpp::expect::returns_apply_result_of creates expectation_policies::ReturnsResultOf.",
	"[expectation][expectation::factories]"
)
{
	using trompeloeil::_;

	SECTION("When a single index is chosen.")
	{
		using SignatureT = std::string&&(int, double, std::string&&);
		using CallInfoT = call::info_for_signature_t<SignatureT>;

		int param0{1337};
		double param1{4.2};
		std::string param2{"Hello, World!"};
		const CallInfoT info{
			.params = {param0, param1, param2},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		expectation_policies::ReturnsResultOf policy = finally::returns_apply_result_of<2>(
			[](std::string& str) noexcept -> std::string&& { return std::move(str); });
		STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

		std::string&& result = policy.finalize_call(info);
		REQUIRE(&param2 == std::addressof(result));
	}

	SECTION("When an arbitrary index sequence is chosen")
	{
		using SignatureT = std::tuple<double, std::string&&, int, double&>(int, double, std::string&&);
		using CallInfoT = call::info_for_signature_t<SignatureT>;

		int param0{1337};
		double param1{4.2};
		std::string param2{"Hello, World!"};
		const CallInfoT info{
			.params = {param0, param1, param2},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		expectation_policies::ReturnsResultOf policy = finally::returns_apply_result_of<1, 2, 0, 1>(
			[](double& p0, std::string& p1, int& p2, double& p3)
			{
				return std::tuple<double, std::string&&, int, double&>{
					p0,
					std::move(p1),
					p2,
					p3
				};
			});
		STATIC_REQUIRE(finalize_policy_for<decltype(policy), SignatureT>);

		auto&& [r0, r1, r2, r3] = policy.finalize_call(info);
		REQUIRE(r0 == param1);
		REQUIRE(std::addressof(r1) == &param2);
		REQUIRE(r2 == param0);
		REQUIRE(&r3 == &param1);
	}
}

TEST_CASE(
	"mimicpp::expect::throws creates expectation_policies::Throws.",
	"[expectation][expectation::factories]"
)
{
	using CallInfoT = call::Info<int>;

	const int value = GENERATE(range(0, 5));
	expectation_policies::Throws policy = finally::throws(value);

	const CallInfoT call{
		.params = {},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	REQUIRE_THROWS_AS(
		policy.finalize_call(call),
		int);
	REQUIRE_THROWS_AS(
		policy.finalize_call(call),
		int);
}
