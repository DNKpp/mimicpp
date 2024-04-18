// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Expectation.hpp"
#include "TestReporter.hpp"
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
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace
{
	class ExpectationMock final
		: public mimicpp::Expectation<void()>
	{
	public:
		using CallInfoT = mimicpp::call::Info<void()>;
		using MatchResultT = mimicpp::call::MatchResultT;

		MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept override);
		MAKE_CONST_MOCK1(matches, MatchResultT(const CallInfoT&), override);
		MAKE_MOCK1(consume, void(const CallInfoT&), override);
		MAKE_MOCK1(finalize_call, void(const CallInfoT&), override);
	};
}

TEST_CASE(
	"mimicpp::ExpectationCollection collects expectations and reports when they are removed but unsatisfied.",
	"[expectation]"
)
{
	using StorageT = mimicpp::ExpectationCollection<void()>;

	StorageT storage{};
	auto expectation = std::make_shared<ExpectationMock>();

	REQUIRE_NOTHROW(storage.push(expectation));

	mimicpp::ScopedReporter reporter{};
	SECTION("When expectation is satisfied, nothing is reported.")
	{
		REQUIRE_CALL(*expectation, is_satisfied())
			.RETURN(true);
		REQUIRE_NOTHROW(storage.remove(expectation));
		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::IsEmpty());
	}

	SECTION("When expectation is unsatisfied, get's reported.")
	{
		REQUIRE_CALL(*expectation, is_satisfied())
			.RETURN(false);
		REQUIRE_NOTHROW(storage.remove(expectation));
		REQUIRE_THAT(
			reporter.unsatisfied_expectations(),
			Catch::Matchers::SizeIs(1));
		auto reportedExpectation = std::any_cast<std::shared_ptr<mimicpp::Expectation<void()>>>(
			reporter.unsatisfied_expectations().at(0));
		REQUIRE(expectation == reportedExpectation);
	}
}

TEST_CASE(
	"mimicpp::ExpectationCollection queries expectation, whether they match the call.",
	"[expectation]"
)
{
	using namespace mimicpp::call;
	using StorageT = mimicpp::ExpectationCollection<void()>;
	using CallInfoT = Info<void()>;
	using trompeloeil::_;

	mimicpp::ScopedReporter reporter{};
	StorageT storage{};
	std::vector<std::shared_ptr<ExpectationMock>> expectations(4);
	for (auto& exp : expectations)
	{
		exp = std::make_shared<ExpectationMock>();
		storage.push(exp);
	}

	constexpr CallInfoT call{
		.params = {},
		.fromUuid = mimicpp::Uuid{1337},
		.fromCategory = mimicpp::ValueCategory::lvalue,
		.fromConst = false
	};

	SECTION("If a full match is found.")
	{
		trompeloeil::sequence sequence{};
		REQUIRE_CALL(*expectations[0], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(MatchResult_NoT{});
		REQUIRE_CALL(*expectations[1], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(MatchResult_NoT{});
		REQUIRE_CALL(*expectations[2], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(MatchResult_OkT{});
		// expectations[3] is never queried
		REQUIRE_CALL(*expectations[2], consume(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence);
		REQUIRE_CALL(*expectations[2], finalize_call(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence);

		REQUIRE_NOTHROW(storage.handle_call(call));
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.partial_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.ok_match_reports(),
			Catch::Matchers::SizeIs(1));
	}

	SECTION("If none matches full, but at least one matches partially.")
	{
		const auto [count, result0, result1, result2, result3] = GENERATE(
			(table<std::size_t, MatchResultT, MatchResultT, MatchResultT, MatchResultT>)(
				{
				{1u, MatchResult_PartialT{}, MatchResult_NoT{}, MatchResult_NoT{}, MatchResult_NoT{}},
				{1u, MatchResult_NoT{}, MatchResult_PartialT{}, MatchResult_NoT{}, MatchResult_NoT{}},
				{1u, MatchResult_NoT{}, MatchResult_NoT{}, MatchResult_PartialT{}, MatchResult_NoT{}},
				{1u, MatchResult_NoT{}, MatchResult_NoT{}, MatchResult_NoT{}, MatchResult_PartialT{}},

				{2u, MatchResult_PartialT{}, MatchResult_NoT{}, MatchResult_PartialT{}, MatchResult_NoT{}},
				{2u, MatchResult_NoT{}, MatchResult_PartialT{}, MatchResult_NoT{}, MatchResult_PartialT{}},
				{3u, MatchResult_PartialT{}, MatchResult_NoT{}, MatchResult_PartialT{}, MatchResult_PartialT{}},
				{4u, MatchResult_PartialT{}, MatchResult_PartialT{}, MatchResult_PartialT{}, MatchResult_PartialT{}}
				}));

		trompeloeil::sequence sequence{};
		REQUIRE_CALL(*expectations[0], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(result0);
		REQUIRE_CALL(*expectations[1], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(result1);
		REQUIRE_CALL(*expectations[2], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(result2);
		REQUIRE_CALL(*expectations[3], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(result3);

		REQUIRE_THROWS_AS(
			storage.handle_call(call),
			TestExpectationError);
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.partial_match_reports(),
			Catch::Matchers::SizeIs(count));
		REQUIRE_THAT(
			reporter.ok_match_reports(),
			Catch::Matchers::IsEmpty());
	}

	SECTION("If all do not match.")
	{
		trompeloeil::sequence sequence{};
		REQUIRE_CALL(*expectations[0], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(MatchResult_NoT{});
		REQUIRE_CALL(*expectations[1], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(MatchResult_NoT{});
		REQUIRE_CALL(*expectations[2], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(MatchResult_NoT{});
		REQUIRE_CALL(*expectations[3], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(MatchResult_NoT{});

		REQUIRE_THROWS_AS(
			storage.handle_call(call),
			TestExpectationError);
		REQUIRE_THAT(
			reporter.no_match_reports(),
			Catch::Matchers::SizeIs(4));
		REQUIRE_THAT(
			reporter.partial_match_reports(),
			Catch::Matchers::IsEmpty());
		REQUIRE_THAT(
			reporter.ok_match_reports(),
			Catch::Matchers::IsEmpty());
	}
}

namespace
{
	template <typename Signature>
	class PolicyMock
	{
	public:
		using CallInfoT = mimicpp::call::Info<Signature>;
		using SubMatchT = mimicpp::call::SubMatchResult;

		static constexpr bool trompeloeil_movable_mock = true;

		MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept);
		MAKE_CONST_MOCK1(matches, SubMatchT(const CallInfoT&), noexcept);
		MAKE_MOCK1(consume, void(const CallInfoT&), noexcept);
	};

	template <typename Signature>
	class FinalizerMock
	{
	public:
		using CallInfoT = mimicpp::call::Info<Signature>;
		using ReturnT = mimicpp::signature_return_type_t<Signature>;

		MAKE_MOCK1(finalize_call, ReturnT(const CallInfoT&));
	};

	class TimesMock
	{
	public:
		MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept);
		MAKE_CONST_MOCK0(is_saturated, bool(), noexcept);
		MAKE_MOCK0(consume, void());
	};
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

namespace
{
	const std::array allSubMatchResultAlternatives = std::to_array<mimicpp::call::SubMatchResult>(
		{
			{.matched = false},
			{.matched = true}
		});

	class CallMatchCategoryMatcher final
		: public Catch::Matchers::MatcherGenericBase
	{
	public:
		using CategoryT = mimicpp::call::MatchCategory;

		[[nodiscard]]
		explicit CallMatchCategoryMatcher(const CategoryT category) noexcept
			: m_Category{category}
		{
		}

		[[maybe_unused]] bool match(const auto& result) const
		{
			return m_Category == std::visit(
						[](const auto& inner) { return inner.value; },
						result);
		}

		std::string describe() const override
		{
			return std::format(
				"matches category: {}",
				m_Category);
		}

	private:
		CategoryT m_Category;
	};

	[[nodiscard]]
	CallMatchCategoryMatcher matches_category(const mimicpp::call::MatchCategory category) noexcept
	{
		return CallMatchCategoryMatcher{category};
	}

	template <typename Range>
		requires std::same_as<bool, std::ranges::range_value_t<Range>>
	[[nodiscard]]
	CallMatchCategoryMatcher matches_match_result_combination(Range&& results) noexcept
	{
		using CategoryT = mimicpp::call::MatchCategory;
		const CategoryT category = std::invoke(
			[&]
			{
				if (std::ranges::all_of(results, std::bind_front(std::equal_to{}, true)))
				{
					return CategoryT::ok;
				}

				if (std::ranges::all_of(results, std::bind_front(std::equal_to{}, false)))
				{
					return CategoryT::no;
				}

				return CategoryT::partial;
			});

		return matches_category(category);
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
	using CallInfoT = mimicpp::call::Info<TestType>;

	const CallInfoT call{
		.params = {},
		.fromUuid = mimicpp::Uuid{1337},
		.fromCategory = mimicpp::ValueCategory::lvalue,
		.fromConst = false
	};

	SECTION("With no policies at all.")
	{
		mimicpp::BasicExpectation<TestType, TimesFake, FinalizerT> expectation{
			TimesFake{.isSatisfied = true},
			FinalizerT{}
		};

		REQUIRE(std::as_const(expectation).is_satisfied());
		REQUIRE(std::holds_alternative<mimicpp::call::MatchResult_OkT>(std::as_const(expectation).matches(call)));
		REQUIRE_NOTHROW(expectation.consume(call));
	}

	SECTION("With one policy.")
	{
		PolicyMockT policy{};
		mimicpp::BasicExpectation<TestType, TimesFake, FinalizerT, PolicyRefT> expectation{
			TimesFake{.isSatisfied = true},
			FinalizerT{},
			PolicyRefT{std::ref(policy)}
		};

		const bool isSatisfied = GENERATE(false, true);
		REQUIRE_CALL(policy, is_satisfied())
			.RETURN(isSatisfied);
		REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());

		const auto matchesResult = GENERATE(from_range(allSubMatchResultAlternatives));
		REQUIRE_CALL(policy, matches(_))
			.LR_WITH(&_1 == &call)
			.RETURN(matchesResult);
		REQUIRE_THAT(
			std::as_const(expectation).matches(call),
			matches_match_result_combination(std::array{matchesResult.matched}));

		REQUIRE_CALL(policy, consume(_))
			.LR_WITH(&_1 == &call);
		expectation.consume(call);
	}

	SECTION("With two policies.")
	{
		PolicyMockT policy1{};
		PolicyMockT policy2{};
		mimicpp::BasicExpectation<TestType, TimesFake, FinalizerT, PolicyRefT, PolicyRefT> expectation{
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

		SECTION("When calling matches()")
		{
			const auto matchResult1 = GENERATE(from_range(allSubMatchResultAlternatives));
			const auto matchResult2 = GENERATE(from_range(allSubMatchResultAlternatives));

			REQUIRE_CALL(policy1, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(matchResult1);
			REQUIRE_CALL(policy2, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(matchResult2);

			REQUIRE_THAT(
				std::as_const(expectation).matches(call),
				matches_match_result_combination(std::array{matchResult1.matched, matchResult2.matched}));
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
	using CallInfoT = mimicpp::call::Info<SignatureT>;

	const CallInfoT call{
		.params = {},
		.fromUuid = mimicpp::Uuid{1337},
		.fromCategory = mimicpp::ValueCategory::lvalue,
		.fromConst = false
	};

	FinalizerT finalizer{};
	mimicpp::BasicExpectation<SignatureT, TimesFake, FinalizerRefT> expectation{
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
	using PolicyMockT = PolicyMock<void()>;
	using PolicyRefT = PolicyFacade<void(), std::reference_wrapper<PolicyMock<void()>>, UnwrapReferenceWrapper>;
	using FinalizerT = FinalizerFake<void()>;
	using ExpectationT = mimicpp::BasicExpectation<void(), TimesFake, FinalizerT, PolicyRefT>;
	using ScopedExpectationT = mimicpp::ScopedExpectation<void()>;
	using CollectionT = mimicpp::ExpectationCollection<void()>;

	auto collection = std::make_shared<CollectionT>();

	PolicyMockT policy{};
	std::optional<ScopedExpectationT> expectation{
		std::in_place,
		collection,
		std::make_unique<ExpectationT>(
			TimesFake{.isSatisfied = true},
			FinalizerT{},
			std::ref(policy))
	};

	SECTION("When calling is_satisfied()")
	{
		const bool isSatisfied = GENERATE(false, true);
		REQUIRE_CALL(policy, is_satisfied())
			.RETURN(isSatisfied);
		REQUIRE(isSatisfied == std::as_const(expectation)->is_satisfied());
	}

	SECTION("When ScopedExpectation is moved.")
	{
		ScopedExpectationT otherExpectation = *std::move(expectation);

		SECTION("When calling is_satisfied()")
		{
			const bool isSatisfied = GENERATE(false, true);
			REQUIRE_CALL(policy, is_satisfied())
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
				REQUIRE_CALL(policy, is_satisfied())
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
				REQUIRE_CALL(policy, is_satisfied())
					.RETURN(isSatisfied);
				REQUIRE(isSatisfied == std::as_const(otherExpectation).is_satisfied());
			}
		}

		// just move back, so we can unify the cleanup process
		expectation = std::move(otherExpectation);
	}

	// indirectly via remove
	REQUIRE_CALL(policy, is_satisfied())
		.RETURN(true);
	expectation.reset();
}

TEMPLATE_TEST_CASE_SIG(
	"Finalize policy of mimicpp::BasicExpectationBuilder can be exchanged only once.",
	"[expectation]",
	((bool expected, typename Builder, typename Policy), expected, Builder, Policy),
	(true, mimicpp::BasicExpectationBuilder<void(), TimesFake, mimicpp::InitFinalizePolicy>, FinalizerFake<void()>),
	(true, mimicpp::BasicExpectationBuilder<int(), TimesFake, mimicpp::InitFinalizePolicy>, FinalizerFake<int()>),
	(false, mimicpp::BasicExpectationBuilder<void(), TimesFake, FinalizerFake<void()>>, FinalizerFake<void()>),
	(false, mimicpp::BasicExpectationBuilder<int(), TimesFake, FinalizerFake<int()>>, FinalizerFake<int()>)
)
{
	STATIC_REQUIRE(expected == requires{ std::declval<Builder&&>() | std::declval<Policy&&>(); });
}

TEST_CASE(
	"Finalize policy of mimicpp::BasicExpectationBuilder for void return may be exchanged.",
	"[expectation]"
)
{
	using trompeloeil::_;

	using SignatureT = void();
	using BaseBuilderT = mimicpp::BasicExpectationBuilder<SignatureT, TimesFake, mimicpp::InitFinalizePolicy>;
	using ScopedExpectationT = mimicpp::ScopedExpectation<SignatureT>;
	using CallInfoT = mimicpp::call::Info<SignatureT>;

	auto collection = std::make_shared<mimicpp::ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.params = {},
		.fromUuid = mimicpp::Uuid{1337},
		.fromCategory = mimicpp::ValueCategory::lvalue,
		.fromConst = false
	};

	BaseBuilderT builder{
		collection,
		TimesFake{},
		mimicpp::InitFinalizePolicy{},
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
	"[expectation]"
)
{
	using trompeloeil::_;

	using SignatureT = int();
	using BaseBuilderT = mimicpp::BasicExpectationBuilder<SignatureT, TimesFake, mimicpp::InitFinalizePolicy>;
	using ScopedExpectationT = mimicpp::ScopedExpectation<SignatureT>;
	using CallInfoT = mimicpp::call::Info<SignatureT>;

	auto collection = std::make_shared<mimicpp::ExpectationCollection<SignatureT>>();
	constexpr CallInfoT call{
		.params = {},
		.fromUuid = mimicpp::Uuid{1337},
		.fromCategory = mimicpp::ValueCategory::lvalue,
		.fromConst = false
	};

	using FinalizerT = FinalizerMock<SignatureT>;
	using FinalizerPolicyT = FinalizerFacade<
		SignatureT,
		std::reference_wrapper<FinalizerMock<SignatureT>>,
		UnwrapReferenceWrapper>;
	FinalizerT finalizer{};
	ScopedExpectationT expectation = BaseBuilderT{collection, TimesFake{}, mimicpp::InitFinalizePolicy{}, std::tuple{}}
									| FinalizerPolicyT{std::ref(finalizer)};

	REQUIRE_CALL(finalizer, finalize_call(_))
		.LR_WITH(&_1 == &call)
		.RETURN(0);

	REQUIRE_NOTHROW(expectation.is_satisfied());
	REQUIRE_NOTHROW(collection->handle_call(call));
}

TEST_CASE(
	"mimicpp::BasicExpectationBuilder allows expectation extension via suitable polices.",
	"[expectation]"
)
{
	using SignatureT = void();
	using BaseBuilderT = mimicpp::BasicExpectationBuilder<SignatureT, TimesFake, mimicpp::InitFinalizePolicy>;
	using ScopedExpectationT = mimicpp::ScopedExpectation<SignatureT>;
	using ExpectationPolicyT = PolicyMock<SignatureT>;
	using PolicyT = PolicyFacade<SignatureT, std::reference_wrapper<ExpectationPolicyT>, UnwrapReferenceWrapper>;

	auto collection = std::make_shared<mimicpp::ExpectationCollection<SignatureT>>();

	SECTION("Just once.")
	{
		ExpectationPolicyT policy{};

		// in ExpectationCollection::remove
		REQUIRE_CALL(policy, is_satisfied())
			.RETURN(true);

		ScopedExpectationT expectation = BaseBuilderT{
											collection,
											TimesFake{.isSatisfied = true},
											mimicpp::InitFinalizePolicy{},
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
											mimicpp::InitFinalizePolicy{},
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
