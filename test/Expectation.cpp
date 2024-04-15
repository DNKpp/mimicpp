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
