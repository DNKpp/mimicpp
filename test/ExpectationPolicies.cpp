// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"
#include "TestTypes.hpp"
#include "mimic++/ExpectationPolicies.hpp"

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
	(true, expectation_policies::Returns<int>, int()),
	(true, expectation_policies::Returns<int>, int(float)),
	(true, expectation_policies::Returns<float>, double()),
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
	expectation_policies::SourceLocation,
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
	const call::Info<void()> call{
		.params = {},
		.fromUuid = Uuid{1337},
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
	"expectation_policies::Times does throw, when invalid config is given.",
	"[expectation][expectation::builder]"
)
{
	const std::size_t max = GENERATE(0u, 1u, 2u, 3u, 4u);
	const std::size_t min = max + GENERATE(1u, 2u, 3u, 4u);

	REQUIRE_THROWS_AS(
		(expectation_policies::RuntimeTimes{min, max}),
		std::runtime_error);
}

TEST_CASE(
	"expectation_policies::SourceLocation just reports the stored source location.",
	"[expectation][expectation::policy]"
)
{
	using SignatureT = void();
	using CallInfoT = call::Info<SignatureT>;
	using PolicyT = expectation_policies::SourceLocation;

	constexpr source_location_data sourceLoc{std::source_location::current()};
	constexpr PolicyT policy{sourceLoc};

	SECTION("Policy is always satisfied.")
	{
		STATIC_REQUIRE(policy.is_satisfied());
	}

	const CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	SECTION("Consume is a no-op.")
	{
		REQUIRE_NOTHROW(policy.consume(call));
	}

	SECTION("Policy does always match.")
	{
		const call::SubMatchResult result = policy.matches(call);
		REQUIRE(result.matched);
		REQUIRE(result.msg);
		REQUIRE_THAT(
			*result.msg,
			Catch::Matchers::Matches(R"( expectation from .+\(\d+\:\d+\), function `.+`)")
			&& Catch::Matchers::ContainsSubstring(std::format(
				"{}({}:{})",
				sourceLoc.fileName,
				sourceLoc.line,
				sourceLoc.column))
			&& Catch::Matchers::ContainsSubstring(sourceLoc.functionName));
	}
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
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	if (matches(call.fromCategory, category))
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
	((Constness constness), constness),
	(Constness::as_const),
	(Constness::non_const),
	(Constness::any)
)
{
	using SignatureT = void();
	using CallInfoT = call::Info<SignatureT>;
	using PolicyT = expectation_policies::Constness<constness>;
	STATIC_REQUIRE(expectation_policy_for<PolicyT, SignatureT>);

	constexpr PolicyT policy{};
	SECTION("Policy is always satisfied.")
	{
		REQUIRE(policy.is_satisfied());
	}

	const CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	if (matches(call.fromConstness, constness))
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

TEST_CASE(
	"expectation_policies::Returns returns a pre-set value during finalze_call().",
	"[expectation][expectation::policy]"
)
{
	SECTION("When the exact value is returned.")
	{
		using SignatureT = int();
		using CallInfoT = call::Info<SignatureT>;
		using PolicyT = expectation_policies::Returns<int>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromUuid = Uuid{1337},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{42};

		REQUIRE(42 == policy.finalize_call(call));
	}

	SECTION("When a lvalue is returned.")
	{
		using SignatureT = int&();
		using CallInfoT = call::Info<SignatureT>;
		using PolicyT = expectation_policies::Returns<int>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromUuid = Uuid{1337},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{42};

		int& ret = policy.finalize_call(call);
		REQUIRE(42 == ret);
	}

	SECTION("When a const lvalue is returned.")
	{
		using SignatureT = const int&();
		using CallInfoT = call::Info<SignatureT>;
		using PolicyT = expectation_policies::Returns<int>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromUuid = Uuid{1337},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{42};

		const int& ret = policy.finalize_call(call);
		REQUIRE(42 == ret);
	}

	SECTION("When a rvalue is returned.")
	{
		using SignatureT = int&&();
		using CallInfoT = call::Info<SignatureT>;
		using PolicyT = expectation_policies::Returns<int>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromUuid = Uuid{1337},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{42};

		int&& ret = policy.finalize_call(call);
		REQUIRE(42 == ret);
	}

	SECTION("When a const rvalue is returned.")
	{
		using SignatureT = const int&&();
		using CallInfoT = call::Info<SignatureT>;
		using PolicyT = expectation_policies::Returns<int>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromUuid = Uuid{1337},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{42};

		const int&& ret = policy.finalize_call(call);
		REQUIRE(42 == ret);
	}

	SECTION("When a convertible value is returned.")
	{
		using SignatureT = int();
		using CallInfoT = call::Info<SignatureT>;
		using PolicyT = expectation_policies::Returns<unsigned int>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromUuid = Uuid{1337},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{42u};

		REQUIRE(42 == policy.finalize_call(call));
	}
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
		using CallInfoT = call::Info<SignatureT>;
		using PolicyT = expectation_policies::Throws<test_exception>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromUuid = Uuid{1337},
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
		using CallInfoT = call::Info<SignatureT>;
		using PolicyT = expectation_policies::Throws<test_exception>;
		STATIC_REQUIRE(finalize_policy_for<PolicyT, SignatureT>);

		const CallInfoT call{
			.params = {},
			.fromUuid = Uuid{1337},
			.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
			.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
		};

		PolicyT policy{test_exception{}};

		REQUIRE_THROWS_AS(
			policy.finalize_call(call),
			test_exception);
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
	"mimicpp::expect::returns creates expectation_policies::Returns.",
	"[expectation][expectation::factories]"
)
{
	using CallInfoT = call::Info<int()>;

	const int value = GENERATE(range(0, 5));
	expectation_policies::Returns policy = expect::returns(value);

	const CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
		.fromCategory = GENERATE(ValueCategory::lvalue, ValueCategory::rvalue, ValueCategory::any),
		.fromConstness = GENERATE(Constness::non_const, Constness::as_const, Constness::any)
	};

	REQUIRE(value == policy.finalize_call(call));
	REQUIRE(value == policy.finalize_call(call));
}

TEST_CASE(
	"mimicpp::expect::throws creates expectation_policies::Throws.",
	"[expectation][expectation::factories]"
)
{
	using CallInfoT = call::Info<int()>;

	const int value = GENERATE(range(0, 5));
	expectation_policies::Throws policy = expect::throws(value);

	const CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
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
