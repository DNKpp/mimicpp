// //          Copyright Dominic (DNKpp) Koepke 2024 - 2024.
// // Distributed under the Boost Software License, Version 1.0.
// //    (See accompanying file LICENSE_1_0.txt or copy at
// //          https://www.boost.org/LICENSE_1_0.txt)

#include "mimic++/Matcher.hpp"

#include "../TestTypes.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace mimicpp;

TEST_CASE(
	"matches::range::eq matches when target range compares element-wise equal to the stored one.",
	"[matcher][matcher::range]"
)
{
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
	"[matcher][matcher::range]"
)
{
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
	"[matcher][matcher::range]"
)
{
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
	"[matcher][matcher::range]"
)
{
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
	"[matcher][matcher::range]"
)
{
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

TEST_CASE(
	"matches::range::elements matches when all target elements matches the specified matchers.",
	"[matcher][matcher::range]"
)
{
	SECTION("Plain matcher.")
	{
		SECTION("When all elements matches the matcher, it's a match.")
		{
			const auto matcher = matches::range::elements(matches::ge(42));
			const std::vector target = GENERATE(
				std::vector<int>{},
				(std::vector{42, 1337}));

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(),
				Catch::Matchers::Equals("each el in range: el >= 42"));
		}

		SECTION("When at least on element does not match the matcher, it's no match.")
		{
			const auto threshold = GENERATE(42, 1337);
			const auto matcher = matches::range::elements(matches::gt(threshold));
			const std::vector target{42, 1337};

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(),
				Catch::Matchers::StartsWith("each el in range: el > "));
		}
	}

	SECTION("Matcher can be inverted.")
	{
		SECTION("When all elements matches the matcher, it's no match.")
		{
			const auto matcher = !matches::range::elements(matches::ge(42));
			const std::vector target = GENERATE(
				std::vector<int>{},
				(std::vector{42, 1337}));

			REQUIRE(!matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(),
				Catch::Matchers::Equals("not each el in range: el >= 42"));
		}

		SECTION("When at least on element does not match the matcher, it's a match.")
		{
			const auto threshold = GENERATE(42, 1337);
			const auto matcher = !matches::range::elements(matches::gt(threshold));
			const std::vector target{42, 1337};

			REQUIRE(matcher.matches(target));
			REQUIRE_THAT(
				matcher.describe(),
				Catch::Matchers::StartsWith("not each el in range: el > "));
		}
	}
}
