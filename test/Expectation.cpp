// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Expectation.hpp"

#include <functional>

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
		using CallT = mimicpp::Call<void()>;

		MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept override);
		MAKE_CONST_MOCK0(is_saturated, bool(), noexcept override);
		MAKE_CONST_MOCK1(matches, bool(const CallT&), noexcept override);
		MAKE_MOCK1(consume, void(const CallT&), noexcept override);
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
	using CallT = mimicpp::Call<void()>;
	using trompeloeil::_;

	StorageT storage{};
	std::vector<std::shared_ptr<ExpectationMock>> expectations(4);
	for (auto& exp : expectations)
	{
		exp = std::make_shared<ExpectationMock>();
		storage.push(exp);
	}

	const CallT call{
		.params = {},
		.fromUuid = 0,
		.fromCategory = mimicpp::ValueCategory::lvalue,
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
		using CallT = mimicpp::Call<Signature>;

		static constexpr bool trompeloeil_movable_mock = true;

		MAKE_CONST_MOCK0(is_satisfied, bool(), noexcept);
		MAKE_CONST_MOCK0(is_saturated, bool(), noexcept);
		MAKE_CONST_MOCK1(matches, bool(const CallT&), noexcept);
		MAKE_MOCK1(consume, void(const CallT&), noexcept);
	};

	template <typename Signature>
	class PolicyFake
	{
	public:
		using CallT = mimicpp::Call<Signature>;

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
		static constexpr bool matches(const CallT& call) noexcept
		{
			return matches(call);
		}

		static constexpr void consume(const CallT& call) noexcept
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
		using CallT = mimicpp::Call<Signature>;

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

