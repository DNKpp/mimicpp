// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/ExpectationPolicies/CallProperties.hpp"
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

TEMPLATE_TEST_CASE_SIG(
	"expectation_policies::Category checks whether the given call::Info matches.",
	"[expectation][expectation::policy]",
	((ValueCategory category), category),
	(ValueCategory::lvalue),
	(ValueCategory::rvalue)
)
{
	using SignatureT = void();
	using CallInfoT = call::Info<SignatureT>;
	using PolicyT = expectation_policies::Category<category>;
	STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

	constexpr PolicyT policy{};
	SECTION("Policy is always satisfied.")
	{
		REQUIRE(policy.is_satisfied());
	}

	const CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue),
		.fromConst = false
	};

	if (call.fromCategory == category)
	{
		SECTION("When call and policy category matches, success is returned.")
		{
			const auto result = policy.matches(call);

			REQUIRE(result.matched);
			REQUIRE_THAT(
				result.msg.value(),
				Catch::Matchers::Equals(std::format(" matches Category {}", category)));
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
				Catch::Matchers::Equals(std::format(" does not match Category {}", category)));
		}
	}
}

TEMPLATE_TEST_CASE_SIG(
	"expectation_policies::Constness checks whether the given call::Info matches.",
	"[expectation][expectation::policy]",
	((bool constness), constness),
	(true),
	(false)
)
{
	using SignatureT = void();
	using CallInfoT = call::Info<SignatureT>;
	using PolicyT = expectation_policies::Constness<SignatureT, constness>;
	STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

	constexpr PolicyT policy{};
	SECTION("Policy is always satisfied.")
	{
		REQUIRE(policy.is_satisfied());
	}

	const CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
		.fromCategory = ValueCategory::lvalue,
		.fromConst = GENERATE(true, false)
	};

	if (call.fromConst == constness)
	{
		SECTION("When call and policy constness matches, success is returned.")
		{
			const auto result = policy.matches(call);

			REQUIRE(result.matched);
			REQUIRE_THAT(
				result.msg.value(),
				Catch::Matchers::Equals(std::format(" matches Constness {}", constness)));
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
				Catch::Matchers::Equals(std::format(" does not match Constness {}", constness)));
		}
	}
}

namespace
{
	template <typename T>
	class PredicateMock
	{
	public:
		MAKE_CONST_MOCK1(Call, bool(const T&));

		[[nodiscard]]
		auto operator ()(const T& value) const
		{
			return Call(value);
		}
	};

	template <typename T>
	class DescribeMock
	{
	public:
		MAKE_CONST_MOCK2(Call, std::optional<StringT>(bool, const T&));

		[[nodiscard]]
		auto operator ()(const bool result, const T& value) const
		{
			return Call(result, value);
		}
	};
}

TEST_CASE(
	"expectation_policies::ArgumentMatcher checks whether the given call::Info matches.",
	"[expectation][expectation::policy]"
)
{
	SECTION("Policy works on unary signature.")
	{
		using SignatureT = void(int);
		using CallInfoT = call::Info<SignatureT>;

		PredicateMock<int> predicate{};
		DescribeMock<int> describe{};
		const auto policy = expectation_policies::make_argument_matcher<SignatureT, 0>(
			std::ref(predicate),
			std::ref(describe)
		);
		STATIC_REQUIRE(expectation_policy_for<std::remove_const_t<decltype(policy)>, SignatureT>);

		int param{42};
		const CallInfoT call{
			.params = {param},
			.fromUuid = Uuid{1337},
			.fromCategory = ValueCategory::lvalue,
			.fromConst = false
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
					{false, " does not match"},
					{true, " matches"}
					}));

			trompeloeil::sequence sequence{};
			REQUIRE_CALL(predicate, Call(42))
				.IN_SEQUENCE(sequence)
				.RETURN(match);
			REQUIRE_CALL(describe, Call(match, 42))
				.IN_SEQUENCE(sequence)
				.RETURN(msg);

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
		using CallInfoT = call::Info<SignatureT>;

		PredicateMock<int> predicate1{};
		DescribeMock<int> describe1{};
		const auto policy1 = expectation_policies::make_argument_matcher<SignatureT, 0>(
			std::ref(predicate1),
			std::ref(describe1)
		);
		STATIC_REQUIRE(expectation_policy_for<std::remove_const_t<decltype(policy1)>, SignatureT>);

		PredicateMock<std::string> predicate2{};
		DescribeMock<std::string> describe2{};
		const auto policy2 = expectation_policies::make_argument_matcher<SignatureT, 1>(
			std::ref(predicate2),
			std::ref(describe2)
		);
		STATIC_REQUIRE(expectation_policy_for<std::remove_const_t<decltype(policy2)>, SignatureT>);

		int param0{42};
		const std::string param1{"Hello, World!"};
		const CallInfoT call{
			.params = {param0, param1},
			.fromUuid = Uuid{1337},
			.fromCategory = ValueCategory::lvalue,
			.fromConst = false
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
						{false, " does not match"},
						{true, " matches"}
						}));

				trompeloeil::sequence sequence{};
				REQUIRE_CALL(predicate1, Call(42))
					.IN_SEQUENCE(sequence)
					.RETURN(match);
				REQUIRE_CALL(describe1, Call(match, 42))
					.IN_SEQUENCE(sequence)
					.RETURN(msg);

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
						{false, " does not match"},
						{true, " matches"}
						}));

				trompeloeil::sequence sequence{};
				REQUIRE_CALL(predicate2, Call(param1))
					.IN_SEQUENCE(sequence)
					.RETURN(match);
				REQUIRE_CALL(describe2, Call(match, param1))
					.IN_SEQUENCE(sequence)
					.RETURN(msg);

				const auto result = policy2.matches(call);
				REQUIRE(match == result.matched);
				REQUIRE_THAT(
					result.msg.value(),
					Catch::Matchers::Equals(msg));
			}
		}
	}
}
