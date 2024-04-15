// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Expectation.hpp"

#include <functional>
#include <optional>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace
{
	class ExpectationMock final
		: public mimicpp::Expectation<void()>
	{
	public:
		using CallInfoT = mimicpp::call::Info<void()>;

		MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept override);
		MAKE_CONST_MOCK0(is_saturated, bool(), noexcept override);
		MAKE_CONST_MOCK1(matches, bool(const CallInfoT&), noexcept override);
		MAKE_MOCK1(consume, void(const CallInfoT&), noexcept override);
	};
}

TEST_CASE(
	"mimicpp::ExpectationCollection collects expectation references.",
	"[expectation]"
)
{
	using StorageT = mimicpp::ExpectationCollection<void()>;

	StorageT storage{};
	auto expectation = std::make_shared<ExpectationMock>();

	REQUIRE_NOTHROW(storage.push(expectation));

	ALLOW_CALL(*expectation, is_satisfied())
		.RETURN(true);
	REQUIRE_NOTHROW(storage.remove(expectation));
}

TEST_CASE(
	"mimicpp::ExpectationCollection queries expectation, whether they match the call.",
	"[expectation]"
)
{
	using StorageT = mimicpp::ExpectationCollection<void()>;
	using CallInfoT = mimicpp::call::Info<void()>;
	using trompeloeil::_;

	StorageT storage{};
	std::vector<std::shared_ptr<ExpectationMock>> expectations(4);
	for (auto& exp : expectations)
	{
		exp = std::make_shared<ExpectationMock>();
		storage.push(exp);
	}

	const CallInfoT call{
		.params = {},
		.fromUuid = 0,
		.fromCategory = mimicpp::call::ValueCategory::lvalue,
		.fromConst = false
	};

	trompeloeil::sequence sequence{};
	REQUIRE_CALL(*expectations[0], is_saturated())
		.IN_SEQUENCE(sequence)
		.RETURN(true);
	REQUIRE_CALL(*expectations[1], is_saturated())
		.IN_SEQUENCE(sequence)
		.RETURN(false);
	REQUIRE_CALL(*expectations[1], matches(_))
		.LR_WITH(&_1 == &call)
		.IN_SEQUENCE(sequence)
		.RETURN(false);
	REQUIRE_CALL(*expectations[2], is_saturated())
		.IN_SEQUENCE(sequence)
		.RETURN(false);
	REQUIRE_CALL(*expectations[2], matches(_))
		.LR_WITH(&_1 == &call)
		.IN_SEQUENCE(sequence)
		.RETURN(true);
	// expectations[3] is never queried
	REQUIRE_CALL(*expectations[2], consume(_))
		.LR_WITH(&_1 == &call)
		.IN_SEQUENCE(sequence);

	REQUIRE(storage.consume(call));

	SECTION("All expectations are still part of the collection.")
	{
		REQUIRE_CALL(*expectations[0], is_saturated())
			.IN_SEQUENCE(sequence)
			.RETURN(true);
		REQUIRE_CALL(*expectations[1], is_saturated())
			.IN_SEQUENCE(sequence)
			.RETURN(true);
		REQUIRE_CALL(*expectations[2], is_saturated())
			.IN_SEQUENCE(sequence)
			.RETURN(true);
		REQUIRE_CALL(*expectations[3], is_saturated())
			.IN_SEQUENCE(sequence)
			.RETURN(false);
		REQUIRE_CALL(*expectations[3], matches(_))
			.LR_WITH(&_1 == &call)
			.IN_SEQUENCE(sequence)
			.RETURN(false);

		REQUIRE(!storage.consume(call));
	}
}

namespace
{
	template <typename Signature>
	class PolicyMock
	{
	public:
		using CallInfoT = mimicpp::call::Info<Signature>;

		static constexpr bool trompeloeil_movable_mock = true;

		MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept);
		MAKE_CONST_MOCK0(is_saturated, bool(), noexcept);
		MAKE_CONST_MOCK1(matches, bool(const CallInfoT&), noexcept);
		MAKE_MOCK1(consume, void(const CallInfoT&), noexcept);
	};

	template <typename Signature>
	class PolicyFake
	{
	public:
		using CallInfoT = mimicpp::call::Info<Signature>;

		[[nodiscard]]
		static constexpr bool is_satisfied() noexcept
		{
			return false;
		}

		[[nodiscard]]
		static constexpr bool is_saturated() noexcept
		{
			return false;
		}

		[[nodiscard]]
		static constexpr bool matches(const CallInfoT& call) noexcept
		{
			return matches(call);
		}

		static constexpr void consume(const CallInfoT& call) noexcept
		{
		}
	};

	template <typename Signature, typename Policy, typename Projection>
	// enable, when trompeloeil fully supports movable mocks
	//requires mimicpp::expectation_policy_for<
	//	std::remove_cvref_t<std::invoke_result_t<Projection, Policy>>,
	//	Signature>
	class PolicyFacade
	{
	public:
		using CallT = mimicpp::call::Info<Signature>;

		Policy policy{};
		Projection projection{};

		[[nodiscard]]
		bool is_satisfied() const noexcept
		{
			return std::invoke(projection, policy)
				.is_satisfied();
		}

		[[nodiscard]]
		bool is_saturated() const noexcept
		{
			return std::invoke(projection, policy)
				.is_saturated();
		}

		[[nodiscard]]
		bool matches(const CallT& call) const noexcept
		{
			return std::invoke(projection, policy)
				.matches(call);
		}

		void consume(const CallT& call) noexcept
		{
			std::invoke(projection, policy)
				.consume(call);
		}
	};

	class UnwrapReferenceWrapper
	{
	public:
		template <typename T>
		constexpr T& operator ()(const std::reference_wrapper<T> ref) const noexcept
		{
			return ref.get();
		}
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

TEMPLATE_TEST_CASE(
	"mimicpp::BasicExpectation can be extended with an arbitrary policy amount.",
	"[expectation]",
	void(),
	int()
)
{
	using trompeloeil::_;
	using PolicyMockT = PolicyMock<TestType>;
	using PolicyRefT = PolicyFacade<TestType, std::reference_wrapper<PolicyMock<TestType>>, UnwrapReferenceWrapper>;
	using CallInfoT = mimicpp::call::Info<TestType>;

	const CallInfoT call{
		.params = {},
		.fromUuid = 0,
		.fromCategory = mimicpp::call::ValueCategory::lvalue,
		.fromConst = false
	};

	SECTION("With no policies at all.")
	{
		mimicpp::BasicExpectation<TestType> expectation{};

		REQUIRE(std::as_const(expectation).is_satisfied());
		REQUIRE(!std::as_const(expectation).is_saturated());
		REQUIRE(std::as_const(expectation).matches(call));
		REQUIRE_NOTHROW(expectation.consume(call));
	}

	SECTION("With one policy.")
	{
		PolicyMockT policy{};
		mimicpp::BasicExpectation<TestType, PolicyRefT> expectation{PolicyRefT{std::ref(policy)}};

		const bool isSatisfied = GENERATE(false, true);
		REQUIRE_CALL(policy, is_satisfied())
			.RETURN(isSatisfied);
		REQUIRE(isSatisfied == std::as_const(expectation).is_satisfied());

		const bool isSaturated = GENERATE(false, true);
		REQUIRE_CALL(policy, is_saturated())
			.RETURN(isSaturated);
		REQUIRE(isSaturated == std::as_const(expectation).is_saturated());

		const bool matches = GENERATE(false, true);
		REQUIRE_CALL(policy, matches(_))
			.LR_WITH(&_1 == &call)
			.RETURN(matches);
		REQUIRE(matches == std::as_const(expectation).matches(call));

		REQUIRE_CALL(policy, consume(_))
			.LR_WITH(&_1 == &call);
		expectation.consume(call);
	}

	SECTION("With two policies.")
	{
		PolicyMockT policy1{};
		PolicyMockT policy2{};
		mimicpp::BasicExpectation<TestType, PolicyRefT, PolicyRefT> expectation{
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

		SECTION("When calling is_saturated()")
		{
			const bool isSaturated1 = GENERATE(false, true);
			const bool isSaturated2 = GENERATE(false, true);
			const bool expectedIsSaturated = isSaturated1 || isSaturated2;
			REQUIRE_CALL(policy1, is_saturated())
				.RETURN(isSaturated1);
			auto policy2Expectation = std::invoke(
				[&]() -> std::unique_ptr<trompeloeil::expectation>
				{
					if (!isSaturated1)
					{
						return NAMED_REQUIRE_CALL(policy2, is_saturated())
							.RETURN(isSaturated2);
					}
					return nullptr;
				});

			REQUIRE(expectedIsSaturated == std::as_const(expectation).is_saturated());
		}

		SECTION("When calling matches()")
		{
			const bool matches1 = GENERATE(false, true);
			const bool matches2 = GENERATE(false, true);
			const bool expectedMatches = matches1 && matches2;
			REQUIRE_CALL(policy1, matches(_))
				.LR_WITH(&_1 == &call)
				.RETURN(matches1);
			auto policy2Expectation = std::invoke(
				[&]() -> std::unique_ptr<trompeloeil::expectation>
				{
					if (matches1)
					{
						return NAMED_REQUIRE_CALL(policy2, matches(_))
								.LR_WITH(&_1 == &call)
								.RETURN(matches2);
					}
					return nullptr;
				});

			REQUIRE(expectedMatches == std::as_const(expectation).matches(call));
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

TEST_CASE("ScopedExpectation is a non-copyable, but movable type.")
{
	using ScopedExpectationT = mimicpp::ScopedExpectation<void()>;

	STATIC_REQUIRE(!std::is_copy_constructible_v<ScopedExpectationT>);
	STATIC_REQUIRE(!std::is_copy_assignable_v<ScopedExpectationT>);
	STATIC_REQUIRE(std::is_move_constructible_v<ScopedExpectationT>);
	STATIC_REQUIRE(std::is_move_assignable_v<ScopedExpectationT>);
}

TEST_CASE(
	"ScopedExpectation handles expectation lifetime and reports when not satisfied.",
	"[expectation]")
{
	using trompeloeil::_;
	using PolicyMockT = PolicyMock<void()>;
	using PolicyRefT = PolicyFacade<void(), std::reference_wrapper<PolicyMock<void()>>, UnwrapReferenceWrapper>;
	using ExpectationT = mimicpp::BasicExpectation<void(), PolicyRefT>;
	using ScopedExpectationT = mimicpp::ScopedExpectation<void()>;
	using CollectionT = mimicpp::ExpectationCollection<void()>;

	auto collection = std::make_shared<CollectionT>();

	PolicyMockT policy{};
	std::optional<ScopedExpectationT> expectation{
		std::in_place,
		collection,
		std::make_unique<ExpectationT>(std::ref(policy))
	};

	SECTION("When calling is_satisfied()")
	{
		const bool isSatisfied = GENERATE(false, true);
		REQUIRE_CALL(policy, is_satisfied())
			.RETURN(isSatisfied);
		REQUIRE(isSatisfied == std::as_const(expectation)->is_satisfied());
	}

	SECTION("When calling is_saturated()")
	{
		const bool isSaturated = GENERATE(false, true);
		REQUIRE_CALL(policy, is_saturated())
			.RETURN(isSaturated);
		REQUIRE(isSaturated == std::as_const(expectation)->is_saturated());
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

		SECTION("When calling is_saturated()")
		{
			const bool isSaturated = GENERATE(false, true);
			REQUIRE_CALL(policy, is_saturated())
				.RETURN(isSaturated);
			REQUIRE(isSaturated == std::as_const(otherExpectation).is_saturated());
			REQUIRE_THROWS_AS(std::as_const(expectation)->is_saturated(), std::runtime_error);
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

			SECTION("When calling is_saturated()")
			{
				const bool isSaturated = GENERATE(false, true);
				REQUIRE_CALL(policy, is_saturated())
					.RETURN(isSaturated);
				REQUIRE(isSaturated == std::as_const(expectation)->is_saturated());
				REQUIRE_THROWS_AS(std::as_const(otherExpectation).is_saturated(), std::runtime_error);
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

			SECTION("When calling is_saturated()")
			{
				const bool isSaturated = GENERATE(false, true);
				REQUIRE_CALL(policy, is_saturated())
					.RETURN(isSaturated);
				REQUIRE(isSaturated == std::as_const(otherExpectation).is_saturated());
			}
		}

		// just move back, so we can unify the cleanup process
		expectation = std::move(otherExpectation);
	}

	// collection asserts on that in remove
	ALLOW_CALL(policy, is_satisfied())
		.RETURN(true);
	expectation.reset();
}
