// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "TestReporter.hpp"

#include "mimic++/ExpectationBuilder.hpp"

#include "TestTypes.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace mimicpp;

namespace
{
	constexpr auto zeroTimesConfig = std::invoke(
		[]
		{
			TimesConfig config{};
			config.set_limits(0, 0);
			return config;
		});

	using InitSeqConfigT = sequence::detail::Config<>;
}

TEMPLATE_TEST_CASE_SIG(
	"Times limites of mimicpp::BasicExpectationBuilder can be exchanged only once.",
	"[expectation][expectation::builder]",
	((bool expected, typename Builder, typename Policy), expected, Builder, Policy),
	(false, mimicpp::BasicExpectationBuilder<true, InitSeqConfigT, void(), expectation_policies::InitFinalize>, FinalizerFake<void()>),
	(false, mimicpp::BasicExpectationBuilder<true, InitSeqConfigT, int(), expectation_policies::InitFinalize>, FinalizerFake<int()>),
	(true, mimicpp::BasicExpectationBuilder<false, InitSeqConfigT, void(), expectation_policies::InitFinalize>, FinalizerFake<void()>),
	(true, mimicpp::BasicExpectationBuilder<false, InitSeqConfigT, int(), expectation_policies::InitFinalize>, FinalizerFake<int()>),
	(false, mimicpp::BasicExpectationBuilder<true, InitSeqConfigT, void(), FinalizerFake<void()>>, FinalizerFake<void()>),
	(false, mimicpp::BasicExpectationBuilder<true, InitSeqConfigT, int(), FinalizerFake<int()>>, FinalizerFake<int()>),
	(true, mimicpp::BasicExpectationBuilder<false, InitSeqConfigT, void(), FinalizerFake<void()>>, FinalizerFake<void()>),
	(true, mimicpp::BasicExpectationBuilder<false, InitSeqConfigT, int(), FinalizerFake<int()>>, FinalizerFake<int()>)
)
{
	STATIC_REQUIRE(expected == requires{ std::declval<Builder&&>() | TimesConfig{}; });
}

TEST_CASE(
	"BasicExpectationBuilder times-limits can be configured.",
	"[expectation][expectation::builder]"
)
{
	using SignatureT = void();
	using BaseBuilderT = BasicExpectationBuilder<false, InitSeqConfigT, SignatureT, expectation_policies::InitFinalize>;
	using ScopedExpectationT = ScopedExpectation<SignatureT>;
	using CallInfoT = call::info_for_signature_t<SignatureT>;

	ScopedReporter reporter{};

	auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.args = {},
		.fromCategory = ValueCategory::any,
		.fromConstness = Constness::any
	};

	BaseBuilderT builder{
		collection,
		TimesConfig{},
		InitSeqConfigT{},
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
		ScopedExpectationT expectation = std::move(builder)
										| zeroTimesConfig;

		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"BasicExpectationBuilder sequences can be configured.",
	"[expectation][expectation::builder]"
)
{
	using SignatureT = void();
	using BaseBuilderT = BasicExpectationBuilder<false, InitSeqConfigT, SignatureT, expectation_policies::InitFinalize>;
	using ScopedExpectationT = ScopedExpectation<SignatureT>;
	using CallInfoT = call::info_for_signature_t<SignatureT>;

	ScopedReporter reporter{};

	auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.args = {},
		.fromCategory = ValueCategory::any,
		.fromConstness = Constness::any
	};

	BaseBuilderT builder{
		collection,
		TimesConfig{},
		InitSeqConfigT{},
		expectation_policies::InitFinalize{},
		std::tuple{}};

	SequenceT sequence{};

	SECTION("Can be specified once.")
	{
		ScopedExpectationT expectation = std::move(builder)
										| expect::in_sequence(sequence);

		REQUIRE(!expectation.is_satisfied());
		REQUIRE_NOTHROW(collection->handle_call(call));
		REQUIRE(expectation.is_satisfied());
	}

	SECTION("Can be specified with times.")
	{
		ScopedExpectationT expectation = std::move(builder)
										| expect::twice()
										| expect::in_sequence(sequence);

		REQUIRE(!expectation.is_satisfied());
		REQUIRE_NOTHROW(collection->handle_call(call));
		REQUIRE(!expectation.is_satisfied());
		REQUIRE_NOTHROW(collection->handle_call(call));
		REQUIRE(expectation.is_satisfied());
	}

	SECTION("Can be specified multiple times.")
	{
		SequenceT secondSequence{};
		ScopedExpectationT expectation = std::move(builder)
										| expect::in_sequence(sequence)
										| expect::in_sequence(secondSequence);

		REQUIRE(!expectation.is_satisfied());
		REQUIRE_NOTHROW(collection->handle_call(call));
		REQUIRE(expectation.is_satisfied());
	}
}

TEMPLATE_TEST_CASE_SIG(
	"Finalize policy of mimicpp::BasicExpectationBuilder can be exchanged only once.",
	"[expectation][expectation::builder]",
	((bool expected, typename Builder, typename Policy), expected, Builder, Policy),
	(true, mimicpp::BasicExpectationBuilder<true, InitSeqConfigT, void(), expectation_policies::InitFinalize>, FinalizerFake<void()>),
	(true, mimicpp::BasicExpectationBuilder<true, InitSeqConfigT, int(), expectation_policies::InitFinalize>, FinalizerFake<int()>),
	(true, mimicpp::BasicExpectationBuilder<false, InitSeqConfigT, void(), expectation_policies::InitFinalize>, FinalizerFake<void()>),
	(true, mimicpp::BasicExpectationBuilder<false, InitSeqConfigT, int(), expectation_policies::InitFinalize>, FinalizerFake<int()>),
	(false, mimicpp::BasicExpectationBuilder<true, InitSeqConfigT, void(), FinalizerFake<void()>>, FinalizerFake<void()>),
	(false, mimicpp::BasicExpectationBuilder<true, InitSeqConfigT, int(), FinalizerFake<int()>>, FinalizerFake<int()>),
	(false, mimicpp::BasicExpectationBuilder<false, InitSeqConfigT, void(), FinalizerFake<void()>>, FinalizerFake<void()>),
	(false, mimicpp::BasicExpectationBuilder<false, InitSeqConfigT, int(), FinalizerFake<int()>>, FinalizerFake<int()>)
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
	using BaseBuilderT = BasicExpectationBuilder<false, InitSeqConfigT, SignatureT, expectation_policies::InitFinalize>;
	using ScopedExpectationT = ScopedExpectation<SignatureT>;
	using CallInfoT = call::info_for_signature_t<SignatureT>;

	auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.args = {},
		.fromCategory = ValueCategory::any,
		.fromConstness = Constness::any
	};

	BaseBuilderT builder{
		collection,
		TimesConfig{},
		InitSeqConfigT{},
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
	using BaseBuilderT = BasicExpectationBuilder<false, InitSeqConfigT, SignatureT, expectation_policies::InitFinalize>;
	using ScopedExpectationT = ScopedExpectation<SignatureT>;
	using CallInfoT = call::info_for_signature_t<SignatureT>;

	auto collection = std::make_shared<ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.args = {},
		.fromCategory = ValueCategory::any,
		.fromConstness = Constness::any
	};

	using FinalizerT = FinalizerMock<SignatureT>;
	using FinalizerPolicyT = FinalizerFacade<
		SignatureT,
		std::reference_wrapper<FinalizerMock<SignatureT>>,
		UnwrapReferenceWrapper>;
	FinalizerT finalizer{};
	ScopedExpectationT expectation = BaseBuilderT{
										collection,
										TimesConfig{},
										InitSeqConfigT{},
										expectation_policies::InitFinalize{},
										std::tuple{}
									}
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
	using BaseBuilderT = BasicExpectationBuilder<false, InitSeqConfigT, SignatureT, expectation_policies::InitFinalize>;
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
											zeroTimesConfig,
											InitSeqConfigT{},
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
											zeroTimesConfig,
											InitSeqConfigT{},
											expectation_policies::InitFinalize{},
											std::tuple{}
										}
										| PolicyT{std::ref(policy1)}
										| PolicyT{std::ref(policy2)};

		REQUIRE_CALL(policy1, is_satisfied())
			.RETURN(true);
		REQUIRE_CALL(policy2, is_satisfied())
			.RETURN(true);
		REQUIRE(expectation.is_satisfied());
	}
}

TEST_CASE(
	"ScopedExpectation forwards source_location to finalize.",
	"[expectation][expectation::builder]"
)
{
	namespace Matches = Catch::Matchers;

	using SignatureT = void();
	using FinalizerT = FinalizerFake<SignatureT>;
	using BaseBuilderT = BasicExpectationBuilder<false, InitSeqConfigT, SignatureT, FinalizerT>;

	const auto collection = std::make_shared<ExpectationCollection<SignatureT>>();

	const std::source_location beforeLoc = std::source_location::current();
	ScopedExpectation expectation = BaseBuilderT{
		collection,
		zeroTimesConfig,
		InitSeqConfigT{},
		FinalizerT{},
		std::tuple{}
	};
	const std::source_location afterLoc = std::source_location::current();

	const auto& inner = dynamic_cast<const BasicExpectation<SignatureT, ControlPolicy<>, FinalizerT>&>(
		expectation.expectation());
	REQUIRE_THAT(
		inner.from().file_name(),
		Matches::Equals(beforeLoc.file_name()));
	REQUIRE_THAT(
		inner.from().function_name(),
		Matches::Equals(beforeLoc.function_name()));
	REQUIRE(beforeLoc.line() < inner.from().line());
	REQUIRE(inner.from().line() < afterLoc.line());
}

TEST_CASE(
	"MIMICPP_SCOPED_EXPECTATION ScopedExpectation with unique name from a builder.",
	"[expectation][expectation::builder]"
)
{
	using SignatureT = void();
	using BaseBuilderT = BasicExpectationBuilder<false, InitSeqConfigT, SignatureT, expectation_policies::InitFinalize>;

	ScopedReporter reporter{};

	{
		const auto collection = std::make_shared<ExpectationCollection<SignatureT>>();

		MIMICPP_SCOPED_EXPECTATION BaseBuilderT{
			collection,
			zeroTimesConfig,
			InitSeqConfigT{},
			expectation_policies::InitFinalize{},
			std::tuple{}};

		MIMICPP_SCOPED_EXPECTATION BaseBuilderT{
			collection,
			zeroTimesConfig,
			InitSeqConfigT{},
			expectation_policies::InitFinalize{},
			std::tuple{}};
	}

	REQUIRE_THAT(
		reporter.unfulfilled_expectations(),
		Catch::Matchers::IsEmpty());
}
