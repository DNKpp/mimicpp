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

namespace
{
	class CommonMatcher
	{
		MAKE_CONST_MOCK1(matches, bool(const int&));
		MAKE_CONST_MOCK0(describe, StringViewT());
	};

	class CustomMatcher
	{
		MAKE_CONST_MOCK1(my_matches, bool(const int&));
		MAKE_CONST_MOCK0(my_describe, StringViewT());
	};

	class Mixed1Matcher
	{
		MAKE_CONST_MOCK1(matches, bool(const int&));
		MAKE_CONST_MOCK0(my_describe, StringViewT());
	};

	class Mixed2Matcher
	{
		MAKE_CONST_MOCK1(my_matches, bool(const int&));
		MAKE_CONST_MOCK0(describe, StringViewT());
	};
}

template <>
struct custom::matcher_traits<CustomMatcher>
{
	[[nodiscard]]
	static bool matches(const CustomMatcher& matcher, const int& value)
	{
		return matcher.my_matches(value);
	}

	[[nodiscard]]
	static StringViewT describe(const CustomMatcher& matcher)
	{
		return matcher.my_describe();
	}
};

template <>
struct custom::matcher_traits<Mixed1Matcher>
{
	[[nodiscard]]
	static StringViewT describe(const Mixed1Matcher& matcher)
	{
		return matcher.my_describe();
	}
};

template <>
struct custom::matcher_traits<Mixed2Matcher>
{
	[[nodiscard]]
	static bool matches(const Mixed2Matcher& matcher, const int& value)
	{
		return matcher.my_matches(value);
	}
};

TEST_CASE(
	"detail::matches_hook::matches chooses correct implementation.",
	"[matcher][matcher::detail]"
)
{
	using trompeloeil::_;

	const bool result = GENERATE(true, false);
	int value = 42;

	SECTION("For member matches.")
	{
		SECTION("For pure common matchers.")
		{
			CommonMatcher matcher{};
			REQUIRE_CALL(matcher, matches(_))
				.LR_WITH(&_1 == &value)
				.RETURN(result);

			REQUIRE(result == detail::matches_hook::matches(matcher, value));
		}

		SECTION("For mixed matchers.")
		{
			Mixed1Matcher matcher{};
			REQUIRE_CALL(matcher, matches(_))
				.LR_WITH(&_1 == &value)
				.RETURN(result);

			REQUIRE(result == detail::matches_hook::matches(matcher, value));
		}
	}

	SECTION("For custom matches.")
	{
		SECTION("For pure custom matchers.")
		{
			CustomMatcher matcher{};
			REQUIRE_CALL(matcher, my_matches(_))
				.LR_WITH(&_1 == &value)
				.RETURN(result);

			REQUIRE(result == detail::matches_hook::matches(matcher, value));
		}

		SECTION("For mixed matchers.")
		{
			Mixed2Matcher matcher{};
			REQUIRE_CALL(matcher, my_matches(_))
				.LR_WITH(&_1 == &value)
				.RETURN(result);

			REQUIRE(result == detail::matches_hook::matches(matcher, value));
		}
	}
}

TEST_CASE(
	"detail::describe_hook::describe chooses correct implementation.",
	"[matcher][matcher::detail]"
)
{
	namespace Matches = Catch::Matchers;

	const StringViewT result = GENERATE("Hello, World!", " Hello, 2, World! ");

	SECTION("For member describe.")
	{
		SECTION("For pure common matchers.")
		{
			CommonMatcher matcher{};
			REQUIRE_CALL(matcher, describe())
				.RETURN(result);

			REQUIRE_THAT(
				StringT{detail::describe_hook::describe(matcher)},
				Matches::Equals(StringT{result}));
		}

		SECTION("For mixed matchers.")
		{
			Mixed2Matcher matcher{};
			REQUIRE_CALL(matcher, describe())
				.RETURN(result);

			REQUIRE_THAT(
				StringT{detail::describe_hook::describe(matcher)},
				Matches::Equals(StringT{result}));
		}
	}

	SECTION("For custom describe.")
	{
		SECTION("For pure custom matchers.")
		{
			CustomMatcher matcher{};
			REQUIRE_CALL(matcher, my_describe())
				.RETURN(result);

			REQUIRE_THAT(
				StringT{detail::describe_hook::describe(matcher)},
				Matches::Equals(StringT{result}));
		}

		SECTION("For mixed matchers.")
		{
			Mixed1Matcher matcher{};
			REQUIRE_CALL(matcher, my_describe())
				.RETURN(result);

			REQUIRE_THAT(
				StringT{detail::describe_hook::describe(matcher)},
				Matches::Equals(StringT{result}));
		}
	}
}

TEMPLATE_TEST_CASE(
	"Given types satisfy mimicpp::matcher_for concept.",
	"[matcher]",
	std::remove_const_t<decltype(matches::_)>,
	std::remove_const_t<decltype(matches::eq(42))>,
	std::remove_const_t<decltype(!matches::eq(42))>,
	CommonMatcher,
	CustomMatcher,
	Mixed1Matcher,
	Mixed2Matcher
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
}

TEST_CASE(
	"matcher::PredicateMatcher is a generic matcher.",
	"[matcher]")
{
	MatcherPredicateMock<int> predicate{};
	PredicateMatcher matcher{
		std::ref(predicate),
		"Hello, World!",
		"not Hello, World!"
	};

	SECTION("When matches() is called, argument is forwarded to the predicate.")
	{
		const bool result = GENERATE(true, false);

		REQUIRE_CALL(predicate, check(42))
			.RETURN(result);

		constexpr int value{42};
		REQUIRE(result == matcher.matches(value));
	}

	SECTION("When describe() is called.")
	{
		REQUIRE("Hello, World!" == matcher.describe());
	}
}

TEST_CASE(
	"matcher::PredicateMatcher can be negated.",
	"[matcher]")
{
	SECTION("As rvalue.")
	{
		MatcherPredicateMock<int> predicate{};
		PredicateMatcher matcher{
			std::ref(predicate),
			"Hello, World!",
			"not Hello, World!"
		};

		PredicateMatcher negatedMatcher = !std::move(matcher);
		STATIC_REQUIRE(matcher_for<decltype(negatedMatcher), int>);

		SECTION("When matches() is called, argument is forwarded to the predicate.")
		{
			const bool result = GENERATE(true, false);

			REQUIRE_CALL(predicate, check(42))
				.RETURN(result);

			constexpr int value{42};
			REQUIRE(result == !negatedMatcher.matches(value));
		}

		SECTION("When describe() is called.")
		{
			REQUIRE("not Hello, World!" == negatedMatcher.describe());
		}
	}

	SECTION("As const lvalue.")
	{
		MatcherPredicateMock<int> predicate{};
		const PredicateMatcher matcher{
			std::ref(predicate),
			"Hello, World!",
			"not Hello, World!"
		};

		PredicateMatcher negatedMatcher = !matcher;
		STATIC_REQUIRE(matcher_for<decltype(negatedMatcher), int>);

		SECTION("When matches() is called, argument is forwarded to the predicate.")
		{
			const bool result = GENERATE(true, false);

			REQUIRE_CALL(predicate, check(42))
				.RETURN(result);

			constexpr int value{42};
			REQUIRE(result == !negatedMatcher.matches(value));
		}

		SECTION("When describe() is called.")
		{
			REQUIRE("not Hello, World!" == negatedMatcher.describe());
		}

		SECTION("And original matcher is still working.")
		{
			SECTION("When matches() is called, argument is forwarded to the predicate.")
			{
				const bool result = GENERATE(true, false);

				REQUIRE_CALL(predicate, check(42))
					.RETURN(result);

				constexpr int value{42};
				REQUIRE(result == matcher.matches(value));
			}

			SECTION("When describe() is called.")
			{
				REQUIRE("Hello, World!" == matcher.describe());
			}
		}
	}
}

TEST_CASE(
	"matches::_ matches always.",
	"[matcher]"
)
{
	namespace Matches = Catch::Matchers;

	using AnyT = std::remove_cvref_t<decltype(matches::_)>;
	STATIC_REQUIRE(matcher_for<AnyT, int>);
	STATIC_REQUIRE(matcher_for<AnyT, const std::string&>);

	constexpr int value{42};
	REQUIRE(matches::_.matches(value));
	REQUIRE_THAT(
		StringT{matches::_.describe()},
		Matches::Equals("has no constraints"));
}

TEST_CASE(
	"matches::eq matches when target value compares equal to the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::eq(42);
	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("== 42"));

	SECTION("When target is equal.")
	{
		constexpr int target{42};
		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is not equal.")
	{
		constexpr int target{1337};
		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::eq(42);

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("!= 42"));

		SECTION("When target is equal.")
		{
			constexpr int target{42};
			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is not equal.")
		{
			constexpr int target{1337};
			REQUIRE(invertedMatcher.matches(target));
		}
	}
}

TEST_CASE(
	"matches::ne matches when target value does not compare equal to the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::ne(42);

	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("!= 42"));

	SECTION("When target is not equal.")
	{
		constexpr int target{1337};
		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is equal.")
	{
		constexpr int target{42};
		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::ne(42);

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("== 42"));

		SECTION("When target is not equal.")
		{
			constexpr int target{1337};
			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is equal.")
		{
			constexpr int target{42};
			REQUIRE(invertedMatcher.matches(target));
		}
	}
}

TEST_CASE(
	"matches::lt matches when target value is less than the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::lt(42);

	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("< 42"));

	SECTION("When target is less.")
	{
		const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is not less.")
	{
		const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::lt(42);

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals(">= 42"));

		SECTION("When target is less.")
		{
			const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is not less.")
		{
			const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
			REQUIRE(invertedMatcher.matches(target));
		}
	}
}

TEST_CASE(
	"matches::le matches when target value is less than or equal to the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::le(42);

	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("<= 42"));

	SECTION("When target is less or equal.")
	{
		const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 42);
		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is greater.")
	{
		const int target = GENERATE(43, std::numeric_limits<int>::max());
		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::le(42);

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("> 42"));

		SECTION("When target is less or equal.")
		{
			const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 42);
			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is greater.")
		{
			const int target = GENERATE(43, std::numeric_limits<int>::max());
			REQUIRE(invertedMatcher.matches(target));
		}
	}
}

TEST_CASE(
	"matches::gt matches when target value is greater than the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::gt(42);

	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("> 42"));

	SECTION("When target is greater.")
	{
		const int target = GENERATE(43, std::numeric_limits<int>::max());
		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is not greater.")
	{
		const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41, 42);
		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::gt(42);

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("<= 42"));

		SECTION("When target is greater.")
		{
			const int target = GENERATE(43, std::numeric_limits<int>::max());
			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is not greater.")
		{
			const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41, 42);
			REQUIRE(invertedMatcher.matches(target));
		}
	}
}

TEST_CASE(
	"matches::ge matches when target value is greater than or equal to the stored one.",
	"[matcher]"
)
{
	const auto matcher = matches::ge(42);

	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals(">= 42"));

	SECTION("When target is greater or equal.")
	{
		const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is less.")
	{
		const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::ge(42);

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("< 42"));

		SECTION("When target is greater or equal.")
		{
			const int target = GENERATE(42, 43, std::numeric_limits<int>::max());
			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is less.")
		{
			const int target = GENERATE(std::numeric_limits<int>::min(), -1, 0, 1, 41);
			REQUIRE(invertedMatcher.matches(target));
		}
	}
}

TEST_CASE(
	"Ordering matches support different, but comparable types, on both sides.",
	"[matcher]"
)
{
	SECTION("matches::eq")
	{
		const auto matcher = matches::eq(std::nullopt);

		std::optional<int> opt{};
		REQUIRE(matcher.matches(opt));

		opt = 42;
		REQUIRE_FALSE(matcher.matches(opt));
	}

	SECTION("matches::ne")
	{
		const auto matcher = matches::ne(std::nullopt);

		std::optional opt{42};
		REQUIRE(matcher.matches(opt));

		opt.reset();
		REQUIRE_FALSE(matcher.matches(opt));
	}

	SECTION("matches::lt")
	{
		const auto matcher = matches::lt(std::nullopt);

		std::optional opt{42};
		REQUIRE_FALSE(matcher.matches(opt));

		opt.reset();
		REQUIRE_FALSE(matcher.matches(opt));
	}

	SECTION("matches::le")
	{
		const auto matcher = matches::le(std::nullopt);

		std::optional opt{42};
		REQUIRE_FALSE(matcher.matches(opt));

		opt.reset();
		REQUIRE(matcher.matches(opt));
	}

	SECTION("matches::gt")
	{
		const auto matcher = matches::gt(std::nullopt);

		std::optional opt{42};
		REQUIRE(matcher.matches(opt));

		opt.reset();
		REQUIRE_FALSE(matcher.matches(opt));
	}

	SECTION("matches::ge")
	{
		const auto matcher = matches::ge(std::nullopt);

		std::optional opt{42};
		REQUIRE(matcher.matches(opt));

		opt.reset();
		REQUIRE(matcher.matches(opt));
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
		matcher.describe(),
		Catch::Matchers::Equals("passes predicate"));

	SECTION("When matcher is inverted.")
	{
		const auto invertedMatcher = !matches::predicate(std::ref(predicate));

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("fails predicate"));

		REQUIRE_CALL(predicate, Invoke(_))
			.LR_WITH(&_1 == &target)
			.RETURN(expectedResult);

		REQUIRE(expectedResult == !invertedMatcher.matches(target));
	}

	SECTION("Custom descriptions are supported.")
	{
		const auto customMatcher = matches::predicate(
			std::ref(predicate),
			"custom predicate is passed",
			"custom predicate is failed");

		REQUIRE_CALL(predicate, Invoke(_))
			.LR_WITH(&_1 == &target)
			.RETURN(expectedResult);
		REQUIRE(expectedResult == customMatcher.matches(target));
		REQUIRE_THAT(
			customMatcher.describe(),
			Catch::Matchers::Equals("custom predicate is passed"));

		REQUIRE_THAT(
			(!customMatcher).describe(),
			Catch::Matchers::Equals("custom predicate is failed"));
	}
}

TEST_CASE(
	"matches::str::eq matches when target string compares equal to the stored one.",
	"[matcher]"
)
{
	using trompeloeil::_;

	const auto matcher = matches::str::eq("Hello, World!");
	REQUIRE_THAT(
		matcher.describe(),
		Catch::Matchers::Equals("is equal to \"Hello, World!\""));

	SECTION("When target is equal, they match.")
	{
		const std::string target{"Hello, World!"};

		REQUIRE(matcher.matches(target));
	}

	SECTION("When target is not equal, they do not match.")
	{
		const std::string target{"Hello, WOrld!"};

		REQUIRE(!matcher.matches(target));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto invertedMatcher = !matches::str::eq("Hello, World!");

		REQUIRE_THAT(
			invertedMatcher.describe(),
			Catch::Matchers::Equals("is not equal to \"Hello, World!\""));

		SECTION("When target is equal, they do not match.")
		{
			const std::string target{"Hello, World!"};

			REQUIRE(!invertedMatcher.matches(target));
		}

		SECTION("When target is not equal, they do match.")
		{
			const std::string target{"Hello, WOrld!"};

			REQUIRE(invertedMatcher.matches(target));
		}
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

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("elements are {  }"));

		SECTION("When target is also empty, they match.")
		{
			const std::vector<int> target{};

			REQUIRE(matcher.matches(target));
		}

		SECTION("When target is not empty, they do not match.")
		{
			const std::vector target{42};

			REQUIRE(!matcher.matches(target));
		}
	}

	SECTION("When a non-empty range is stored.")
	{
		const auto matcher = matches::range::eq(std::vector{1337, 42});

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("elements are { 1337, 42 }"));

		SECTION("When target is equal, they match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(matcher.matches(target));
		}

		SECTION("When target has same elements, but in different order, they do not match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
		}

		SECTION("When target is not equal, they do not match.")
		{
			const std::vector target{42};

			REQUIRE(!matcher.matches(target));
		}
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::eq(std::vector{1337, 42});

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("elements are not { 1337, 42 }"));

		SECTION("When target is equal, they do not match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(!matcher.matches(target));
		}

		SECTION("When target has same elements, but in different order, they do match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(matcher.matches(target));
		}

		SECTION("When target is not equal, they do match.")
		{
			const std::vector target{42};

			REQUIRE(matcher.matches(target));
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
			matcher.describe(),
			Catch::Matchers::Equals("elements are { 1337, 42 }"));
	}
}

TEST_CASE(
	"matches::range::unordered_eq matches when target range is a permutation of the stored one.",
	"[matcher]"
)
{
	using trompeloeil::_;

	SECTION("When an empty range is stored.")
	{
		const auto matcher = matches::range::unordered_eq(std::vector<int>{});

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is a permutation of {  }"));

		SECTION("When target is also empty, they match.")
		{
			const std::vector<int> target{};

			REQUIRE(matcher.matches(target));
		}

		SECTION("When target is not empty, they do not match.")
		{
			const std::vector target{42};

			REQUIRE(!matcher.matches(target));
		}
	}

	SECTION("When a non-empty range is stored.")
	{
		const auto matcher = matches::range::unordered_eq(std::vector{1337, 42});

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is a permutation of { 1337, 42 }"));

		SECTION("When target is equal, they match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(matcher.matches(target));
		}

		SECTION("When target has same elements, but in different order, they do match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(matcher.matches(target));
		}

		SECTION("When target is not equal, they do not match.")
		{
			const std::vector target{42};

			REQUIRE(!matcher.matches(target));
		}
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::unordered_eq(std::vector{1337, 42});

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is not a permutation of { 1337, 42 }"));

		SECTION("When target is equal, they do not match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(!matcher.matches(target));
		}

		SECTION("When target has same elements, but in different order, they do not match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
		}

		SECTION("When target is not equal, they do match.")
		{
			const std::vector target{42};

			REQUIRE(matcher.matches(target));
		}
	}

	SECTION("Custom comparators can be provided.")
	{
		using ComparatorT = InvocableMock<bool, int, int>;
		ComparatorT comparator{};
		const auto matcher = matches::range::unordered_eq(
			std::vector{1337, 42},
			std::ref(comparator));

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is a permutation of { 1337, 42 }"));

		const std::vector target{1337, 42};

		REQUIRE_CALL(comparator, Invoke(1337, 1337))
			.RETURN(true);
		REQUIRE_CALL(comparator, Invoke(42, 42))
			.RETURN(true);

		REQUIRE(matcher.matches(target));
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
			matcher.describe(),
			Catch::Matchers::Equals("is a sorted range"));
	}

	SECTION("When a non-empty range is stored.")
	{
		const auto matcher = matches::range::is_sorted();

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is a sorted range"));

		SECTION("When target is sorted, it's a match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(matcher.matches(target));
		}

		SECTION("When target is not sorted, it's no match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(!matcher.matches(target));
		}
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::is_sorted();

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is an unsorted range"));

		SECTION("When target is sorted, it's no match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
		}

		SECTION("When target is not sorted, it's a match.")
		{
			const std::vector target{1337, 42};

			REQUIRE(matcher.matches(target));
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
			matcher.describe(),
			Catch::Matchers::Equals("is a sorted range"));
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
			matcher.describe(),
			Catch::Matchers::Equals("is an empty range"));
	}

	SECTION("When a non-empty range is stored, it's no match.")
	{
		const auto matcher = matches::range::is_empty();

		const std::vector target{42};

		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is an empty range"));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::is_empty();

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("is not an empty range"));

		SECTION("When target is empty, it's no match.")
		{
			const std::vector<int> target{};

			REQUIRE(!matcher.matches(target));
		}

		SECTION("When a non-empty range is stored, it's a match.")
		{
			const std::vector target{42};

			REQUIRE(matcher.matches(target));
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
			matcher.describe(),
			Catch::Matchers::Equals("has size of 2"));
	}

	SECTION("When target has different size, it's no match.")
	{
		const auto matcher = matches::range::has_size(1);
		const std::vector target = GENERATE(
			std::vector<int>{},
			(std::vector{42, 1337}));

		REQUIRE(!matcher.matches(target));
		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("has size of 1"));
	}

	SECTION("Matcher can be inverted.")
	{
		const auto matcher = !matches::range::has_size(2);

		REQUIRE_THAT(
			matcher.describe(),
			Catch::Matchers::Equals("has different size than 2"));

		SECTION("When target has the expected size, it's no match.")
		{
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
		}

		SECTION("When target has different size, it's a match.")
		{
			const std::vector target{42};

			REQUIRE(matcher.matches(target));
		}
	}
}
