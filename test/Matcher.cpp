// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Matcher.hpp"
#include "mimic++/Printer.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace mimicpp;

TEMPLATE_TEST_CASE(
	"Given types satisfy mimicpp::matcher_for concept.",
	"[matcher]",
	std::remove_const_t<decltype(matches::_)>,
	std::remove_const_t<decltype(matches::eq(42))>,
	std::remove_const_t<decltype(!matches::eq(42))>
)
{
	STATIC_REQUIRE(matcher_for<TestType, int>);
	STATIC_REQUIRE(matcher_for<TestType, const int>);
	STATIC_REQUIRE(matcher_for<TestType, int&>);
	STATIC_REQUIRE(matcher_for<TestType, const int&>);
	STATIC_REQUIRE(matcher_for<TestType, int&&>);
	STATIC_REQUIRE(matcher_for<TestType, const int&&>);
}

namespace
{
	template <typename T>
	class MatcherPredicateMock
	{
	public:
		MAKE_CONST_MOCK1(check, bool(T));

		[[nodiscard]]
		constexpr bool operator ()(T value) const
		{
			return check(value);
		}
	};

	template <typename T>
	class MatcherDescriberMock
	{
	public:
		MAKE_CONST_MOCK1(describe, StringT(T));

		[[nodiscard]]
		constexpr StringT operator ()(T value) const
		{
			return describe(value);
		}
	};
}

TEST_CASE(
	"matcher::PredicateMatcher is a generic matcher.",
	"[matcher]")
{
	MatcherPredicateMock<int> predicate{};
	MatcherDescriberMock<int> describer{};
	matcher::PredicateMatcher matcher{
		std::ref(predicate),
		std::ref(describer)
	};

	SECTION("When matches() is called, argument is forwarded to the predicate.")
	{
		const bool result = GENERATE(true, false);

		REQUIRE_CALL(predicate, check(42))
			.RETURN(result);

		constexpr int value{42};
		REQUIRE(result == matcher.matches(value));
	}

	SECTION("When describe() is called, argument is forwarded to the functional.")
	{
		REQUIRE_CALL(describer, describe(42))
			.RETURN("Hello, World!");

		constexpr int value{42};
		REQUIRE("Hello, World!" == matcher.describe(value));
	}
}

TEST_CASE(
	"matcher::PredicateMatcher can be extended with matcher::InvertiblePolicy.",
	"[matcher]")
{
	MatcherPredicateMock<int> predicate{};
	MatcherDescriberMock<int> describer{};
	matcher::PredicateMatcher<
			std::reference_wrapper<MatcherPredicateMock<int>>,
			std::reference_wrapper<MatcherDescriberMock<int>>,
			matcher::InvertiblePolicy
		>
		matcher{
			std::ref(predicate),
			std::ref(describer)
		};

	matcher::PredicateMatcher negatedMatcher = !std::move(matcher);
	STATIC_REQUIRE(matcher_for<decltype(negatedMatcher), int>);

	SECTION("When matches() is called, argument is forwarded to the predicate.")
	{
		const bool result = GENERATE(true, false);

		REQUIRE_CALL(predicate, check(42))
			.RETURN(result);

		constexpr int value{42};
		REQUIRE(result == !negatedMatcher.matches(value));
	}

	SECTION("When describe() is called, argument is forwarded to the functional.")
	{
		REQUIRE_CALL(describer, describe(42))
				.RETURN("Hello, World!");

		constexpr int value{42};
		REQUIRE("!(Hello, World!)" == negatedMatcher.describe(value));
	}
}

TEST_CASE(
	"matcher::TargetPredicateDescriber serves as a helper for pre-defined formatting descriptions.",
	"[matcher]"
)
{
	const matcher::TargetPredicateDescriber describer{"Hello, {} World!"};

	constexpr int value{42};
	REQUIRE("Hello, 42 World!" == describer(value));

	const StringT str{" test "};
	REQUIRE("Hello,  test  World!" == describer(str));
}

TEST_CASE(
	"matches::_ matches always.",
	"[matcher]"
)
{
	constexpr int value{42};
	REQUIRE(matches::_.matches(value));
	REQUIRE_THAT(
		matches::_.describe(value),
		Catch::Matchers::EndsWith(" without constraints")
		&& Catch::Matchers::StartsWith("42"));
}

TEST_CASE(
	"matches::eq matches when target value compares equal to the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::eq(42);

	SECTION("When target is equal.")
	{
		constexpr int target{42};
		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::EndsWith(" == 42")
			&& Catch::Matchers::StartsWith("42"));
	}

	SECTION("When target is not equal.")
	{
		constexpr int target{1337};
		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::EndsWith(" == 42")
			&& Catch::Matchers::StartsWith("1337"));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::eq(42);

		SECTION("When target is equal.")
		{
			constexpr int target{42};
			REQUIRE(!invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::EndsWith(" == 42)")
				&& Catch::Matchers::StartsWith("!(42"));
		}

		SECTION("When target is not equal.")
		{
			constexpr int target{1337};
			REQUIRE(invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::EndsWith(" == 42)")
				&& Catch::Matchers::StartsWith("!(1337"));
		}
	}
}

TEST_CASE(
	"matches::ne matches when target value does not compare equal to the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::ne(42);

	SECTION("When target is not equal.")
	{
		constexpr int target{1337};
		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::EndsWith(" != 42")
			&& Catch::Matchers::StartsWith("1337"));
	}

	SECTION("When target is equal.")
	{
		constexpr int target{42};
		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::EndsWith(" != 42")
			&& Catch::Matchers::StartsWith("42"));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::ne(42);

		SECTION("When target is not equal.")
		{
			constexpr int target{1337};
			REQUIRE(!invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::EndsWith(" != 42)")
				&& Catch::Matchers::StartsWith("!(1337"));
		}

		SECTION("When target is equal.")
		{
			constexpr int target{42};
			REQUIRE(invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::EndsWith(" != 42)")
				&& Catch::Matchers::StartsWith("!(42"));
		}
	}
}

TEST_CASE(
	"matches::lt matches when target value is less than the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::lt(42);

	SECTION("When target is less.")
	{
		const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(format::format("{} < 42", target)));
	}

	SECTION("When target is not less.")
	{
		const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(format::format("{} < 42", target)));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::lt(42);

		SECTION("When target is less.")
		{
			const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
			REQUIRE(!invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::Equals(format::format("!({} < 42)", target)));
		}

		SECTION("When target is not less.")
		{
			const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
			REQUIRE(invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::Equals(format::format("!({} < 42)", target)));
		}
	}
}
