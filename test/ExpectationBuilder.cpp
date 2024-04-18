#include "mimic++/ExpectationBuilder.hpp"
#include "TestReporter.hpp"
#include "TestTypes.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

using namespace mimicpp;

TEMPLATE_TEST_CASE_SIG(
	"Times policy of mimicpp::BasicExpectationBuilder can be exchanged only once.",
	"[expectation][expectation::builder]",
	((bool expected, typename Builder, typename Policy), expected, Builder, Policy),
	(true, BasicExpectationBuilder<void(), expectation_policies::InitTimes, FinalizerFake<void()>>, TimesFake),
	(true, BasicExpectationBuilder<int(), expectation_policies::InitTimes, FinalizerFake<int()>>, TimesFake),
	(false, BasicExpectationBuilder<void(), TimesFake, FinalizerFake<void()>>, TimesFake),
	(false,BasicExpectationBuilder<int(), TimesFake, FinalizerFake<int()>>, TimesFake)
)
{
	STATIC_REQUIRE(expected == requires{ std::declval<Builder&&>() | std::declval<Policy&&>(); });
}

TEST_CASE(
	"Times policy of mimicpp::BasicExpectationBuilder may be exchanged.",
	"[expectation][expectation::builder]"
)
{
	using trompeloeil::_;

	using SignatureT = void();
	using BaseBuilderT = BasicExpectationBuilder<SignatureT, expectation_policies::InitTimes, expectation_policies::InitFinalize>;
	using ScopedExpectationT = ScopedExpectation<SignatureT>;
	using CallInfoT = call::Info<SignatureT>;

	auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
		.fromCategory = ValueCategory::lvalue,
		.fromConst = false
	};

	BaseBuilderT builder{
		collection,
		expectation_policies::InitTimes{},
		expectation_policies::InitFinalize{},
		std::tuple{}};

	SECTION("It is allowed to omit the times policy.")
	{
		ScopedExpectationT expectation = std::move(builder);

		REQUIRE(!expectation.is_satisfied());
		REQUIRE_NOTHROW(collection->handle_call(call));
		REQUIRE(expectation.is_satisfied());
	}

	SECTION("Or exchange it once.")
	{
		using TimesPolicyT = TimesFacade<
			std::reference_wrapper<TimesMock>,
			UnwrapReferenceWrapper>;
		TimesMock times{};

		// during destruction
		REQUIRE_CALL(times, is_satisfied())
			.RETURN(true);
		ScopedExpectationT expectation = std::move(builder)
										| TimesPolicyT{std::ref(times)};

		SECTION("When is_satisfied() is called.")
		{
			const bool isSatisfied = GENERATE(false, true);
			REQUIRE_CALL(times, is_satisfied())
				.RETURN(isSatisfied);
			REQUIRE(isSatisfied == expectation.is_satisfied());
		}

		SECTION("When handle_call() is called.")
		{
			SECTION("And when times is not saturated.")
			{
				REQUIRE_CALL(times, is_saturated())
					.RETURN(false);
				REQUIRE_CALL(times, consume());
				REQUIRE_NOTHROW(collection->handle_call(call));
			}

			SECTION("And when times is saturated.")
			{
				REQUIRE_CALL(times, is_saturated())
					.RETURN(true);
				REQUIRE_THROWS_AS(
					collection->handle_call(call),
					TestExpectationError);
			}
		}
	}
}

TEMPLATE_TEST_CASE_SIG(
	"Finalize policy of mimicpp::BasicExpectationBuilder can be exchanged only once.",
	"[expectation][expectation::builder]",
	((bool expected, typename Builder, typename Policy), expected, Builder, Policy),
	(true, mimicpp::BasicExpectationBuilder<void(), TimesFake, expectation_policies::InitFinalize>, FinalizerFake<void()>),
	(true, mimicpp::BasicExpectationBuilder<int(), TimesFake, expectation_policies::InitFinalize>, FinalizerFake<int()>),
	(false, mimicpp::BasicExpectationBuilder<void(), TimesFake, FinalizerFake<void()>>, FinalizerFake<void()>),
	(false, mimicpp::BasicExpectationBuilder<int(), TimesFake, FinalizerFake<int()>>, FinalizerFake<int()>)
)
{
	STATIC_REQUIRE(expected == requires{ std::declval<Builder&&>() | std::declval<Policy&&>(); });
}

TEST_CASE(
	"Finalize policy of mimicpp::BasicExpectationBuilder for void return may be exchanged.",
	"[expectation][expectation::builder]"
)
{
	using trompeloeil::_;

	using SignatureT = void();
	using BaseBuilderT = BasicExpectationBuilder<SignatureT, TimesFake, expectation_policies::InitFinalize>;
	using ScopedExpectationT = ScopedExpectation<SignatureT>;
	using CallInfoT = call::Info<SignatureT>;

	auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
		.fromCategory = ValueCategory::lvalue,
		.fromConst = false
	};

	BaseBuilderT builder{
		collection,
		TimesFake{},
		expectation_policies::InitFinalize{},
		std::tuple{}};

	SECTION("It is allowed to omit the finalize policy.")
	{
		ScopedExpectationT expectation = std::move(builder);

		REQUIRE_NOTHROW(expectation.is_satisfied());
		REQUIRE_NOTHROW(collection->handle_call(call));
	}

	SECTION("Or exchange it once.")
	{
		using FinalizerT = FinalizerMock<SignatureT>;
		using FinalizerPolicyT = FinalizerFacade<
			SignatureT,
			std::reference_wrapper<FinalizerMock<SignatureT>>,
			UnwrapReferenceWrapper>;
		FinalizerT finalizer{};
		ScopedExpectationT expectation = std::move(builder)
										| FinalizerPolicyT{std::ref(finalizer)};

		REQUIRE_CALL(finalizer, finalize_call(_))
			.LR_WITH(&_1 == &call);

		REQUIRE_NOTHROW(expectation.is_satisfied());
		REQUIRE_NOTHROW(collection->handle_call(call));
	}
}

TEST_CASE(
	"Finalize policy of mimicpp::BasicExpectationBuilder for non-void return must be exchanged.",
	"[expectation][expectation::builder]"
)
{
	using trompeloeil::_;

	using SignatureT = int();
	using BaseBuilderT = BasicExpectationBuilder<SignatureT, TimesFake, expectation_policies::InitFinalize>;
	using ScopedExpectationT = ScopedExpectation<SignatureT>;
	using CallInfoT = call::Info<SignatureT>;

	auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.params = {},
		.fromUuid = Uuid{1337},
		.fromCategory = ValueCategory::lvalue,
		.fromConst = false
	};

	using FinalizerT = FinalizerMock<SignatureT>;
	using FinalizerPolicyT = FinalizerFacade<
		SignatureT,
		std::reference_wrapper<FinalizerMock<SignatureT>>,
		UnwrapReferenceWrapper>;
	FinalizerT finalizer{};
	ScopedExpectationT expectation = BaseBuilderT{collection, TimesFake{}, expectation_policies::InitFinalize{}, std::tuple{}}
									| FinalizerPolicyT{std::ref(finalizer)};

	REQUIRE_CALL(finalizer, finalize_call(_))
		.LR_WITH(&_1 == &call)
		.RETURN(0);

	REQUIRE_NOTHROW(expectation.is_satisfied());
	REQUIRE_NOTHROW(collection->handle_call(call));
}

TEST_CASE(
	"mimicpp::BasicExpectationBuilder allows expectation extension via suitable polices.",
	"[expectation][expectation::builder]"
)
{
	using SignatureT = void();
	using BaseBuilderT = BasicExpectationBuilder<SignatureT, TimesFake, expectation_policies::InitFinalize>;
	using ScopedExpectationT = ScopedExpectation<SignatureT>;
	using ExpectationPolicyT = PolicyMock<SignatureT>;
	using PolicyT = PolicyFacade<SignatureT, std::reference_wrapper<ExpectationPolicyT>, UnwrapReferenceWrapper>;

	auto collection = std::make_shared<ExpectationCollection<SignatureT>>();

	SECTION("Just once.")
	{
		ExpectationPolicyT policy{};

		// in ExpectationCollection::remove
		REQUIRE_CALL(policy, is_satisfied())
			.RETURN(true);

		ScopedExpectationT expectation = BaseBuilderT{
											collection,
											TimesFake{.isSatisfied = true},
											expectation_policies::InitFinalize{},
											std::tuple{}}
										| PolicyT{std::ref(policy)};

		REQUIRE_CALL(policy, is_satisfied())
			.RETURN(true);
		REQUIRE(expectation.is_satisfied());
	}

	SECTION("Just twice.")
	{
		ExpectationPolicyT policy1{};
		ExpectationPolicyT policy2{};

		// in ExpectationCollection::remove
		REQUIRE_CALL(policy1, is_satisfied())
			.RETURN(true);
		REQUIRE_CALL(policy2, is_satisfied())
			.RETURN(true);

		ScopedExpectationT expectation = BaseBuilderT{
											collection,
											TimesFake{.isSatisfied = true},
											expectation_policies::InitFinalize{},
											std::tuple{}}
										| PolicyT{std::ref(policy1)}
										| PolicyT{std::ref(policy2)};

		REQUIRE_CALL(policy1, is_satisfied())
			.RETURN(true);
		REQUIRE_CALL(policy2, is_satisfied())
			.RETURN(true);
		REQUIRE(expectation.is_satisfied());
	}
}
