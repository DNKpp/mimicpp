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

#include "TestTypes.hpp"

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
	matcher::PredicateMatcher matcher{
		std::ref(predicate),
		"Hello, {}!",
		std::tuple<>{}
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
		constexpr int value{42};
		REQUIRE("Hello, 42!" == matcher.describe(value));
	}
}

TEST_CASE(
	"matcher::PredicateMatcher can be extended with matcher::InvertiblePolicy.",
	"[matcher]")
{
	MatcherPredicateMock<int> predicate{};
	matcher::PredicateMatcher<
			std::reference_wrapper<MatcherPredicateMock<int>>,
			std::tuple<>,
			matcher::InvertiblePolicy
		>
		matcher{
			std::ref(predicate),
			"Hello, {}!"
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
		constexpr int value{42};
		REQUIRE("!(Hello, 42!)" == negatedMatcher.describe(value));
	}
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

TEST_CASE(
	"matches::le matches when target value is less than or equal to the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::le(42);

	SECTION("When target is less or equal.")
	{
		const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 42);
		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(format::format("{} <= 42", target)));
	}

	SECTION("When target is greater.")
	{
		const int target = GENERATE(43, std::numeric_limits<int>::max());
		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(format::format("{} <= 42", target)));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::le(42);

		SECTION("When target is less or equal.")
		{
			const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 42);
			REQUIRE(!invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::Equals(format::format("!({} <= 42)", target)));
		}

		SECTION("When target is greater.")
		{
			const int target = GENERATE(43, std::numeric_limits<int>::max());
			REQUIRE(invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::Equals(format::format("!({} <= 42)", target)));
		}
	}
}

TEST_CASE(
	"matches::gt matches when target value is greater than the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::gt(42);

	SECTION("When target is greater.")
	{
		const int target = GENERATE(43, std::numeric_limits<int>::max());
		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(format::format("{} > 42", target)));
	}

	SECTION("When target is not greater.")
	{
		const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41, 42);
		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(format::format("{} > 42", target)));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::gt(42);

		SECTION("When target is greater.")
		{
			const int target = GENERATE(43, std::numeric_limits<int>::max());
			REQUIRE(!invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::Equals(format::format("!({} > 42)", target)));
		}

		SECTION("When target is not greater.")
		{
			const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41, 42);
			REQUIRE(invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::Equals(format::format("!({} > 42)", target)));
		}
	}
}

TEST_CASE(
	"matches::ge matches when target value is greater than or equal to the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::ge(42);

	SECTION("When target is greater or equal.")
	{
		const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(format::format("{} >= 42", target)));
	}

	SECTION("When target is less.")
	{
		const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(format::format("{} >= 42", target)));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::ge(42);

		SECTION("When target is greater or equal.")
		{
			const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
			REQUIRE(!invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::Equals(format::format("!({} >= 42)", target)));
		}

		SECTION("When target is less.")
		{
			const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
			REQUIRE(invertedMatcher.matches(target));
			REQUIRE_THAT(
				invertedMatcher.describe(target),
				Catch::Matchers::Equals(format::format("!({} >= 42)", target)));
		}
	}
}

TEST_CASE(
	"matches::predicate matches when the given predicate is satisfied.",
	"[matcher]"
)
{
	using trompeloeil::_;

	const int target = GENERATE(42, 43, std::numeric_limits<int>::max());

	InvocableMock<bool, const int&> predicate{};
	const auto expectedResult = GENERATE(true, false);
	REQUIRE_CALL(predicate, Invoke(_))
		.LR_WITH(&_1 == &target)
		.RETURN(expectedResult);

	const auto matcher = matches::predicate(std::ref(predicate));
	REQUIRE(expectedResult == matcher.matches(target));
	REQUIRE_THAT(
		matcher.describe(target),
		Catch::Matchers::Equals(format::format("{} satisfies predicate", target)));

	SECTION("When matcher is inverted.")
	{
		const auto invertedMatcher = !matches::predicate(std::ref(predicate));

		REQUIRE_CALL(predicate, Invoke(_))
			.LR_WITH(&_1 == &target)
			.RETURN(expectedResult);

		REQUIRE(expectedResult == !invertedMatcher.matches(target));
		REQUIRE_THAT(
			invertedMatcher.describe(target),
			Catch::Matchers::Equals(format::format("!({} satisfies predicate)", target)));
	}
}

TEST_CASE(
	"matches::range::eq matches when target range compares element-wise equal to the stored one.",
	"[matcher]"
)
{
	using trompeloeil::_;

	SECTION("When an empty range is stored.")
	{
		const auto matcher = matches::range::eq(std::vector<int>{});

		SECTION("When target is also empty, they match.")
		{
			const std::vector<int> target{};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range {  } is equal to {  }"));
		}

		SECTION("When target is not empty, they do not match.")
		{
			const std::vector target{42};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 42 } is equal to {  }"));
		}
	}

	SECTION("When a non-empty range is stored.")
	{
		const auto matcher = matches::range::eq(std::vector{1337, 42});

		SECTION("When target is equal, they match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 1337, 42 } is equal to { 1337, 42 }"));
		}

		SECTION("When target has same elements, but in different order, they do not match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 42, 1337 } is equal to { 1337, 42 }"));
		}

		SECTION("When target is not equal, they do not match.")
		{
			const std::vector target{42};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 42 } is equal to { 1337, 42 }"));
		}
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::eq(std::vector{1337, 42});

		SECTION("When target is equal, they do not match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 1337, 42 } is equal to { 1337, 42 })"));
		}

		SECTION("When target has same elements, but in different order, they do match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 42, 1337 } is equal to { 1337, 42 })"));
		}

		SECTION("When target is not equal, they do match.")
		{
			const std::vector target{42};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 42 } is equal to { 1337, 42 })"));
		}
	}

	SECTION("Custom comparators can be provided.")
	{
		using ComparatorT = InvocableMock<bool, int, int>;
		ComparatorT comparator{};
		const auto matcher = matches::range::eq(
			std::vector{1337, 42},
			std::ref(comparator));

		const std::vector target{1337, 42};

		REQUIRE_CALL(comparator, Invoke(1337, 1337))
			.RETURN(true);
		REQUIRE_CALL(comparator, Invoke(42, 42))
			.RETURN(true);

		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals("range { 1337, 42 } is equal to { 1337, 42 }"));
	}
}

TEST_CASE(
	"matches::range::unordered_eq matches when target range is a permuation of the stored one.",
	"[matcher]"
)
{
	using trompeloeil::_;

	SECTION("When an empty range is stored.")
	{
		const auto matcher = matches::range::unordered_eq(std::vector<int>{});

		SECTION("When target is also empty, they match.")
		{
			const std::vector<int> target{};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range {  } is permutation of {  }"));
		}

		SECTION("When target is not empty, they do not match.")
		{
			const std::vector target{42};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 42 } is permutation of {  }"));
		}
	}

	SECTION("When a non-empty range is stored.")
	{
		const auto matcher = matches::range::unordered_eq(std::vector{1337, 42});

		SECTION("When target is equal, they match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 1337, 42 } is permutation of { 1337, 42 }"));
		}

		SECTION("When target has same elements, but in different order, they do match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 42, 1337 } is permutation of { 1337, 42 }"));
		}

		SECTION("When target is not equal, they do not match.")
		{
			const std::vector target{42};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 42 } is permutation of { 1337, 42 }"));
		}
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::unordered_eq(std::vector{1337, 42});

		SECTION("When target is equal, they do not match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 1337, 42 } is permutation of { 1337, 42 })"));
		}

		SECTION("When target has same elements, but in different order, they do not match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 42, 1337 } is permutation of { 1337, 42 })"));
		}

		SECTION("When target is not equal, they do match.")
		{
			const std::vector target{42};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 42 } is permutation of { 1337, 42 })"));
		}
	}

	SECTION("Custom comparators can be provided.")
	{
		using ComparatorT = InvocableMock<bool, int, int>;
		ComparatorT comparator{};
		const auto matcher = matches::range::unordered_eq(
			std::vector{1337, 42},
			std::ref(comparator));

		const std::vector target{1337, 42};

		REQUIRE_CALL(comparator, Invoke(1337, 1337))
			.RETURN(true);
		REQUIRE_CALL(comparator, Invoke(42, 42))
			.RETURN(true);

		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals("range { 1337, 42 } is permutation of { 1337, 42 }"));
	}
}

TEST_CASE(
	"matches::range::is_sorted matches when target range is sorted.",
	"[matcher]"
)
{
	using trompeloeil::_;

	SECTION("When target is empty, it's a match.")
	{
		const auto matcher = matches::range::is_sorted();

		const std::vector<int> target{};

		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals("range {  } is sorted"));
	}

	SECTION("When a non-empty range is stored.")
	{
		const auto matcher = matches::range::is_sorted();

		SECTION("When target is sorted, it's a match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 42, 1337 } is sorted"));
		}

		SECTION("When target is not sorted, it's no match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("range { 1337, 42 } is sorted"));
		}
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::is_sorted();

		SECTION("When target is sorted, it's no match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 42, 1337 } is sorted)"));
		}

		SECTION("When target is not sorted, it's a match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 1337, 42 } is sorted)"));
		}
	}

	SECTION("Custom relations can be provided.")
	{
		using ComparatorT = InvocableMock<bool, int, int>;
		ComparatorT comparator{};
		const auto matcher = matches::range::is_sorted(
			std::ref(comparator));

		const std::vector target{1337, 42};

		REQUIRE_CALL(comparator, Invoke(42, 1337))
			.RETURN(false);

		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals("range { 1337, 42 } is sorted"));
	}
}

TEST_CASE(
	"matches::range::is_empty matches when target range is empty.",
	"[matcher]"
)
{
	using trompeloeil::_;

	SECTION("When target is empty, it's a match.")
	{
		const auto matcher = matches::range::is_empty();

		const std::vector<int> target{};

		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals("range {  } is empty"));
	}

	SECTION("When a non-empty range is stored, it's no match.")
	{
		const auto matcher = matches::range::is_empty();

		const std::vector target{42};

		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals("range { 42 } is empty"));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::is_empty();

		SECTION("When target is empty, it's no match.")
		{
			const std::vector<int> target{};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range {  } is empty)"));
		}

		SECTION("When a non-empty range is stored, it's a match.")
		{
			const std::vector target{42};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 42 } is empty)"));
		}
	}
}

TEST_CASE(
	"matches::range::has_size matches when target range has the expected size.",
	"[matcher]"
)
{
	using trompeloeil::_;

	SECTION("When target has the expected size, it's a match.")
	{
		const auto matcher = matches::range::has_size(2);
		const std::vector target{42, 1337};

		REQUIRE(matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals("range { 42, 1337 } has size 2"));
	}

	SECTION("When target has different size, it's no match.")
	{
		const auto matcher = matches::range::has_size(1);
		const std::vector target = GENERATE(
			std::vector<int>{},
			(std::vector{42, 1337}));

		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(target),
			Catch::Matchers::Equals(
				format::format(
					"range {} has size 1",
					mimicpp::print(target))));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::has_size(2);

		SECTION("When target has the expected size, it's no match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 42, 1337 } has size 2)"));
		}

		SECTION("When target has different size, it's a match.")
		{
			const std::vector target{42};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(target),
				Catch::Matchers::Equals("!(range { 42 } has size 2)"));
		}
	}
}
